/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4BARREL_LArCoudes_H
#define LARG4BARREL_LArCoudes_H

#include "PhysicalVolumeAccessor.h"

class LArCoudes {
private:
  static const PhysicalVolumeAccessor& theCoudes(const std::string& strDetector="");

public:
  LArCoudes(const std::string& strDetector="") ;
  double XCentCoude(int stackid, int cellid) const;
  double YCentCoude(int stackid, int cellid) const;
};

#endif // LARG4BARREL_LArCoudes_H
