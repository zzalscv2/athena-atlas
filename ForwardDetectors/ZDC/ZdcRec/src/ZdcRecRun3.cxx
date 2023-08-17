/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ZdcRecRun3.cxx
 *
 *  Created on: June 23, 2022 (never forget)
 *      Author: steinberg@bnl.gov
 */


#include <memory>

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

#include "ZdcRec/ZdcRecRun3.h"
#include "xAODForward/ZdcModuleToString.h"
#include "ZdcByteStream/ZdcToString.h"

//==================================================================================================
ZdcRecRun3::ZdcRecRun3(const std::string& name, ISvcLocator* pSvcLocator) :

	AthAlgorithm(name, pSvcLocator),
	m_ownPolicy(static_cast<int> (SG::OWN_ELEMENTS))
{
  declareProperty("OwnPolicy",m_ownPolicy) ;
}

//==================================================================================================

//==================================================================================================
ZdcRecRun3::~ZdcRecRun3() {}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecRun3::initialize()
{
	MsgStream mLog(msgSvc(), name());

	// Reconstruction Tool chain

	ATH_CHECK( m_zdcTools.retrieve() );
	
	ATH_CHECK( m_zdcModuleContainerName.initialize() );
	ATH_CHECK( m_zdcSumContainerName.initialize() );

	if (m_ownPolicy == SG::OWN_ELEMENTS)
		mLog << MSG::DEBUG << "...will OWN its cells." << endmsg;
	else
		mLog << MSG::DEBUG << "...will VIEW its cells." << endmsg;


	mLog << MSG::DEBUG << "--> ZDC: ZdcRecRun3 initialization complete" << endmsg;

	return StatusCode::SUCCESS;
}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecRun3::execute()
{

  ATH_MSG_DEBUG("In ZdRecRun3");

  const EventContext& ctx = Gaudi::Hive::currentContext();

  ATH_MSG_DEBUG ("--> ZDC: ZdcRecRun3 execute starting on "
                 << ctx.evt()
                 << "th event");


  SG::ReadHandle<xAOD::ZdcModuleContainer> moduleContainer (m_zdcModuleContainerName, ctx);
  SG::ReadHandle<xAOD::ZdcModuleContainer> moduleSumContainer (m_zdcSumContainerName, ctx);
  
  // re-reconstruct big tubes 
  for (ToolHandle<ZDC::IZdcAnalysisTool>& tool : m_zdcTools)
    {
      ATH_CHECK( tool->recoZdcModules(*moduleContainer.get(), *moduleSumContainer.get()) );
    }

  return StatusCode::SUCCESS;

}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecRun3::finalize()
{

  ATH_MSG_DEBUG( "--> ZDC: ZdcRecRun3 finalize complete" );

  return StatusCode::SUCCESS;

}
//==================================================================================================

