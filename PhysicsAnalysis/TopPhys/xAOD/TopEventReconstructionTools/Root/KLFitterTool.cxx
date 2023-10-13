/*
   Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
 */

// $Id: KLFitterTool.cxx 790035 2016-12-15 19:47:38Z aknue $

// Local include(s):
#include "TopEventReconstructionTools/KLFitterTool.h"
#include "TopEvent/Event.h"
#include "TopEvent/EventTools.h"
#include "TopConfiguration/TopConfig.h"
#include "PathResolver/PathResolver.h"

#include <algorithm>

namespace top {
  KLFitterTool::KLFitterTool(const std::string& name) :
    asg::AsgTool(name),
    m_config(nullptr),
    m_massTop(172.5), // This is the MC top mass in GeV - only change if you change the MC mass
    m_bTagCutValue(9999.9),
    m_isWorkingPoint(false),
    m_transferFunctionsPathPrefix("SetMe"),
    m_transferFunctionsPath("SetMe"),
    m_selectionName("SetMe"),
    m_leptonType("SetMe"),
    m_customParameters("SetMe"),
    m_LHType("SetMe"),
    m_canBeBJets(std::vector<unsigned int>(0)),
    m_canBeLFJets(std::vector<unsigned int>(0)),
    m_myFitter(nullptr) {
    declareProperty("config", m_config, "Set the configuration");
    declareProperty("LeptonType", m_leptonType = "kUndefined", "Define the lepton type");
    declareProperty("CustomParameters", m_customParameters = "", "Define the custom parameters");
    declareProperty("SelectionName", m_selectionName = "kUndefined", "Define the name of the selection");
    declareProperty("LHType", m_LHType = "kUndefined", "Define the Likelihood type");
    declareProperty("Njcut",m_Njcut=-1,"max number of jets to run on");
    declareProperty("Nb",m_nb=-1,"number of jets that can only be treated as b-jets");
    declareProperty("Delta",m_delta=-1,"number of jets that can be treated both as b and LF jets");
    declareProperty("Nbmin",m_Nbmin=-1,"minimum number of jets that can be treated as b-jets");
    declareProperty("N5max",m_N5max=-1,"maximum number of jets that can be treated ONLY as b-jets (lie in PCBT bin 5)");
  }

