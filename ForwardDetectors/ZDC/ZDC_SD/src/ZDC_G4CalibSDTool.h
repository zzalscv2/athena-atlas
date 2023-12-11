/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCG4SDTOOL_H
#define ZDCG4SDTOOL_H

// Base class
#include "G4AtlasTools/SensitiveDetectorBase.h"

// For escaped energy
#include "CaloG4Sim/EscapedEnergyRegistry.h"
#include "CaloG4Sim/CalibrationDefaultProcessing.h"
#include "ZDC_EscapedEnergyProcessing.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"

class G4VSensitiveDetector;

class ZDC_G4CalibSDTool : public SensitiveDetectorBase
{
 public:
  // Constructor
  ZDC_G4CalibSDTool(const std::string& type, const std::string& name, const IInterface *parent);
  // Destructor
  virtual ~ZDC_G4CalibSDTool() {}
  // Calls down to all the SDs to get them to pack their hits into a central collection
  StatusCode Gather() override final;

 protected:
  //Make an SD
  G4VSensitiveDetector* makeSD()const override final;

 private:
  bool m_deadSD;
  bool m_doPid;
};

#endif
