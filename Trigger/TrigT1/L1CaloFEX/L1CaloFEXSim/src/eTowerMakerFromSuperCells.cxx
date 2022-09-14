/*
    Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#undef NDEBUG

#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"

#include "xAODTrigL1Calo/TriggerTowerContainer.h"

#include "L1CaloFEXSim/eTower.h"
#include "L1CaloFEXSim/eTowerBuilder.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "L1CaloFEXSim/eTowerMakerFromSuperCells.h"

#include "L1CaloFEXSim/eSuperCellTowerMapper.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

#include <cassert>
#include "SGTools/TestStore.h"

#include <ctime>

#include <iostream>
#include <fstream>

#define DEBUG_VHB 1


namespace LVL1 {

  eTowerMakerFromSuperCells::eTowerMakerFromSuperCells(const std::string& name, ISvcLocator* pSvcLocator)
    :  AthAlgorithm(name, pSvcLocator)//AthReentrantAlgorithm(name, pSvcLocator)
  { 
  
  }


StatusCode eTowerMakerFromSuperCells::initialize()
{

  m_numberOfEvents = 1;

  ATH_CHECK( m_eTowerBuilderTool.retrieve() );

  ATH_CHECK( m_eSuperCellTowerMapperTool.retrieve() );

  ATH_CHECK( m_eTowerContainerSGKey.initialize() );

  return StatusCode::SUCCESS;

}


  StatusCode eTowerMakerFromSuperCells::execute(/*const EventContext& ctx*/) //const
{

  ATH_MSG_DEBUG("Executing " << name() << ", processing event number " << m_numberOfEvents );

  // STEP 0 - Make a fresh local eTowerContainer
  std::unique_ptr<eTowerContainer> local_eTowerContainerRaw = std::make_unique<eTowerContainer>();

  // STEP 1 - Make some eTowers and fill the local container
  m_eTowerBuilderTool->init(local_eTowerContainerRaw);
  local_eTowerContainerRaw->clearContainerMap();
  local_eTowerContainerRaw->fillContainerMap();

  // STEP 2 - Do the supercell-tower mapping - put this information into the eTowerContainer
  ATH_CHECK(m_eSuperCellTowerMapperTool->AssignSuperCellsToTowers(local_eTowerContainerRaw));
  ATH_CHECK(m_eSuperCellTowerMapperTool->AssignTriggerTowerMapper(local_eTowerContainerRaw));

  // STEP 2.5 - Set up a the first CSV file if necessary (should only need to be done if the mapping changes, which should never happen unless major changes to the simulation are required)
  if(false){ // CSV CODE TO BE RE-INTRODUCED VERY SOON
    if(m_numberOfEvents == 1){
      std::ofstream sc_tower_map;
      sc_tower_map.open("./sc_tower_map.csv");
      sc_tower_map << "#eTowerID,scID,slot,isSplit" << "\n";
      
      for (auto thistower : *local_eTowerContainerRaw){
        int slotcount = 0;
        for (int layer = 0; layer<=4; layer++){
          std::vector<Identifier> scIDs = thistower->getLayerSCIDs(layer);
          std::vector<unsigned int> splits = thistower->getETSplits();
          for (auto scid : scIDs){
            sc_tower_map << thistower->id() << "," << scid << "," << slotcount << "," << splits[slotcount] << "\n";
            slotcount++;
          }
        }
      }
      sc_tower_map.close();
      
    }
  }

  // STEP 3 - Write the completed eTowerContainer into StoreGate (move the local copy in memory)
  SG::WriteHandle<LVL1::eTowerContainer> eTowerContainerSG(m_eTowerContainerSGKey/*, ctx*/);
  ATH_CHECK(eTowerContainerSG.record(std::move(/*my_eTowerContainerRaw*/local_eTowerContainerRaw)));

  // STEP 4 - Close and clean the event  
  m_eSuperCellTowerMapperTool->reset();
  m_eTowerBuilderTool->reset();

  ATH_MSG_DEBUG("Executed " << name() << ", closing event number " << m_numberOfEvents );

  m_numberOfEvents++;

  return StatusCode::SUCCESS;
}

  

} // end of LVL1 namespace