  /// Function initialising the tool
  StatusCode KLFitterTool::initialize() {
    // Have you set the config??
    if (m_config == nullptr) {
      ATH_MSG_ERROR("Please set the top::TopConfig");
      return StatusCode::FAILURE;
    }
    //Retrieving configuration from TopConfig
    m_config->setKLFitter();
    //Setting configuration, giving priority to the ones passed inside the selection
    std::vector<std::string> custom_tokens;
    tokenize(m_customParameters, custom_tokens, " ");
    std::string temp_option = "";
    // Setting event topology
    if (findOption(custom_tokens, "KLFitterLH", temp_option)) m_LHType = temp_option;
    else m_LHType = m_config->KLFitterLH();
    // Find KLFitter ATLAS transfer functions. As of May '18, stored in
    // AnalysisTop group data area on cvmfs.
    m_transferFunctionsPathPrefix = PathResolverFindCalibDirectory("dev/AnalysisTop/KLFitterTFs/");
    if (findOption(custom_tokens, "KLFitterTransferFunctionsPath", temp_option)) m_transferFunctionsPath = temp_option;
    else m_transferFunctionsPath = m_config->KLFitterTransferFunctionsPath();
    std::string transferFunctionAbsPath = m_transferFunctionsPathPrefix + m_transferFunctionsPath + "/";

    // 1) create an instance of the fitter
    m_myFitter = std::unique_ptr<KLFitter::Fitter>(new KLFitter::Fitter {});

    // 2) create an instance of the detector, which holds the information on the resolutions (transfer functions);
    // it takes as an argument the folder which contains the parameter files for the transfer functions
    m_myDetector = std::make_unique<KLFitter::DetectorAtlas_8TeV>(transferFunctionAbsPath);

    // 3) tell the fitter which detector to use
    top::check(m_myFitter->SetDetector(
                 m_myDetector.get()), "KLFitterTool::initialize() ERROR setting detector to fitter");

    // 4) create an instance of the likelihood for ttbar->l+jets channel and customize it according to your needs
    m_myLikelihood = std::make_unique<KLFitter::LikelihoodTopLeptonJets>();

    // 4) create an instance of the likelihood for ttH -> l+jets channel and customize it according to your needs
    m_myLikelihood_TTH = std::make_unique<KLFitter::LikelihoodTTHLeptonJets>();

    // 4) create an instance of the likelihood for ttbar->l+jets channel using jet angles channel and customize it
    // according to your needs
    m_myLikelihood_JetAngles = std::make_unique<KLFitter::LikelihoodTopLeptonJets_JetAngles>();

    // 4) create an instance of the likelihood for ttbar->l+jets channel using angular W-helicity info and
    // customize it according to your needs
    m_myLikelihood_Angular = std::make_unique<KLFitter::LikelihoodTopLeptonJets_Angular>();

    // 4) create an instance of the likelihood for ttZ -> trilepton channel and customize it according to your needs
    m_myLikelihood_TTZ = std::make_unique<KLFitter::LikelihoodTTZTrilepton>();

    // 4) create an instance of the likelihood for ttbar -> allhadronic channel and customize it according to your needs
    m_myLikelihood_AllHadronic = std::make_unique<KLFitter::LikelihoodTopAllHadronic>();

    // 4) create an instance of the likelihood for ttbar -> boosted ljets and customize it according to your needs
    m_myLikelihood_BoostedLJets = std::make_unique<KLFitter::BoostedLikelihoodTopLeptonJets>();

    // 4) create an instance of the likelihood for ttbar -> allHad (single top fashon)
    m_myLikelihood_SingleTop = std::make_unique<KLFitter::LikelihoodOneHadronicTop>();

    // 4.a) SetleptonType
    if (m_LHType != "ttbar_AllHadronic" && m_LHType != "ttbar_AllHadronic_SingleT") { // no lepton type for all hadronic
      if (m_leptonType == "kElectron") {
        m_leptonTypeKLFitterEnum = KLFitter::LikelihoodTopLeptonJets::LeptonType::kElectron;
        m_leptonTypeKLFitterEnum_TTH = KLFitter::LikelihoodTTHLeptonJets::LeptonType::kElectron;
        m_leptonTypeKLFitterEnum_JetAngles = KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kElectron;
        m_leptonTypeKLFitterEnum_Angular = KLFitter::LikelihoodTopLeptonJets_Angular::LeptonType::kElectron;
        m_leptonTypeKLFitterEnum_TTZ = KLFitter::LikelihoodTTZTrilepton::LeptonType::kElectron;
        m_leptonTypeKLFitterEnum_BoostedLJets = KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kElectron;
      } else if (m_leptonType == "kMuon") {
        m_leptonTypeKLFitterEnum = KLFitter::LikelihoodTopLeptonJets::LeptonType::kMuon;
        m_leptonTypeKLFitterEnum_TTH = KLFitter::LikelihoodTTHLeptonJets::LeptonType::kMuon;
        m_leptonTypeKLFitterEnum_JetAngles = KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kMuon;
        m_leptonTypeKLFitterEnum_Angular = KLFitter::LikelihoodTopLeptonJets_Angular::LeptonType::kMuon;
        m_leptonTypeKLFitterEnum_TTZ = KLFitter::LikelihoodTTZTrilepton::LeptonType::kMuon;
        m_leptonTypeKLFitterEnum_BoostedLJets = KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kMuon;
      } else if (m_leptonType == "kTriElectron") {
        if (m_LHType != "ttZTrilepton") {
          ATH_MSG_ERROR(" LeptonType kTriElectron is only defined for the ttZTrilepton likelihood");
          return StatusCode::FAILURE;
        }
        m_leptonTypeKLFitterEnum = KLFitter::LikelihoodTopLeptonJets::LeptonType::kElectron;
        m_leptonTypeKLFitterEnum_TTH = KLFitter::LikelihoodTTHLeptonJets::LeptonType::kElectron;
        m_leptonTypeKLFitterEnum_JetAngles = KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kElectron;
        m_leptonTypeKLFitterEnum_TTZ = KLFitter::LikelihoodTTZTrilepton::LeptonType::kElectron;
        m_leptonTypeKLFitterEnum_BoostedLJets = KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kElectron;
      } else if (m_leptonType == "kTriMuon") {
        if (m_LHType != "ttZTrilepton") {
          ATH_MSG_ERROR(" LeptonType kTriMuon is only defined for the ttZTrilepton likelihood");
          return StatusCode::FAILURE;
        }
        m_leptonTypeKLFitterEnum = KLFitter::LikelihoodTopLeptonJets::LeptonType::kMuon;
        m_leptonTypeKLFitterEnum_TTH = KLFitter::LikelihoodTTHLeptonJets::LeptonType::kMuon;
        m_leptonTypeKLFitterEnum_JetAngles = KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kMuon;
        m_leptonTypeKLFitterEnum_TTZ = KLFitter::LikelihoodTTZTrilepton::LeptonType::kMuon;
        m_leptonTypeKLFitterEnum_BoostedLJets = KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kMuon;
      } else {
        ATH_MSG_ERROR(" Please supply a valid LeptonType : kElectron or kMuon");
        return StatusCode::FAILURE;
      }

      m_myLikelihood->SetLeptonType(m_leptonTypeKLFitterEnum);
      m_myLikelihood_TTH->SetLeptonType(m_leptonTypeKLFitterEnum_TTH);
      m_myLikelihood_JetAngles->SetLeptonType(m_leptonTypeKLFitterEnum_JetAngles);
      m_myLikelihood_Angular->SetLeptonType(m_leptonTypeKLFitterEnum_Angular); 
      m_myLikelihood_TTZ->SetLeptonType(m_leptonTypeKLFitterEnum_TTZ);
      m_myLikelihood_BoostedLJets->SetLeptonType(m_leptonTypeKLFitterEnum_BoostedLJets);
    }

    // 4.b) Jet Selection Mode
    std::string JetSelectionMode = "";
    if (findOption(custom_tokens, "KLFitterJetSelectionMode", temp_option)) JetSelectionMode = temp_option;
    else JetSelectionMode = m_config->KLFitterJetSelectionMode();

    if(JetSelectionMode.find("kAutoSetPCBT")!=std::string::npos) m_jetSelectionModeKLFitterEnum = top::KLFitterJetSelection::kAutoSetPCBT;
    else if(JetSelectionMode.find("kAutoSet")!=std::string::npos) m_jetSelectionModeKLFitterEnum = top::KLFitterJetSelection::kAutoSet;
    else if (JetSelectionMode == "kLeadingThree") m_jetSelectionModeKLFitterEnum = top::KLFitterJetSelection::kLeadingThree;
    else if (JetSelectionMode ==
             "kLeadingFour") m_jetSelectionModeKLFitterEnum = top::KLFitterJetSelection::kLeadingFour;
    else if (JetSelectionMode ==
             "kLeadingFive") m_jetSelectionModeKLFitterEnum = top::KLFitterJetSelection::kLeadingFive;
    else if (JetSelectionMode == "kLeadingSix") m_jetSelectionModeKLFitterEnum = top::KLFitterJetSelection::kLeadingSix;

    else if (JetSelectionMode ==
             "kLeadingSeven") m_jetSelectionModeKLFitterEnum = top::KLFitterJetSelection::kLeadingSeven;
    else if (JetSelectionMode ==
             "kLeadingEight") m_jetSelectionModeKLFitterEnum = top::KLFitterJetSelection::kLeadingEight;
    else if (JetSelectionMode ==
             "kBtagPriorityThreeJets") m_jetSelectionModeKLFitterEnum =
        top::KLFitterJetSelection::kBtagPriorityThreeJets;
    else if (JetSelectionMode ==
             "kBtagPriorityFourJets") m_jetSelectionModeKLFitterEnum = top::KLFitterJetSelection::kBtagPriorityFourJets;

    else if (JetSelectionMode ==
             "kBtagPriorityFiveJets") m_jetSelectionModeKLFitterEnum = top::KLFitterJetSelection::kBtagPriorityFiveJets;

    else if (JetSelectionMode ==
             "kBtagPrioritySixJets") m_jetSelectionModeKLFitterEnum = top::KLFitterJetSelection::kBtagPrioritySixJets;
    else if (JetSelectionMode ==
             "kBtagPrioritySevenJets") m_jetSelectionModeKLFitterEnum =
        top::KLFitterJetSelection::kBtagPrioritySevenJets;
    else if (JetSelectionMode ==
             "kBtagPriorityEightJets") m_jetSelectionModeKLFitterEnum =
        top::KLFitterJetSelection::kBtagPriorityEightJets;
    else {
      ATH_MSG_ERROR(
        "Please supply a valid JetSelectionMode : kLeadingFour , kLeadingFive , kLeadingSix , kLeadingSeven , kLeadingEight , kBtagPriorityFourJets , kBtagPriorityFiveJets , kBtagPrioritySixJets , kBtagPrioritySevenJets , kBtagPriorityEightJets, AutoSet, AutoSetPCBT");
      return StatusCode::FAILURE;
    }

    if (m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kLeadingSix &&
        m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kBtagPrioritySixJets &&
        m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kLeadingSeven &&
        m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kBtagPrioritySevenJets &&
        m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kLeadingEight &&
        m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kBtagPriorityEightJets)
      if (m_LHType == "ttH" || ((m_LHType == "ttbar_AllHadronic" || m_LHType == "ttbar_AllHadronic_SingleT") && (m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kAutoSet && m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kAutoSetPCBT))) {
        ATH_MSG_ERROR(
          "You want to run the ttH or ttbar_AllHadronic Likelihood, you need to use either : kLeadingSix , kBtagPrioritySixJets , kLeadingSeven , kBtagPrioritySevenJets , kLeadingEight , kBtagPriorityEightJets, AutoSet, AutoSetPCBT (latter two only for ttbar_AllHadronic)");
        return StatusCode::FAILURE;
      }

    if((m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kAutoSet || m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kAutoSetPCBT) && m_LHType != "ttbar_AllHadronic" && m_LHType != "ttbar_AllHadronic_SingleT") {
      ATH_MSG_ERROR("AutoSet only works for fully hadronic ttbar");
      return StatusCode::FAILURE;
  }

    if(m_LHType == "ttbar_AllHadronic_SingleT" && m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kAutoSet && m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kAutoSetPCBT) {
      ATH_MSG_ERROR("SingleT option only tested for 'AutoSet' and 'AutoSetPCBT' selection mode. Other selection types might lead to errors.");
      return StatusCode::FAILURE;
    }

    // 4.c) SetBTagging method
    std::string BTaggingMethod = "";
    if (findOption(custom_tokens, "KLFitterBTaggingMethod", temp_option)) BTaggingMethod = temp_option;
    else BTaggingMethod = m_config->KLFitterBTaggingMethod();

    if (BTaggingMethod == "kNotag") m_bTaggingMethodKLFitterEnum = KLFitter::LikelihoodBase::BtaggingMethod::kNotag;
    else if (BTaggingMethod ==
             "kVetoNoFit") m_bTaggingMethodKLFitterEnum = KLFitter::LikelihoodBase::BtaggingMethod::kVetoNoFit;
    else if (BTaggingMethod ==
             "kVetoNoFitLight") m_bTaggingMethodKLFitterEnum =
        KLFitter::LikelihoodBase::BtaggingMethod::kVetoNoFitLight;
    else if (BTaggingMethod ==
             "kVetoNoFitBoth") m_bTaggingMethodKLFitterEnum = KLFitter::LikelihoodBase::BtaggingMethod::kVetoNoFitBoth;
    else if (BTaggingMethod ==
             "kVetoHybridNoFit") m_bTaggingMethodKLFitterEnum =
        KLFitter::LikelihoodBase::BtaggingMethod::kVetoHybridNoFit;
    else if (BTaggingMethod == "kWorkingPoint") {
      m_bTaggingMethodKLFitterEnum = KLFitter::LikelihoodBase::BtaggingMethod::kWorkingPoint;
      m_isWorkingPoint = true;
    } else if (BTaggingMethod ==
               "kVeto") m_bTaggingMethodKLFitterEnum = KLFitter::LikelihoodBase::BtaggingMethod::kVeto;
    else if (BTaggingMethod ==
             "kVetoLight") m_bTaggingMethodKLFitterEnum = KLFitter::LikelihoodBase::BtaggingMethod::kVetoLight;
    else if (BTaggingMethod ==
             "kVetoBoth") m_bTaggingMethodKLFitterEnum = KLFitter::LikelihoodBase::BtaggingMethod::kVetoBoth;
    else {
      ATH_MSG_ERROR(
        "Please supply a valid BTaggingMethod : kNotag,kVetoNoFit,kVetoNoFitLight,kVetoNoFitBoth,kVetoHybridNoFit,kWorkingPoint,kVeto,kVetoLight or kVetoBoth");
      return StatusCode::FAILURE;
    }
    m_myLikelihood->SetBTagging(m_bTaggingMethodKLFitterEnum);
    m_myLikelihood_TTH->SetBTagging(m_bTaggingMethodKLFitterEnum);
    m_myLikelihood_JetAngles->SetBTagging(m_bTaggingMethodKLFitterEnum);
    m_myLikelihood_Angular->SetBTagging(m_bTaggingMethodKLFitterEnum);
    m_myLikelihood_TTZ->SetBTagging(m_bTaggingMethodKLFitterEnum);
    m_myLikelihood_AllHadronic->SetBTagging(m_bTaggingMethodKLFitterEnum);
    m_myLikelihood_BoostedLJets->SetBTagging(m_bTaggingMethodKLFitterEnum);
    m_myLikelihood_SingleTop->SetBTagging(m_bTaggingMethodKLFitterEnum);

    // 4.d) SetTopMass
    m_myLikelihood->PhysicsConstants()->SetMassTop(m_massTop);
    m_myLikelihood_TTH->PhysicsConstants()->SetMassTop(m_massTop);
    m_myLikelihood_JetAngles->PhysicsConstants()->SetMassTop(m_massTop);
    m_myLikelihood_Angular->PhysicsConstants()->SetMassTop(m_massTop);
    m_myLikelihood_TTZ->PhysicsConstants()->SetMassTop(m_massTop);
    m_myLikelihood_AllHadronic->PhysicsConstants()->SetMassTop(m_massTop);
    m_myLikelihood_BoostedLJets->PhysicsConstants()->SetMassTop(m_massTop);
    m_myLikelihood_SingleTop->PhysicsConstants()->SetMassTop(m_massTop);

    // 4.e) TopMassFixed
    bool FixTopMass = true;
    if (findOption(custom_tokens, "KLFitterTopMassFixed", temp_option)) {
      if (temp_option.compare("True") == 0) FixTopMass = true;
      else if (temp_option.compare("False") == 0) FixTopMass = false;
      else {
        ATH_MSG_ERROR("Invalid KLFitterTopMassFixed custom option! Exiting.");
        return StatusCode::FAILURE;
      }
    } else FixTopMass = m_config->KLFitterTopMassFixed();

    m_myLikelihood->SetFlagTopMassFixed(FixTopMass);
    m_myLikelihood_TTH->SetFlagTopMassFixed(FixTopMass);
    m_myLikelihood_JetAngles->SetFlagTopMassFixed(FixTopMass);
    m_myLikelihood_Angular->SetFlagTopMassFixed(FixTopMass);
    m_myLikelihood_TTZ->SetFlagTopMassFixed(FixTopMass);
    m_myLikelihood_AllHadronic->SetFlagTopMassFixed(FixTopMass);
    m_myLikelihood_BoostedLJets->SetFlagTopMassFixed(FixTopMass);
    m_myLikelihood_SingleTop->SetFlagTopMassFixed(FixTopMass);

    // 5) tell the fitter which likelihood to use
    if (m_LHType == "ttbar") top::check(m_myFitter->SetLikelihood(
                                          m_myLikelihood.get()),
                                        "KLFitterTool::initialize() ERROR setting likelihood for KLFitter");
    else if (m_LHType == "ttH") top::check(m_myFitter->SetLikelihood(
                                             m_myLikelihood_TTH.get()),
                                           "KLFitterTool::initialize() ERROR setting likelihood for KLFitter");
    else if (m_LHType == "ttbar_JetAngles") top::check(m_myFitter->SetLikelihood(
                                                         m_myLikelihood_JetAngles.get()),
                                                       "KLFitterTool::initialize() ERROR setting likelihood for KLFitter");
    else if (m_LHType == "ttbar_Angular") top::check(m_myFitter->SetLikelihood(
                                                       m_myLikelihood_Angular.get()),
                                                     "KLFitterTool::initialize() ERROR setting likelihood for KLFitter");
    else if (m_LHType == "ttZTrilepton" && (m_leptonType == "kTriElectron" || m_leptonType == "kTriMuon")) {
      // For ttZ->trilepton, we can have difficult combinations of leptons in the
      // final state (3x same flavour, or mixed case). The latter is trivial, for
      // which we can default back to the ljets likelihood. So we distinguish here:
      //  - kTriMuon, kTriElectron: dedicated TTZ->trilepton likelihood,
      //  - kMuon, kElectron: standard ttbar->l+jets likelihood.
      top::check(m_myFitter->SetLikelihood(
                   m_myLikelihood_TTZ.get()), "KLFitterTool::initialize() ERROR setting likelihood for KLFitter");
    } else if (m_LHType == "ttZTrilepton") {
      top::check(m_myFitter->SetLikelihood(
                   m_myLikelihood.get()), "KLFitterTool::initialize() ERROR setting likelihood for KLFitter");
    } else if (m_LHType == "ttbar_AllHadronic") {
      top::check(m_myFitter->SetLikelihood(
                   m_myLikelihood_AllHadronic.get()),
                 "KLFitterTool::initialize() ERROR setting likelihood for KLFitter");
    } else if (m_LHType == "ttbar_BoostedLJets") {
      top::check(m_myFitter->SetLikelihood(
                   m_myLikelihood_BoostedLJets.get()),
                 "KLFitterTool::initialize() ERROR setting likelihood for KLFitter");
    } else if (m_LHType == "ttbar_AllHadronic_SingleT") {
      top::check(m_myFitter->SetLikelihood(
                   m_myLikelihood_SingleTop.get()),
                 "KLFitterTool::initialize() ERROR setting likelihood for KLFitter");
    } else {
      ATH_MSG_ERROR("KLFitter: This likelihood is not defined...");
      return StatusCode::FAILURE;
    }

    // 6) Figure out the b tagging working point
    // All the blame for this horrible code rests with the b-tagging people

    if (findOption(custom_tokens, "KLFitterBTaggingWP", temp_option)) {
      m_btagWP = temp_option;
    } else {
      if (m_config->bTagWP_available().size() != 1) {
        ATH_MSG_ERROR(
          m_config->bTagWP_available().size() <<
            " b-tagging WP - cannot pick b-jets. Please select only 1 WP or specify the desired one in your selection!");
        return StatusCode::FAILURE;
      }
      m_btagWP = m_config->bTagWP_available()[0];
    }

    m_isPCBT=(m_btagWP.find("Continuous") != std::string::npos);

    if (m_isPCBT && m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kAutoSet && m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kAutoSetPCBT) {
      ATH_MSG_ERROR(
        "KLFitter is not able to run with (pseudo)continuous b-tagging! Please specify a different WP either in your configuration file or in your selection!");
      return StatusCode::FAILURE;
    }
    if (!m_isPCBT && m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kAutoSetPCBT) {
      ATH_MSG_ERROR(
        "AutoSetPCBT selection mode can only run with (pseudo)continuous b-tagging! Please specify the proper WP either in your configuration file or in your selection!");
      return StatusCode::FAILURE;
    }

    if (m_isWorkingPoint || (!m_isPCBT &&  m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kAutoSet)) {
      m_btagging_eff_tool = "BTaggingEfficiencyTool_" + m_btagWP + "_" + m_config->sgKeyJets();
      top::check(m_btagging_eff_tool.retrieve(), "KLFitterTool:: Failed to retrieve b-tagging Efficiency tool");
    }

    //6b) use btagging info to figure KLF ordering
    if(m_isPCBT) {
      if(findOption(custom_tokens, "JetOrderingVariable", temp_option)) {
	top::check((temp_option=="pt" || temp_option=="TrackSumPt" || temp_option=="SUM_NumTrkPt500" || temp_option=="NumTrkPt500" || temp_option=="SUM_NumTrkPt1000" || temp_option=="NumTrkPt1000"),"KLFitterTool: invalid JetOrderingVariable option. Accepted names are: 'pt', 'TrackSumPt', 'SUM_NumTrkPt500, NumTrkPt500,SUM_NumTrkPt1000, NumTrkPt1000'. (Default='pt')");
	m_ordVar=temp_option;
      } else {
	m_ordVar="pt";
      }
    }

    ATH_MSG_INFO("++++++++++++++++++++++++++++++");
    ATH_MSG_INFO("Configured KLFitter with name " << name());
    ATH_MSG_INFO("  For selection " << m_selectionName);
    ATH_MSG_INFO("  Using " << m_btagging_eff_tool);
    ATH_MSG_INFO("  Using transfer functions with full path " << transferFunctionAbsPath);
    ATH_MSG_INFO("  Using Lepton \t\t" << m_leptonType);
    ATH_MSG_INFO("  Using JetSelectionMode \t" << JetSelectionMode);
    ATH_MSG_INFO("  Using BTaggingMethod \t" << BTaggingMethod);
    if(m_isPCBT) ATH_MSG_INFO("  Using JetOrderingVariable \t" << m_ordVar);
    ATH_MSG_INFO("  Using TopMassFixed \t" << FixTopMass);

    if (m_config->KLFitterSaveAllPermutations()) ATH_MSG_INFO("  Saving All permutations");
    else ATH_MSG_INFO("  Saving only the permutation with the highest event probability");
    ATH_MSG_INFO("++++++++++++++++++++++++++++++");

    /// Return gracefully:
    return StatusCode::SUCCESS;
  }

