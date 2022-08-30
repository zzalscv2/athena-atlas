/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4BARREL_LArStraightElectrodes_H
#define LARG4BARREL_LArStraightElectrodes_H

#include "PhysicalVolumeAccessor.h"
#include <string>

class LArStraightElectrodes {
private:
  LArStraightElectrodes(const std::string& strDetector="");
  void initXYCentEle(const PhysicalVolumeAccessor& theElectrodes, int stackid, int cellid);
  void initHalfLength(const PhysicalVolumeAccessor& theElectrodes, int stackid, int cellid);
  double SlantEle(const PhysicalVolumeAccessor& theElectrodes, int stackid, int cellid) const;
  double m_xcent[1024][14]{};
  double m_ycent[1024][14]{};
  double m_cosu[1024][14]{};
  double m_sinu[1024][14]{};
  double m_halflength[1024][14]{};
  int m_parity;

public:
  static const LArStraightElectrodes* GetInstance(const std::string& strDetector="");
  double XCentEle(int stackid, int cellid) const { return m_xcent[cellid][stackid]; }
  double YCentEle(int stackid, int cellid) const { return m_ycent[cellid][stackid]; }
  double HalfLength(int stackid, int cellid) const { return m_halflength[cellid][stackid]; }
  double Cosu(int stackid, int cellid) const { return m_cosu[cellid][stackid]; }
  double Sinu(int stackid, int cellid) const { return m_sinu[cellid][stackid]; }
};

#endif // LARG4BARREL_LArStraightElectrodes_H
