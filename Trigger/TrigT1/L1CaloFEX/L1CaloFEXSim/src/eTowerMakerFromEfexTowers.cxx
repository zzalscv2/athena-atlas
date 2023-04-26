/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#undef NDEBUG

#include "xAODTrigL1Calo/eFexTowerContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"

#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include "L1CaloFEXSim/eTower.h"
#include "L1CaloFEXSim/eTowerBuilder.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "./eTowerMakerFromEfexTowers.h"
#include "L1CaloFEXSim/eFEXCompression.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

#include <cassert>
#include "SGTools/TestStore.h"

#include <ctime>


#include <iostream>
#include <fstream>

#define DEBUG_VHB 1


namespace LVL1 {

  eTowerMakerFromEfexTowers::eTowerMakerFromEfexTowers(const std::string& name, ISvcLocator* pSvcLocator)
    :  AthAlgorithm(name, pSvcLocator)//AthReentrantAlgorithm(name, pSvcLocator)
  { 
  
  }


StatusCode eTowerMakerFromEfexTowers::initialize()
{

  m_numberOfEvents = 1;

  ATH_CHECK( m_eTowerBuilderTool.retrieve() );
  ATH_CHECK( m_eFexTowerContainerSGKey.initialize() );
  ATH_CHECK( m_eFexTowerContainer2SGKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_eTowerContainerSGKey.initialize() );
  ATH_CHECK( m_noiseCutsKey.initialize(SG::AllowEmpty) );

  if (!m_noiseCutsKey.empty()) {
      // disable built-in noise cuts because will be loading them from conditions db instead
      eFEXCompression::s_disableNoiseCuts = true;
  }

  return StatusCode::SUCCESS;

}


  StatusCode eTowerMakerFromEfexTowers::execute(/*const EventContext& ctx*/) //const
{
  ATH_MSG_DEBUG("Executing " << name() << ", processing event number " << m_numberOfEvents );


  // load noise cuts .. should really only need to do this at start of runs, not every event!
  std::map<std::pair<int, int>, int> noiseCutsMap; // key is [eta,layer]
  if(!m_noiseCutsKey.empty()) {
      SG::ReadCondHandle <CondAttrListCollection> noiseCuts{m_noiseCutsKey/*, ctx*/ };
      if (noiseCuts.isValid()) {
          for (auto itr = noiseCuts->begin(); itr != noiseCuts->end(); ++itr) {
              if (itr->first >= 50) continue;
              noiseCutsMap[std::pair(itr->first, 0)] = itr->second["EmPS"].data<int>();
              noiseCutsMap[std::pair(itr->first, 1)] = itr->second["EmFR"].data<int>();
              noiseCutsMap[std::pair(itr->first, 2)] = itr->second["EmMD"].data<int>();
              noiseCutsMap[std::pair(itr->first, 3)] = itr->second["EmBK"].data<int>();
              noiseCutsMap[std::pair(itr->first, 4)] = (itr->first >= 10 && itr->first < 35)
                                                       ? itr->second["Tile"].data<int>()
                                                       : itr->second["HEC"].data<int>();
          }
      }
  }

  // STEP 0 - Make a fresh local eTowerContainer
  std::unique_ptr<eTowerContainer> local_eTowerContainerRaw = std::make_unique<eTowerContainer>();

  // STEP 1 - Make some eTowers and fill the local container
  m_eTowerBuilderTool->init(local_eTowerContainerRaw);
  local_eTowerContainerRaw->clearContainerMap();
  local_eTowerContainerRaw->fillContainerMap();

  SG::ReadHandle<xAOD::eFexTowerContainer> eFexTowers(m_eFexTowerContainerSGKey/*, ctx*/);
  if((!eFexTowers.isValid() || eFexTowers->size() < m_minTowersRequired) && !m_eFexTowerContainer2SGKey.empty()) {
      eFexTowers = SG::ReadHandle<xAOD::eFexTowerContainer>(m_eFexTowerContainer2SGKey);
      const xAOD::EventInfo* ei = nullptr;
      CHECK( evtStore()->retrieve(ei) );
      ei->auxdecor<bool>("eTowerMakerFromEfexTowers_usedSecondary") = true;
  }

  // STEP 2 - Do the efexTower-tower mapping - put this information into the eTowerContainer
  for(auto eFexTower : *eFexTowers) {
      // need to ensure this eFexTower is a "core" tower in a module ... so that there aren't disconnected inputs
      // and also need to only do one tower per location, of course
      auto tower = local_eTowerContainerRaw->findTower(eFexTower->eFEXtowerID());
      auto counts = eFexTower->et_count();
      for(size_t i=0;i<counts.size();i++) {
          if (eFexTower->disconnectedCount(i)) continue;
          if (counts.at(i)==0 || counts.at(i)>1020) continue; // disconnected or masked channel
          // special case logic for reordering |eta|=2.5 and overlap
          // and l1 1.8-2.0 ... need to put the merged sc counts into slots that wont be split
          int layer; int cell=i;
          if(i<1 || (i==4 && std::abs(eFexTower->eta()+0.025)>2.4)) {layer = 0;cell=0;}
          else if(i<5) layer = 1;
          else if(i<9) layer = 2;
          else if(i<10) layer = 3;
          else layer = 4;

          // apply noise cut ... for runs up to 14th April 2023 was killing with <, then from run 449180 onwards kills with <=
          // since long-term behaviour is latter, will use that
          if(counts.at(i) <= noiseCutsMap[std::pair( int( (eFexTower->eta() + 2.525)/0.1 ), layer)]) continue;

          // checking we haven't already filled this tower (happens when using efexDataTowers ... multiple towers per loc for different modules)
          // this is ugly as ...
          if(!tower->getET_float(layer,cell-(layer==1)*1-(layer==2)*5-(layer==3)*9-(layer==4)*10)) {
              // if tile then count is in steps of 500 MeV, not in latome multilinear encoding
              bool isTile = (std::abs(eFexTower->eta()+0.025)<1.5 && layer==4);
              tower->setET(cell,isTile ? (counts.at(i)*500.) : eFEXCompression::expand(counts.at(i)),layer + isTile);
          }
      }
  }



    // STEP 3 - Write the completed eTowerContainer into StoreGate (move the local copy in memory)
  SG::WriteHandle<LVL1::eTowerContainer> eTowerContainerSG(m_eTowerContainerSGKey/*, ctx*/);
  ATH_CHECK(eTowerContainerSG.record(std::move(/*my_eTowerContainerRaw*/local_eTowerContainerRaw)));

  // STEP 4 - Close and clean the event
  m_eTowerBuilderTool->reset();

  ATH_MSG_DEBUG("Executed " << name() << ", closing event number " << m_numberOfEvents );

  m_numberOfEvents++;

  return StatusCode::SUCCESS;
}

  

} // end of LVL1 namespace