  /// Config helpers
  bool KLFitterTool::findOption(std::vector<std::string> full_options, std::string option, std::string& op_value) {
    //Find option in full_options and put in op_value, then return true. Otherwise return false
    for (unsigned int t = 0; t < full_options.size(); ++t) {
      if (full_options.at(t).compare("") == 0) continue; //Skip void strings
      top::check((full_options.at(t).find(
                    ":") != std::string::npos),
                 "KLFitterTool::findOption Error! You specified an invalid option: " + full_options.at(
                   t) + ". Expected format is A:a B:b C:c...");
      std::string key = full_options.at(t).substr(0, full_options.at(t).find(":"));
      std::string value = full_options.at(t).substr(full_options.at(t).find(":") + 1);
      if (key.compare(option) == 0) {
        op_value = value;
        return true;
      }
    }
    return false;
  }

  /// Function executing the tool
  StatusCode KLFitterTool::execute(const top::Event& event) {
    // run KLFitter
    // create an instance of the particles class filled with the particles to be fitted;
    // here, you need to make sure that
    // - the particles are in the range allowed by the transfer functions (eta and pt)
    // - the energies and momenta are in GeV
    // - be aware that *all* particles you're adding are considered in the fit
    //   (many particles lead to many permutations to be considered and hence a long
    //   running time and not necessarily good fitting results due to the many available
    //   permutations)
    // the arguments taken py AddParticle() are
    // - TLorentzVector of the physics 4-momentum
    // - detector eta for the evaluation of the transfer functions (for muons: just use the physics eta)
    // - type of particle
    // - an optional name of the particle (pass empty string in case you don't want to give your particle a name)
    // - index of the particle in your original collection (for convenience)
    // - for jets:
    //   * bool isBtagged : mandatory only if you want to use b-tagging in the fit

/*
    FIXME: this may be useful to cache results, that's why I am not deleting this piece of commented code
           maybe we could concatenate the full KLFitter selection and then calculate an hash...
    if( (event.m_info->isAvailable< int >( "KLFitterHasRun" ) ) )
       if( ( event.m_info->auxdata< int >("KLFitterHasRun") )!=0 ) return StatusCode::SUCCESS;
 */


    KLFitter::Particles* myParticles = new KLFitter::Particles {};

    if (m_LHType == "ttbar") {
      if (m_leptonTypeKLFitterEnum == KLFitter::LikelihoodTopLeptonJets::LeptonType::kElectron) {
        TLorentzVector el;
        el.SetPtEtaPhiE(event.m_electrons.at(0)->pt() / 1.e3, event.m_electrons.at(0)->eta(), event.m_electrons.at(
                          0)->phi(), event.m_electrons.at(0)->e() / 1.e3);
        myParticles->AddParticle(&el, event.m_electrons.at(0)->caloCluster()->etaBE(2), KLFitter::Particles::kElectron);
      }
      if (m_leptonTypeKLFitterEnum == KLFitter::LikelihoodTopLeptonJets::LeptonType::kMuon) {
        TLorentzVector mu;
        mu.SetPtEtaPhiE(event.m_muons.at(0)->pt() / 1.e3, event.m_muons.at(0)->eta(), event.m_muons.at(
                          0)->phi(), event.m_muons.at(0)->e() / 1.e3);
        myParticles->AddParticle(&mu, mu.Eta(), KLFitter::Particles::kMuon);
      }
    }
    if (m_LHType == "ttH") {
      if (m_leptonTypeKLFitterEnum_TTH == KLFitter::LikelihoodTTHLeptonJets::LeptonType::kElectron) {
        TLorentzVector el;
        el.SetPtEtaPhiE(event.m_electrons.at(0)->pt() / 1.e3, event.m_electrons.at(0)->eta(), event.m_electrons.at(
                          0)->phi(), event.m_electrons.at(0)->e() / 1.e3);
        myParticles->AddParticle(&el, event.m_electrons.at(0)->caloCluster()->etaBE(2), KLFitter::Particles::kElectron);
      }
      if (m_leptonTypeKLFitterEnum_TTH == KLFitter::LikelihoodTTHLeptonJets::LeptonType::kMuon) {
        TLorentzVector mu;
        mu.SetPtEtaPhiE(event.m_muons.at(0)->pt() / 1.e3, event.m_muons.at(0)->eta(), event.m_muons.at(
                          0)->phi(), event.m_muons.at(0)->e() / 1.e3);
        myParticles->AddParticle(&mu, mu.Eta(), KLFitter::Particles::kMuon);
      }
    }
    if (m_LHType == "ttbar_JetAngles") {
      if (m_leptonTypeKLFitterEnum_JetAngles == KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kElectron) {
        TLorentzVector el;
        el.SetPtEtaPhiE(event.m_electrons.at(0)->pt() / 1.e3, event.m_electrons.at(0)->eta(), event.m_electrons.at(
                          0)->phi(), event.m_electrons.at(0)->e() / 1.e3);
        myParticles->AddParticle(&el, event.m_electrons.at(0)->caloCluster()->etaBE(2), KLFitter::Particles::kElectron);
      }
      if (m_leptonTypeKLFitterEnum_JetAngles == KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kMuon) {
        TLorentzVector mu;
        mu.SetPtEtaPhiE(event.m_muons.at(0)->pt() / 1.e3, event.m_muons.at(0)->eta(), event.m_muons.at(
                          0)->phi(), event.m_muons.at(0)->e() / 1.e3);
        myParticles->AddParticle(&mu, mu.Eta(), KLFitter::Particles::kMuon);
      }
    }
    if (m_LHType == "ttbar_Angular") {
      if (m_leptonTypeKLFitterEnum_Angular == KLFitter::LikelihoodTopLeptonJets_Angular::LeptonType::kElectron) {
        TLorentzVector el;
        el.SetPtEtaPhiE(event.m_electrons.at(0)->pt() / 1.e3, event.m_electrons.at(0)->eta(), event.m_electrons.at(
                          0)->phi(), event.m_electrons.at(0)->e() / 1.e3);
        myParticles->AddParticle(&el, event.m_electrons.at(0)->caloCluster()->etaBE(2), KLFitter::Particles::kElectron);
      }
      if (m_leptonTypeKLFitterEnum_Angular == KLFitter::LikelihoodTopLeptonJets_Angular::LeptonType::kMuon) {
        TLorentzVector mu;
        mu.SetPtEtaPhiE(event.m_muons.at(0)->pt() / 1.e3, event.m_muons.at(0)->eta(), event.m_muons.at(
                          0)->phi(), event.m_muons.at(0)->e() / 1.e3);
        myParticles->AddParticle(&mu, mu.Eta(), KLFitter::Particles::kMuon);
      }
    }
    if (m_LHType == "ttZTrilepton") {
      if (m_leptonTypeKLFitterEnum_TTZ == KLFitter::LikelihoodTTZTrilepton::LeptonType::kElectron) {
        if (m_leptonType == "kTriElectron") {
          // This is the "true" trilepton case with three leptons of the same flavour.
          if (event.m_electrons.size() < 3) {
            ATH_MSG_ERROR("KLFitter: kTriElectron requires three electrons...");
            return StatusCode::FAILURE;
          }
          TLorentzVector el;
          for (unsigned int i = 0; i < 3; ++i) {
            const auto& electron = event.m_electrons.at(i);
            el.SetPtEtaPhiE(electron->pt() / 1.e3, electron->eta(), electron->phi(), electron->e() / 1.e3);
            myParticles->AddParticle(&el, electron->caloCluster()->etaBE(2), KLFitter::Particles::kElectron, "", i);
          }
        } else {
          // Trivial case of mixed lepton flavours. Use ttbar->l+jets likelihood and only add the single lepton.
          TLorentzVector el;
          el.SetPtEtaPhiE(event.m_electrons.at(0)->pt() / 1.e3, event.m_electrons.at(0)->eta(), event.m_electrons.at(
                            0)->phi(), event.m_electrons.at(0)->e() / 1.e3);
          myParticles->AddParticle(&el, event.m_electrons.at(0)->caloCluster()->etaBE(2),
                                   KLFitter::Particles::kElectron);
        }
      }

      if (m_leptonTypeKLFitterEnum_TTZ == KLFitter::LikelihoodTTZTrilepton::LeptonType::kMuon) {
        if (m_leptonType == "kTriMuon") {
          // This is the "true" trilepton case with three leptons of the same flavour.
          if (event.m_muons.size() < 3) {
            ATH_MSG_ERROR("KLFitter: kTriMuon requires three muons...");
            return StatusCode::FAILURE;
          }
          TLorentzVector mu;
          for (unsigned int i = 0; i < 3; ++i) {
            const auto& muon = event.m_muons.at(i);
            mu.SetPtEtaPhiE(muon->pt() / 1.e3, muon->eta(), muon->phi(), muon->e() / 1.e3);
            myParticles->AddParticle(&mu, mu.Eta(), KLFitter::Particles::kMuon, "", i);
          }
        } else {
          // Trivial case of mixed lepton flavours. Use ttbar->l+jets likelihood and only add the single lepton.
          TLorentzVector mu;
          mu.SetPtEtaPhiE(event.m_muons.at(0)->pt() / 1.e3, event.m_muons.at(0)->eta(), event.m_muons.at(
                            0)->phi(), event.m_muons.at(0)->e() / 1.e3);
          myParticles->AddParticle(&mu, mu.Eta(), KLFitter::Particles::kMuon);
        }
      }
    }
    if (m_LHType == "ttbar_BoostedLJets") {
      if (m_leptonTypeKLFitterEnum_BoostedLJets == KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kElectron) {
        TLorentzVector el;
        el.SetPtEtaPhiE(event.m_electrons.at(0)->pt() / 1.e3, event.m_electrons.at(0)->eta(), event.m_electrons.at(
                          0)->phi(), event.m_electrons.at(0)->e() / 1.e3);
        myParticles->AddParticle(&el, event.m_electrons.at(0)->caloCluster()->etaBE(2), KLFitter::Particles::kElectron);
      }
      if (m_leptonTypeKLFitterEnum_BoostedLJets == KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kMuon) {
        TLorentzVector mu;
        mu.SetPtEtaPhiE(event.m_muons.at(0)->pt() / 1.e3, event.m_muons.at(0)->eta(), event.m_muons.at(
                          0)->phi(), event.m_muons.at(0)->e() / 1.e3);
        myParticles->AddParticle(&mu, mu.Eta(), KLFitter::Particles::kMuon);
      }
    }

    // set the jets, depending on the Jet Selection Mode
    if (!setJets(event, myParticles)) {
      ATH_MSG_INFO(
        "KLFitterTool::execute: error at event " << event.m_info->eventNumber() <<
          ". It was not possible to properly fill the jets. Are you trying to use a KLeadingX Jet Selection mode with a signal region with less than X jets? Please check your configuration!");
      return StatusCode::FAILURE;
    }

    // add the particles to the fitter
    if(m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kAutoSet && m_jetSelectionModeKLFitterEnum != top::KLFitterJetSelection::kAutoSetPCBT) { //NOTE: moving this inside loop on jet combinations. It only works for allHadronic ttbar (otherwise one looses the leptons)
      if (!m_myFitter->SetParticles(myParticles)) {
	ATH_MSG_ERROR("KLFitter: Error adding particles to fitter...");
	return StatusCode::FAILURE;
      }
    }

    // add the MET x and y components as well as the SumET to the fitter
    const double met_ex = event.m_met->mpx() / 1.e3;
    const double met_ey = event.m_met->mpy() / 1.e3;
    const double met_sumet = event.m_met->sumet() / 1.e3;

    if (!m_myFitter->SetET_miss_XY_SumET(met_ex, met_ey, met_sumet)) {
      ATH_MSG_ERROR("KLFitter: Error adding MET to fitter...");
      return StatusCode::FAILURE;
    }
    // define StoreGate names
    std::string outputSGKey("SetMe");
    if (!event.m_isLoose) {
      outputSGKey = m_config->sgKeyKLFitter(event.m_hashValue);
    }
    if (event.m_isLoose) {
      outputSGKey = m_config->sgKeyKLFitterLoose(event.m_hashValue);
    }
    std::string outputSGKeyAux = outputSGKey + "Aux.";
    // create or retrieve (if existent) the xAOD::KLFitterResultContainer
    xAOD::KLFitterResultAuxContainer* resultAuxContainer = nullptr;
    xAOD::KLFitterResultContainer* resultContainer = nullptr;
    if ((!m_config->KLFitterSaveAllPermutations()) ||
        ((m_config->KLFitterSaveAllPermutations()) &&
         (!evtStore()->tds()->contains<xAOD::KLFitterResultContainer>(outputSGKey)))) {
      resultAuxContainer = new xAOD::KLFitterResultAuxContainer {};
      resultContainer = new xAOD::KLFitterResultContainer {};
      resultContainer->setStore(resultAuxContainer);
    } else top::check(evtStore()->tds()->retrieve(resultContainer,
                                                  outputSGKey),
                      "KLFitterTools::execute(): can not retrieve xAOD::KLFitterResultContainer from evtStore()");

    // loop over all permutations
    xAOD::KLFitterResult* result = nullptr;
    if(m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kAutoSet || m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kAutoSetPCBT) {
      if(m_LHType == "ttbar_AllHadronic_SingleT") {
	if(!permutationLoopAutoSetSingleT(result,resultContainer,event)) {
	  ATH_MSG_ERROR("KLFitter: error in the 'AutoSet' loop (SingleT option)");
	  return StatusCode::FAILURE;
	}
      } else {
	if(!permutationLoopAutoSet(result,resultContainer,event)) {
	  ATH_MSG_ERROR("KLFitter: error in the 'AutoSet' loop (ttbar option)");
	  return StatusCode::FAILURE;
	}
      }
    }
    else permutationLoopStandard(result,resultContainer);


    // Normalize event probability to unity
    // work out best permutation
    float sumEventProbability(0.);
    unsigned int bestPermutation(999), iPerm(0);
    unsigned int bestPermutation2(999); //for SingleT option

    // First loop
    if(m_LHType == "ttbar_AllHadronic_SingleT") findBestPermInd_SingleT(resultContainer,bestPermutation,bestPermutation2,sumEventProbability);
    else findBestPermInd_Standard(resultContainer,bestPermutation,sumEventProbability);

    // Second loop
    iPerm = 0;
    for (auto x : *resultContainer) {
      if(m_LHType == "ttbar_AllHadronic_SingleT") x->setEventProbability(x->eventProbability() / TMath::Sqrt(sumEventProbability)); //we'll take the product of two of them, so use the square root to get the proper result in the end
      else x->setEventProbability(x->eventProbability() / sumEventProbability);

      if (iPerm == bestPermutation || (m_LHType == "ttbar_AllHadronic_SingleT" && iPerm == bestPermutation2)) {
        x->setBestPermutation(1);
      } else {
        x->setBestPermutation(0);
      }
      ++iPerm;
    }

    // Save to StoreGate / TStore

    // Save all permutations or only the highest event probability?

    // Save all
    if (m_config->KLFitterSaveAllPermutations()) {
      if (!evtStore()->tds()->contains<xAOD::KLFitterResultContainer>(outputSGKey)) {
        top::check(evtStore()->tds()->record(resultContainer,
                                             outputSGKey),
                   "KLFitterTools: ERROR! Was not able to write KLFitterResultContainer");
        top::check(evtStore()->tds()->record(resultAuxContainer,
                                             outputSGKeyAux),
                   "KLFitterTools: ERROR! Was not able to write KLFitterResultAuxContainer");
      }
    }
    // Save only the best
    else {
      // create ore retrieve the xAOD::KLFitterResultContainer
      xAOD::KLFitterResultAuxContainer* bestAuxContainer = nullptr;
      xAOD::KLFitterResultContainer* bestContainer = nullptr;
      if (!evtStore()->tds()->contains<xAOD::KLFitterResultContainer>(outputSGKey)) {
        bestAuxContainer = new xAOD::KLFitterResultAuxContainer {};
        bestContainer = new xAOD::KLFitterResultContainer {};
        bestContainer->setStore(bestAuxContainer);
      } else top::check(evtStore()->tds()->retrieve(bestContainer,
                                                    outputSGKey),
                        "KLFitterTools::execute(): can not retrieve xAOD::KLFitterResultContainer from evtStore()");


      if(m_LHType != "ttbar_AllHadronic_SingleT") {
	for (auto x : *resultContainer) {
	  if (x->bestPermutation() == 1) {
	    xAOD::KLFitterResult* result = new xAOD::KLFitterResult {};
	    result->makePrivateStore(*x);
	    bestContainer->push_back(result);
	  }
	}
      } else {

	//combine the two best permutation to get the result
	xAOD::KLFitterResult* combinedResult = new xAOD::KLFitterResult {};
	bestContainer->push_back(combinedResult);	

	if(bestPermutation<resultContainer->size() && bestPermutation2<resultContainer->size()) {
	  xAOD::KLFitterResult* st1 = resultContainer->at(bestPermutation);
	  xAOD::KLFitterResult* st2 = resultContainer->at(bestPermutation2);
	  top::check(st1->bestPermutation() == 1 && st2->bestPermutation() == 1,"KLFitterTools: ERROR! You are picking the wrong best permutations in SingleT option!");
	  
	  std::vector<double> param(0);
	  for(double pr : st1->parameters()) param.push_back(pr);
	  for(double pr : st2->parameters()) param.push_back(pr);
	  combinedResult->setParameters(param);
	  std::vector<double> paramEr(0);
	  for(double pr : st1->parameterErrors()) paramEr.push_back(pr);
	  for(double pr : st2->parameterErrors()) paramEr.push_back(pr);
	  combinedResult->setParameterErrors(paramEr);
	  
	  
	  combinedResult->setSelectionCode(st1->selectionCode());
	  if(st1->minuitDidNotConverge()>0 || st2->minuitDidNotConverge()>0) std::cout << "minuitDidNotConverge" << std::endl;
	  combinedResult->setMinuitDidNotConverge(std::max(st1->minuitDidNotConverge(),st2->minuitDidNotConverge()));
	  combinedResult->setFitAbortedDueToNaN(std::max(st1->fitAbortedDueToNaN(),st2->fitAbortedDueToNaN()));
	  if(st1->fitAbortedDueToNaN()>0 || st2->fitAbortedDueToNaN()>0) std::cout << "fitAbortedDueToNaN" << std::endl;
	  
	  combinedResult->setAtLeastOneFitParameterAtItsLimit(std::max(st1->atLeastOneFitParameterAtItsLimit(),st2->atLeastOneFitParameterAtItsLimit()));
	  if(st1->atLeastOneFitParameterAtItsLimit()>0 || st2->atLeastOneFitParameterAtItsLimit()>0) std::cout << "atLeastOneFitParameterAtItsLimit" << std::endl;
	  combinedResult->setInvalidTransferFunctionAtConvergence(std::max(st1->invalidTransferFunctionAtConvergence(),st2->invalidTransferFunctionAtConvergence()));
	  if(st1->invalidTransferFunctionAtConvergence()>0 || st2->invalidTransferFunctionAtConvergence()>0) std::cout << "invalidTransferFunctionAtConvergence()" << std::endl;
	  
	  combinedResult->setLogLikelihood(st1->logLikelihood()+st2->logLikelihood());
	  combinedResult->setEventProbability(st1->eventProbability()*st2->eventProbability());
	  
	  combinedResult->setLogLikelihood_t1(st1->logLikelihood());

	  combinedResult->setModel_b_from_top1_pt(st1->model_b_from_top1_pt());
	  combinedResult->setModel_b_from_top1_eta(st1->model_b_from_top1_eta());
	  combinedResult->setModel_b_from_top1_phi(st1->model_b_from_top1_phi());
	  combinedResult->setModel_b_from_top1_E(st1->model_b_from_top1_E());
	  combinedResult->setModel_b_from_top1_jetIndex(st1->model_b_from_top1_jetIndex());
	  
	  combinedResult->setModel_lj1_from_top1_pt(st1->model_lj1_from_top1_pt());
	  combinedResult->setModel_lj1_from_top1_eta(st1->model_lj1_from_top1_eta());
	  combinedResult->setModel_lj1_from_top1_phi(st1->model_lj1_from_top1_phi());
	  combinedResult->setModel_lj1_from_top1_E(st1->model_lj1_from_top1_E());
	  combinedResult->setModel_lj1_from_top1_jetIndex(st1->model_lj1_from_top1_jetIndex());
	  
	  combinedResult->setModel_lj2_from_top1_pt(st1->model_lj2_from_top1_pt());
	  combinedResult->setModel_lj2_from_top1_eta(st1->model_lj2_from_top1_eta());
	  combinedResult->setModel_lj2_from_top1_phi(st1->model_lj2_from_top1_phi());
	  combinedResult->setModel_lj2_from_top1_E(st1->model_lj2_from_top1_E());
	  combinedResult->setModel_lj2_from_top1_jetIndex(st1->model_lj2_from_top1_jetIndex());
	  
	  combinedResult->setLogLikelihood_t2(st2->logLikelihood());
	  
	  combinedResult->setModel_b_from_top2_pt(st2->model_b_from_top1_pt());
	  combinedResult->setModel_b_from_top2_eta(st2->model_b_from_top1_eta());
	  combinedResult->setModel_b_from_top2_phi(st2->model_b_from_top1_phi());
	  combinedResult->setModel_b_from_top2_E(st2->model_b_from_top1_E());
	  combinedResult->setModel_b_from_top2_jetIndex(st2->model_b_from_top1_jetIndex());
	  
	  combinedResult->setModel_lj1_from_top2_pt(st2->model_lj1_from_top1_pt());
	  combinedResult->setModel_lj1_from_top2_eta(st2->model_lj1_from_top1_eta());
	  combinedResult->setModel_lj1_from_top2_phi(st2->model_lj1_from_top1_phi());
	  combinedResult->setModel_lj1_from_top2_E(st2->model_lj1_from_top1_E());
	  combinedResult->setModel_lj1_from_top2_jetIndex(st2->model_lj1_from_top1_jetIndex());
	  
	  combinedResult->setModel_lj2_from_top2_pt(st2->model_lj2_from_top1_pt());
	  combinedResult->setModel_lj2_from_top2_eta(st2->model_lj2_from_top1_eta());
	  combinedResult->setModel_lj2_from_top2_phi(st2->model_lj2_from_top1_phi());
	  combinedResult->setModel_lj2_from_top2_E(st2->model_lj2_from_top1_E());
	  combinedResult->setModel_lj2_from_top2_jetIndex(st2->model_lj2_from_top1_jetIndex());
	} else ATH_MSG_WARNING("KLFitterTool:: WARNING! Best permutation not found, an empty result is saved");
      }

      if (!evtStore()->tds()->contains<xAOD::KLFitterResultContainer>(outputSGKey)) {
        top::check(evtStore()->tds()->record(bestContainer,
                                             outputSGKey),
                   "KLFitterTools: ERROR! Was not able to write KLFitterResultContainer with best permutation");
        top::check(evtStore()->tds()->record(bestAuxContainer,
                                             outputSGKeyAux),
                   "KLFitterTools: ERROR! Was not able to write KLFitterResultAuxContainer with best permutation");
      }
      // watch out for memory leaks!
      // raw pointers have not been put into a DataVector
      // we still actually own them
      // AnalysisTop will actually do some memory management (which is very wierd and we don't like it)
      delete resultContainer;
      delete resultAuxContainer;
    }

    // Pull the const result back out of StoreGate and attach to top::Event
    top::check(evtStore()->retrieve(event.m_KLFitterResults, outputSGKey),
               "Failed to add KLFitterResults to top::Event");

    delete myParticles;

    /// Return gracefully:
    return StatusCode::SUCCESS;
  }

