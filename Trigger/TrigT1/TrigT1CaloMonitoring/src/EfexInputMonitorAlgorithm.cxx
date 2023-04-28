/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "EfexInputMonitorAlgorithm.h"
#include "TTree.h"
#include "PathResolver/PathResolver.h"
#include "TFile.h"

EfexInputMonitorAlgorithm::EfexInputMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode EfexInputMonitorAlgorithm::initialize() {

  ATH_MSG_DEBUG("EfexInputMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);
  ATH_MSG_DEBUG("m_eFexTowerContainer"<< m_eFexTowerContainerKey);

  // we initialise all the containers that we need
  ATH_CHECK( m_eFexTowerContainerKey.initialize() );
  ATH_CHECK( m_eFexTowerContainerRefKey.initialize() );


  // load the scid map
    if (auto fileName = PathResolverFindCalibFile( "L1CaloFEXByteStream/2023-02-13/scToEfexTowers.root" ); !fileName.empty()) {
        std::unique_ptr<TFile> f( TFile::Open(fileName.c_str()) );
        if (f) {
            TTree* t = f->Get<TTree>("mapping");
            if(t) {
                unsigned long long scid = 0;
                std::pair<int,int> coord = {0,0};
                std::pair<int,int> slot;
                t->SetBranchAddress("scid",&scid);
                t->SetBranchAddress("etaIndex",&coord.first);
                t->SetBranchAddress("phiIndex",&coord.second);
                t->SetBranchAddress("slot1",&slot.first);
                t->SetBranchAddress("slot2",&slot.second);
                for(Long64_t i=0;i<t->GetEntries();i++) {
                    t->GetEntry(i);
                    m_scMap[std::make_pair(coord,slot.first)].first.insert(scid);
                    m_scMap[std::make_pair(coord,slot.second)].first.insert(scid);
                }
                // now convert scid list to list of strings
                for(auto& [key,scs] : m_scMap) {
                    std::stringstream s;
                    for(auto id : scs.first) {
                        s << (id>>32) << ",";
                    }
                    scs.second = s.str();
                    scs.first.clear(); // not needed any more, so clear it
                }
            }
        }
    }



  
  return AthMonitorAlgorithm::initialize();
}

StatusCode EfexInputMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  ATH_MSG_DEBUG("EfexInputMonitorAlgorithm::fillHistograms");

  //std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> variables;

  // Access eFex eTower container
  SG::ReadHandle<xAOD::eFexTowerContainer> eFexTowerContainer{m_eFexTowerContainerKey, ctx};
  if(!eFexTowerContainer.isValid()){
    ATH_MSG_ERROR("No eFex Tower container found in storegate  "<< m_eFexTowerContainerKey);
    return StatusCode::SUCCESS;
  }

    auto Towereta = Monitored::Scalar<float>("TowerEta",0.0);
    auto Towerphi = Monitored::Scalar<float>("TowerPhi",0.0);
    auto TowerRefCount = Monitored::Scalar<int>("RefTowerCount",0);

    // Access eFexTower ref container, if possible
    SG::ReadHandle<xAOD::eFexTowerContainer> eFexTowerContainerRef{m_eFexTowerContainerRefKey, ctx};
    auto etaIndex = [](float eta) { return int( eta*10 ) + ((eta<0) ? -1 : 1); };
    auto phiIndex = [](float phi) { return int( phi*32./M_PI ) + (phi<0 ? -1 : 1); };
    std::map<std::pair<int,int>,const xAOD::eFexTower*> refTowers;
    bool missingLAr = false;
    if (eFexTowerContainerRef.isValid()) {
        for (auto eTower: *eFexTowerContainerRef) {
            refTowers[std::pair(etaIndex(eTower->eta() + 0.025), phiIndex(eTower->phi() + 0.025))] = eTower;

            // fill profile histograms for each slot so that we can identify when a slot is being noisy
            Towereta = eTower->eta(); Towerphi = eTower->phi();
            std::vector<uint16_t> Toweret_count=eTower->et_count();
            for(size_t i=0;i<Toweret_count.size();i++) {
                TowerRefCount = Toweret_count[i];
                if (TowerRefCount==1025) missingLAr=true;
                if (TowerRefCount==0 || TowerRefCount==1025) continue; // skip masked and when not available
                fill(m_packageName+"_slot" + std::to_string(i + (i==10 && std::abs(eTower->eta())>1.5)),Towereta,Towerphi,TowerRefCount);
            }
        }


    }

  // monitored variables for histograms
  auto evtNumber = Monitored::Scalar<ULong64_t>("EventNumber",0.0);
    evtNumber = GetEventInfo(ctx)->eventNumber();
  auto nEfexTowers = Monitored::Scalar<std::string>("NEfexTowers","0");

  auto Towermodule = Monitored::Scalar<uint8_t>("TowerModule",0.0);
  auto Towerfpga = Monitored::Scalar<uint8_t>("TowerFpga",0.0);
  auto Toweretcount1 = Monitored::Scalar<uint16_t>("TowerEtcount1",0.0);
  auto Toweretcount2 = Monitored::Scalar<uint16_t>("TowerEtcount2",0.0);
  auto Toweretcount3 = Monitored::Scalar<uint16_t>("TowerEtcount3",0.0);
  auto Toweretcount4 = Monitored::Scalar<uint16_t>("TowerEtcount4",0.0);
  auto Toweretcount5 = Monitored::Scalar<uint16_t>("TowerEtcount5",0.0);
  auto Toweretcount6 = Monitored::Scalar<uint16_t>("TowerEtcount6",0.0);
  auto Toweretcount7 = Monitored::Scalar<uint16_t>("TowerEtcount7",0.0);
  auto Toweretcount8 = Monitored::Scalar<uint16_t>("TowerEtcount8",0.0);
  auto Toweretcount9 = Monitored::Scalar<uint16_t>("TowerEtcount9",0.0);
  auto Toweretcount10 = Monitored::Scalar<uint16_t>("TowerEtcount10",0.0);
  auto Toweretcount11 = Monitored::Scalar<uint16_t>("TowerEtcount11",0.0);
  auto Toweretcount12 = Monitored::Scalar<uint16_t>("TowerEtcount12",0.0);
  auto Toweremstatus = Monitored::Scalar<uint32_t>("TowerEmstatus",0.0);
  auto Towerhadstatus = Monitored::Scalar<uint32_t>("TowerHadstatus",0.0);

  auto TowerId = Monitored::Scalar<int32_t>("TowerId",0);
  auto TowerSlot = Monitored::Scalar<int32_t>("TowerSlot",0);
  auto TowerSlotSplitHad = Monitored::Scalar<int32_t>("TowerSlotSplitHad",0); // same as TowerSlot but +1 if was LAr (|eta|>1.5)
  auto TowerCount = Monitored::Scalar<int>("TowerCount",0);
  auto SlotSCID = Monitored::Scalar<std::string>("SlotSCID","");


  auto lbnString = Monitored::Scalar<std::string>("LBNString",std::to_string(GetEventInfo(ctx)->lumiBlock()));

  ATH_MSG_VERBOSE("number of efex towers = "<< eFexTowerContainer->size());


  nEfexTowers=std::to_string(eFexTowerContainer->size());
  fill(m_packageName,nEfexTowers);
  if (!eFexTowerContainer->empty()) {
      ATH_MSG_INFO("l1id = " << GetEventInfo(ctx)->extendedLevel1ID());
  }

  std::set<std::pair<std::pair<int,int>,int>> doneCounts; // only fill each count once

  for(const xAOD::eFexTower* efexTowerRoI : *eFexTowerContainer){


    Towereta=efexTowerRoI->eta();
    Towerphi=efexTowerRoI->phi();
    Towermodule=efexTowerRoI->module();
    Towerfpga=efexTowerRoI->fpga();
    Toweremstatus=efexTowerRoI->em_status();
    Towerhadstatus=efexTowerRoI->had_status();
    std::vector<uint16_t> Toweret_count=efexTowerRoI->et_count();
    Toweretcount1=Toweret_count.at(0);
    Toweretcount2=Toweret_count.at(1);
    Toweretcount3=Toweret_count.at(2);
    Toweretcount4=Toweret_count.at(3);
    Toweretcount5=Toweret_count.at(4);
    Toweretcount6=Toweret_count.at(5);
    Toweretcount7=Toweret_count.at(6);
    Toweretcount8=Toweret_count.at(7);
    Toweretcount9=Toweret_count.at(8);
    Toweretcount10=Toweret_count.at(9);
    Toweretcount11= (std::abs(Towereta)<1.5) ? Toweret_count.at(10) : 0.;
    Toweretcount12= (std::abs(Towereta)<1.5) ? 0. : Toweret_count.at(10);

    fill(m_packageName+"_fexTowers",Towereta,Towerphi,Towermodule,Towerfpga,Toweremstatus,Towerhadstatus,
         Toweretcount1,Toweretcount2,Toweretcount3,Toweretcount4,Toweretcount5,Toweretcount6,Toweretcount7,
            Toweretcount8,Toweretcount9,Toweretcount10,(std::abs(Towereta)<1.5) ? Toweretcount11 : Toweretcount12);

      if(!refTowers.empty()) {
          auto& eTower = efexTowerRoI;
          auto& counts = Toweret_count;

          TowerId = eTower->id();
          auto coord = std::pair(etaIndex(eTower->eta() + 0.025), phiIndex(eTower->phi() + 0.025));
          auto itr = refTowers.find(coord);
          if (itr == refTowers.end()) {
              // treat every slot in tower as mismatch to a count of -1 for ref;
              // exception if no LAr ... then we expect no ref tower outside of tile region (see eFexTowerBuilder)
              if (missingLAr && std::abs(Towereta)>1.5) continue;
              TowerRefCount = -1;
              for(size_t i=0;i<counts.size();i++) {
                  TowerSlot = i; TowerCount = counts[i];
                  TowerSlotSplitHad = i + (i==10 && std::abs(Towereta)>1.5);
                  fill(m_packageName+"_RefCompareTree",evtNumber,lbnString,TowerId,Towereta,Towerphi,Toweremstatus,Towerhadstatus,TowerSlot,TowerCount,TowerRefCount,TowerSlotSplitHad);
              }
              continue;
          }
          const xAOD::eFexTower *eeTower = itr->second;
          auto ecounts = eeTower->et_count();
          if (ecounts.size() != counts.size()) {
              // flag the excess slots in the tree
              TowerRefCount = -1;
              for(size_t i=ecounts.size();i<counts.size();i++) {
                  TowerSlot = i; TowerCount = counts[i];
                  TowerSlotSplitHad = i + (i==10 && std::abs(Towereta)>1.5);
                  fill(m_packageName+"_RefCompareTree",evtNumber,lbnString,TowerId,Towereta,Towerphi,Toweremstatus,Towerhadstatus,TowerSlot,TowerCount,TowerRefCount,TowerSlotSplitHad);
              }
              continue;
          }
          std::stringstream cStr;
          std::stringstream ecStr;
          bool mismatch = false;
          for (size_t i = 0; i < ecounts.size(); i++) {
              cStr << " " << counts[i];
              ecStr << " " << ecounts[i];
              if (eTower->disconnectedCount(i)) {
                  cStr << "x";
                  ecStr << " ";
                  continue;
              } // skip disconnected counts
              if (ecounts[i]==1025) {
                  // indicates LAr is absent ... so don't compare
                  ecStr << "x";
                  cStr << " ";
                  continue;
              }
              TowerSlotSplitHad = i + (i==10 && std::abs(Towereta)>1.5);
              TowerCount = counts[i];
              if (counts[i] != ecounts[i] && doneCounts.find(std::pair(coord,i))==doneCounts.end()) {
                  if(counts[i]==1025 && ecounts[i]==0) {
                      // this can occur when the input has been zero-suppressed (indicated with 1025 by the decoders and builders)
                      // so 1025 is actually a match to 0
                  } else {
                      mismatch = true;
                      TowerSlot = i;
                      TowerRefCount = ecounts[i];
                      std::string s;
                      if (auto itr = m_scMap.find(std::make_pair(coord, i)); itr != m_scMap.end()) {
                          s = itr->second.second;
                      }
                      SlotSCID = s;
                      fill(m_packageName + "_RefCompareTree", evtNumber, lbnString, TowerId, Towereta, Towerphi,
                           Toweremstatus, Towerhadstatus, TowerSlot, TowerCount, TowerRefCount, TowerSlotSplitHad,
                           SlotSCID);
                  }
              }
              if (doneCounts.find(std::pair(coord,i))==doneCounts.end()) {
                  fill(m_packageName + "_fex_slot" + std::to_string(TowerSlotSplitHad), Towereta, Towerphi, TowerCount);
                  doneCounts.insert(std::pair(coord,i));
              }
          }
          if (mismatch) {
              ATH_MSG_DEBUG(eTower->id() << " efex:" << cStr.str());
              ATH_MSG_DEBUG(eeTower->id() << " calo:" << ecStr.str());
          }
          Monitored::Scalar<float> weight( "Weight", 1.-float(mismatch));
          fill(m_packageName+"_RefCompareFrac",Towereta,Towerphi,weight);
      }
  }

  return StatusCode::SUCCESS;
}


