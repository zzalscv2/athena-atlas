/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "TopSystematicObjectMaker/MissingETObjectCollectionMaker.h"
#include "TopConfiguration/TopConfig.h"
#include "TopConfiguration/TreeFilter.h"
#include "TopEvent/EventTools.h"
#include "TopEvent/SystematicEventContainer.h"


#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETAuxContainer.h"
#include "METUtilities/METHelpers.h"
#include "xAODCore/ShallowCopy.h"
#include "PATInterfaces/SystematicsUtil.h"

namespace top {
  MissingETObjectCollectionMaker::MissingETObjectCollectionMaker(const std::string& name) :
    asg::AsgTool(name),
    m_config(nullptr),

    m_specifiedSystematics(),
    m_recommendedSystematics(),

    m_MET_core("setMe"),
    m_met_maker("met::METMaker"),
    m_met_systematics("met::METSystematicsTool"),
    m_metSignif("metSignificance"){
    declareProperty("config", m_config);
    declareProperty("METMaker", m_met_maker);
    declareProperty("met_systematics", m_met_systematics);
    declareProperty("metSignificance", m_metSignif);
  }

  StatusCode MissingETObjectCollectionMaker::initialize() {
    ATH_MSG_INFO(" top::MissingETObjectCollectionMaker initialize");

    top::check(m_met_maker.retrieve(), "Failed to retrieve met maker tool");
    top::check(m_met_systematics.retrieve(), "Failed to retrieve met systematic tool");
    if (m_config->METSignificance()) top::check(m_metSignif.retrieve(), "Failed to retrieve metSignificance!");

    std::string jet_collection = m_config->sgKeyJets();
    jet_collection.erase(jet_collection.length() - 4); //erase "Jets" from jet collection name

    m_MET_core = "MET_Core_" + jet_collection;

    ///-- Set Systematics Information --///
    const std:: string& syststr = m_config->systematics();
    std::set<std::string> syst;

    if (!m_config->isSystNominal(syststr) && !m_config->isSystAll(syststr)) {
      bool ok = m_config->getSystematicsList(syststr, syst);
      if (!ok) {
        ATH_MSG_ERROR(" top::MissingETObjectCollectionMaker could not determine systematic list");
        return StatusCode::FAILURE;
      }
      //here the idea is that if the user specifies AllXXX, we leave syst as an empty string, so that all recommended CP
      // systematics are then used
      if (m_config->contains(syst, "AllMET")) {
        syst.clear();
      }
    }

    specifiedSystematics(syst);

    m_config->systematicsMET(specifiedSystematics());

    return StatusCode::SUCCESS;
  }

  StatusCode MissingETObjectCollectionMaker::recalculateMET(const bool executeNominal) {
    // met core contains the soft terms we need to add to our met calculation
    const xAOD::MissingETContainer* xaod_met_core(nullptr);

    top::check(evtStore()->retrieve(xaod_met_core, m_MET_core), "Failed to retrieve MET Core");

    if (m_config->doTightEvents()) {
      // All the tight systematic events
      const xAOD::SystematicEventContainer* systEvents(nullptr);
      if (executeNominal) {
        top::check(evtStore()->retrieve(systEvents,
                                        m_config->sgKeyTopSystematicEvents() + "Nominal"),
                   "Failed to retrieve nominal TopEvents");
      } else {
        top::check(evtStore()->retrieve(systEvents,
                                        m_config->sgKeyTopSystematicEvents()), "Failed to retrieve TopEvents");
      }
      for (auto x : *systEvents) {
        ///-- if executeNominal, skip other systematics (and vice-versa) --///
        if (!executeNominal && m_config->isSystNominal(m_config->systematicName(x->hashValue()))) continue;
        if (executeNominal && !m_config->isSystNominal(m_config->systematicName(x->hashValue()))) continue;
        top::check(recalculateEventMET(x, xaod_met_core), "Failed to recalculate MET for event");
        if(m_config->writeMETBuiltWithLooseObjects()) top::check(recalculateEventMET(x, xaod_met_core, true, "WithLooseObjects"), "Failed to recalculate MET using loose leptons for event");
      }
    } // tight events
    if (m_config->doLooseEvents()) {
      // All the loose systematic events
      const xAOD::SystematicEventContainer* systEventsLoose(nullptr);
      if (executeNominal) {
        top::check(evtStore()->retrieve(systEventsLoose,
                                        m_config->sgKeyTopSystematicEventsLoose() + "Nominal"),
                   "Failed to retrieve nominal TopEventsLoose");
      } else {
        top::check(evtStore()->retrieve(systEventsLoose,
                                        m_config->sgKeyTopSystematicEventsLoose()),
                   "Failed to retrieve TopEventsLoose");
      }

      for (auto x : *systEventsLoose) {
        ///-- if executeNominal, skip other systematics (and vice-versa) --///
        if (!executeNominal && m_config->isSystNominal(m_config->systematicName(x->hashValue()))) continue;
        if (executeNominal && !m_config->isSystNominal(m_config->systematicName(x->hashValue()))) continue;
        top::check(recalculateEventMET(x, xaod_met_core), "Failed to recalculate MET for loose event");
        if(m_config->writeMETBuiltWithLooseObjects()) top::check(recalculateEventMET(x, xaod_met_core, true, "WithLooseObjects"), "Failed to recalculate MET using loose leptons for event");
      }
    } // Loose events

    return StatusCode::SUCCESS;
  }