  bool KLFitterTool::HasTag(const xAOD::Jet& jet, double& weight) const {
    weight = -99.;

    for (const auto& tagWP : m_config->bTagWP_available()) {
      if (tagWP == "DL1_Continuous") continue;
      if (!jet.isAvailable<char>("isbtagged_" + tagWP)) {
        ATH_MSG_ERROR("Failed to retrieve jet decoration isbtagged_" + tagWP);
        break;
      }
      return jet.auxdataConst<char>("isbtagged_" + tagWP);
    }
    return false;
  }

  void KLFitterTool::retrieveEfficiencies(const xAOD::Jet& jet, float* efficiency, float* inefficiency) {
    *efficiency = .7725;        // dummy values
    *inefficiency = 1. / 125.93;  // dummy values
    //copy jet
    xAOD::JetContainer jets;
    xAOD::JetAuxContainer jetsAux;
    jets.setStore(&jetsAux);
    xAOD::Jet* jet_copy = new xAOD::Jet();
    jets.push_back(jet_copy);
    *jet_copy = jet;
    jet_copy->setJetP4(jet.jetP4());
    //treat jet as b-tagged
    jet_copy->setAttribute("HadronConeExclTruthLabelID", 5);
    top::check(m_btagging_eff_tool->getMCEfficiency(*jet_copy, *efficiency),
               "Could not retrieve tagging efficiency for b-jet");
    //treat jet as light
    jet_copy->setAttribute("HadronConeExclTruthLabelID", 0);
    top::check(m_btagging_eff_tool->getMCEfficiency(*jet_copy, *inefficiency),
               "Could not retrieve tagging efficiency for light jet");
  }

