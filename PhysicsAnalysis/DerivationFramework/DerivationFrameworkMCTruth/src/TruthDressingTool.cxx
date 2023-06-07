/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthDressingTool.cxx
// Author: Kevin Finelli (kevin.finelli@cern.ch)
// Create dressed (i.e. including FSR photons) 4-vectors of truth objects

#include "DerivationFrameworkMCTruth/TruthDressingTool.h"
#include "MCTruthClassifier/MCTruthClassifier.h"
#include "xAODTruth/TruthEventContainer.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
namespace {
  static const SG::AuxElement::ConstAccessor<unsigned int> acc_origin("Classification");    
}
// Constructor
DerivationFramework::TruthDressingTool::TruthDressingTool(const std::string& t,
        const std::string& n,
        const IInterface* p )
   : AthAlgTool(t,n,p)
{
    declareInterface<DerivationFramework::IAugmentationTool>(this);
}

// Destructor
DerivationFramework::TruthDressingTool::~TruthDressingTool() {
}

// Athena initialize and finalize
StatusCode DerivationFramework::TruthDressingTool::initialize()
{
    // Initialise handle keys
    ATH_CHECK(m_particlesKey.initialize());
    ATH_CHECK(m_dressParticlesKey.initialize());
    m_decorator_eKey = m_dressParticlesKey.key() + ".e_dressed";
    ATH_CHECK(m_decorator_eKey.initialize());
    m_decorator_ptKey = m_dressParticlesKey.key() + ".pt_dressed";
    ATH_CHECK(m_decorator_ptKey.initialize());
    m_decorator_etaKey= m_dressParticlesKey.key() + ".eta_dressed";
    ATH_CHECK(m_decorator_etaKey.initialize());
    m_decorator_phiKey= m_dressParticlesKey.key() + ".phi_dressed";
    ATH_CHECK(m_decorator_phiKey.initialize());
    m_decorator_pt_visKey = m_dressParticlesKey.key() + ".pt_vis_dressed";
    ATH_CHECK(m_decorator_pt_visKey.initialize());
    m_decorator_eta_visKey= m_dressParticlesKey.key() + ".eta_vis_dressed";
    ATH_CHECK(m_decorator_eta_visKey.initialize());
    m_decorator_phi_visKey= m_dressParticlesKey.key() + ".phi_vis_dressed";
    ATH_CHECK(m_decorator_phi_visKey.initialize());
    m_decorator_m_visKey= m_dressParticlesKey.key() + ".m_vis_dressed";
    ATH_CHECK(m_decorator_m_visKey.initialize());
    m_decorator_nphotonKey = m_dressParticlesKey.key() + ".nPhotons_dressed";
    ATH_CHECK(m_decorator_nphotonKey.initialize());
    if (!m_decorationName.empty()) {m_decorationKey = m_particlesKey.key()+"."+m_decorationName;}
    else {m_decorationKey = m_particlesKey.key()+".unusedPhotonDecoration";} 
    ATH_CHECK(m_decorationKey.initialize());
    m_truthClassKey = m_dressParticlesKey.key() + "." + SG::AuxTypeRegistry::instance().getName(acc_origin.auxid());
    ATH_CHECK(m_truthClassKey.initialize());
    return StatusCode::SUCCESS;
}

