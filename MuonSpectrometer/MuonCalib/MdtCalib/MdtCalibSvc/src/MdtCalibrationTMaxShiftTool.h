/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTCALIBSVC_MDTCALIBRATIONTMAXSHIFTTOOL_H
#define MDTCALIBSVC_MDTCALIBRATIONTMAXSHIFTTOOL_H

#include "MdtCalibrationShiftMapBase.h"

/*
   @class MdtCalibrationTMaxShiftTool
   Provides a per-tube shifting of the TMax value.
   @author Andreas Hoenle
*/
class MdtCalibrationTMaxShiftTool : virtual public MdtCalibrationShiftMapBase {
public:
  /* constructor */
  MdtCalibrationTMaxShiftTool(const std::string& type, const std::string& name, const IInterface* parent);

  /* destructor */
  ~MdtCalibrationTMaxShiftTool()=default;

  /*
   * initalization of map cannot happen before first event
   * special function required
   */
  StatusCode initializeMap() override final;

  StatusCode setTUpper(const float tUpper);
  float getTUpper() const { return m_tUpper; }

private:
   float m_tUpper{688.1818};
};

#endif
