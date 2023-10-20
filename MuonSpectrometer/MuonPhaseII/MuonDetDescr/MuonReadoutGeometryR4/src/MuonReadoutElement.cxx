/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/MuonReadoutElement.h"

#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"


using namespace ActsTrk;
using SubDetAlignments = ActsGeometryContext::SubDetAlignments;
namespace {
   
    /// Dummy transformation
    static const Amg::Transform3D dummyTrans{Amg::Transform3D::Identity()};

}
namespace MuonGMR4 {

MuonReadoutElement::MuonReadoutElement(defineArgs&& args)
    : GeoVDetectorElement(args.physVol),
      AthMessaging("MuonReadoutElement"),
      m_args{std::move(args)} {
    if (!m_idHelperSvc.retrieve().isSuccess()) {
        ATH_MSG_FATAL("Failed to retrieve the MuonIdHelperSvc");
    }
    m_stName = m_idHelperSvc->stationName(identify());
    m_stEta = m_idHelperSvc->stationEta(identify());
    m_stPhi = m_idHelperSvc->stationPhi(identify());
    m_detElHash = m_idHelperSvc->detElementHash(identify());
    m_chIdx = m_idHelperSvc->chamberIndex(identify());
    insertTransform(geoTransformHash(), [this](RawGeomAlignStore* store, const IdentifierHash&){
            return toStation(store);
    }).ignore();
}
IdentifierHash MuonReadoutElement::geoTransformHash() {     
    static const IdentifierHash hash{static_cast<unsigned>(~0)-1};
    return hash;
}

const Amg::Transform3D& MuonReadoutElement::globalToLocalTrans(const ActsGeometryContext& ctx, 
                                                               const IdentifierHash& hash) const {
    SubDetAlignments::const_iterator map_itr = ctx.alignmentStores.find(detectorType());
    const ActsTrk::AlignmentStore* store = map_itr != ctx.alignmentStores.end() ? 
                                                        map_itr->second.get() : nullptr;

    MuonTransformSet::const_iterator cache = m_globalToLocalCaches.find(hash);
    if (cache != m_globalToLocalCaches.end()) return (*cache)->getTransform(store);
    ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<"() -- Hash "<<hash
                <<" is unknown to "<<idHelperSvc()->toStringDetEl(identify()));    
    return dummyTrans;
}
const Amg::Transform3D& MuonReadoutElement::localToGlobalTrans(const ActsGeometryContext& ctx, 
                                                               const IdentifierHash& hash) const {
    SubDetAlignments::const_iterator map_itr = ctx.alignmentStores.find(detectorType());
    const ActsTrk::AlignmentStore* store = map_itr != ctx.alignmentStores.end() ? 
                                                        map_itr->second.get() : nullptr;

    MuonTransformSet::const_iterator cache = m_localToGlobalCaches.find(hash);
    if (cache != m_localToGlobalCaches.end()) return (*cache)->getTransform(store);
    ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<"() -- Hash "<<hash
               <<" is unknown to "<<idHelperSvc()->toStringDetEl(identify()));    
    return dummyTrans;
}
std::shared_ptr<Acts::Surface> MuonReadoutElement::surfacePtr(const IdentifierHash& hash) const {
    MuonSurfaceSet::const_iterator cache = m_surfaces.find(hash);
    if(cache != m_surfaces.end()) return (*cache)->getSurface();
    ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<"() -- Hash "<<hash
               <<" is unknown to "<<idHelperSvc()->toStringDetEl(identify()));
    return nullptr;
}


Amg::Transform3D MuonReadoutElement::toStation(RawGeomAlignStore* alignStore) const {
   return getMaterialGeom()->getAbsoluteTransform(alignStore ? alignStore->geoModelAlignment.get() : nullptr);
}
const Acts::Transform3& MuonReadoutElement::transform(const Acts::GeometryContext& anygctx) const {
    const ActsGeometryContext *gctx = anygctx.get<const ActsGeometryContext *>();
    return localToGlobalTrans(*gctx, geoTransformHash());
}