  bool KLFitterTool::setJets(const top::Event& event, KLFitter::Particles* inputParticles) {
    if(m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kAutoSet || m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kAutoSetPCBT) {
      ATH_MSG_DEBUG("Only set the lists of jets. Jet selection will be done together with combination loop.");
      return setJetLists(event);
    }
    if (m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kLeadingThree) return setJetskLeadingThree(event,
                                                                                                                inputParticles);



    if (m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kLeadingFour) return setJetskLeadingFour(event,
                                                                                                              inputParticles);



    if (m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kLeadingFive) return setJetskLeadingFive(event,
                                                                                                              inputParticles);



    if (m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kLeadingSix) return setJetskLeadingSix(event,
                                                                                                            inputParticles);



    if (m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kLeadingSeven) return setJetskLeadingSeven(event,
                                                                                                                inputParticles);



    if (m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kLeadingEight) return setJetskLeadingEight(event,
                                                                                                                inputParticles);



    if (m_jetSelectionModeKLFitterEnum ==
        top::KLFitterJetSelection::kBtagPriorityThreeJets) return setJetskBtagPriorityThreeJets(event, inputParticles);

    if (m_jetSelectionModeKLFitterEnum ==
        top::KLFitterJetSelection::kBtagPriorityFourJets) return setJetskBtagPriorityFourJets(event, inputParticles);

    if (m_jetSelectionModeKLFitterEnum ==
        top::KLFitterJetSelection::kBtagPriorityFiveJets) return setJetskBtagPriorityFiveJets(event, inputParticles);

    if (m_jetSelectionModeKLFitterEnum ==
        top::KLFitterJetSelection::kBtagPrioritySixJets) return setJetskBtagPrioritySixJets(event, inputParticles);

    if (m_jetSelectionModeKLFitterEnum ==
        top::KLFitterJetSelection::kBtagPrioritySevenJets) return setJetskBtagPrioritySevenJets(event, inputParticles);

    if (m_jetSelectionModeKLFitterEnum ==
        top::KLFitterJetSelection::kBtagPriorityEightJets) return setJetskBtagPriorityEightJets(event, inputParticles);

    return false;
  }

  bool KLFitterTool::setJetskLeadingThree(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskLeadingX(event, inputParticles, 3);
  }

  bool KLFitterTool::setJetskLeadingFour(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskLeadingX(event, inputParticles, 4);
  }

  bool KLFitterTool::setJetskLeadingFive(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskLeadingX(event, inputParticles, 5);
  }

  bool KLFitterTool::setJetskLeadingSix(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskLeadingX(event, inputParticles, 6);
  }

  bool KLFitterTool::setJetskLeadingSeven(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskLeadingX(event, inputParticles, 7);
  }

  bool KLFitterTool::setJetskLeadingEight(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskLeadingX(event, inputParticles, 8);
  }

  bool KLFitterTool::setJetskLeadingX(const top::Event& event, KLFitter::Particles* inputParticles,
                                      const unsigned int njets) {
    unsigned int index(0);

    //If container has less jets than required, raise error
    if (m_config->KLFitterFailOnLessThanXJets()) {
      if (event.m_jets.size() < njets) {
        ATH_MSG_INFO(
          "KLFitterTool::setJetskLeadingX: You required " << njets << " jets. Event has " << event.m_jets.size() <<
            " jets!");
        return false;
      }
    }
    for (const auto& jet : event.m_jets) {
      if (index > njets - 1) break;

      TLorentzVector jet_p4;
      jet_p4.SetPtEtaPhiE(jet->pt() / 1.e3, jet->eta(), jet->phi(), jet->e() / 1.e3);

      double weight(-99.);
      float eff(0), ineff(0);

      const bool isTagged = HasTag(*jet, weight);

      if (m_isWorkingPoint) {
        retrieveEfficiencies(*jet, &eff, &ineff);

        inputParticles->AddParticle(&jet_p4, jet_p4.Eta(), KLFitter::Particles::kParton, "", index,
                                    isTagged, eff, 1. / ineff, KLFitter::Particles::kNone, weight);
      } else {
        inputParticles->AddParticle(&jet_p4, jet_p4.Eta(), KLFitter::Particles::kParton, "", index, isTagged);
      }
      ++index;
    }
    return true;
  }

  bool KLFitterTool::setJetskBtagPriorityThreeJets(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskBtagPriority(event, inputParticles, 3);
  }

  bool KLFitterTool::setJetskBtagPriorityFourJets(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskBtagPriority(event, inputParticles, 4);
  }

  bool KLFitterTool::setJetskBtagPriorityFiveJets(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskBtagPriority(event, inputParticles, 5);
  }

  bool KLFitterTool::setJetskBtagPrioritySixJets(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskBtagPriority(event, inputParticles, 6);
  }

  bool KLFitterTool::setJetskBtagPrioritySevenJets(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskBtagPriority(event, inputParticles, 7);
  }

  bool KLFitterTool::setJetskBtagPriorityEightJets(const top::Event& event, KLFitter::Particles* inputParticles) {
    return setJetskBtagPriority(event, inputParticles, 8);
  }

  bool KLFitterTool::setJetskBtagPriority(const top::Event& event, KLFitter::Particles* inputParticles,
                                          const unsigned int maxJets) {
    // kBtagPriority mode first adds the b jets, then the light jets
    // If your 6th or 7th jet is a b jet, then you probably want this option

    //If container has less jets than required, raise error
    if (m_config->KLFitterFailOnLessThanXJets()) {
      if (event.m_jets.size() < maxJets) {
        ATH_MSG_INFO(
          "KLFitterTool::setJetskBtagPriority: You required " << maxJets << " jets. Event has " << event.m_jets.size() <<
            " jets!");
        return false;
      }
    }

    unsigned int totalJets(0);

    // First find the b-jets
    unsigned int index(0);
    double weight(0);
    for (const auto& jet : event.m_jets) {
      if (totalJets >= maxJets) break;
      if (HasTag(*jet, weight)) {
        TLorentzVector jet_p4;
        jet_p4.SetPtEtaPhiE(jet->pt() / 1.e3, jet->eta(), jet->phi(), jet->e() / 1.e3);

        if (m_isWorkingPoint) {
          float eff(0), ineff(0);
          retrieveEfficiencies(*jet, &eff, &ineff);

          inputParticles->AddParticle(&jet_p4, jet_p4.Eta(), KLFitter::Particles::kParton, "", index,
                                      true, eff, 1. / ineff, KLFitter::Particles::kNone, weight);
        } else {
          inputParticles->AddParticle(&jet_p4, jet_p4.Eta(), KLFitter::Particles::kParton, "", index,
                                      true);
        }
        ++totalJets;
      }  // HasTag

      ++index;
    }  // for (jet)


    // Second, find the light jets
    index = 0;
    for (const auto& jet : event.m_jets) {
      if (totalJets >= maxJets) break;
      if (!HasTag(*jet, weight)) {
        TLorentzVector jet_p4;
        jet_p4.SetPtEtaPhiE(jet->pt() / 1.e3, jet->eta(), jet->phi(), jet->e() / 1.e3);

        if (m_isWorkingPoint) {
          float eff(0), ineff(0);
          retrieveEfficiencies(*jet, &eff, &ineff);

          inputParticles->AddParticle(&jet_p4, jet_p4.Eta(), KLFitter::Particles::kParton, "", index,
                                      false, eff, 1. / ineff, KLFitter::Particles::kNone, weight);
        } else {
          inputParticles->AddParticle(&jet_p4, jet_p4.Eta(), KLFitter::Particles::kParton, "", index,
                                      false);
        }
        ++totalJets;
      }  // !HasTag

      ++index;
    }  // for (jet)
    return true;
  }

