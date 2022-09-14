/*
    Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


#undef NDEBUG

#include "L1CaloFEXSim/eFEXDriver.h"

#include "L1CaloFEXSim/eSuperCellTowerMapper.h"

#include "L1CaloFEXSim/eFEXSim.h"
#include "L1CaloFEXSim/eFEXOutputCollection.h"
#include "L1CaloFEXSim/eFEXegTOB.h"

#include "TROOT.h"
#include "TH1.h"
#include "TH1F.h"
#include "TPad.h"
#include "TCanvas.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

#include "xAODTrigger/eFexEMRoI.h"
#include "xAODTrigger/eFexEMRoIContainer.h"

#include "xAODTrigger/eFexTauRoI.h"
#include "xAODTrigger/eFexTauRoIContainer.h"

#include <cassert>
#include "SGTools/TestStore.h"

#include <ctime>

#include <iostream>
#include <fstream>

#define DEBUG_VHB 1


namespace LVL1 {

  eFEXDriver::eFEXDriver(const std::string& name, ISvcLocator* pSvcLocator)
    :  AthAlgorithm(name, pSvcLocator)//AthReentrantAlgorithm(name, pSvcLocator)
  { 
  
  }


 eFEXDriver::~eFEXDriver()
{
  ATH_MSG_DEBUG("Destroying " << name() << "...");
}


StatusCode eFEXDriver::initialize()
{

  m_numberOfEvents = 1;

  ATH_CHECK( m_eFEXSysSimTool.retrieve() );

  ATH_CHECK( m_eFEXOutputCollectionSGKey.initialize() );

  return StatusCode::SUCCESS;

}


StatusCode eFEXDriver::finalize()
{
  ATH_MSG_DEBUG("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}


  StatusCode eFEXDriver::execute(/*const EventContext& ctx*/) //const
{

  ATH_MSG_DEBUG("Executing " << name() << ", processing event number " << m_numberOfEvents );

  // STEP 1 - Set up the eFEXSysSim
  m_eFEXSysSimTool->init();

  // STEP 2 - Do some monitoring
  eFEXOutputCollection* my_eFEXOutputCollection = new eFEXOutputCollection();
  my_eFEXOutputCollection->setdooutput(true);

  // STEP 3 - Run THE eFEXSysSim
  ATH_CHECK(m_eFEXSysSimTool->execute(my_eFEXOutputCollection));

  // STEP 4 - Close and clean the event  
  m_eFEXSysSimTool->cleanup();

  // STEP 5 - Write the completed eFEXOutputCollection into StoreGate (move the local copy in memory)
  std::unique_ptr<eFEXOutputCollection> local_eFEXOutputCollection = std::unique_ptr<eFEXOutputCollection>(my_eFEXOutputCollection);
  SG::WriteHandle<LVL1::eFEXOutputCollection> eFEXOutputCollectionSG(m_eFEXOutputCollectionSGKey);
  ATH_CHECK(eFEXOutputCollectionSG.record(std::move(local_eFEXOutputCollection)));

  ATH_MSG_DEBUG("Executed " << name() << ", closing event number " << m_numberOfEvents );

  m_numberOfEvents++;

  return StatusCode::SUCCESS;
}

  

} // end of LVL1 namespace
