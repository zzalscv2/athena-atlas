/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ZdcUtils/ZdcEventInfo.h
 * @author Brian Cole <bcole@cern.ch>
 * @date August 2023
 * @brief Define enumerations for event-level ZDC data
 */

#ifndef ZDCUTILS__ZdcEventInfo__h_
#define ZDCUTILS__ZdcEventInfo__h_

namespace ZdcEventInfo
{
  enum ZdcEventType {ZdcEventUnknown, ZdcEventPhysics, ZdcEventLED, ZdcSimulation, numEventTypes};
  enum DAQMode {DAQModeUndef = 0, Standalone, PhysicsPEB, CombinedPhysics, MCDigits, numDAQModes};
  enum LEDType {Blue1 = 0, Green = 1, Blue2 = 2, NumLEDs, LEDNone};    
  enum FlagType {DECODINGERROR = 0, UNPACKERROR = 1, ZDCRECOERROR = 2, RPDRECOERROR = 3,numFlagTypes};
};

#endif