  bool KLFitterTool::setJetLists(const top::Event& event) {

    //first order in b-weight
    double weight(-99.);
    unsigned int index(0);
    std::vector<std::pair<unsigned int,std::pair<double,double> >* > bwOrd_jets(0);
    std::vector<std::vector<std::pair<unsigned int,std::pair<double,double> >* > > pcbtOrd_jets(5,std::vector<std::pair<unsigned int,std::pair<double,double> >* >(0));

    static SG::AuxElement::ConstAccessor<int> PVIndexAccessor("PVIndex"); //needed later
    int PVind=0;
    if(m_ordVar.find("NumTrkPt")!=std::string::npos && m_ordVar.find("SUM")==std::string::npos) {
      if ( PVIndexAccessor.isAvailable(*(event.m_info)) ) PVind = PVIndexAccessor(*(event.m_info)) ;
      else ATH_MSG_WARNING("KLFitterTool:: WARNING! Could not find PV from eventInfo: using index 0");
    }

    for (const auto& jet : event.m_jets) {
      if(m_isPCBT) { //bin 1=LF, bin 5=b-jet
	// get the PCBT bin
	long int PCBTbin=-2;
	if (!jet->isAvailable<int>("tagWeightBin_" + m_btagWP)) ATH_MSG_ERROR("Failed to retrieve jet decoration tagWeightBin_" + m_btagWP);
	else PCBTbin=-1+jet->auxdataConst<int>("tagWeightBin_" + m_btagWP);

	//get the ordering variable
	if(m_ordVar=="TrackSumPt") {
	  top::check(jet->getAttribute<double>("TrackSumPt", weight), "KLFitterTool ERROR could not find 'TrackSumPt' jet attribute");
	} else {
	  if (m_ordVar.find("NumTrkPt")!=std::string::npos) {
	    std::vector<int> nTrkVec;
	    
	    if(m_ordVar.find("500")!=std::string::npos) {
	      top::check(jet->getAttribute(xAOD::JetAttribute::NumTrkPt500, nTrkVec), "KLFitterTool ERROR could not find 'NumTrkPt500' jet attribute");
	    } else {
	      top::check(jet->getAttribute(xAOD::JetAttribute::NumTrkPt1000, nTrkVec), "KLFitterTool ERROR could not find 'NumTrkPt1000' jet attribute");
	    }
	    
	    if(m_ordVar.find("SUM")!=std::string::npos) {
	      weight=0;
	      for(int nt : nTrkVec) {
		weight+=(double)nt;
	      }
	    } 
	    else {
	      weight=nTrkVec.at(PVind);
	    }
	  } 
	  else {
	    weight=jet->pt();
	  }
	}

	//add info to the jet list
	if(PCBTbin>=0 && PCBTbin<static_cast<long int>(pcbtOrd_jets.size())) pcbtOrd_jets[PCBTbin].push_back(new std::pair<unsigned int,std::pair<double,double> >(index,std::make_pair(weight,jet->pt())));
      }
      else {
      // THIS METHOD DOES NOT SET THE WEIGHT!!!      HasTag(*jet, weight);
      //let's pick it by hand for now:
	const auto& btag_object = jet->btagging();
	const auto& tagger_name =m_btagging_eff_tool->getTaggerName();
	
	double mv_pb = 0;
	bool bw_found=false;
	if(tagger_name!="MV2c10" && tagger_name!="MV2c10mu" && tagger_name!="MV2c10rnn") {
	  btag_object->pb(tagger_name,mv_pb); 
	  weight=mv_pb;
	  bw_found=true;
	}
	else {
	  bw_found=btag_object->MVx_discriminant(tagger_name, weight);
	}
	if (!btag_object || !bw_found) {
	  ATH_MSG_ERROR("Failed to retrieve "+tagger_name+" weight!");
	}
	bwOrd_jets.push_back(new std::pair<unsigned int,std::pair<double,double> >(index,std::make_pair(weight,jet->pt())));
      }
      index++;
    }

    if(m_isPCBT) { //bin 1=LF, bin 5=b-jet  
      for(int bb=pcbtOrd_jets.size()-1;bb>=0;bb--) {
	//make sure that the jets are sorted in pt within each PCBT bin
	std::sort(pcbtOrd_jets[bb].begin(), pcbtOrd_jets[bb].end(),  [](const auto &left, const auto &right) { return left->second.first > right->second.first; });
	bwOrd_jets.insert(bwOrd_jets.end(),pcbtOrd_jets[bb].begin(),pcbtOrd_jets[bb].end());
      }
    } else {
      std::sort(bwOrd_jets.begin(), bwOrd_jets.end(), [](auto &left, auto &right) { return left->second.first > right->second.first; });
    }

    //then separate which jet can be what

    std::vector<std::pair<unsigned int,std::pair<double,double> > *> onlyBjets;
    std::vector<std::pair<unsigned int,std::pair<double,double> > *> canbeBOTHjets;
    std::vector<std::pair<unsigned int,std::pair<double,double> > *> onlyQjets;

    if(m_jetSelectionModeKLFitterEnum == top::KLFitterJetSelection::kAutoSetPCBT) {
      //first, assign by PCBT bin (pre-ordered)

      onlyBjets=pcbtOrd_jets[pcbtOrd_jets.size()-1];  //jets falling into bin 5 of pseudo-continuous b-tagging assigned to 'onlyBjets' list
     
      for(int bb=pcbtOrd_jets.size()-2;bb>0;bb--) canbeBOTHjets.insert(canbeBOTHjets.end(),pcbtOrd_jets[bb].begin(),pcbtOrd_jets[bb].end()); //jets falling into bins 4 to 2 of pseudo-continuous b-tagging assigned to 'canbeBOTHjets' list (can be passed to KLFitter as either a b or a light flavour jet, to test both hypothesis)

      onlyQjets=pcbtOrd_jets[0];  //jets falling into bin 1 of pseudo-continuous b-tagging assigned to 'onlyQjets' list (considered light flavour)

      //then, correct for too few b's
      int availB=onlyBjets.size()+canbeBOTHjets.size();
      if(availB<m_Nbmin) {
	top::check(static_cast<int>(onlyQjets.size())>(m_Nbmin-availB),Form("Too few jets to run KLFitter with this parameters: AutoSetPCBT, Nbmin=%d, n. jets=%d",static_cast<int>(m_Nbmin),static_cast<int>(event.m_jets.size()))); //protect against out-of-range
	canbeBOTHjets.insert(canbeBOTHjets.end(),onlyQjets.begin(),onlyQjets.begin()+(m_Nbmin-availB));
	onlyQjets.erase(onlyQjets.begin(),onlyQjets.begin()+(m_Nbmin-availB));  
      }

      //then, correct for too many b's (i.e. too few LF)
      int availBonly=onlyBjets.size();
      if(m_N5max>0 && availBonly>m_N5max) {
	canbeBOTHjets.insert(canbeBOTHjets.end(),onlyBjets.end()-(availBonly-m_N5max),onlyBjets.end());
	onlyBjets.erase(onlyBjets.end()-(availBonly-m_N5max),onlyBjets.end());
      }
    }
    else {
      onlyBjets=std::vector<std::pair<unsigned int,std::pair<double,double> >* >(bwOrd_jets.begin(),bwOrd_jets.begin()+m_nb);
      canbeBOTHjets=std::vector<std::pair<unsigned int,std::pair<double,double> >* >(bwOrd_jets.begin()+m_nb,bwOrd_jets.begin()+m_nb+m_delta);
      onlyQjets=std::vector<std::pair<unsigned int,std::pair<double,double> >* >(bwOrd_jets.begin()+m_nb+m_delta,bwOrd_jets.end()); 
    }

    //now cut away excess jets (if any)
    //always re-order the LF in pt, before cutting
    std::sort(onlyQjets.begin(), onlyQjets.end(), [](auto &left, auto &right) { return left->second.second > right->second.second; }); 

    //remove the extra LF jets: 
    int NLFjcut=m_Njcut-(onlyBjets.size()+canbeBOTHjets.size());
    if(static_cast<int>(onlyQjets.size())>NLFjcut) onlyQjets.erase(onlyQjets.begin()+NLFjcut,onlyQjets.end());

    //now merge everything to prepare the lists
    std::vector<std::pair<unsigned int,std::pair<double,double> >* > Bjets = onlyBjets;
    Bjets.insert(Bjets.end(),canbeBOTHjets.begin(),canbeBOTHjets.end());
    std::vector<std::pair<unsigned int,std::pair<double,double> >* > LFjets = onlyQjets;
    LFjets.insert(LFjets.end(),canbeBOTHjets.begin(),canbeBOTHjets.end());

    //re-sort in pt because we like it more (won't change a thing on how KLFitter loops)
    std::sort(Bjets.begin(), Bjets.end(), [](auto &left, auto &right) { return left->second.second > right->second.second; }); 
    std::sort(LFjets.begin(), LFjets.end(), [](auto &left, auto &right) { return left->second.second > right->second.second; }); 

    //now fill the output lists
    m_canBeBJets.clear();
    for(uint jj=0;jj<Bjets.size();jj++) m_canBeBJets.push_back(Bjets.at(jj)->first);
    m_canBeLFJets.clear();
    for(uint jj=0;jj<LFjets.size();jj++) m_canBeLFJets.push_back(LFjets.at(jj)->first);

    return (m_canBeBJets.size()>=2 && m_canBeLFJets.size()>=4);
  }


  bool KLFitterTool::setJetsFromAutoSet(const top::Event& event,KLFitter::Particles* inputParticles,std::vector<uint> BjetsToRun,std::vector<uint> LFjetsToRun)
  {
    uint expectedNB = 2;
    uint expectedNLF = 4;
    if(m_LHType=="ttbar_AllHadronic_SingleT") {
      expectedNB=1;
      expectedNLF=2;
    }
    if(BjetsToRun.size()!=expectedNB || LFjetsToRun.size()!=expectedNLF) {
      ATH_MSG_INFO("BjetsToRun.size()!=" << expectedNB << " || LFjetsToRun.size()!=" << expectedNLF);
      return false;
    }

    TLorentzVector jet_p4;
    for(auto bji : BjetsToRun) {
      if(bji>event.m_jets.size()) {
	ATH_MSG_INFO("b-jet index too large");
	return false;
      }
      auto jet = event.m_jets.at(bji);
      jet_p4.SetPtEtaPhiE(jet->pt() / 1.e3, jet->eta(), jet->phi(), jet->e() / 1.e3);
      double weight(999);
      HasTag(*jet, weight); 
      inputParticles->AddParticle(&jet_p4, 
				  jet_p4.Eta(), 
				  KLFitter::Particles::kParton, 
				  "", 
				  bji,
				  true);
    }

    for(auto qji : LFjetsToRun) {
      if(qji>event.m_jets.size()) {
	ATH_MSG_INFO("LF-jet index too large");
	return false;
      }
      auto jet = event.m_jets.at(qji);
      jet_p4.SetPtEtaPhiE(jet->pt() / 1.e3, jet->eta(), jet->phi(), jet->e() / 1.e3);

      double weight(0);
      HasTag(*jet, weight);
      inputParticles->AddParticle(&jet_p4, 
				  jet_p4.Eta(), 
				  KLFitter::Particles::kParton, 
				  "", 
				  qji,
				  false);
    }
    return true;
  }



