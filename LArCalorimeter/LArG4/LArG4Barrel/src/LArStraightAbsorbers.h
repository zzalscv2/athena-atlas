/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4BARREL_LArStraightAbsorbers_H
#define LARG4BARREL_LArStraightAbsorbers_H

#include "PhysicalVolumeAccessor.h"
#include <string>

class LArStraightAbsorbers {
private:
  LArStraightAbsorbers(const std::string& strDetector="") ;
  void initXYCentAbs(const PhysicalVolumeAccessor& theAbsorbers, int stackid, int cellid);
  void initHalfLength(const PhysicalVolumeAccessor& theAbsorbers, int stackid, int cellid);
  double SlantAbs(const PhysicalVolumeAccessor& theAbsorbers, int stackid, int cellid) const;
  double m_xcent[1024][14]{};
  double m_ycent[1024][14]{};
  double m_cosu[1024][14]{};
  double m_sinu[1024][14]{};
  double m_halflength[1024][14]{};
  int m_parity;
public:
  static const LArStraightAbsorbers* GetInstance(const std::string& strDetector="") ;
  double XCentAbs(int stackid, int cellid) const { return m_xcent[cellid][stackid]; }
  double YCentAbs(int stackid, int cellid) const { return m_ycent[cellid][stackid]; }
  double HalfLength(int stackid, int cellid) const { return m_halflength[cellid][stackid]; }
  double Cosu(int stackid, int cellid) const { return m_cosu[cellid][stackid]; }
  double Sinu(int stackid, int cellid) const { return m_sinu[cellid][stackid]; }
};

#endif // LARG4BARREL_LArStraightAbsorbers_H