  StatusCode MissingETObjectCollectionMaker::recalculateEventMET(const xAOD::SystematicEvent* event,
                                                                 const xAOD::MissingETContainer* xaod_met_core,
                                                                 const bool forceUseLooseObjects,
                                                                 const std::string& outputContainerSuffix) {
    // decoration for objects that pass pre OR selection
    std::string passPreORSelection = "passPreORSelection";
    std::string jet_collection = m_config->sgKeyJets();
    jet_collection.erase(jet_collection.length() - 4); //erase "Jets" from jet collection name

    const bool is_loose_event = (event->isLooseEvent() == 1 ? true : false);

    // default behaviour for top analysis - use the "Tight" definitions
    bool looseLeptonOR(is_loose_event);
    if (m_config->doOverlapRemovalOnLooseLeptonDef() || (is_loose_event && m_config->useLooseObjectsInMETInLooseTree()) || (!is_loose_event && m_config->useLooseObjectsInMETInNominalTree()) || forceUseLooseObjects) {
      looseLeptonOR = true;
      // change decoration we check
      passPreORSelection = "passPreORSelectionLoose";
    }

    const std::size_t hash = event->hashValue();

    // Make a new MET container that we are going to use
    xAOD::MissingETContainer* new_met_container = new xAOD::MissingETContainer();
    xAOD::MissingETAuxContainer* new_met_aux_container = new xAOD::MissingETAuxContainer();
    new_met_container->setStore(new_met_aux_container);

    const xAOD::MissingETAssociationMap* metMap(nullptr);
    const std::string metAssocKey = "METAssoc_" + jet_collection;
    top::check(evtStore()->retrieve(metMap, metAssocKey), "Failed to retrive MET map");
    xAOD::MissingETAssociationHelper metHelper(metMap);

    // Reset all the met map associations
    metHelper.resetObjSelectionFlags();

    // Apparently we need to add objects to the METMaker in a specific order
    // Start by retrieving jet container, we need it to perform e-jet ghost-matching
    // 1. Electrons
    const xAOD::ElectronContainer* xaod_el(nullptr);
    if (m_config->useElectrons()) {
      // Get all calibrated electrons
      top::check(evtStore()->retrieve(xaod_el, m_config->sgKeyElectrons(hash)), "Failed to retrieve Electrons");

      // Add those that pass pre-overlap removal selection to met_electrons
      ConstDataVector<xAOD::ElectronContainer> met_electrons(SG::VIEW_ELEMENTS);
      for (const auto *el: *xaod_el)
        if (el->isAvailable<char>(passPreORSelection)
            && el->auxdata<char>(passPreORSelection) == 1) met_electrons.push_back(el);

      // Recalculate electron MET term using electrons that pass overlap removal
      top::check(m_met_maker->rebuildMET("RefEle",
                                         xAOD::Type::Electron,
                                         new_met_container,
                                         met_electrons.asDataVector(),
                                         metHelper),
                                         "Failed to rebuild electron MET term");
    }

    // 2. Photons
    if (m_config->usePhotons()) {
      // Get calibrated photons
      const xAOD::PhotonContainer* xaod_photon(nullptr);
      top::check(evtStore()->retrieve(xaod_photon, m_config->sgKeyPhotons(hash)), "Failed to retrieve Photons");

      // Add those that pass pre-overlap removal to met_photons
      ConstDataVector<xAOD::PhotonContainer> met_photons(SG::VIEW_ELEMENTS);
      for (const auto *photon: *xaod_photon)
        if (photon->isAvailable<char>(passPreORSelection)
            && photon->auxdata<char>(passPreORSelection) == 1) met_photons.push_back(photon);

      top::check(m_met_maker->rebuildMET("RefPhoton",
                                         xAOD::Type::Photon,
                                         new_met_container,
                                         met_photons.asDataVector(),
                                         metHelper),
                                         "Failed to rebuild photon MET term");
    }

    // 3. Taus
    if (m_config->useTaus()) {
      // Get calibrated taus
      const xAOD::TauJetContainer* xaod_tau(nullptr);
      top::check(evtStore()->retrieve(xaod_tau, m_config->sgKeyTaus(hash)), "Failed to retrieve Taus");

      // Add those that pass pre-overlap removal to met_taus
      ConstDataVector<xAOD::TauJetContainer> met_taus(SG::VIEW_ELEMENTS);
      for (const auto *tau: *xaod_tau)
        if (tau->isAvailable<char>(passPreORSelection)
            && tau->auxdata<char>(passPreORSelection) == 1) met_taus.push_back(tau);

      top::check(m_met_maker->rebuildMET("RefTau",
                                         xAOD::Type::Tau,
                                         new_met_container,
                                         met_taus.asDataVector(),
                                         metHelper),
                                         "Failed to rebuild tau MET term");
    }

    xAOD::JetContainer* xaod_jet(nullptr);
    top::check(evtStore()->retrieve(xaod_jet, m_config->sgKeyJets(hash, looseLeptonOR)), "Failed to retrieve Jets");
    // 4. Muons
    const xAOD::MuonContainer* xaod_mu(nullptr); // Need these for ghost matching
    if (m_config->useMuons()) {
      // Get calibrated muons
      top::check(evtStore()->retrieve(xaod_mu, m_config->sgKeyMuons(hash)), "Failed to retrieve Muons");
      if (not xaod_mu){
        return StatusCode::FAILURE;
      }
      // Add those that pass pre-overlap removal to met_muons
      ConstDataVector<xAOD::MuonContainer> met_muons(SG::VIEW_ELEMENTS);
      for (const auto *mu: *xaod_mu)
        if (mu->isAvailable<char>(passPreORSelection)
            && mu->auxdata<char>(passPreORSelection) == 1) met_muons.push_back(mu);

      // Recalculate muon MET term using muons that pass overlap removal
      top::check(m_met_maker->rebuildMET("RefMuon",
                                         xAOD::Type::Muon,
                                         new_met_container,
                                         met_muons.asDataVector(),
                                         metHelper),
                                         "Failed to rebuild muon MET term");

      // Muon-jet ghost association
      // performed after muon handing to METUtility, but before jets
      if (xaod_jet) {
        met::addGhostMuonsToJets(*xaod_mu, *xaod_jet);
      }
    } else {
      ConstDataVector<xAOD::MuonContainer> met_muons(SG::VIEW_ELEMENTS);
      top::check(m_met_maker->rebuildMET("RefMuon",
                                         xAOD::Type::Muon,
                                         new_met_container,
                                         met_muons.asDataVector(),
                                         metHelper),
                                         "Failed to rebuild muon MET term");
    }


    // 5. Jets
    // We do not perfom any selection on jets here. Give them all to the METMaker, it does its own thing
    top::check(m_met_maker->rebuildJetMET("RefJet",
                                          "SoftClus",
                                          "PVSoftTrk",
                                          new_met_container,
                                          xaod_jet,
                                          xaod_met_core,
                                          metHelper,
                                          m_config->doJVTinMET()),
                                          "Failed to rebuild jet MET term");



    for (const auto & systematic : m_specifiedSystematics) {
      if (systematic.hash() == event->hashValue() && systematic.hash() != m_config->nominalHashValue()) {
        ///-- Tell tool which systematic to use --///
        top::check(m_met_systematics->applySystematicVariation(systematic), "Failed to applySystematicVariation");

        // code lifted and modified from:
        // https://svnweb.cern.ch/trac/atlasoff/browser/Reconstruction/MET/METUtilities/tags/METUtilities-00-01-43/util/example_METMaker_METSystematicsTool.cxx

        //get the soft cluster term, and applyCorrection
        xAOD::MissingET* softClusMet = (*new_met_container)["SoftClus"];
        if (softClusMet != nullptr) { //check we retrieved the clust term
          m_met_systematics->setRandomSeed(static_cast<int>(1e6*softClusMet->phi()));
          top::check(m_met_systematics->applyCorrection(*softClusMet, metHelper), "Failed to applyCorrection");
        }

        xAOD::MissingET* softTrkMet = (*new_met_container)["PVSoftTrk"];
        if (softTrkMet != nullptr) { //check we retrieved the soft trk
          m_met_systematics->setRandomSeed(static_cast<int>(1e6*softTrkMet->phi()));
          top::check(m_met_systematics->applyCorrection(*softTrkMet, metHelper), "Failed to applyCorrection");
        }
      }
    }

    // This will sum up all the contributions we've made so far e.g.
    // Total MET = RefEle + RefPhoton + RefTau + RefMuon + RefJet + PVSoftTrk (Track Soft Term)
    top::check(met::buildMETSum("FinalTrk",
                                new_met_container,
                                static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::Track)),
               "Failed to rebuild Final Track MET");
    if(m_config->METSignificance()){
      const xAOD::EventInfo* eventInfo(nullptr);
      top::check(evtStore()->retrieve(eventInfo, m_config->sgKeyEventInfo()), "failded to retrieve event info obj");
      // met significance
      top::check(m_metSignif->varianceMET(new_met_container, eventInfo->averageInteractionsPerCrossing(), "RefJet", "PVSoftTrk","FinalTrk"), "Failed to execute MetSignificance::varianceMET!");

      for (auto mets : *new_met_container){
        mets->auxdecor<float>("metSigET") = m_metSignif->GetMETOverSqrtSumET();
        mets->auxdecor<float>("metSigHT") = m_metSignif->GetMETOverSqrtHT();
        mets->auxdecor<float>("metSig") = m_metSignif->GetSignificance();
        mets->auxdecor<float>("metSigRho") = m_metSignif->GetRho();
        mets->auxdecor<float>("metSigVarL") = m_metSignif->GetVarL();
        mets->auxdecor<float>("metSigVarT") = m_metSignif->GetVarT();
      }
    }

