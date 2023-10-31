/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 global functions and stuff ...
 -----------------------------------------
 ***************************************************************************/

#ifndef MUONREADOUTGEOMETRY_GLOBALUTILITIES_H
#define MUONREADOUTGEOMETRY_GLOBALUTILITIES_H

#include <string>
#include <string_view>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <CxxUtils/ArrayHelper.h>

namespace MuonGM {
    std::string buildString(int i, int ncha);
    int strtoint(std::string_view str, unsigned int istart, unsigned int length);

    /// Converts the AMDB phi index to the Identifier phi Index
    int stationPhiTGC(std::string_view stName, int fi, int zi_input);
    /// Converts the Identifier phi index to the AMDB phi index
    int amdbPhiTGC(std::string_view stName, int phiIndex, int eta_index);
    
}  // namespace MuonGM

#endif  // MUONREADOUTGEOMETRY_GLOBALUTILITIES_H
