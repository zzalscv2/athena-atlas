/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4BARREL_LArCoudeElectrodes_H
#define LARG4BARREL_LArCoudeElectrodes_H

#include <string>

class LArCoudeElectrodes {
private:
  LArCoudeElectrodes(const std::string& strDetector="");
  double m_xcent[1024][15]{};
  double m_ycent[1024][15]{};
  double m_phirot[1024][15]{};

public:
  static const LArCoudeElectrodes* GetInstance(const std::string& strDetector="");
  double XCentCoude(int stackid, int cellid) const { return m_xcent[cellid][stackid]; }
  double YCentCoude(int stackid, int cellid) const { return m_ycent[cellid][stackid]; }
  double PhiRot(int stackid, int cellid) const { return m_phirot[cellid][stackid]; }
};

#endif // LARG4BARREL_LArCoudeElectrodes_H
