/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ZdcRecChannelToolLucrod.h
 *
 *      Author: Peter Steinberg (steinberg@bnl.gov)
 */

#ifndef ZDCRECCHANNELTOOLLucrod_H_
#define ZDCRECCHANNELTOOLLucrod_H_


#include <string>
#include <map>

#include "AsgTools/AsgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "ZdcByteStream/ZdcLucrodDataContainer.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODForward/ZdcModuleAuxContainer.h"

class StatusCode;
class Identifier;
class ZdcID ;

class ZdcRecChannelToolLucrod: public asg::AsgTool, virtual public IIncidentListener 
{
  ASG_TOOL_INTERFACE(ZdcRecChannelToolLucrod)
  ASG_TOOL_CLASS0(ZdcRecChannelToolLucrod)
 public:
  ZdcRecChannelToolLucrod(const std::string& name);
  virtual ~ZdcRecChannelToolLucrod() {};
  
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual void handle( const Incident& ) override;

  int convertLucrod2ZM(const ZdcLucrodDataContainer* lucrodContainer, xAOD::ZdcModuleContainer* zdcModules, xAOD::ZdcModuleContainer* zdcSums) const;

private:

  const ZdcID* m_zdcId{};

};


#endif /* ZDCRECCHANNELTOOLLucrod_H_ */
