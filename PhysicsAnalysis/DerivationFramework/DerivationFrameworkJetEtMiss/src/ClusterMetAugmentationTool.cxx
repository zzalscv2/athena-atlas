/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ClusterMetAugmentationTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: Alex Bunka (alexandertarasbunka@cern.ch)
///////////////////////////////////////////////////////////////////
// The goal of this tool is to provide a simple clusterMET 
// variable(s) to EventInfo for use in analysis. The 
// clusterMET consists of a summation of cluster momentum,
// removal of clusters associated with leptons, addition of
// the leptons themselves, and re-addition of the pileup 
// lost when removing clusters earlier.
// CaloCalTopoCLusters are not available in JETM1, so this 
// tool provides the means to calculate MET without METMaKER
// in JETM1 (and other derivations).
///////////////////////////////////////////////////////////////////

#include "ClusterMetAugmentationTool.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODEventShape/EventShape.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCore/ShallowCopy.h"
#include <xAODJet/JetContainer.h>
#include <xAODJet/JetAuxContainer.h>
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "TMath.h"
#include <cmath>
#include <Math/Vector4D.h>
#include <Math/VectorUtil.h>

namespace DerivationFramework {

    ClusterMetAugmentationTool::ClusterMetAugmentationTool(const std::string& t,
        const std::string& n,
        const IInterface* p) :
        AthAlgTool(t, n, p)
    {
        declareInterface<DerivationFramework::IAugmentationTool>(this);
        declareProperty("jetsContainer", m_jetContainerName = "AntiKt4EMTopoJets");
        declareProperty("electronsContainer", m_electronContainerName = "Electrons");
        declareProperty("muonsContainer", m_muonContainerName = "Muons");
    }

    StatusCode ClusterMetAugmentationTool::initialize()
    {
        ATH_MSG_INFO("Init ClusterMetAugmentationTool");

        m_ranNumGen = new TRandom3();

        return StatusCode::SUCCESS;
    }

    StatusCode ClusterMetAugmentationTool::finalize()
    {
        ATH_MSG_INFO("Finalize ClusterMetAugmentationTool");
        return StatusCode::SUCCESS;
    }