    /************************************************************
       // We are only going to build a single MET Sum now for clarity. The final argument is the soft term we
       // want to use. The recommended one is the track soft term (TST) as above.

       top::check(mer::buildMETSum("FinalClus", new_met_container, MissingETBase::Source::EMTopo),
              "Failed to rebuild Final Cluster MET");
    ************************************************************/

    // Save corrected xAOD Container to StoreGate / TStore
    std::string outputSGKey = m_config->sgKeyMissingEt(hash);
    if (is_loose_event) outputSGKey = m_config->sgKeyMissingEtLoose(hash);

    outputSGKey+=outputContainerSuffix;
    const std::string outputSGKeyAux = outputSGKey + "Aux.";

    StatusCode save = evtStore()->tds()->record(new_met_container, outputSGKey);
    StatusCode saveAux = evtStore()->tds()->record(new_met_aux_container, outputSGKeyAux);

    if (save.isFailure() || saveAux.isFailure()) {
      return StatusCode::FAILURE;
    }


    return StatusCode::SUCCESS;
  }

  void MissingETObjectCollectionMaker::specifiedSystematics(const std::set<std::string>& specifiedSystematics) {
    ///-- Get the recommended systematics from the tool, in std::vector format --///
    const std::vector<CP::SystematicSet> systList = CP::make_systematics_vector(
      m_met_systematics->recommendedSystematics());

    for (auto s : systList) {

      if(!m_config->getTreeFilter()->filterTree(s.name())) continue; // Applying tree filter

      ///-- Recommendation is to use soft track terms and not soft calo terms --///
      ///-- Soft calo systematics are irrelevant, let's ignore them --///
      if (s.name().find("SoftCalo") == std::string::npos) {
        m_recommendedSystematics.push_back(s);
        if (s.name() == "") {
          m_specifiedSystematics.push_back(s);
        }

        ///-- MC only --///
        if (m_config->isMC()) {
          ///-- Are we only doing Nominal? Did the user specify specific systematics to use? --///
          if (!m_config->isSystNominal(m_config->systematics())) {
            if (specifiedSystematics.size() == 0) {
              m_specifiedSystematics.push_back(s);
            }
            if (specifiedSystematics.size() > 0) {
              for (auto i : specifiedSystematics) {
                TreeFilter filter(i);
                if (!filter.filterTree(s.name())) {
                  m_specifiedSystematics.push_back(s);
                }
              }
            }
          }
        }
      }
    }
    m_recommendedSystematics.sort();
    m_recommendedSystematics.unique();
    m_specifiedSystematics.sort();
    m_specifiedSystematics.unique();
  }
}
