/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// MT-friendly LAr SD tools
#include "../EMBSDTool.h"
#include "../EMECSDTool.h"
#include "../HECSDTool.h"
#include "../FCALSDTool.h"
#include "../ActiveSDTool.h"
#include "../InactiveSDTool.h"
#include "../DeadSDTool.h"

#include "../CalibrationDefaultCalculator.h"
#include "../CalibrationHitMerger.h"

DECLARE_COMPONENT( LArG4::EMBSDTool )
DECLARE_COMPONENT( LArG4::EMECSDTool )
DECLARE_COMPONENT( LArG4::HECSDTool )
DECLARE_COMPONENT( LArG4::FCALSDTool )
DECLARE_COMPONENT( LArG4::ActiveSDTool )
DECLARE_COMPONENT( LArG4::InactiveSDTool )
DECLARE_COMPONENT( LArG4::DeadSDTool )

DECLARE_COMPONENT( LArG4::CalibrationDefaultCalculator )
DECLARE_COMPONENT( LArG4::CalibrationHitMerger )