  void KLFitterTool::permutationLoopStandard(xAOD::KLFitterResult* result,xAOD::KLFitterResultContainer* resultContainer) {
    const int nperm = m_myFitter->Permutations()->NPermutations();
    for (int iperm = 0; iperm < nperm; ++iperm) {
      // Perform the fit
      m_myFitter->Fit(iperm);

      // create a result
      result = new xAOD::KLFitterResult {};                                                                                                                                          
      resultContainer->push_back(result); 

      //Set name hash. This is because it seems std::string is not supported by AuxContainers...
      std::hash<std::string> hash_string;
      result->setSelectionCode(hash_string(m_selectionName));

      unsigned int ConvergenceStatusBitWord = m_myFitter->ConvergenceStatus();
      bool MinuitDidNotConverge = (ConvergenceStatusBitWord & m_myFitter->MinuitDidNotConvergeMask) != 0;
      bool FitAbortedDueToNaN = (ConvergenceStatusBitWord & m_myFitter->FitAbortedDueToNaNMask) != 0;
      bool AtLeastOneFitParameterAtItsLimit =
        (ConvergenceStatusBitWord & m_myFitter->AtLeastOneFitParameterAtItsLimitMask) != 0;
      bool InvalidTransferFunctionAtConvergence =
        (ConvergenceStatusBitWord & m_myFitter->InvalidTransferFunctionAtConvergenceMask) != 0;

      result->setMinuitDidNotConverge(((MinuitDidNotConverge) ? 1 : 0));
      result->setFitAbortedDueToNaN(((FitAbortedDueToNaN) ? 1 : 0));
      result->setAtLeastOneFitParameterAtItsLimit(((AtLeastOneFitParameterAtItsLimit) ? 1 : 0));
      result->setInvalidTransferFunctionAtConvergence(((InvalidTransferFunctionAtConvergence) ? 1 : 0));

      result->setLogLikelihood(m_myFitter->Likelihood()->LogLikelihood(m_myFitter->Likelihood()->GetBestFitParameters()));
      result->setEventProbability(std::exp(m_myFitter->Likelihood()->LogEventProbability()));
      result->setParameters(m_myFitter->Likelihood()->GetBestFitParameters());
      result->setParameterErrors(m_myFitter->Likelihood()->GetBestFitParameterErrors());

      KLFitter::Particles* myModelParticles = m_myFitter->Likelihood()->ParticlesModel();
      KLFitter::Particles** myPermutedParticles = m_myFitter->Likelihood()->PParticlesPermuted();

      if (m_LHType == "ttbar" || m_LHType == "ttH" || m_LHType == "ttbar_JetAngles" || m_LHType == "ttbar_Angular" ||
          m_LHType == "ttZTrilepton" || m_LHType == "ttbar_BoostedLJets") {
        result->setModel_bhad_pt(myModelParticles->Parton(0)->Pt());
        result->setModel_bhad_eta(myModelParticles->Parton(0)->Eta());
        result->setModel_bhad_phi(myModelParticles->Parton(0)->Phi());
        result->setModel_bhad_E(myModelParticles->Parton(0)->E());
        result->setModel_bhad_jetIndex((*myPermutedParticles)->JetIndex(0));

        result->setModel_blep_pt(myModelParticles->Parton(1)->Pt());
        result->setModel_blep_eta(myModelParticles->Parton(1)->Eta());
        result->setModel_blep_phi(myModelParticles->Parton(1)->Phi());
        result->setModel_blep_E(myModelParticles->Parton(1)->E());
        result->setModel_blep_jetIndex((*myPermutedParticles)->JetIndex(1));

        result->setModel_lq1_pt(myModelParticles->Parton(2)->Pt());
        result->setModel_lq1_eta(myModelParticles->Parton(2)->Eta());
        result->setModel_lq1_phi(myModelParticles->Parton(2)->Phi());
        result->setModel_lq1_E(myModelParticles->Parton(2)->E());
        result->setModel_lq1_jetIndex((*myPermutedParticles)->JetIndex(2));

        // boosted likelihood has only one light jet
        if (m_LHType != "ttbar_BoostedLJets") {
          result->setModel_lq2_pt(myModelParticles->Parton(3)->Pt());
          result->setModel_lq2_eta(myModelParticles->Parton(3)->Eta());
          result->setModel_lq2_phi(myModelParticles->Parton(3)->Phi());
          result->setModel_lq2_E(myModelParticles->Parton(3)->E());
          result->setModel_lq2_jetIndex((*myPermutedParticles)->JetIndex(3));

          if (m_LHType == "ttH") {
            result->setModel_Higgs_b1_pt(myModelParticles->Parton(4)->Pt());
            result->setModel_Higgs_b1_eta(myModelParticles->Parton(4)->Eta());
            result->setModel_Higgs_b1_phi(myModelParticles->Parton(4)->Phi());
            result->setModel_Higgs_b1_E(myModelParticles->Parton(4)->E());
            result->setModel_Higgs_b1_jetIndex((*myPermutedParticles)->JetIndex(4));

            result->setModel_Higgs_b2_pt(myModelParticles->Parton(5)->Pt());
            result->setModel_Higgs_b2_eta(myModelParticles->Parton(5)->Eta());
            result->setModel_Higgs_b2_phi(myModelParticles->Parton(5)->Phi());
            result->setModel_Higgs_b2_E(myModelParticles->Parton(5)->E());
            result->setModel_Higgs_b2_jetIndex((*myPermutedParticles)->JetIndex(5));
          }
        }

        if (m_leptonTypeKLFitterEnum == KLFitter::LikelihoodTopLeptonJets::LeptonType::kElectron ||
            m_leptonTypeKLFitterEnum_TTH == KLFitter::LikelihoodTTHLeptonJets::LeptonType::kElectron ||
            m_leptonTypeKLFitterEnum_TTZ == KLFitter::LikelihoodTTZTrilepton::LeptonType::kElectron ||
            m_leptonTypeKLFitterEnum_BoostedLJets == KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kElectron ||
            m_leptonTypeKLFitterEnum_JetAngles == KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kElectron ||
            m_leptonTypeKLFitterEnum_Angular == KLFitter::LikelihoodTopLeptonJets_Angular::LeptonType::kElectron) {
          result->setModel_lep_pt(myModelParticles->Electron(0)->Pt());
          result->setModel_lep_eta(myModelParticles->Electron(0)->Eta());
          result->setModel_lep_phi(myModelParticles->Electron(0)->Phi());
          result->setModel_lep_E(myModelParticles->Electron(0)->E());

          if (m_leptonType == "kTriElectron") {
            result->setModel_lep_index((*myPermutedParticles)->ElectronIndex(0));

            result->setModel_lepZ1_pt(myModelParticles->Electron(1)->Pt());
            result->setModel_lepZ1_eta(myModelParticles->Electron(1)->Eta());
            result->setModel_lepZ1_phi(myModelParticles->Electron(1)->Phi());
            result->setModel_lepZ1_E(myModelParticles->Electron(1)->E());
            result->setModel_lepZ1_index((*myPermutedParticles)->ElectronIndex(1));

            result->setModel_lepZ2_pt(myModelParticles->Electron(2)->Pt());
            result->setModel_lepZ2_eta(myModelParticles->Electron(2)->Eta());
            result->setModel_lepZ2_phi(myModelParticles->Electron(2)->Phi());
            result->setModel_lepZ2_E(myModelParticles->Electron(2)->E());
            result->setModel_lepZ2_index((*myPermutedParticles)->ElectronIndex(2));
          }
        }

        if (m_leptonTypeKLFitterEnum == KLFitter::LikelihoodTopLeptonJets::LeptonType::kMuon ||
            m_leptonTypeKLFitterEnum_TTH == KLFitter::LikelihoodTTHLeptonJets::LeptonType::kMuon ||
            m_leptonTypeKLFitterEnum_TTZ == KLFitter::LikelihoodTTZTrilepton::LeptonType::kMuon ||
            m_leptonTypeKLFitterEnum_BoostedLJets == KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kMuon ||
            m_leptonTypeKLFitterEnum_JetAngles == KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kMuon ||
            m_leptonTypeKLFitterEnum_Angular == KLFitter::LikelihoodTopLeptonJets_Angular::LeptonType::kMuon) {
          result->setModel_lep_pt(myModelParticles->Muon(0)->Pt());
          result->setModel_lep_eta(myModelParticles->Muon(0)->Eta());
          result->setModel_lep_phi(myModelParticles->Muon(0)->Phi());
          result->setModel_lep_E(myModelParticles->Muon(0)->E());

          if (m_leptonType == "kTriMuon") {
            result->setModel_lep_index((*myPermutedParticles)->MuonIndex(0));

            result->setModel_lepZ1_pt(myModelParticles->Muon(1)->Pt());
            result->setModel_lepZ1_eta(myModelParticles->Muon(1)->Eta());
            result->setModel_lepZ1_phi(myModelParticles->Muon(1)->Phi());
            result->setModel_lepZ1_E(myModelParticles->Muon(1)->E());
            result->setModel_lepZ1_index((*myPermutedParticles)->MuonIndex(1));

            result->setModel_lepZ2_pt(myModelParticles->Muon(2)->Pt());
            result->setModel_lepZ2_eta(myModelParticles->Muon(2)->Eta());
            result->setModel_lepZ2_phi(myModelParticles->Muon(2)->Phi());
            result->setModel_lepZ2_E(myModelParticles->Muon(2)->E());
            result->setModel_lepZ2_index((*myPermutedParticles)->MuonIndex(2));
          }
        }

        result->setModel_nu_pt(myModelParticles->Neutrino(0)->Pt());
        result->setModel_nu_eta(myModelParticles->Neutrino(0)->Eta());
        result->setModel_nu_phi(myModelParticles->Neutrino(0)->Phi());
        result->setModel_nu_E(myModelParticles->Neutrino(0)->E());
      }  else if (m_LHType == "ttbar_AllHadronic_SingleT") {
        result->setModel_b_from_top1_pt(myModelParticles->Parton(0)->Pt());
        result->setModel_b_from_top1_eta(myModelParticles->Parton(0)->Eta());
        result->setModel_b_from_top1_phi(myModelParticles->Parton(0)->Phi());
        result->setModel_b_from_top1_E(myModelParticles->Parton(0)->E());
        result->setModel_b_from_top1_jetIndex((*myPermutedParticles)->JetIndex(0));

        result->setModel_lj1_from_top1_pt(myModelParticles->Parton(1)->Pt());
        result->setModel_lj1_from_top1_eta(myModelParticles->Parton(1)->Eta());
        result->setModel_lj1_from_top1_phi(myModelParticles->Parton(1)->Phi());
        result->setModel_lj1_from_top1_E(myModelParticles->Parton(1)->E());
        result->setModel_lj1_from_top1_jetIndex((*myPermutedParticles)->JetIndex(1));

        result->setModel_lj2_from_top1_pt(myModelParticles->Parton(2)->Pt());
        result->setModel_lj2_from_top1_eta(myModelParticles->Parton(2)->Eta());
        result->setModel_lj2_from_top1_phi(myModelParticles->Parton(2)->Phi());
        result->setModel_lj2_from_top1_E(myModelParticles->Parton(2)->E());
        result->setModel_lj2_from_top1_jetIndex((*myPermutedParticles)->JetIndex(2));
      } else if (m_LHType == "ttbar_AllHadronic") { 
        result->setModel_b_from_top1_pt(myModelParticles->Parton(0)->Pt());
        result->setModel_b_from_top1_eta(myModelParticles->Parton(0)->Eta());
        result->setModel_b_from_top1_phi(myModelParticles->Parton(0)->Phi());
        result->setModel_b_from_top1_E(myModelParticles->Parton(0)->E());
        result->setModel_b_from_top1_jetIndex((*myPermutedParticles)->JetIndex(0));

        result->setModel_b_from_top2_pt(myModelParticles->Parton(1)->Pt());
        result->setModel_b_from_top2_eta(myModelParticles->Parton(1)->Eta());
        result->setModel_b_from_top2_phi(myModelParticles->Parton(1)->Phi());
        result->setModel_b_from_top2_E(myModelParticles->Parton(1)->E());
        result->setModel_b_from_top2_jetIndex((*myPermutedParticles)->JetIndex(1));

        result->setModel_lj1_from_top1_pt(myModelParticles->Parton(2)->Pt());
        result->setModel_lj1_from_top1_eta(myModelParticles->Parton(2)->Eta());
        result->setModel_lj1_from_top1_phi(myModelParticles->Parton(2)->Phi());
        result->setModel_lj1_from_top1_E(myModelParticles->Parton(2)->E());
        result->setModel_lj1_from_top1_jetIndex((*myPermutedParticles)->JetIndex(2));

        result->setModel_lj2_from_top1_pt(myModelParticles->Parton(3)->Pt());
        result->setModel_lj2_from_top1_eta(myModelParticles->Parton(3)->Eta());
        result->setModel_lj2_from_top1_phi(myModelParticles->Parton(3)->Phi());
        result->setModel_lj2_from_top1_E(myModelParticles->Parton(3)->E());
        result->setModel_lj2_from_top1_jetIndex((*myPermutedParticles)->JetIndex(3));

        result->setModel_lj1_from_top2_pt(myModelParticles->Parton(4)->Pt());
        result->setModel_lj1_from_top2_eta(myModelParticles->Parton(4)->Eta());
        result->setModel_lj1_from_top2_phi(myModelParticles->Parton(4)->Phi());
        result->setModel_lj1_from_top2_E(myModelParticles->Parton(4)->E());
        result->setModel_lj1_from_top2_jetIndex((*myPermutedParticles)->JetIndex(4));

        result->setModel_lj2_from_top2_pt(myModelParticles->Parton(5)->Pt());
        result->setModel_lj2_from_top2_eta(myModelParticles->Parton(5)->Eta());
        result->setModel_lj2_from_top2_phi(myModelParticles->Parton(5)->Phi());
        result->setModel_lj2_from_top2_E(myModelParticles->Parton(5)->E());
        result->setModel_lj2_from_top2_jetIndex((*myPermutedParticles)->JetIndex(5));
      }
    }
  }


