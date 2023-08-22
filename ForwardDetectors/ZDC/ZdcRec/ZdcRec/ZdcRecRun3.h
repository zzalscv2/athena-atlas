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
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODForward/ZdcModuleAuxContainer.h"
#include "ZdcAnalysis/IZdcAnalysisTool.h"

/** @class ZdcRecRun3

    Class definition for the ZDC Reconstruction class for Run 3

    This class is responsible for running the ZDC signal processing and decorating the ZdcModules accordingly.  In principle, this module runs appropriately on simulation and raw.

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

	SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleContainerName
	  { this, "ZdcModuleContainerName", "ZdcModules", "" };

	SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcSumContainerName
	  { this, "ZdcSumContainerName", "ZdcSums", "" };

	ToolHandleArray<ZDC::IZdcAnalysisTool> m_zdcTools{ this, "ZdcAnalysisTools",{} };

};

#endif
