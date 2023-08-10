/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IZDCTRIGVALIDTOOL_H__
#define IZDCTRIGVALIDTOOL_H__

#include "AsgTools/IAsgTool.h"
#include "xAODForward/ZdcModuleContainer.h"

namespace ZDC
{
  
class IZdcTrigValidTool : virtual public asg::IAsgTool 
{
  ASG_TOOL_INTERFACE( ZDC::IZdcTrigValidTool )

 public:
 
  virtual StatusCode addTrigStatus(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer& moduleSumContainer) = 0;

};

}
#endif
