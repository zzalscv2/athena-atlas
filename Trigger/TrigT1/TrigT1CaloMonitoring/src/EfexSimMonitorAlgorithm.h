/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_EFEXSIMMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_EFEXSIMMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
//
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexEMRoI.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/eFexTauRoI.h"

#include "xAODTrigL1Calo/eFexTowerContainer.h"
#include "xAODTrigL1Calo/eFexTower.h"
#include "FourMomUtils/P4Helpers.h"

class EfexSimMonitorAlgorithm : public AthMonitorAlgorithm {
public:EfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~EfexSimMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:

  StringProperty m_packageName{this,"PackageName","EfexSimMonitor","group name for histograming"};
  IntegerProperty m_maxDebugTreeEntries{this,"MaxDebugTreeEntries",-1,"Maximum number of entries in the debug tree, or -1 for no max"};
  int m_treeEntries = 0; // how many fills have happened

  // container keys including this, steering parameter, default value and help description
  SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFexEmContainerKey{this,"eFexEMRoIContainer","L1_eEMRoI","SG key of the data eFex Em RoI container"};
  SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eFexTauContainerKey{this,"eFexTauRoIContainer","L1_eTauRoI","SG key of the data eFex Tau RoI container"};
  SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFexEmSimContainerKey{this,"eFexEMRoISimContainer","L1_eEMRoISim","SG key of the simulated eFex Em RoI container"};
  SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eFexTauSimContainerKey{this,"eFexTauSimRoIContainer","L1_eTauRoISim","SG key of the simulated eFex Tau RoI container"};

  SG::ReadDecorHandleKey<xAOD::EventInfo> m_decorKey;

  StatusCode fillEmErrorHistos(const std::string &errName, const xAOD::eFexEMRoIContainer *emcont, const std::set<uint32_t> &simEqDataCoords) const;
  StatusCode fillTauErrorHistos(const std::string &errName, const xAOD::eFexTauRoIContainer *taucont, const std::set<uint32_t> &simEqDataCoords) const;


    template <typename T> void fillVectors(const SG::ReadHandleKey<T>& key, const EventContext& ctx, std::vector<float>& etas, std::vector<float>& phis, std::vector<unsigned int>& word0s) const {
        etas.clear();phis.clear();word0s.clear();
        SG::ReadHandle<T> tobs{key, ctx};
        if(tobs.isValid()) {
            etas.reserve(tobs->size());
            phis.reserve(tobs->size());
            word0s.reserve(tobs->size());
            for(auto tob : *tobs) {
                etas.push_back(tob->eta());
                phis.push_back(tob->phi());
                word0s.push_back(tob->word0());
            }
        }
    }

  template <typename T> unsigned int fillHistos(const SG::ReadHandleKey<T>& key1, const SG::ReadHandleKey<T>& key2, const std::string& groupSuffix, const EventContext& ctx ) const {
      SG::ReadHandle<T> tobs1{key1, ctx};
      SG::ReadHandle<T> tobs2{key2, ctx};

      std::set<uint32_t> word0s2;
      std::set<uint32_t> partword0s2; // just the location bits
      if(tobs2.isValid()) {
          for(auto tob : *tobs2) {
              word0s2.insert(tob->word0());
              partword0s2.insert(tob->word0()&0xff000000);
          }

      }

      auto tobEta = Monitored::Scalar<float>("tobEta",0.0);
      auto tobPhi = Monitored::Scalar<float>("tobPhi",0.0);
      auto tobMismatched = Monitored::Scalar<float>("tobMismatched",0.0);

      std::string groupPrefix = m_packageName+"_"+tobs1.key();

      // for each collection record if TOB is matched or not
      unsigned int nUnmatched = 0;
      if(tobs1.isValid()) {
          for(auto tob : *tobs1) {
              tobEta = tob->eta();
              tobPhi = tob->phi();
              tobMismatched=1;
              if(partword0s2.find(tob->word0()&0xff000000)==partword0s2.end()) {
                  fill(groupPrefix+"_unmatched"+groupSuffix,tobEta,tobPhi);
                  nUnmatched++;
              } else if(word0s2.find(tob->word0()) == word0s2.end()) {
                  fill(groupPrefix+"_partmatched"+groupSuffix,tobEta,tobPhi);
                  nUnmatched++;
              } else {
                  fill(groupPrefix+"_matched"+groupSuffix,tobEta,tobPhi);
                  tobMismatched=0;
              }
              fill(groupPrefix+"_mismatchedFrac"+groupSuffix,tobEta,tobPhi,tobMismatched);
              if(tobMismatched && this->msgLevel(MSG::DEBUG)) {
                  const xAOD::eFexTowerContainer* towers = nullptr;
                  evtStore()->retrieve(towers,groupSuffix=="2" ? "L1_eFexDataTowers" : "L1_eFexEmulatedTowers").ignore();
                  std::cout << "evtNumber " << GetEventInfo(ctx)->eventNumber() << " " << tobs1.key() << " " << (groupSuffix=="2" ? "L1_eFexDataTowers" : "L1_eFexEmulatedTowers") << " mismatched: 0x" << std::hex << tob->word0() << std::dec << " (" << tob->eta() << "," << tob->phi() << ")" << std::endl;
                  for(auto tower : *towers) {
                      if (std::abs(tower->eta() - tob->eta()) < 0.2 && std::abs(P4Helpers::deltaPhi(tower->phi(),tob->phi()))<0.2) {
                          std::cout << tower->eta() << " " << tower->phi() << " : ";
                          for(auto& c : tower->et_count()) std::cout << c << ",";
                          std::cout << std::endl;
                      }
                  }
              }
          }
      }
      return nUnmatched;

  }

    template <typename T> void compareTOBs(T& dataTOBs, T& simTOBs, std::set<uint32_t> &simEqDataWord0s) const {

        // loop over input TOBs and simulated TOBs and fill a std set for those where the first TOB word matches
        //

        ATH_MSG_DEBUG("compareTOBs ndata " << dataTOBs->size() << " nsim " << simTOBs->size());

        // Use a std::set of TOB word0s to match TOBs and simTOBs
        std::set<uint32_t> tobWord0sData;

        // Fill set with word0 of TOBs
        for (auto t : *dataTOBs) {
            uint32_t word0Data = t->word0();
            ATH_MSG_DEBUG("compareTOBs data " << word0Data);
            tobWord0sData.insert(word0Data);
        }

        // Set simEqData if the TOB word0s match
        if (simTOBs.isValid()) {
            for (auto t: *simTOBs) {
                uint32_t word0Sim = t->word0();
                ATH_MSG_DEBUG("compareTOBs sim " << word0Sim);
                if (tobWord0sData.find(word0Sim) != tobWord0sData.end()) simEqDataWord0s.insert(word0Sim);
            }
        }
    }

};
#endif
