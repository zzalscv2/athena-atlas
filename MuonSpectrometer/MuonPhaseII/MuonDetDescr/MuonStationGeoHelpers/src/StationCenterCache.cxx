/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonStationGeoHelpers/StationCenterCache.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>

using SubDetAlignments = ActsGeometryContext::SubDetAlignments;
using AlignStorePtr = GeoModel::TransientConstSharedPtr<ActsTrk::AlignmentStore>;
namespace MuonGMR4{
    StationCenterCache::StationCenterCache(const MuonReadoutElement* readOutEleA,
                               const IdentifierHash hashLayA,
                               const MuonReadoutElement* readOutEleB,
                               const IdentifierHash hashLayB):
         m_elA{readOutEleA},
         m_elB{readOutEleB},
         m_surfA{hashLayA},
         m_surfB{hashLayB} {
        if (!m_elB) {
            m_elB = m_elA;
            m_surfB = m_surfB;
        }
    }
    bool StationCenterCache::storeAlignment(ActsTrk::RawGeomAlignStore& store) const {
        if (store.detType != detectorType()) return false;
        m_localToGlobal.storeAlignment(store);
        m_globalToLocal.storeAlignment(store);
        return true;
    }
    Amg::Transform3D StationCenterCache::fromLayerToGlobal(ActsTrk::RawGeomAlignStore* store) const {
        ActsGeometryContext gctx{};
        /// If the store is given, assume that the tracking alignment already caches the transformations
        /// of the needed detector surfaces --> We can build a geo context on the fly.
        if (store) {
            gctx.alignmentStores[detectorType()] = store->trackingAlignment;
        }        
        Amg::Vector3D center = 0.5 * (m_elA->center(gctx, m_surfA) + 
                                          m_elB->center(gctx, m_surfB)); 
        Amg::RotationMatrix3D rot {m_elA->localToGlobalTrans(gctx, m_surfA).linear()};

        return Amg::getTransformFromRotTransl(std::move(rot), 
                                             std::move(center));
    }

    const Amg::Transform3D& StationCenterCache::localToGlobal(const ActsGeometryContext& gctx) const {
        SubDetAlignments::const_iterator itr = gctx.alignmentStores.find(detectorType());
        return m_localToGlobal.getTransform(itr != gctx.alignmentStores.end() ? itr->second.get() : nullptr);
    }            
    const Amg::Transform3D& StationCenterCache::globalToLocal(const ActsGeometryContext& gctx) const {
        SubDetAlignments::const_iterator itr = gctx.alignmentStores.find(detectorType());
        return m_globalToLocal.getTransform(itr != gctx.alignmentStores.end() ? itr->second.get() : nullptr); 
    }

    bool StationCenterCache::operator<(const StationCenterCache& other) const {
        if (other.stationIndex() != stationIndex()) return stationIndex() < other.stationIndex();
        if (other.stationEta() != stationEta()) return stationEta() < other.stationEta();
        return stationPhi() < other.stationPhi();
    }

    bool operator<(const Identifier& a, const StationCenterCache& b){
        const int stIdxA = b.idHelperSvc()->stationIndex(a);
        if (stIdxA != b.stationIndex()) return stIdxA < b.stationIndex();
        const int stEtaA = b.idHelperSvc()->stationEta(a);
        if (stEtaA != b.stationEta()) return stEtaA < b.stationEta();
        return b.idHelperSvc()->stationPhi(a) < b.stationPhi();
    }
    bool operator<(const StationCenterCache& a, const Identifier& b){
        const int stIdxB = a.idHelperSvc()->stationIndex(b);
        if (stIdxB != a.stationIndex()) return a.stationIndex() < stIdxB;
        const int stEtaB = a.idHelperSvc()->stationEta(b);
        if (stEtaB != a.stationEta()) return a.stationEta() < stEtaB;
        return a.stationPhi() < a.idHelperSvc()->stationPhi(b);
    }
    bool operator<(const std::unique_ptr<StationCenterCache>& a, 
                   const std::unique_ptr<StationCenterCache>& b){
        return *a < *b;
    }
    bool operator<(const std::unique_ptr<StationCenterCache>& a, const Identifier& b){
        return *a < b;
    }
    bool operator<(const Identifier&a , const std::unique_ptr<StationCenterCache>& b){
        return a < *b;
    }
}