  bool KLFitterTool::permutationLoopAutoSet(xAOD::KLFitterResult* result,xAOD::KLFitterResultContainer* resultContainer,const top::Event& event) {
    //loop on jet combinations
   bool hasrun=false;
   for(uint ib1=0;ib1<m_canBeBJets.size();ib1++) {
	for(uint ib2=ib1+1;ib2<m_canBeBJets.size();ib2++) {
	  for(uint iq1=0;iq1<m_canBeLFJets.size();iq1++) {
	    if(m_canBeLFJets[iq1]==m_canBeBJets[ib1] || m_canBeLFJets[iq1]==m_canBeBJets[ib2]) continue;
	    for(uint iq2=iq1+1;iq2<m_canBeLFJets.size();iq2++) {
	      if(m_canBeLFJets[iq2]==m_canBeBJets[ib1] || m_canBeLFJets[iq2]==m_canBeBJets[ib2]) continue;
	      for(uint iq3=iq2+1;iq3<m_canBeLFJets.size();iq3++) {
		if(m_canBeLFJets[iq3]==m_canBeBJets[ib1] || m_canBeLFJets[iq3]==m_canBeBJets[ib2]) continue;
		for(uint iq4=iq3+1;iq4<m_canBeLFJets.size();iq4++) {
		  if(m_canBeLFJets[iq4]==m_canBeBJets[ib1] || m_canBeLFJets[iq4]==m_canBeBJets[ib2]) continue;

		  //these is the jet combination you are running on
		  std::vector<unsigned int> bjts = {m_canBeBJets[ib1],m_canBeBJets[ib2]};
		  std::vector<unsigned int> lfjts = {m_canBeLFJets[iq1], m_canBeLFJets[iq2], m_canBeLFJets[iq3], m_canBeLFJets[iq4]};

		  //set this jet combination as KLFitter inputs
		  KLFitter::Particles* myInputParticles = new KLFitter::Particles {};
		  if(!setJetsFromAutoSet(event,myInputParticles,bjts,lfjts)) {
		    ATH_MSG_INFO("KLFitterTool::execute: It was not possible to properly fill the jets in AutoSet mode");
		    return false;
		  }
		  //now pass the inputs to the fitter
		  if (!m_myFitter->SetParticles(myInputParticles)) {
		    ATH_MSG_ERROR("KLFitter: Error adding particles to fitter...");
		    return false;
		  }
		  //now do your permutation thing
		  permutationLoopStandard(result,resultContainer);

		  //check that you ran at least once
		  hasrun=true;

		  //and delete this
		  delete myInputParticles;
		}//iq4
	      }//iq3
	    }//iq2
	  }//iq1
	}//ib2
      }//ib1

   if(!hasrun) ATH_MSG_ERROR("WARNING   KLFitterRun:: KLF hasn't run. Something wrong with the combinatorics!");
   return hasrun;
  }


  bool KLFitterTool::permutationLoopAutoSetSingleT(xAOD::KLFitterResult* result,xAOD::KLFitterResultContainer* resultContainer,const top::Event& event) {    
    //loop on jet combinations
    bool hasrun=false;
    for(uint ib1=0;ib1<m_canBeBJets.size();ib1++) {
      for(uint iq1=0;iq1<m_canBeLFJets.size();iq1++) {
	if(m_canBeLFJets[iq1]==m_canBeBJets[ib1]) continue;
	for(uint iq2=iq1+1;iq2<m_canBeLFJets.size();iq2++) {
	  if(m_canBeLFJets[iq2]==m_canBeBJets[ib1]) continue;
	  
	  //these is the jet combination you are running on
	  std::vector<unsigned int> bjts = {m_canBeBJets[ib1]};
	  std::vector<unsigned int> lfjts = {m_canBeLFJets[iq1], m_canBeLFJets[iq2]};
	  
	  //set this jet combination as KLFitter inputs
	  KLFitter::Particles* myInputParticles = new KLFitter::Particles {};
	  if(!setJetsFromAutoSet(event,myInputParticles,bjts,lfjts)) {
	    ATH_MSG_INFO("KLFitterTool::execute: It was not possible to properly fill the jets in AutoSet mode");
	    return false;
	  }
	  //now pass the inputs to the fitter
	  if (!m_myFitter->SetParticles(myInputParticles)) {
	    ATH_MSG_ERROR("KLFitter: Error adding particles to fitter...");
	    return false;
	  }
	  //now do your permutation thing
	  permutationLoopStandard(result,resultContainer);
	  
	  //check that you ran at least once
	  hasrun=true;
	  
	  //and delete this
	  delete myInputParticles;
	}//iq2
      }//iq1
    }//ib1
    
    if(!hasrun) ATH_MSG_ERROR("WARNING   KLFitterTool:: KLF hasn't run. Something wrong with the combinatorics!");
    return hasrun;
  }
 
  void KLFitterTool::findBestPermInd_Standard(xAOD::KLFitterResultContainer* resultContainer,unsigned int& bestPermutation,float& sumEventProbability) {
    float bestEventProbability(0.);
    unsigned int iPerm(0);

    for (auto x : *resultContainer) {
      float prob = x->eventProbability();
      short minuitDidNotConverge = x->minuitDidNotConverge();
      short fitAbortedDueToNaN = x->fitAbortedDueToNaN();
      short atLeastOneFitParameterAtItsLimit = x->atLeastOneFitParameterAtItsLimit();
      short invalidTransferFunctionAtConvergence = x->invalidTransferFunctionAtConvergence();
      sumEventProbability += prob;
      ++iPerm;
      
      // check if the best value has the highest event probability AND converged
      if (minuitDidNotConverge) continue;
      if (fitAbortedDueToNaN) continue;
      if (atLeastOneFitParameterAtItsLimit) continue;
      if (invalidTransferFunctionAtConvergence) continue;
      
      if (prob > bestEventProbability) {
	bestEventProbability = prob;
	// Using iPerm -1 because it has already been incremented before
	bestPermutation = iPerm - 1;
      }
    }
    
  }

  void KLFitterTool::findBestPermInd_SingleT(xAOD::KLFitterResultContainer* resultContainer,unsigned int& bestPermutation1,unsigned int& bestPermutation2,float& sumEventProbability) {      

    float bestLLH(-1e5);

    for (unsigned int iPerm1=0; iPerm1<resultContainer->size(); iPerm1++) {
      xAOD::KLFitterResult *x = resultContainer->at(iPerm1);

      short minuitDidNotConverge_1 = x->minuitDidNotConverge();
      short fitAbortedDueToNaN_1 = x->fitAbortedDueToNaN();
      short atLeastOneFitParameterAtItsLimit_1 = x->atLeastOneFitParameterAtItsLimit();
      short invalidTransferFunctionAtConvergence_1 = x->invalidTransferFunctionAtConvergence();

      if(minuitDidNotConverge_1 || fitAbortedDueToNaN_1 || atLeastOneFitParameterAtItsLimit_1 || invalidTransferFunctionAtConvergence_1) continue;

      unsigned int ib1 = x->model_b_from_top1_jetIndex();
      unsigned int iljI1 = x->model_lj1_from_top1_jetIndex();
      unsigned int iljII1 = x->model_lj2_from_top1_jetIndex();
      float prob_1 = x->eventProbability();
      float log_1 = x->logLikelihood();

      for (unsigned int iPerm2=iPerm1+1;iPerm2<resultContainer->size(); iPerm2++) {

	xAOD::KLFitterResult *y = resultContainer->at(iPerm2);

	//first check that it's not the same jets
	unsigned int ib2 = y->model_b_from_top1_jetIndex();
	unsigned int iljI2 = y->model_lj1_from_top1_jetIndex();
	unsigned int iljII2 = y->model_lj2_from_top1_jetIndex();

	if(ib2==ib1 || ib2==iljI1 || ib2==iljII1 ||
	   iljI2==ib1 || iljI2==iljI1 || iljI2==iljII1 ||
	   iljII2==ib1 || iljII2==iljI1 || iljII2==iljII1) continue;

	//now check that this one is ok
	short minuitDidNotConverge_2 = y->minuitDidNotConverge();
	short fitAbortedDueToNaN_2 = y->fitAbortedDueToNaN();
	short atLeastOneFitParameterAtItsLimit_2 = y->atLeastOneFitParameterAtItsLimit();
	short invalidTransferFunctionAtConvergence_2 = y->invalidTransferFunctionAtConvergence();	
	
	// check if the best value has the highest event probability AND converged
	if(minuitDidNotConverge_2 || fitAbortedDueToNaN_2 || atLeastOneFitParameterAtItsLimit_2 || invalidTransferFunctionAtConvergence_2) continue;

	//compute sum event probability (to keep things the same as in the standard version)
	float prob = prob_1*y->eventProbability();
	sumEventProbability += prob;

	float log = log_1+y->logLikelihood();
	if (log > bestLLH) {
	  bestLLH = log;
	  bestPermutation1 = iPerm1;
	  bestPermutation2 = iPerm2;
	}
      }
    }
  }

  /// Function finalizing the tool
  StatusCode KLFitterTool::finalize() {

    /// Return gracefully:
    return StatusCode::SUCCESS;
  }
}
