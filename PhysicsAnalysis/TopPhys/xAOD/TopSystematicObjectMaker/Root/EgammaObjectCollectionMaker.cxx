/*
   Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
 */

// $Id: EgammaObjectCollectionMaker.cxx 811374 2017-10-24 13:04:52Z iconnell $
#include "TopSystematicObjectMaker/EgammaObjectCollectionMaker.h"
#include "TopConfiguration/TopConfig.h"
#include "TopConfiguration/TreeFilter.h"
#include "TopEvent/EventTools.h"

#include "AthContainers/AuxElement.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/PhotonAuxContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODBase/IParticleHelpers.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "PATInterfaces/SystematicsUtil.h"

#include <list>

namespace top {
  EgammaObjectCollectionMaker::EgammaObjectCollectionMaker(const std::string& name) :
    asg::AsgTool(name),
    m_config(nullptr),

    m_specifiedSystematicsPhotons(),
    m_specifiedSystematicsElectrons(),
    m_specifiedSystematicsFwdElectrons(),
    m_recommendedSystematicsPhotons(),
    m_recommendedSystematicsElectrons(),
    m_recommendedSystematicsFwdElectrons(),

    m_calibrationTool("CP::EgammaCalibrationAndSmearingTool"),
    m_photonFudgeTool("PhotonFudgeTool"),

    m_isolationTool_Tight_VarRad("CP::IsolationTool_Tight_VarRad"),
    m_isolationTool_Loose_VarRad("CP::IsolationTool_Loose_VarRad"),
    m_isolationTool_TightTrackOnly_VarRad("CP::IsolationTool_TightTrackOnly_VarRad"),
    m_isolationTool_LooseTrackOnly("CP::IsolationTool_LooseTrackOnly"),
    m_isolationTool_Loose("CP::IsolationTool_Loose"),
    m_isolationTool_PflowLoose("CP::IsolationTool_PflowLoose"),
    m_isolationTool_Tight("CP::IsolationTool_Tight"),
    m_isolationTool_PflowTight("CP::IsolationTool_PflowTight"),
    m_isolationTool_Gradient("CP::IsolationTool_Gradient"),
    m_isolationTool_GradientLoose("CP::IsolationTool_GradientLoose"),
    m_isolationTool_FixedCutTight("CP::IsolationTool_FixedCutTight"),
    m_isolationTool_FixedCutTightTrackOnly("CP::IsolationTool_FixedCutTightTrackOnly"),
    m_isolationTool_TightTrackOnly("CP::IsolationTool_TightTrackOnly"),
    m_isolationTool_TightTrackOnly_FixedRad("CP::IsolationTool_TightTrackOnly_FixedRad"),
    m_isolationTool_FixedCutTightCaloOnly("CP::IsolationTool_FixedCutTightCaloOnly"),
    m_isolationTool_TightCaloOnly("CP::IsolationTool_TightCaloOnly"),
    m_isolationTool_FixedCutLoose("CP::IsolationTool_FixedCutLoose"),
    m_isolationTool_FixedCutHighPtCaloOnly("CP::IsolationTool_FixedCutHighPtCaloOnly"),
    m_isolationTool_FCHighPtCaloOnly("CP::IsolationTool_FCHighPtCaloOnly"),
    m_isolationTool_HighPtCaloOnly("CP::IsolationTool_HighPtCaloOnly"),
    m_isolationTool_FCTight("CP::IsolationTool_FCTight"),
    m_isolationTool_FCLoose("CP::IsolationTool_FCLoose"),
    m_isolationTool_PLVTight("CP::IsolationTool_PLVTight"),
    m_isolationTool_PLVLoose("CP::IsolationTool_PLVLoose"),
    m_isolationTool_LowPtPLV("CP::IsolationTool_LowPtPLV"),
    m_isolationTool_PLImprovedTight("CP::IsolationTool_PLImprovedTight"),
    m_isolationTool_PLImprovedVeryTight("CP::IsolationTool_PLImprovedVeryTight"),
    m_isolationCorr("CP::IsolationCorrectionTool"),
    m_IFFTruthTool("TruthClassificationTool"){
    declareProperty("config", m_config);

    declareProperty("EgammaCalibrationAndSmearingTool", m_calibrationTool);

    declareProperty("IsolationTool_Tight_VarRad", m_isolationTool_Tight_VarRad);
    declareProperty("IsolationTool_Loose_VarRad", m_isolationTool_Loose_VarRad);
    declareProperty("IsolationTool_TightTrackOnly_VarRad", m_isolationTool_TightTrackOnly_VarRad);
    declareProperty("IsolationTool_LooseTrackOnly", m_isolationTool_LooseTrackOnly);
    declareProperty("IsolationTool_Loose", m_isolationTool_Loose);
    declareProperty("IsolationTool_PflowLoose", m_isolationTool_PflowLoose);
    declareProperty("IsolationTool_Tight", m_isolationTool_Tight);
    declareProperty("IsolationTool_PflowTight", m_isolationTool_PflowTight);
    declareProperty("IsolationTool_Gradient", m_isolationTool_Gradient);
    declareProperty("IsolationTool_GradientLoose", m_isolationTool_GradientLoose);
    declareProperty("IsolationTool_FixedCutTight", m_isolationTool_FixedCutTight);
    declareProperty("IsolationTool_FixedCutTightTrackOnly", m_isolationTool_FixedCutTightTrackOnly);
    declareProperty("IsolationTool_TightTrackOnly", m_isolationTool_TightTrackOnly);
    declareProperty("IsolationTool_TightTrackOnly_FixedRad", m_isolationTool_TightTrackOnly_FixedRad);
    declareProperty("IsolationTool_FixedCutTightCaloOnly", m_isolationTool_FixedCutTightCaloOnly);
    declareProperty("IsolationTool_TightCaloOnly", m_isolationTool_TightCaloOnly);
    declareProperty("IsolationTool_FixedCutLoose", m_isolationTool_FixedCutLoose);
    declareProperty("IsolationTool_FixedCutHighPtCaloOnly", m_isolationTool_FixedCutHighPtCaloOnly);
    declareProperty("IsolationTool_FCHighPtCaloOnly", m_isolationTool_FCHighPtCaloOnly);
    declareProperty("IsolationTool_HighPtCaloOnly", m_isolationTool_HighPtCaloOnly);
    declareProperty("IsolationTool_FCTight", m_isolationTool_FCTight);
    declareProperty("IsolationTool_FCLoose", m_isolationTool_FCLoose);
    declareProperty("IsolationTool_PLVTight", m_isolationTool_PLVTight);
    declareProperty("IsolationTool_PLVLoose", m_isolationTool_PLVLoose);
    declareProperty("IsolationTool_LowPtPLV", m_isolationTool_LowPtPLV);
    declareProperty("IsolationTool_PLImprovedTight", m_isolationTool_PLImprovedTight);
    declareProperty("IsolationTool_PLImprovedVeryTight", m_isolationTool_PLImprovedVeryTight);
    declareProperty("IsolationCorrectionTool", m_isolationCorr);
    declareProperty("IFFTruthClassificationTool", m_IFFTruthTool);
  }

