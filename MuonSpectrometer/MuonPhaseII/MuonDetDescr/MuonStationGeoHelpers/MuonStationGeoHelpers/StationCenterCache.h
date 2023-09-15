/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSTATIONLAYERSURFACESVC_StationCenterCache_H
#define MUONSTATIONLAYERSURFACESVC_StationCenterCache_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <MuonReadoutGeometryR4/MuonTransformCache.h>
#include <MuonReadoutGeometryR4/MuonReadoutElement.h>

#include <Identifier/IdentifierHash.h>
#include <set>

namespace MuonGMR4{

    class StationCenterCache{
        public:
            
            StationCenterCache(const MuonReadoutElement* readOutEleA,
                               const IdentifierHash hashLayA,
                               const MuonReadoutElement* readOutEleB,
                               const IdentifierHash hashLayB);

            const Amg::Transform3D& localToGlobal(const ActsGeometryContext& gctx) const;
            
            const Amg::Transform3D& globalToLocal(const ActsGeometryContext& gctx) const;
            
            /// @brief Returns the station index of the associated detector element
            inline int stationName() const {
                return m_elA->stationName();
            }
            /// @brief Returns the station eta of the associated detector element  
            inline int stationEta() const {
                return m_elA->stationEta();
            }
            /// @brief Returns the station phi of the associated detector element
            inline int stationPhi() const {
                return m_elA->stationPhi();
            }
            /// @brief Returns the type of the associated detector element            
            inline ActsTrk::DetectorType detectorType() const{
                return m_elA->detectorType();
            }

            inline const Muon::IMuonIdHelperSvc* idHelperSvc() const {
                return m_elA->idHelperSvc();
            }

            bool operator<(const StationCenterCache& other) const;
            
            /// Applies the alignment transformations to the middle layers
            /// Returns false if the alignment store does not cache the constants
            /// for the correpsonding detector element
            bool storeAlignment(ActsTrk::RawGeomAlignStore& store) const;

        private:
            Amg::Transform3D fromLayerToGlobal(ActsTrk::RawGeomAlignStore* store) const;
            
            /// Pointer to the first detector. This detector element
            /// Is coming prior in dR. Rotation matrix is taken from this 
            /// detector eleement
            const MuonReadoutElement* m_elA{nullptr};
            /// Pointer to the second detector element. 
            /// Can conincide with the first one if only one element is parsed
            const MuonReadoutElement* m_elB{nullptr};
            /// Hash to the surface transformation to be used to obtain the
            /// reference point of detector element A
            IdentifierHash m_surfA{};
            IdentifierHash m_surfB{};

            MuonTransformCache m_localToGlobal{m_surfA,
                    [this](ActsTrk::RawGeomAlignStore* store, const IdentifierHash&){
                        return fromLayerToGlobal(store);
            }};
            MuonTransformCache m_globalToLocal{m_surfA, 
                    [this](ActsTrk::RawGeomAlignStore* store, const IdentifierHash& hash){
                        return m_localToGlobal.transformMaker()(store,hash).inverse();
            }};
            

    };

    bool operator<(const std::unique_ptr<StationCenterCache>& a, 
                   const std::unique_ptr<StationCenterCache>& b);

    bool operator<(const Identifier& a, const StationCenterCache& b);
    bool operator<(const StationCenterCache& a, const Identifier& b);
    
    using StationCacheSet = std::set<StationCenterCache, std::less<>>;

}
#endif