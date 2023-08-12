/*
    Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


#include "L1CaloFEXSim/gFEXDriver.h"
#include "L1CaloFEXSim/gFEXOutputCollection.h"
#include "StoreGate/WriteHandle.h"

#include <cassert>
#include "SGTools/TestStore.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include <ctime>

#define DEBUG_VHB 1


namespace LVL1 {

  gFEXDriver::gFEXDriver(const std::string& name, ISvcLocator* pSvcLocator)
    :  AthAlgorithm(name, pSvcLocator)
  {

  }


 gFEXDriver::~gFEXDriver()
{
  ATH_MSG_DEBUG("Destroying " << name() << "...");
}


StatusCode gFEXDriver::initialize()
{

  m_numberOfEvents = 1;

  ServiceHandle<ITHistSvc> histSvc("THistSvc","");

  ATH_CHECK( histSvc.retrieve() );

  ATH_CHECK( m_gFEXSysSimTool.retrieve() );

  ATH_CHECK( m_gFEXOutputCollectionSGKey.initialize() );

  return StatusCode::SUCCESS;

}


  StatusCode gFEXDriver::execute() //const
{

  ATH_MSG_DEBUG("Executing " << name() << ", processing event number " << m_numberOfEvents );


  // STEP 1 - Do some monitoring
  gFEXOutputCollection* my_gFEXOutputCollection = new gFEXOutputCollection();
  my_gFEXOutputCollection->setdooutput(true);

  // STEP 2 - Run the gFEXSysSim
  ATH_CHECK(m_gFEXSysSimTool->execute(my_gFEXOutputCollection));

  // STEP 3 - Close and clean the event
  m_gFEXSysSimTool->cleanup();

  // STEP 4 - Write the completed gFEXOutputCollection into StoreGate (move the local copy in memory)
  std::unique_ptr<gFEXOutputCollection> local_gFEXOutputCollection = std::unique_ptr<gFEXOutputCollection>(my_gFEXOutputCollection);
  SG::WriteHandle<LVL1::gFEXOutputCollection> gFEXOutputCollectionSG(m_gFEXOutputCollectionSGKey);
  ATH_CHECK(gFEXOutputCollectionSG.record(std::move(local_gFEXOutputCollection)));

  ATH_MSG_DEBUG("Executed " << name() << ", closing event number " << m_numberOfEvents );

  m_numberOfEvents++;

  return StatusCode::SUCCESS;
}


} // end of LVL1 namespace