StatusCode MuonReadoutElement::insertTransform(const IdentifierHash& hash,
                                               TransformMaker make) {
    
    MuonTransformSet::const_iterator cache = m_localToGlobalCaches.find(hash);
    if (cache != m_localToGlobalCaches.end()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toStringDetEl(identify())
                   <<" has already a transformation cached for hash "<<hash);
        return StatusCode::FAILURE;
    }
    m_localToGlobalCaches.insert(std::make_unique<MuonTransformCache>(hash, make));
    m_globalToLocalCaches.insert(std::make_unique<MuonTransformCache>(hash,
                                            [make](RawGeomAlignStore* store, const IdentifierHash& hash){
                                                return make(store,hash).inverse();
                                            }));
    return StatusCode::SUCCESS;
}
bool MuonReadoutElement::storeAlignment(RawGeomAlignStore& store) const{ 
    if (store.detType != detectorType()) return false;
    for (const std::unique_ptr<MuonTransformCache>& cache: m_localToGlobalCaches) {
       cache->storeAlignment(store);
    }
    for (const std::unique_ptr<MuonTransformCache>& cache : m_globalToLocalCaches){
        cache->storeAlignment(store);
    }
    return true;
}
const Amg::Transform3D& MuonReadoutElement::globalToLocalTrans(const ActsGeometryContext& ctx) const {
    return globalToLocalTrans(ctx, geoTransformHash());
}
const Amg::Transform3D& MuonReadoutElement::localToGlobalTrans(const ActsGeometryContext& ctx) const {
    return localToGlobalTrans(ctx, geoTransformHash());
}

const Acts::Surface& MuonReadoutElement::surface() const { return surface(geoTransformHash()); }
Acts::Surface& MuonReadoutElement::surface() { return surface(geoTransformHash()); }
const Acts::Surface& MuonReadoutElement::surface(const IdentifierHash& hash) const { return *surfacePtr(hash); }
Acts::Surface& MuonReadoutElement::surface(const IdentifierHash& hash) { return *surfacePtr(hash); }

StatusCode MuonReadoutElement::strawSurfaceFactory(const IdentifierHash& hash, 
                                                   std::shared_ptr<Acts::LineBounds> lBounds) {

    //get the local to global transform cache
    MuonTransformSet::const_iterator transformCache = m_localToGlobalCaches.find(hash);
    if (transformCache == m_localToGlobalCaches.end()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" no transform cache available for hash "<<hash);
        return StatusCode::FAILURE;
    }

    auto insert_itr = m_surfaces.insert(std::make_unique<MuonSurfaceCache>((*transformCache).get(), detectorType()));
    if(!insert_itr.second){
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" Insertion to muon surface cache failed for hash "<<hash);
        return StatusCode::FAILURE;
    }
    //Create straw surface for the surface cache
    (*insert_itr.first)->setSurface(Acts::Surface::makeShared<Acts::StrawSurface>(lBounds, **insert_itr.first));
    return StatusCode::SUCCESS;

}

StatusCode MuonReadoutElement::planeSurfaceFactory(const IdentifierHash& hash, std::shared_ptr<Acts::PlanarBounds> pBounds){

    //get the local to global transform cache
    MuonTransformSet::const_iterator transformCache = m_localToGlobalCaches.find(hash);

    if (transformCache == m_localToGlobalCaches.end()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" no transform cache available for hash "<<hash);
        return StatusCode::FAILURE;
    }
    
    auto insert_itr = m_surfaces.insert(std::make_unique<MuonSurfaceCache>((*transformCache).get(),detectorType()));
    if(!insert_itr.second){
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
                   <<" Insertion to muon surface cache failed for hash "<<hash);
        return StatusCode::FAILURE;
    }
    //Create a plane surface for the surface cache
    (*insert_itr.first)->setSurface(Acts::Surface::makeShared<Acts::PlaneSurface>(pBounds, **insert_itr.first));
    return StatusCode::SUCCESS;
}

}  // namespace MuonGMR4