// Function to do dressing, implements interface in IAugmentationTool
StatusCode DerivationFramework::TruthDressingTool::addBranches() const
{
    // Get the event context
    const EventContext& ctx = Gaudi::Hive::currentContext();

    // Retrieve the truth collections
    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles(m_particlesKey,ctx);
    if (!truthParticles.isValid()) {
        ATH_MSG_ERROR("Couldn't retrieve TruthParticle collection with name " << m_particlesKey);
        return StatusCode::FAILURE;
    }
 
    SG::ReadHandle<xAOD::TruthParticleContainer> dressTruthParticles(m_dressParticlesKey,ctx);
    if (!dressTruthParticles.isValid()) {
        ATH_MSG_ERROR("Couldn't retrieve TruthParticle collection with name " << m_dressParticlesKey);
        return StatusCode::FAILURE;
    } 

    // Decorators
    SG::WriteDecorHandle< xAOD::TruthParticleContainer,float > decorator_e(m_decorator_eKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer,float > decorator_pt(m_decorator_ptKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer,float > decorator_eta(m_decorator_etaKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer,float > decorator_phi(m_decorator_phiKey, ctx);
    // for truth taus, use 'vis' in the decoration name to avoid prompt/visible tau momentum ambiguity
    // use (pt,eta,phi,m) for taus, for consistency with other TauAnalysisTools decorations
    SG::WriteDecorHandle< xAOD::TruthParticleContainer,float > decorator_pt_vis(m_decorator_pt_visKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer,float > decorator_eta_vis(m_decorator_eta_visKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer,float > decorator_phi_vis(m_decorator_phi_visKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer,float > decorator_m_vis(m_decorator_m_visKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer,int > decorator_nphoton(m_decorator_nphotonKey, ctx);
    // One for the photons as well
    SG::WriteDecorHandle< xAOD::TruthParticleContainer, char > dressDec (m_decorationKey, ctx);
    // If we want to decorate, then we need to decorate everything with false to begin with
    if (!m_decorationKey.key().empty()){
      if (!dressDec.isAvailable()) {
        for (const auto * particle : *truthParticles){
          dressDec(*particle);
        }
      } // Loop over particles
    } // We are using the decoration

    //get struct of helper functions
    DerivationFramework::DecayGraphHelper decayHelper;

    std::vector<const xAOD::TruthParticle*> listOfParticlesToDress;
    std::vector<xAOD::TruthParticle::FourMom_t> listOfDressedParticles;
    std::vector<int> dressedParticlesNPhot;

    if(m_listOfPIDs.size()==1 && abs(m_listOfPIDs[0])==15) {
      // when dressing only truth taus, it is assumed that the truth tau container has
      // been built beforehand and is used as input
      for (auto *pItr : *dressTruthParticles) {
        listOfParticlesToDress.push_back(pItr);
      }
    } 
    else {
      // non-prompt particles are still included here to ensure all particles
      // will get the decoration; however further down only the prompt particles
      // are actually dressed depending on the value of m_useLeptonsFromHadrons
      decayHelper.constructListOfFinalParticles(dressTruthParticles.ptr(), listOfParticlesToDress, m_listOfPIDs, true);
    }

    //initialize list of dressed particles
    for (const auto& part : listOfParticlesToDress) {
      listOfDressedParticles.push_back(part->p4());
      dressedParticlesNPhot.push_back(0);
    }

    //fill the photon list
    std::vector<const xAOD::TruthParticle*>  photonsFSRList;
    std::vector<int> photonPID{22};
    const bool pass = decayHelper.constructListOfFinalParticles(truthParticles.ptr(), photonsFSRList, 
                                                                photonPID, m_usePhotonsFromHadrons);
    if (!pass) {
      ATH_MSG_WARNING("Cannot construct the list of final state particles "<<m_truthClassKey.fullKey());
    }

    // Do dR-based photon dressing (default)
    if (!m_useAntiKt){
      //loop over photons, uniquely associate each to nearest bare particle
      for (const auto& phot : photonsFSRList ) {
        double dRmin = m_coneSize;
        int idx = -1;
  
        for (size_t i = 0; i < listOfParticlesToDress.size(); ++i) {
          if (!m_useLeptonsFromHadrons) {
            if (!acc_origin.isAvailable(*listOfParticlesToDress[i])) {
              ATH_MSG_WARNING("MCTruthClassifier "<<m_truthClassKey.fullKey() <<" not available, cannot apply notFromHadron veto!");
            }
            unsigned int result = acc_origin(*listOfParticlesToDress[i]);
            const bool isPrompt = MCTruthClassifier::isPrompt(result, true);
            if (!isPrompt)  continue;
          }
          xAOD::TruthParticle::FourMom_t bare_part;
          if(listOfParticlesToDress[i]->isTau()) {
  
            if( !listOfParticlesToDress[i]->isAvailable<double>("pt_vis") ||
                !listOfParticlesToDress[i]->isAvailable<double>("eta_vis") ||
                !listOfParticlesToDress[i]->isAvailable<double>("phi_vis") ||
                !listOfParticlesToDress[i]->isAvailable<double>("m_vis")) {
              ATH_MSG_ERROR("Visible momentum not available for truth taus, cannot perform dressing!");
              return StatusCode::FAILURE;
            }
  
            bare_part.SetPtEtaPhiM(listOfParticlesToDress[i]->auxdata<double>("pt_vis"),
                                   listOfParticlesToDress[i]->auxdata<double>("eta_vis"),
                                   listOfParticlesToDress[i]->auxdata<double>("phi_vis"),
                                   listOfParticlesToDress[i]->auxdata<double>("m_vis"));
          }
          else {
            bare_part = listOfParticlesToDress[i]->p4();
          }
  
          double dR = bare_part.DeltaR(phot->p4());
          if (dR < dRmin) {
            dRmin = dR;
            idx = i;
          }
        }
  
        if(idx > -1) {
          listOfDressedParticles[idx] += phot->p4();
          dressedParticlesNPhot[idx]++;
          if (!m_decorationName.empty()){
            dressDec(*phot) = 1;
          }
        }
      }
  
      //loop over particles and add decorators
      //for (const auto& part : listOfDressedParticles) {
      for (size_t i = 0; i < listOfParticlesToDress.size(); ++i) {
          const xAOD::TruthParticle* part = listOfParticlesToDress[i];
          xAOD::TruthParticle::FourMom_t& dressedVec = listOfDressedParticles[i];
  
        if(part->isTau()) {
          decorator_pt_vis(*part)      = dressedVec.Pt();
          decorator_eta_vis(*part)     = dressedVec.Eta();
          decorator_phi_vis(*part)     = dressedVec.Phi();
          decorator_m_vis(*part)       = dressedVec.M();
        }
        else {
          decorator_e(*part)       = dressedVec.E();
          decorator_pt(*part)      = dressedVec.Pt();
          decorator_eta(*part)     = dressedVec.Eta();
          decorator_phi(*part)     = dressedVec.Phi();
        }
        decorator_nphoton(*part) = dressedParticlesNPhot[i];
      }
    } // end of the dR matching part

    //build the anti-kt jet list
    if (m_useAntiKt) {
      std::vector<fastjet::PseudoJet> sorted_jets;
      std::vector<fastjet::PseudoJet> fj_particles;
      for (const auto& part : listOfParticlesToDress) {

        if(part->isTau()) {
          if(!part->isAvailable<double>("pt_vis") || !part->isAvailable<double>("eta_vis")
              || !part->isAvailable<double>("phi_vis") || !part->isAvailable<double>("m_vis")) {
            ATH_MSG_ERROR("Visible momentum not available for truth taus, cannot perform dressing!");
            return StatusCode::FAILURE;
          }

          TLorentzVector tauvis;
          tauvis.SetPtEtaPhiM(part->auxdata<double>("pt_vis"), 
                              part->auxdata<double>("eta_vis"), 
                              part->auxdata<double>("phi_vis"), 
                              part->auxdata<double>("m_vis"));
          fj_particles.emplace_back(tauvis.Px(), tauvis.Py(), tauvis.Pz(), tauvis.E());
        }
        else {
          fj_particles.emplace_back(part->px(), part->py(), part->pz(), part->e());
        }

        fj_particles.back().set_user_index(part->barcode());
      }
      for (const auto& part : photonsFSRList) {
        fj_particles.emplace_back(part->px(), part->py(), part->pz(), part->e());
        fj_particles.back().set_user_index(part->barcode());
      }

      //run the clustering
      fastjet::JetAlgorithm alg=fastjet::antikt_algorithm;
      const fastjet::JetDefinition jet_def(alg, m_coneSize);
      fastjet::ClusterSequence cseq(fj_particles, jet_def);
      sorted_jets = sorted_by_pt(cseq.inclusive_jets(0));
      //associate clustered jets back to bare particles
      std::vector<int> photon_barcodes(50);
      photon_barcodes.clear();
      for (const auto& part : listOfParticlesToDress) {
        //loop over fastjet pseudojets and associate one with this particle by barcode
        bool found=false;
        auto pjItr=sorted_jets.begin();
        while(!found && pjItr!=sorted_jets.end()) {
          std::vector<fastjet::PseudoJet> constituents = pjItr->constituents();
          for(const auto& constit : constituents) {
            if (part->barcode()==constit.user_index()) {

              // shall we count the number of photons among the pseudojet constituents 
              // to decorate leptons with the number of dressing photons found by the anti-kt algorithm?

              if(part->isTau()) {
                decorator_pt_vis(*part)      = pjItr->pt();
                decorator_eta_vis(*part)     = pjItr->pseudorapidity();
                decorator_phi_vis(*part)     = pjItr->phi_std(); //returns phi in [-pi,pi]
                decorator_m_vis(*part)       = pjItr->m();
              }
              else {
                decorator_e(*part)       = pjItr->e();
                decorator_pt(*part)      = pjItr->pt();
                decorator_eta(*part)     = pjItr->pseudorapidity();
                decorator_phi(*part)     = pjItr->phi_std(); //returns phi in [-pi,pi]
              }
              found=true;
              break;
            } // Found the matching barcode
          } // Loop over the jet constituents
          if (found){
            for(const auto& constit : constituents) {
              photon_barcodes.push_back(constit.user_index());
            } // Loop over the constituents
          } // Found one of the key leptons in this jet
          ++pjItr;
        }
        if (!found) {
          if(part->isTau()) {
            decorator_pt_vis(*part)      = 0.;
            decorator_eta_vis(*part)     = 0.;
            decorator_phi_vis(*part)     = 0.;
            decorator_m_vis(*part)       = 0.;
          }
          else {
            decorator_e(*part)       = 0;
            decorator_pt(*part)      = 0;
            decorator_eta(*part)     = 0;
            decorator_phi(*part)     = 0;
          }
          ATH_MSG_WARNING("Bare particle not found in constituents ");
        }
      }
      // Check if we wanted to decorate photons used for dressing
      if (!m_decorationName.empty()){
        //loop over photons, uniquely associate each to nearest bare particle
        for (const auto& phot : photonsFSRList ) {
          bool found=std::find(photon_barcodes.begin(),photon_barcodes.end(),phot->barcode())!=photon_barcodes.end();
          if (found){
            dressDec(*phot) = 1;
          }
        } // End of loop over photons
      } // End of decoration of photons used in dressing
    } // End of anti-kT dressing

    return StatusCode::SUCCESS;
}
