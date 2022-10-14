/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ZdcRecRun3.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ZDCRECRun3_H
#define ZDCRECRun3_H

#include <string>
#include <map>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

class ISvcLocator;
class StatusCode;
class Identifier;
class StoreGateSvc;

class ZdcRecChannelToolLucrod;
#include "ZdcByteStream/ZdcLucrodDataContainer.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODForward/ZdcModuleAuxContainer.h"
#include "ZdcAnalysis/ZdcAnalysisTool.h"

/** @class ZdcRecRun3

    Class definition for the ZDC Reconstruction class for Run 3

    @author  Brian Cole and Peter Steinberg, bcole@cern.ch, steinberg@bnl.gov
*/


class ZdcRecRun3 : public AthAlgorithm
{

public:

	ZdcRecRun3(const std::string& name, ISvcLocator* pSvcLocator);
	~ZdcRecRun3();

	StatusCode initialize() override;
	StatusCode execute() override;
	StatusCode finalize() override;

private:
	
	int m_ownPolicy;

	SG::ReadHandleKey<ZdcLucrodDataContainer> m_zldContainerName
          { this, "ZdcLucrodDataContainerKey", "ZdcLucrodDataContainer", "" };

	SG::WriteHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleContainerName
	  { this, "ZdcModuleContainerName", "ZdcModules", "" };

	SG::WriteHandleKey<xAOD::ZdcModuleContainer> m_zdcSumContainerName
	  { this, "ZdcSumContainerName", "ZdcSums", "" };


	//Include here all tools to do the job. They will be called by the algorithm execute method
	//Another option is to use ToolHandleArray<IZdcRecTool>, where IZdcRecTool is the factory for
	//the tools

	ToolHandle<ZdcRecChannelToolLucrod> m_ChannelTool
	  { this, "ChannelTool", "ZdcRecChannelToolLucrod", "" };
	ToolHandle<ZDC::IZdcAnalysisTool> m_zdcTool
	  { this, "ZdcAnalysisTool", "ZDC::ZdcAnalysisTool/ZdcAnalysisTool", "" };

};

#endif
