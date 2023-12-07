/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_SD_ZDC_FIBER_SDTOOL_H
#define ZDC_SD_ZDC_FIBER_SDTOOL_H

#include "G4AtlasTools/SensitiveDetectorBase.h"

// STL headers
#include <string>

class G4VSensitiveDetector;

class ZDC_FiberSDTool : public SensitiveDetectorBase
{

 public:
  // Constructor
  ZDC_FiberSDTool(const std::string& type, const std::string& name, const IInterface* parent);
  // Destructor
  ~ZDC_FiberSDTool() {};
  /** End of an athena event */
  StatusCode Gather() override final; //FIXME would be good to be able to avoid this.

protected:
  // Make me an SD!
  G4VSensitiveDetector* makeSD() const override final;

private:
  float m_readoutPos; // Y position of the top of the readout fibers

};

#endif //ZDC_SD_ZDC_STRIP_SDTOOL_H
