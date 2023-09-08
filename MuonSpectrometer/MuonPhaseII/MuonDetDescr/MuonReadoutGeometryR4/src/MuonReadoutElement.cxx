/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/MuonReadoutElement.h"

using namespace ActsTrk;
using SubDetAlignments = ActsGeometryContext::SubDetAlignments;
namespace {
    /// Identifier used to access the transformation into the Detector center
    static const IdentifierHash stationHash{static_cast<unsigned>(~0)-1};
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
    m_stIdx = m_idHelperSvc->stationIndex(identify());
    m_stEta = m_idHelperSvc->stationEta(identify());
    m_stPhi = m_idHelperSvc->stationPhi(identify());
    m_detElHash = m_idHelperSvc->detElementHash(identify());
    insertTransform(stationHash,[this](RawGeomAlignStore* store, const IdentifierHash&){
            return toStation(store);
    }).ignore();
}

const Amg::Transform3D& MuonReadoutElement::globalToLocalTrans(const ActsGeometryContext& ctx, const IdentifierHash& hash) const {
    SubDetAlignments::const_iterator map_itr = ctx.alignmentStores.find(detectorType());
    const ActsTrk::AlignmentStore* store = map_itr != ctx.alignmentStores.end() ? 
                                                        map_itr->second.get() : nullptr;

    MuonTransformSet::const_iterator cache = m_globalToLocalCaches.find(hash);
    if (cache != m_globalToLocalCaches.end()) return (*cache)->getTransform(store);
    ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<"() -- Hash "<<hash
                <<" is unknown to "<<idHelperSvc()->toStringDetEl(identify()));    
    return dummyTrans;
}
const Amg::Transform3D& MuonReadoutElement::localToGlobalTrans(const ActsGeometryContext& ctx, const IdentifierHash& hash) const {
    SubDetAlignments::const_iterator map_itr = ctx.alignmentStores.find(detectorType());
    const ActsTrk::AlignmentStore* store = map_itr != ctx.alignmentStores.end() ? 
                                                        map_itr->second.get() : nullptr;

    MuonTransformSet::const_iterator cache = m_localToGlobalCaches.find(hash);
    if (cache != m_localToGlobalCaches.end()) return (*cache)->getTransform(store);
    ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<"() -- Hash "<<hash
               <<" is unknown to "<<idHelperSvc()->toStringDetEl(identify()));    
    return dummyTrans;
}

Amg::Transform3D MuonReadoutElement::toStation(RawGeomAlignStore* alignStore) const {
   return getMaterialGeom()->getAbsoluteTransform(alignStore ? alignStore->geoModelAlignment.get() : nullptr);
}
const Acts::Transform3& MuonReadoutElement::transform(const Acts::GeometryContext& anygctx) const {
    const ActsGeometryContext *gctx = anygctx.get<const ActsGeometryContext *>();
    return localToGlobalTrans(*gctx, stationHash);
}

StatusCode MuonReadoutElement::insertTransform(const IdentifierHash& hash,
                                               TransformMaker make) {
    
    MuonTransformSet::const_iterator cache = m_localToGlobalCaches.find(hash);
    if (cache != m_localToGlobalCaches.end()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" - "<<idHelperSvc()->toString(identify())
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
    return globalToLocalTrans(ctx, stationHash);
}
const Amg::Transform3D& MuonReadoutElement::localToGlobalTrans(const ActsGeometryContext& ctx) const {
    return localToGlobalTrans(ctx, stationHash);
}


}  // namespace MuonGMR4
