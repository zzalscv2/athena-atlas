/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSTATIONGEOHELPERS_MUONCHAMBER_H
#define MUONSTATIONGEOHELPERS_MUONCHAMBER_H

/**
 *  A muon chamber is a collection of readout elements belonging to the same station.
 *  In the barrel, it's typically 2 Mdt multi layers + a couple of RPC layers, while the
 *  endcape chambers either consist of Mdt multi layers, multiple TGC layers or in the 
 *  case of the NSW, 1 sTGC layer or 2 Micromega multilayers.
 * 
 *  The MuonChamber class provides the sets of radout elements and the definitions of the
 *  minimal surrounding box. 
*/

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <Acts/Geometry/Volume.hpp>

#include <set>

namespace MuonGMR4 {
    class MuonChamber{
        public:
            using ReadoutSet = std::vector<const MuonReadoutElement*>;                
            struct defineArgs{
                /// List of readout elements in the chamber
                ReadoutSet readoutEles{};
                /// Pointer to the physical volume associated with the chamber.
                PVConstLink chamberVol{nullptr};
                /// Definition of the surrounding box
                double halfX{0.};
                double halfY{0.};
                double halfZ{0.};
                /// Transformation to the chamber volume
                Amg::Transform3D centerTrans{Amg::Transform3D::Identity()};
            };

            MuonChamber(defineArgs&& args);
            /// Returns the chamber index
            Muon::MuonStationIndex::ChIndex chamberIndex() const;
            // Returns the station name of the chamber
            int stationName() const;
            
            /// Returns the station eta of the chamber
            int stationEta() const;
            /// Returns the station phi of the chamber
            int stationPhi() const;
            /// Returns the list of all associated readout elements 
            const ReadoutSet& readOutElements() const;
            /// Returns the transformation of the MuonChamber
            Amg::Transform3D localToGlobalTrans(const ActsGeometryContext& gctx) const;
            /// Surrounding box dimensions
            double halfX() const;
            double halfY() const;
            double halfZ() const;

            std::shared_ptr<Acts::Volume> boundingVolume(const ActsGeometryContext& gctx) const;

        private:
           const defineArgs m_args{};
    };

    bool operator<(const MuonChamber& a, const MuonChamber& b);
    bool operator<(const MuonReadoutElement& a, const MuonChamber& b);
    bool operator<(const MuonChamber& a, const MuonReadoutElement& b);
    using ChamberSet = std::set<MuonChamber, std::less<>>;
}

#endif