/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_MDTCUTOUT_H
#define MUONREADOUTGEOMETRYR4_MDTCUTOUT_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <Identifier/IdentifierHash.h>
#include <set>
namespace MuonGMR4 {
    
    struct MdtCutOut {
        /// Cutout on the left site of the chamber (positive Y)
        double leftX{0.};
        /// Cutout on the right site of the chamber (negative Y)
        double rightX{0.};
        /// Layer in which the cut out is applied
        uint16_t layer{std::numeric_limits<uint16_t>::max()};
        /// First tube affected by the cutout
        uint16_t firstTube{std::numeric_limits<uint16_t>::max()};
        /// Last tube affected by the cutout (Inclusive)
        uint16_t lastTube{std::numeric_limits<uint16_t>::max()};
        /// Smaller operator
        bool operator<(const MdtCutOut& other) const;
    };
    
    bool operator<(const MdtCutOut& cutout, const IdentifierHash& hash);
    bool operator<(const IdentifierHash& hash, const MdtCutOut& cutout);
    std::ostream& operator<<(std::ostream& ostr, const MdtCutOut& cutout);

    using MdtCutOuts = std::set<MdtCutOut, std::less<>>;
}
#endif