  StatusCode EgammaObjectCollectionMaker::initialize() {
    ATH_MSG_INFO(" top::EgammaObjectCollectionMaker initialize");

    top::check(m_calibrationTool.retrieve(), "Failed to retrieve egamma calibration tool");

    // These flags were for early R21 when we were asked not to calibrate egamma objects
    calibrateElectrons = true;
    calibrateFwdElectrons = true;
    calibratePhotons = true;

    if (m_config->usePhotons()) {
      top::check(m_isolationTool_FixedCutTight.retrieve(),
                 "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_FixedCutTightCaloOnly.retrieve(),
                 "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_FixedCutLoose.retrieve(),
                 "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_Tight.retrieve(),
                 "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_TightCaloOnly.retrieve(),
                 "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_Loose.retrieve(),
                 "Failed to retrieve Isolation Tool");
      top::check(m_photonFudgeTool.retrieve(),
                 "Failed to retrieve photon shower shape fudge tool");
    }

    if (m_config->useElectrons()) {
      top::check(m_isolationTool_Tight_VarRad.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_Loose_VarRad.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_TightTrackOnly_VarRad.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_Gradient.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_FCTight.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_FCLoose.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_FCHighPtCaloOnly.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_HighPtCaloOnly.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_Loose.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_Tight.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_TightTrackOnly.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_TightTrackOnly_FixedRad.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_PflowLoose.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_PflowTight.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_PLVTight.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_PLVLoose.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_PLImprovedTight.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_PLImprovedVeryTight.retrieve(), "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_LowPtPLV.retrieve(), "Failed to retrieve Isolation Tool");
    }

    top::check(m_isolationCorr.retrieve(), "Failed to retrieve Isolation Correction Tool");
    if (m_config->isMC()) top::check(m_IFFTruthTool.retrieve(), "Failed to retrieve IFF Truth Classification Tool");

    std::set<std::string> systPhoton;
    std::set<std::string> systElectron;
    std::set<std::string> systFwdElectron;

    const std:: string& syststr = m_config->systematics();
    std::set<std::string> syst;

    if (!m_config->isSystNominal(syststr) && !m_config->isSystAll(syststr)) {
      bool ok = m_config->getSystematicsList(syststr, syst);
      if (!ok) {
        ATH_MSG_ERROR(" top::EgammaObjectCollectionMaker could not determine systematic list");
        return StatusCode::FAILURE;
      }
      //here the idea is that if the user specifies AllXXX, we leave syst as an empty string, so that all recommended CP
      // systematics are then used
      if (!m_config->contains(syst, "AllElectrons")) {
        systElectron = syst;
      }
      if (!m_config->contains(syst, "AllPhotons")) {
        systPhoton = syst;
      }
      if (!m_config->contains(syst, "AllFwdElectrons")) {
        systFwdElectron = syst;
      }
    }

    specifiedSystematicsPhotons(systPhoton);
    specifiedSystematicsElectrons(systElectron);
    specifiedSystematicsFwdElectrons(systFwdElectron);

    if (m_config->usePhotons()) {
      m_config->systematicsPhotons(specifiedSystematicsPhotons());
    }
    if (m_config->useElectrons()) {
      m_config->systematicsElectrons(specifiedSystematicsElectrons());
    }
    if (m_config->useFwdElectrons()) {
      m_config->systematicsFwdElectrons(specifiedSystematicsFwdElectrons());
    }

    // bool to decide whether to use certain Egamma tools
    m_recomputePhotonFudge = m_config->recomputeCPvars();

    return StatusCode::SUCCESS;
  }

  StatusCode EgammaObjectCollectionMaker::executePhotons(bool executeNominal) {
    ///-- Get base photons from xAOD --///
    const xAOD::PhotonContainer* xaod(nullptr);

    top::check(evtStore()->retrieve(xaod, m_config->sgKeyPhotons()), "Failed to retrieve Photons");

    ///-- Loop over all systematics --///
    for (auto systematic : m_specifiedSystematicsPhotons) {
      ///-- if executeNominal, skip other systematics (and vice-versa) --///
      if (executeNominal && !m_config->isSystNominal(m_config->systematicName(systematic.hash()))) continue;
      if (!executeNominal && m_config->isSystNominal(m_config->systematicName(systematic.hash()))) continue;

      ///-- Tell tool which systematic to use --///
      top::check(m_calibrationTool->applySystematicVariation(systematic), "Failed to applySystematicVariation");

      ///-- Shallow copy of the xAOD --///
      std::pair< xAOD::PhotonContainer*, xAOD::ShallowAuxContainer* > shallow_xaod_copy = xAOD::shallowCopyContainer(
        *xaod);

      ///-- Loop over the xAOD Container and apply corrections--///
      for (auto photon : *(shallow_xaod_copy.first)) {
        ///-- Apply correction to object --///
        // Needs a calo cluster so carry on if no cluster
        if (!photon->caloCluster()) continue;

        if (calibratePhotons) {
          top::check(m_calibrationTool->applyCorrection(*photon),
                     "Failed to applyCorrection");
          top::check(m_isolationCorr->applyCorrection(*photon),
                     "Failed to apply photon isolation leakage correction");

          // Only apply shower shape fudging on full simulation MC
          if (m_config->isMC() && !m_config->isAFII() && m_recomputePhotonFudge && m_config->getDerivationStream() != "PHYS") {
            if (m_photonFudgeTool->applyCorrection(*photon) == 0) { // 0: error, 1: OutOfRange (not possible), 2: OK
              // ElectronPhotonShowerShapeFudgeTool::applyCorrection can return an error for 3 reasons
              // 1) shower shapes not all found, 2) bad cluster, 3) shower shapes not all set.
              // 1 & 3 are most likely due to the smart slimming (no PhotonsDetailedCPContent), whereas 2 is an actual
              // issue.
              // Check for case 2 now:
              if (photon->caloCluster() == nullptr) {
                ATH_MSG_ERROR("Photon " << photon << " had no calo cluster - this is bad!");
                return StatusCode::FAILURE;
              } else {
                // We're now in case 1 or 3
                ATH_MSG_WARNING(
                  " Didn't find the necessary photon shower shapes variables for the ElectronPhotonShowerShapeFudgeTool! (but don't worry, you're still getting correctly ID'd photons)");
                // Keep going, but don't try to use the tool anymore
                m_recomputePhotonFudge = false;
              }
            }
          }
        }
        ///-- Isolation selection --///
        static const SG::AuxElement::ConstAccessor<float> ptcone20_TightTTVA_pt1000("ptcone20_TightTTVA_pt1000");
        char passIsol_FixedCutTight(0);
        char passIsol_FixedCutTightCaloOnly(0);
        char passIsol_FixedCutLoose(0);
        char passIsol_Tight(0);
        char passIsol_TightCaloOnly(0);
        char passIsol_Loose(0);
        if (m_isolationTool_FixedCutTight->accept(*photon)) {
          passIsol_FixedCutTight = 1;
        }
        if (m_isolationTool_FixedCutTightCaloOnly->accept(*photon)) {
          passIsol_FixedCutTightCaloOnly = 1;
        }
        if (m_isolationTool_FixedCutLoose->accept(*photon)) {
          passIsol_FixedCutLoose = 1;
        }
        if (m_isolationTool_TightCaloOnly->accept(*photon)) {
          passIsol_TightCaloOnly = 1;
        }
        if (ptcone20_TightTTVA_pt1000.isAvailable(*photon)) {
          if (m_isolationTool_Tight->accept(*photon)) {
            passIsol_Tight = 1;
          }
          if (m_isolationTool_Loose->accept(*photon)) {
            passIsol_Loose = 1;
          }
        }
        photon->auxdecor<char>("AnalysisTop_Isol_FixedCutTight") = passIsol_FixedCutTight;
        photon->auxdecor<char>("AnalysisTop_Isol_FixedCutTightCaloOnly") = passIsol_FixedCutTightCaloOnly;
        photon->auxdecor<char>("AnalysisTop_Isol_FixedCutLoose") = passIsol_FixedCutLoose;
        photon->auxdecor<char>("AnalysisTop_Isol_Tight") = passIsol_Tight;
        photon->auxdecor<char>("AnalysisTop_Isol_TightCaloOnly") = passIsol_TightCaloOnly;
        photon->auxdecor<char>("AnalysisTop_Isol_Loose") = passIsol_Loose;
      }

      ///-- set links to original objects- needed for MET calculation --///
      bool setLinks = xAOD::setOriginalObjectLink(*xaod, *shallow_xaod_copy.first);
      if (!setLinks) ATH_MSG_ERROR(" Cannot set original object links for photons, MET recalculation may struggle");

      ///-- Save corrected xAOD Container to StoreGate / TStore --///
      std::string outputSGKey = m_config->sgKeyPhotons(systematic.hash());
      std::string outputSGKeyAux = outputSGKey + "Aux.";

      xAOD::TReturnCode save = evtStore()->tds()->record(shallow_xaod_copy.first, outputSGKey);
      xAOD::TReturnCode saveAux = evtStore()->tds()->record(shallow_xaod_copy.second, outputSGKeyAux);

      if (!save || !saveAux) {
        return StatusCode::FAILURE;
      }
    }  // Loop over all systematics
    return StatusCode::SUCCESS;
  }

  StatusCode EgammaObjectCollectionMaker::executeElectrons(bool executeNominal) {
    static const SG::AuxElement::ConstAccessor<float> ptcone20_TightTTVALooseCone_pt1000("ptcone20_TightTTVALooseCone_pt1000");
    static const SG::AuxElement::ConstAccessor<float> ptvarcone20_TightTTVA_pt1000("ptvarcone20_TightTTVA_pt1000");
    static const SG::AuxElement::ConstAccessor<float> ptvarcone30_TightTTVALooseCone_pt1000("ptvarcone30_TightTTVALooseCone_pt1000");
    static const SG::AuxElement::ConstAccessor<float> ptvarcone30_TightTTVALooseCone_pt500("ptvarcone30_TightTTVALooseCone_pt500");
    static const SG::AuxElement::ConstAccessor<float> neflowisol20("neflowisol20");
    static const SG::AuxElement::ConstAccessor<float> topoetcone20("topoetcone20");
    static const SG::AuxElement::ConstAccessor<short> PLV_TrackJetNTrack("PromptLeptonInput_TrackJetNTrack");
    static const SG::AuxElement::ConstAccessor<float> PLV_DRlj("PromptLeptonInput_DRlj");
    static const SG::AuxElement::ConstAccessor<float> PLV_PtRel("PromptLeptonInput_PtRel");
    static const SG::AuxElement::ConstAccessor<float> PLV_PtFrac("PromptLeptonInput_PtFrac");
    static const SG::AuxElement::ConstAccessor<float> PLV_PromptLeptonVeto("PromptLeptonVeto");
    static const SG::AuxElement::ConstAccessor<short> PromptLeptonImprovedInput_MVAXBin("PromptLeptonImprovedInput_MVAXBin");
    static const SG::AuxElement::ConstAccessor<float> PromptLeptonImprovedVetoECAP("PromptLeptonImprovedVetoECAP");
    static const SG::AuxElement::ConstAccessor<float> PromptLeptonImprovedVetoBARR("PromptLeptonImprovedVetoBARR");
    static const SG::AuxElement::ConstAccessor<float> ptvarcone30_TightTTVA_pt500("ptvarcone30_TightTTVA_pt500");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_Tight_VarRad("AnalysisTop_Isol_Tight_VarRad");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_Loose_VarRad("AnalysisTop_Isol_Loose_VarRad");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_TightTrackOnly_VarRad("AnalysisTop_Isol_TightTrackOnly_VarRad");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_FCTight("AnalysisTop_Isol_FCTight");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_FCLoose("AnalysisTop_Isol_FCLoose");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_Tight("AnalysisTop_Isol_Tight");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_Loose("AnalysisTop_Isol_Loose");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_TightTrackOnly("AnalysisTop_Isol_TightTrackOnly");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_TightTrackOnly_FixedRad("AnalysisTop_Isol_TightTrackOnly_FixedRad");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_PflowTight("AnalysisTop_Isol_PflowTight");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_PflowLoose("AnalysisTop_Isol_PflowLoose");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_PLVTight("AnalysisTop_Isol_PLVTight");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_PLVLoose("AnalysisTop_Isol_PLVLoose");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_PLImprovedTight("AnalysisTop_Isol_PLImprovedTight");
    static SG::AuxElement::Accessor<char> AnalysisTop_Isol_PLImprovedVeryTight("AnalysisTop_Isol_PLImprovedVeryTight");
    static const SG::AuxElement::Decorator<float> byhand_LowPtPLV("LowPtPLV");
    static const SG::AuxElement::Decorator<int> AnalysisTop_IFFTruthClass("AnalysisTop_IFFTruthClass");

    const xAOD::EventInfo* eventInfo(nullptr);

    top::check(evtStore()->retrieve(eventInfo, m_config->sgKeyEventInfo()), "Failed to retrieve EventInfo");
    float beam_pos_sigma_x = eventInfo->beamPosSigmaX();
    float beam_pos_sigma_y = eventInfo->beamPosSigmaY();
    float beam_pos_sigma_xy = eventInfo->beamPosSigmaXY();

    ///-- Get base electrons from xAOD --///
    const xAOD::ElectronContainer* xaod(nullptr);
    top::check(evtStore()->retrieve(xaod, m_config->sgKeyElectrons()), "Failed to retrieve Electrons");

    ///-- Loop over all systematics --///
    for (auto systematic : m_specifiedSystematicsElectrons) {
      ///-- if executeNominal, skip other systematics (and vice-versa) --///
      if (executeNominal && !m_config->isSystNominal(m_config->systematicName(systematic.hash()))) continue;
      if (!executeNominal && m_config->isSystNominal(m_config->systematicName(systematic.hash()))) continue;

      ///-- Tell tool which systematic to use -///
      top::check(m_calibrationTool->applySystematicVariation(systematic), "Failed to applySystematicVariation");

      ///-- Shallow copy of the xAOD --///
      std::pair< xAOD::ElectronContainer*, xAOD::ShallowAuxContainer* > shallow_xaod_copy = xAOD::shallowCopyContainer(
        *xaod);

      ///-- Loop over the xAOD Container --///
      for (auto electron : *(shallow_xaod_copy.first)) {
        // Apply correction to object
        // should not affect derivations if there is no CC or track thinning
        if (electron->caloCluster() != nullptr && electron->trackParticle() != nullptr) { // derivations might remove CC
                                                                                          // and tracks for low pt
                                                                                          // electrons
          if (calibrateElectrons) {
            top::check(m_calibrationTool->applyCorrection(*electron), "Failed to applyCorrection");
            top::check(m_isolationCorr->applyCorrection(*electron), "Failed to apply leakage correction");
          }

          double d0sig = xAOD::TrackingHelpers::d0significance(electron->trackParticle(),
                                                               beam_pos_sigma_x,
                                                               beam_pos_sigma_y,
                                                               beam_pos_sigma_xy);
          electron->auxdecor<float>("d0sig") = d0sig;

          if (eventInfo->isAvailable<float>("AnalysisTop_PRIVTX_z_position")) {
            float vtx_z = eventInfo->auxdata<float>("AnalysisTop_PRIVTX_z_position");
            float delta_z0 = electron->trackParticle()->z0() + electron->trackParticle()->vz() - vtx_z;
            electron->auxdecor<float>("delta_z0") = delta_z0;
            electron->auxdecor<float>("delta_z0_sintheta") = delta_z0 * std::sin(electron->trackParticle()->theta());
          }
        }

	//Decorate electrons with IFF-truth classification
        if(m_config->isMC()){
	  unsigned int IFFclass(0);
          top::check( m_IFFTruthTool->classify(*electron, IFFclass), "Unable the classify electron");
          AnalysisTop_IFFTruthClass(*electron) = IFFclass;
        }

        ///-- Isolation selection --///
        char passIsol_Gradient(0);
        char passIsol_FCHighPtCaloOnly(0);
        char passIsol_HighPtCaloOnly(0);
        if (m_isolationTool_Gradient->accept(*electron)) {
          passIsol_Gradient = 1;
        }
        if (m_isolationTool_FCHighPtCaloOnly->accept(*electron)) {
          passIsol_FCHighPtCaloOnly = 1;
        }
        if (m_isolationTool_HighPtCaloOnly->accept(*electron)) {
          passIsol_HighPtCaloOnly = 1;
        }

        electron->auxdecor<char>("AnalysisTop_Isol_Gradient") = passIsol_Gradient;
        electron->auxdecor<char>("AnalysisTop_Isol_FCHighPtCaloOnly") = passIsol_FCHighPtCaloOnly;
        electron->auxdecor<char>("AnalysisTop_Isol_HighPtCaloOnly") = passIsol_HighPtCaloOnly;

        if (ptvarcone30_TightTTVALooseCone_pt1000.isAvailable(*electron) && topoetcone20.isAvailable(*electron)) {
          AnalysisTop_Isol_Tight_VarRad(*electron) = (m_isolationTool_Tight_VarRad->accept(*electron) ? 1 : 0);
          AnalysisTop_Isol_Loose_VarRad(*electron) = (m_isolationTool_Loose_VarRad->accept(*electron) ? 1 : 0);
        }
        if (ptvarcone20_TightTTVA_pt1000.isAvailable(*electron)) {
          AnalysisTop_Isol_FCTight(*electron) = (m_isolationTool_FCTight->accept(*electron) ? 1 : 0);
          AnalysisTop_Isol_FCLoose(*electron) = (m_isolationTool_FCLoose->accept(*electron) ? 1 : 0);
        }
        if (ptvarcone30_TightTTVALooseCone_pt1000.isAvailable(*electron)) {
          AnalysisTop_Isol_TightTrackOnly_VarRad(*electron) = (m_isolationTool_TightTrackOnly_VarRad->accept(*electron) ? 1 : 0);
          AnalysisTop_Isol_Tight(*electron) = (m_isolationTool_Tight->accept(*electron) ? 1 : 0);
          AnalysisTop_Isol_Loose(*electron) = (m_isolationTool_Loose->accept(*electron) ? 1 : 0);
          AnalysisTop_Isol_TightTrackOnly(*electron) = (m_isolationTool_TightTrackOnly->accept(*electron) ? 1 : 0);
          if (ptcone20_TightTTVALooseCone_pt1000.isAvailable(*electron) && neflowisol20.isAvailable(*electron)) {
            AnalysisTop_Isol_TightTrackOnly_FixedRad(*electron) = (m_isolationTool_TightTrackOnly_FixedRad->accept(*electron) ? 1 : 0);
          }
        }
        if (ptvarcone30_TightTTVALooseCone_pt500.isAvailable(*electron) && neflowisol20.isAvailable(*electron)) {
          AnalysisTop_Isol_PflowTight(*electron) = (m_isolationTool_PflowTight->accept(*electron) ? 1 : 0);
          AnalysisTop_Isol_PflowLoose(*electron) = (m_isolationTool_PflowLoose->accept(*electron) ? 1 : 0);
        }
        // Prompt Electron Tagging (PLV): https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PromptLeptonTagging
        // This is not recommended, purely experimental! (no plans for electron SFs from e/gamma any time soon, unless
        // strong motivation from analyses)
        // The r20.7 BDT is called "Iso", the r21 one is "Veto". The cut on the BDT weight is <-0.5, with the
        // FixedCutLoose WP. But this WP is no longer
        // supported by e/gamma, so here let's just decorate that check, and we'll let the user access the BDT weights
        // themselves if needed.
        if (electron->isAvailable<float>("PromptLeptonIso")) // r20.7
          electron->auxdecor<char>("AnalysisTop_Isol_PromptLeptonIso") =
            (electron->auxdata<float>("PromptLeptonIso") < -0.5) ? 1 : 0;
        if (electron->isAvailable<float>("PromptLeptonVeto")) // r21
          electron->auxdecor<char>("AnalysisTop_Isol_PromptLeptonVeto") =
            (electron->auxdata<float>("PromptLeptonVeto") < -0.5) ? 1 : 0;

        // New PLV: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PromptLeptonTaggerIFF
        // For PLV isolation, we need to compute additional variables in the low-pT regime (<12 GeV)
        if ( PLV_TrackJetNTrack.isAvailable(*electron) &&
             PLV_DRlj.isAvailable(*electron) &&
             PLV_PtRel.isAvailable(*electron) &&
             PLV_PtFrac.isAvailable(*electron) )
          top::check(m_isolationTool_LowPtPLV->augmentPLV(*electron), "Failed to augment electron with LowPtPLV decorations");
        else
          byhand_LowPtPLV(*electron) = 1.1; // decorate the electron ourselves following IFF default
        if ( PLV_PromptLeptonVeto.isAvailable(*electron) &&
             ptvarcone30_TightTTVALooseCone_pt1000.isAvailable(*electron) ) {
          AnalysisTop_Isol_PLVTight(*electron) = (m_isolationTool_PLVTight->accept(*electron) ? 1 : 0);
          AnalysisTop_Isol_PLVLoose(*electron) = (m_isolationTool_PLVLoose->accept(*electron) ? 1 : 0);
        } else {
          AnalysisTop_Isol_PLVTight(*electron) = 'n';
          AnalysisTop_Isol_PLVLoose(*electron) = 'n';
        }
        if (ptvarcone30_TightTTVA_pt500.isAvailable(*electron) && PromptLeptonImprovedInput_MVAXBin.isAvailable(*electron)
            && PromptLeptonImprovedVetoECAP.isAvailable(*electron) && PromptLeptonImprovedVetoBARR.isAvailable(*electron)) {
          AnalysisTop_Isol_PLImprovedTight(*electron) = (m_isolationTool_PLImprovedTight->accept(*electron) ? 1 : 0);
          AnalysisTop_Isol_PLImprovedVeryTight(*electron) = (m_isolationTool_PLImprovedVeryTight->accept(*electron) ? 1 : 0);
        } else {
          // decorate with special character to indicate failure to retrieve necessary variables
          AnalysisTop_Isol_PLImprovedTight(*electron) = 'n';
          AnalysisTop_Isol_PLImprovedVeryTight(*electron) = 'n';
        }
      }

      ///-- set links to original objects- needed for MET calculation --///
      bool setLinks = xAOD::setOriginalObjectLink(*xaod, *shallow_xaod_copy.first);
      if (!setLinks) ATH_MSG_ERROR(" Cannot set original object links for electrons, MET recalculation may struggle");

      // Save corrected xAOD Container to StoreGate / TStore
      std::string outputSGKey = m_config->sgKeyElectronsStandAlone(systematic.hash());
      std::string outputSGKeyAux = outputSGKey + "Aux.";

      xAOD::TReturnCode save = evtStore()->tds()->record(shallow_xaod_copy.first, outputSGKey);
      xAOD::TReturnCode saveAux = evtStore()->tds()->record(shallow_xaod_copy.second, outputSGKeyAux);

      if (!save || !saveAux) {
        return StatusCode::FAILURE;
      }
    }  // Loop over all systematics

    return StatusCode::SUCCESS;
  }

  StatusCode EgammaObjectCollectionMaker::executeFwdElectrons(bool executeNominal) {
    const xAOD::EventInfo* eventInfo(nullptr);

    top::check(evtStore()->retrieve(eventInfo, m_config->sgKeyEventInfo()), "Failed to retrieve EventInfo");

    ///-- Get base electrons from xAOD --///
    const xAOD::ElectronContainer* xaod(nullptr);
    top::check(evtStore()->retrieve(xaod, m_config->sgKeyFwdElectrons()), "Failed to retrieve Fwd Electrons");

    ///-- Loop over all systematics --///
    for (auto systematic : m_specifiedSystematicsFwdElectrons) {
      ///-- if executeNominal, skip other systematics (and vice-versa) --///
      if (executeNominal && !m_config->isSystNominal(m_config->systematicName(systematic.hash()))) continue;
      if (!executeNominal && m_config->isSystNominal(m_config->systematicName(systematic.hash()))) continue;

      ///-- Tell tool which systematic to use -///
      top::check(m_calibrationTool->applySystematicVariation(systematic), "Failed to applySystematicVariation");

      ///-- Shallow copy of the xAOD --///
      std::pair< xAOD::ElectronContainer*, xAOD::ShallowAuxContainer* > shallow_xaod_copy = xAOD::shallowCopyContainer(
        *xaod);

      ///-- Loop over the xAOD Container --///
      for (auto electron : *(shallow_xaod_copy.first)) {
        // Apply correction to object
        // should not affect derivations if there is no CC or track thinning
        if (electron->caloCluster() != nullptr) { // derivations might remove CC for low pt electrons
          if (calibrateFwdElectrons) {
            top::check(m_calibrationTool->applyCorrection(*electron), "Failed to applyCorrection to fwd electrons");
          }
        }
      }//end of loop on electrons

      ///-- set links to original objects- needed for MET calculation --///
      bool setLinks = xAOD::setOriginalObjectLink(*xaod, *shallow_xaod_copy.first);
      if (!setLinks) ATH_MSG_ERROR(" Cannot set original object links for fwd electrons");

      // Save corrected xAOD Container to StoreGate / TStore
      std::string outputSGKey = m_config->sgKeyFwdElectronsStandAlone(systematic.hash());
      std::string outputSGKeyAux = outputSGKey + "Aux.";

      xAOD::TReturnCode save = evtStore()->tds()->record(shallow_xaod_copy.first, outputSGKey);
      xAOD::TReturnCode saveAux = evtStore()->tds()->record(shallow_xaod_copy.second, outputSGKeyAux);

      if (!save || !saveAux) {
        return StatusCode::FAILURE;
      }
    }  // Loop over all systematics

    return StatusCode::SUCCESS;
  }

  StatusCode EgammaObjectCollectionMaker::printoutPhotons() {
    // Loop over all systematics
    for (auto s : m_specifiedSystematicsPhotons) {
      const xAOD::PhotonContainer* xaod(nullptr);
      top::check(evtStore()->retrieve(xaod, m_config->sgKeyPhotons(s.hash())), "Failed to retrieve Photons");

      ATH_MSG_INFO(" Photons with sgKey = " << m_config->sgKeyPhotons(s.hash()));
      for (auto x : *xaod) {
        float ptcone30(0.);
        x->isolationValue(ptcone30, xAOD::Iso::ptcone30);
        ATH_MSG_INFO("   ph pT , eta , ptcone30 = " << x->pt() << " , " << x->eta() << " , " << ptcone30);
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode EgammaObjectCollectionMaker::printoutElectrons() {
    // Loop over all systematics
    for (auto s : m_specifiedSystematicsElectrons) {
      const xAOD::ElectronContainer* xaod(nullptr);
      top::check(evtStore()->retrieve(xaod, m_config->sgKeyElectronsStandAlone(
                                        s.hash())), "Failed to retrieve Electrons");

      ATH_MSG_INFO(" Electrons with sgKey = " << m_config->sgKeyElectronsStandAlone(s.hash()));
      for (auto x : *xaod) {
        float ptcone30(0.);
        x->isolationValue(ptcone30, xAOD::Iso::ptcone30);
        ATH_MSG_INFO("   El pT , eta , ptcone30 = " << x->pt() << " , " << x->eta() << " , " << ptcone30);
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode EgammaObjectCollectionMaker::printoutFwdElectrons() {
    // Loop over all systematics
    for (auto s : m_specifiedSystematicsFwdElectrons) {
      const xAOD::ElectronContainer* xaod(nullptr);
      top::check(evtStore()->retrieve(xaod, m_config->sgKeyFwdElectronsStandAlone(
                                        s.hash())), "Failed to retrieve Fwd Electrons");

      ATH_MSG_INFO(" Fwd Electrons with sgKey = " << m_config->sgKeyFwdElectronsStandAlone(s.hash()));
      for (auto x : *xaod) {
        ATH_MSG_INFO("   El pT , eta = " << x->pt() << " , " << x->eta());
      }
    }

    return StatusCode::SUCCESS;
  }

  void EgammaObjectCollectionMaker::specifiedSystematicsPhotons(const std::set<std::string>& specifiedSystematics) {
    ///-- Get the recommended systematics from the tool, in std::vector format --///
    const std::vector<CP::SystematicSet> systList = CP::make_systematics_vector(
      m_calibrationTool->recommendedSystematics());

    for (auto s : systList) {

      if(!m_config->getTreeFilter()->filterTree(s.name())) continue; // Applying tree filter
      m_recommendedSystematicsPhotons.push_back(s);
      if (s.name() == "") {
        m_specifiedSystematicsPhotons.push_back(s);
      }

      ///-- MC only --///
      if (m_config->isMC()) {
        ///-- Are we only doing Nominal? Did the user specify specific systematics to use? --///
        if (!m_config->isSystNominal(m_config->systematics())) {
          if (specifiedSystematics.size() == 0) {
            m_specifiedSystematicsPhotons.push_back(s);
          }
          if (specifiedSystematics.size() > 0) {
            for (auto i : specifiedSystematics) {
              TreeFilter filter(i);
              if (!filter.filterTree(s.name())) {
                m_specifiedSystematicsPhotons.push_back(s);
              }
            }
          }
        }
      }
    }
    m_recommendedSystematicsPhotons.sort();
    m_recommendedSystematicsPhotons.unique();
    m_specifiedSystematicsPhotons.sort();
    m_specifiedSystematicsPhotons.unique();
  }

  void EgammaObjectCollectionMaker::specifiedSystematicsElectrons(const std::set<std::string>& specifiedSystematics) {
    ///-- Get the recommended systematics from the tool, in std::vector format --///
    const std::vector<CP::SystematicSet> systList = CP::make_systematics_vector(
      m_calibrationTool->recommendedSystematics());

    for (auto s : systList) {

      if(!m_config->getTreeFilter()->filterTree(s.name())) continue; // Applying tree filter
      m_recommendedSystematicsElectrons.push_back(s);
      if (s.name() == "") {
        m_specifiedSystematicsElectrons.push_back(s);
      }

      ///-- MC only --///
      if (m_config->isMC()) {
        ///-- Are we only doing Nominal? Did the user specify specific systematics to use? --///
        if (!m_config->isSystNominal(m_config->systematics())) {
          if (specifiedSystematics.size() == 0) {
            m_specifiedSystematicsElectrons.push_back(s);
          }
          if (specifiedSystematics.size() > 0) {
            for (auto i : specifiedSystematics) {
              TreeFilter filter(i);
              if (!filter.filterTree(s.name())) {
                m_specifiedSystematicsElectrons.push_back(s);
              }
            }
          }
        }
      }
    }
    m_recommendedSystematicsElectrons.sort();
    m_recommendedSystematicsElectrons.unique();
    m_specifiedSystematicsElectrons.sort();
    m_specifiedSystematicsElectrons.unique();
  }

  void EgammaObjectCollectionMaker::specifiedSystematicsFwdElectrons(const std::set<std::string>& specifiedSystematics)
  {
    ///-- Get the recommended systematics from the tool, in std::vector format --///
    const std::vector<CP::SystematicSet> systList = CP::make_systematics_vector(
      m_calibrationTool->recommendedSystematics());

    for (auto s : systList) {

      if(!m_config->getTreeFilter()->filterTree(s.name())) continue; // Applying tree filter
      m_recommendedSystematicsFwdElectrons.push_back(s);
      if (s.name() == "") {
        m_specifiedSystematicsFwdElectrons.push_back(s);
      }

      ///-- MC only --///
      if (m_config->isMC()) {
        ///-- Are we only doing Nominal? Did the user specify specific systematics to use? --///
        if (!m_config->isSystNominal(m_config->systematics())) {
          if (specifiedSystematics.size() == 0) {
            m_specifiedSystematicsFwdElectrons.push_back(s);
          }
          if (specifiedSystematics.size() > 0) {
            for (auto i : specifiedSystematics) {
              TreeFilter filter(i);
              if (!filter.filterTree(s.name())) {
                m_specifiedSystematicsFwdElectrons.push_back(s);
              }
            }
          }
        }
      }
    }
    m_recommendedSystematicsFwdElectrons.sort();
    m_recommendedSystematicsFwdElectrons.unique();
    m_specifiedSystematicsFwdElectrons.sort();
    m_specifiedSystematicsFwdElectrons.unique();
  }
}
