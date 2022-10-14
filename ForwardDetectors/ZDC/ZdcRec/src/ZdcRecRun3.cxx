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
#include "ZdcRec/ZdcRecChannelToolLucrod.h"
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

	// Reconstruction Tool
	ATH_CHECK( m_ChannelTool.retrieve() );

	// Reconstruction Tool
	ATH_CHECK( m_zdcTool.retrieve() );

	ATH_CHECK( m_zdcModuleContainerName.initialize() );
	ATH_CHECK( m_zdcSumContainerName.initialize() );
	ATH_CHECK( m_zldContainerName.initialize(SG::AllowEmpty) );

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

  //Look for the container presence
  if (m_zldContainerName.empty()) {
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("Trying to get LUCROD DATA!");
  SG::ReadHandle<ZdcLucrodDataContainer> zldContainer    (m_zldContainerName, ctx);
  ATH_MSG_DEBUG("Did I get LUCROD DATA?");
  
  //Create the containers to hold the reconstructed information (you just pass the pointer and the converter does the work)	
  std::unique_ptr<xAOD::ZdcModuleContainer> moduleContainer( new xAOD::ZdcModuleContainer());
  std::unique_ptr<xAOD::ZdcModuleAuxContainer> moduleAuxContainer( new xAOD::ZdcModuleAuxContainer() );
  moduleContainer->setStore( moduleAuxContainer.get() );

  //Create the containers to hold the reconstructed information (you just pass the pointer and the converter does the work)	
  std::unique_ptr<xAOD::ZdcModuleContainer> moduleSumContainer( new xAOD::ZdcModuleContainer());
  std::unique_ptr<xAOD::ZdcModuleAuxContainer> moduleSumAuxContainer( new xAOD::ZdcModuleAuxContainer() );
  moduleSumContainer->setStore( moduleSumAuxContainer.get() );

  ATH_MSG_DEBUG("Trying to convert!");

  // rearrange ZDC channels and perform fast reco on all channels (including non-big tubes)
  int ncha = m_ChannelTool->convertLucrod2ZM(zldContainer.get(), moduleContainer.get(), moduleSumContainer.get() );
  ATH_MSG_DEBUG("m_ChannelTool->convertLucrod2ZM returned " << ncha << " channels");

  ATH_MSG_DEBUG("Dumping modules");
  ATH_MSG_DEBUG( ZdcModuleToString(*moduleContainer) );
  ATH_MSG_DEBUG("Dumping module sums");
  ATH_MSG_DEBUG( ZdcModuleToString(*moduleSumContainer) );
  
  // re-reconstruct big tubes 
  ATH_CHECK( m_zdcTool->recoZdcModules(*moduleContainer.get(), *moduleSumContainer.get()) ); // passes by reference

  // eventually reconstruct RPD, using ML libraries
  // ATH_CHECK( m_rpdTool...)

  SG::WriteHandle<xAOD::ZdcModuleContainer> moduleContainerH (m_zdcModuleContainerName, ctx);
  ATH_CHECK( moduleContainerH.record (std::move(moduleContainer),
				      std::move(moduleAuxContainer)) );

  SG::WriteHandle<xAOD::ZdcModuleContainer> moduleSumContainerH (m_zdcSumContainerName, ctx);
  ATH_CHECK( moduleSumContainerH.record (std::move(moduleSumContainer),
					 std::move(moduleSumAuxContainer)) );

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