    StatusCode ClusterMetAugmentationTool::addBranches() const
    {
        const xAOD::EventInfo* ei = nullptr;
        ATH_CHECK(evtStore()->retrieve(ei, "EventInfo"));

        //#############//
        //# ELECTRONS #//
        //#############//

        const xAOD::ElectronContainer* electrons = nullptr;
        if (evtStore()->retrieve(electrons, m_electronContainerName).isFailure()) {
            ATH_MSG_WARNING("Couldn't retrieve electrons from TEvent");
            return StatusCode::FAILURE;
        }

        //#########//
        //# MUONS #//
        //#########//

        const xAOD::MuonContainer* muons = nullptr;
        if (evtStore()->retrieve(muons, m_muonContainerName).isFailure()) {
            ATH_MSG_WARNING("Couldn't retrieve muons from TEvent");
            return StatusCode::FAILURE;
        }

        //########//
        //# JETS #//
        //########//
        
        // retrieve container
        const xAOD::JetContainer* jets(0);
        if (evtStore()->retrieve(jets, m_jetContainerName).isFailure()) {
            ATH_MSG_ERROR("Couldn't retrieve jets with key: AntiKt4EMTopoJets");
            return StatusCode::FAILURE;
        }

        std::vector<TLorentzVector> Jmomenta;

        int numJets = 0;

        bool seeded = false;

        for (auto jet : *jets) {
            numJets++;

            if (numJets == 1) { //seeding RNG here for later use
                unsigned long seed = static_cast<unsigned long>(1.e5 * std::abs(jet->phi()));
                if (seed == 0) seed = 45583453;
                m_ranNumGen->SetSeed(seed);
                seeded = true;
            }

            TLorentzVector tempMomenta;
            const float& pt(jet->auxdata<float>("DFCommonJets_Calib_pt"));
            const float& eta(jet->auxdata<float>("DFCommonJets_Calib_eta"));
            const float& phi(jet->auxdata<float>("DFCommonJets_Calib_phi"));
            const float& m(jet->auxdata<float>("DFCommonJets_Calib_m"));
            tempMomenta.SetPtEtaPhiM(pt, eta, phi, m);
            Jmomenta.push_back(tempMomenta);
        }


        //##############//
        //# CLUSTERMET #//
        //##############//

        const static SG::AuxElement::Decorator<float> cluster_METx("DFCommonJets_clusterMETx");
        const static SG::AuxElement::Decorator<float> cluster_METy("DFCommonJets_clusterMETy");

        double clusterMETx = 0;
        double clusterMETy = 0;
        double METx = 0;
        double METy = 0;

        double pi = TMath::Pi();

        // retrieve the clusters  
        const xAOD::CaloClusterContainer* clusters = nullptr;
        ATH_CHECK(evtStore()->retrieve(clusters, "CaloCalTopoClusters"));

        // loop over clusters
        for (auto cluster : *clusters) {
            // check if clusters overlap with the leptons

            bool notElectronOverlap = true;
            for (auto el : *electrons) {
                if (cluster->p4(xAOD::CaloCluster_v1::State::UNCALIBRATED).DeltaR(el->p4()) < 0.2) {
                    notElectronOverlap = false;
                    break;
                }    
            }

            bool notMuonOverlap = true;
            for (auto mu : *muons) {
                if (cluster->p4(xAOD::CaloCluster_v1::State::UNCALIBRATED).DeltaR(mu->p4()) < 0.2) {
                    notMuonOverlap = false;
                    break;
                }     
            }

            // only keep the clusters that are not overlapping with the leptons
            if (notElectronOverlap && notMuonOverlap) {
                // negative sum over the clusters
                clusterMETx -= cluster->p4(xAOD::CaloCluster_v1::State::UNCALIBRATED).Px();
                clusterMETy -= cluster->p4(xAOD::CaloCluster_v1::State::UNCALIBRATED).Py();
            } // end of overlap check
       
        } // end of cluster loop
      
        //Random cones to estimate pileup
        double totalConesEnergy = 0;
        int conesCount = 0;

        if(!seeded) m_ranNumGen->SetSeed(56561239);

        for (int j = 0; j < 100; j++) {

            bool notJetOverlap = true;
            bool notEOverlap = true;
            bool notMuOverlap = true;

            double randomPhi = m_ranNumGen->Uniform(2 * pi);
            double randomEta = m_ranNumGen->Uniform(4);

            ROOT::Math::PtEtaPhiMVector conePosition(0., randomEta, randomPhi, 0.);

            for (auto jet : Jmomenta) {
                if (ROOT::Math::VectorUtil::DeltaR(jet, conePosition) < 0.4) {
                    notJetOverlap = false;
                    break;
                }      
            }

            for (auto el : *electrons) {
                if (ROOT::Math::VectorUtil::DeltaR(el->p4(), conePosition) < 0.2) {
                    notEOverlap = false;
                    break;
                }    
            }

            for (auto mu : *muons) {
                if (ROOT::Math::VectorUtil::DeltaR(mu->p4(), conePosition) < 0.2) {
                    notMuOverlap = false;
                    break;
                }    
            }

            if (notEOverlap && notJetOverlap && notMuOverlap) {
                conesCount++;
                for (auto cluster : *clusters) {
                    if (ROOT::Math::VectorUtil::DeltaR(cluster->p4(xAOD::CaloCluster_v1::State::UNCALIBRATED), conePosition) < 0.2) {
                        totalConesEnergy += std::abs(cluster->p4(xAOD::CaloCluster_v1::State::UNCALIBRATED).E());
                    }
                }
            }
        }//end cone loop

        double averageConeEnergy = (totalConesEnergy) / conesCount;
        
        double electronXPUCones = 0;
        double electronYPUCones = 0;
        double electronPx = 0;
        double electronPy = 0;

        for (auto el : *electrons) { //cone of pileup around each electron. Sum Px and Py
            double electronXCone = (el->p4().Px() / el->p4().Pt()) * averageConeEnergy;
            double electronYCone = (el->p4().Py() / el->p4().Pt()) * averageConeEnergy;
            electronXPUCones += electronXCone;
            electronYPUCones += electronYCone;
            electronPx += el->p4().Px();
            electronPy += el->p4().Py();
        }

        double muonXPUCones = 0;
        double muonYPUCones = 0;
        double muonPx = 0;
        double muonPy = 0;

        for (auto mu : *muons) { //cone of pileup around each muon. Sum Px and Py 
            double muonXCone = (mu->p4().Px() / mu->p4().Pt()) * averageConeEnergy;
            double muonYCone = (mu->p4().Py() / mu->p4().Pt()) * averageConeEnergy;
            muonXPUCones += muonXCone;
            muonYPUCones += muonYCone;
            muonPx += mu->p4().Px();
            muonPy += mu->p4().Py();
        }
        
        METx = clusterMETx - electronPx - muonPx - electronXPUCones - muonXPUCones;
        METy = clusterMETy - electronPy - muonPy - electronYPUCones - muonYPUCones;
  
        cluster_METx(*ei) = METx;
        cluster_METy(*ei) = METy;

        return StatusCode::SUCCESS;
    }
}
