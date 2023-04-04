/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JetJvtEfficiency/JetJvtEfficiency.h"
#include "AsgMessaging/StatusCode.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include <AsgTools/AsgToolConfig.h>
#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/SystematicVariation.h"
#include "PathResolver/PathResolver.h"
#include "xAODJet/JetContainer.h"
#include "TFile.h"


namespace CP {

static const SG::AuxElement::Decorator<char>  isPUDec("isJvtPU");
static const SG::AuxElement::ConstAccessor<float> acc_jetTiming("Timing");

JetJvtEfficiency::JetJvtEfficiency( const std::string& name): asg::AsgTool( name ),
  m_appliedSystEnum(NONE),
  m_NNJvtTool_handle("", this),
  m_h_JvtHist(nullptr),
  m_h_EffHist(nullptr),
  m_passJvtDecName(""),
  m_useMuBinsSF(false),
  m_useDummySFs(false),
  m_jvtCut(0),
  m_jvtCutBorder(0),
  m_jetJvtMomentAcc(nullptr),
  m_passJvtAcc(nullptr),
  m_jetEtaAcc(nullptr),
  m_passORAcc(nullptr),
  m_sfDec(nullptr),
  m_isHSDec(nullptr),
  m_isHSAcc(nullptr)
  {
    declareProperty( "TaggingAlg",                m_tagger = CP::JvtTagger::NNJvt                                 );
    declareProperty( "WorkingPoint",              m_wp = "Default"                                                );
    declareProperty( "SFFile",                    m_file = ""                                                     );
    declareProperty( "JetContainerName",          m_jetContainerName = "AntiKt4EMPFlowJets"                       );
    declareProperty( "ScaleFactorDecorationName", m_sf_decoration_name = ""                                       );
    declareProperty( "OverlapDecorator",          m_ORdec = ""                                                    );
    declareProperty( "JetEtaName",                m_jetEtaName   = "DetectorEta"                                  );
    declareProperty( "MaxPtForJvt",               m_maxPtForJvt   = 60e3                                          );
    declareProperty( "DoTruthReq",                m_doTruthRequirement   = true                                   );
    declareProperty( "TruthLabel",                m_isHS_decoration_name  = "isJvtHS"                             );
    declareProperty( "TruthJetContainerName",     m_truthJetContName = "AntiKt4TruthDressedWZJets"                );
    // Allow to configure NNJvt Tool directly instead via WP property
    declareProperty( "NNJvtParamFile",            m_NNJvtParamFile= ""                                            );
    declareProperty( "NNJvtCutFile",              m_NNJvtCutFile= ""                                              );
    // Legacy properties, kept for backwards compatibility
    declareProperty( "JetJvtMomentName",          m_jetJvtMomentName   = "Jvt"                                    );

}

StatusCode JetJvtEfficiency::initialize(){

  // Check if specified tagger is defined (Athena/Gaudi doesn't allow enums as properties so we make the conversion manually)
  m_taggingAlg = static_cast<CP::JvtTagger>(m_tagger);
  if (m_taggingAlg != CP::JvtTagger::NNJvt && m_taggingAlg != CP::JvtTagger::fJvt && m_taggingAlg != CP::JvtTagger::Jvt) {
    ATH_MSG_ERROR("Invalid Jvt tagger selected: " << m_tagger << "! Choose between CP::JvtTagger::NNJvt, CP::JvtTagger::fJvt and CP::JvtTagger::Jvt (deprecated).");
    return StatusCode::FAILURE;
  }

  // NN Jvt has been developed and tested only on EMPflow jets so far and R22 calibrations are only provided for that jet collection
  if (m_jetContainerName != "AntiKt4EMPFlowJets") {
        ATH_MSG_WARNING("Only AntiKt4EMPFlowJets are supported, use other jet collections at your own risk.");
  }

  ATH_CHECK(m_passJvtKey.initialize());

  // Configure for NNJvt mode
  if (m_taggingAlg == JvtTagger::NNJvt){

    ATH_MSG_INFO("Configuring JetJvtEfficiency tool for NNJvt algorithm.");

    // select cut file according to WP
    if (m_wp == "Default") { m_wp = "FixedEffPt"; }
    if (m_NNJvtCutFile.empty()) {
        if (m_wp == "FixedEffPt") {
                m_NNJvtCutFile = "NNJVT.Cuts.FixedEffPt.Offline.Nonprompt_All_MaxW.json";
        }
        else if (m_wp == "TightFwd") {
                m_NNJvtCutFile = "NNJVT.Cuts.TightFwd.Offline.Nonprompt_All_MaxWeight.json";
        }
        else {
          ATH_MSG_ERROR("Unkown NNJvt WP " << m_wp << ", choose between FixedEffPt (Default) and TightFwd.");
          return StatusCode::FAILURE;
        }
    }

    // point to latest recommendation if not explicitly specified
    if (m_file.empty()) {
      // no calibration in R22 yet so leave blank for now
      m_file = "DummySFs.root"; // for reference: should be of the form "JetJvtEfficiency/Moriond2018/JvtSFFile_EMTopoJets.root"
    }

    // verify applicability of supplied SF file
    if (m_file.find("DummySFs") == std::string::npos && m_file.find("NNJvtSFFile") == std::string::npos) {
      ATH_MSG_WARNING("Supplied SF file " << m_file << " doesn't seem to contain SFs for NNJvt, falling back to dummy SFs ...");
      m_file = "DummySFs.root";
    }

    // set a default SF decoration name if not explicitly specified
    if (m_sf_decoration_name.empty()){
      m_sf_decoration_name = "NNJvtSF";
    }

    if (m_NNJvtParamFile.empty()) {
      m_NNJvtParamFile = "NNJVT.Network.graph.Offline.Nonprompt_All_MaxWeight.json";
    }

    // setup the NNJvt tool for recalculating NNJvt scores
    if (m_NNJvtTool_handle.empty()) {
      asg::AsgToolConfig config_NNjvt ("JetPileupTag::JetVertexNNTagger/NNJvt");
      ATH_CHECK(config_NNjvt.setProperty("JetContainer", m_jetContainerName+"_NNJvtCopy"));
      ATH_CHECK(config_NNjvt.setProperty("NNParamFile", m_NNJvtParamFile));
      ATH_CHECK(config_NNjvt.setProperty("NNCutFile", m_NNJvtCutFile));
      ATH_CHECK(config_NNjvt.setProperty("SuppressInputDependence", true)); // otherwise decorations can't be accessed properly
      ATH_CHECK(config_NNjvt.makePrivateTool(m_NNJvtTool_handle));
    }
    ATH_CHECK(m_NNJvtTool_handle.retrieve());

    // NNJvt tool will decorate decision on jets that we can retrieve
    m_passJvtDecName = "NNJvtPass";

    // will not be used to evaluate the decisions but for completeness we will point to the correct moment
    m_jetJvtMomentName = "NNJvt";

    // configure NNJvt systematics
    if (!addAffectingSystematic(NNJvtEfficiencyUp,true)	|| !addAffectingSystematic(NNJvtEfficiencyDown,true)) {
	    ATH_MSG_ERROR("failed to set up NNJvt systematics");
	    return StatusCode::FAILURE;
    }
  }
  // configure for fJvt mode
  else if (m_taggingAlg == JvtTagger::fJvt){

    ATH_MSG_INFO("Configuring JetJvtEfficiency tool for fJvt algorithm.");

    // select fJvt cut according to WP
    if (m_wp == "Default") { m_wp = "Loose"; }
    if (m_wp == "Loose") {
      m_jvtCut = 0.5;
      }
    else if (m_wp == "Tight") {
      m_jvtCut = 0.4;
    }
    else {
      ATH_MSG_ERROR("Unkown fJvt WP, choose between Loose (Default) and Tight.");
      return StatusCode::FAILURE;
    }

    // point to latest recommendation if not explicitly specified
    if (m_file.empty()) {
      // point to R21 recommendations until we have calibrations available
       m_file = "JetJvtEfficiency/May2020/fJvtSFFile.EMPFlow.root";
    }

    // set a default SF decoration name if not explicitly specified
    if (m_sf_decoration_name.empty()){
      m_sf_decoration_name = "fJvtSF";
    }

    // fJvt uses mu vs pT binning
    m_useMuBinsSF = true;

    // fJvt scores are calculated at derivation level
    m_jetJvtMomentName = "DFCommonJets_fJvt";

    // configure fJvt systematics
    if (!addAffectingSystematic(fJvtEfficiencyUp,true)	|| !addAffectingSystematic(fJvtEfficiencyDown,true)) {
	    ATH_MSG_ERROR("failed to set up fJvt systematics");
	    return StatusCode::FAILURE;
    }
  }
  // configure for Jvt mode (deprecated)
  else {

    ATH_MSG_INFO("Configuring JetJvtEfficiency tool for Jvt algorithm.");
    ATH_MSG_WARNING("Jvt is deprecated in R22 and no calibrations will be provided, please move to NNJvt.");

    // configure cuts according to WP
    bool ispflow = (m_jetContainerName.find("EMPFlow") != std::string::npos);
    if (m_wp=="Default" && !ispflow) m_wp = "Medium";
    if (m_wp=="Default" && ispflow) m_wp = "Tight";

    m_jvtCutBorder = -2.;
    if (m_wp=="Loose" && !ispflow) m_jvtCut = 0.11;
    else if (m_wp=="Medium" && ispflow) m_jvtCut = 0.2;
    else if (m_wp=="Tight" && ispflow) m_jvtCut = 0.5;
    else if (m_wp=="Medium"){
      m_jvtCut = 0.59;
      m_jvtCutBorder = 0.11;
    }
    else if (m_wp=="Tight") m_jvtCut = 0.91;
    else if (m_wp=="None") {
      m_jvtCut = -2;
      m_maxPtForJvt = 0;
      m_wp = "Medium";
    }
    else {
      ATH_MSG_ERROR("Invalid jvt working point name");
      return StatusCode::FAILURE;
    }

    // point to latest recommendation if not explicitly specified
    if (m_file.empty()) {
      // point to R21 recommendations
      m_file = "JetJvtEfficiency/Moriond2018/JvtSFFile_EMTopoJets.root";
    }

    // set a default SF decoration name if not explicitly specified
    if (m_sf_decoration_name.empty()){
      m_sf_decoration_name = "JvtSF";
    }
  }

  if (m_file.find("DummySF") != std::string::npos) {
    m_useDummySFs = true;
    ATH_MSG_INFO("Using dummy SFs of 1 +/ 10% for NNJvt.");
  }

  // Configure for nominal systematics
  if (applySystematicVariation(CP::SystematicSet()) != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Could not configure for nominal settings");
    return StatusCode::FAILURE;
  }

  // configurable accessors/decorators
  m_jetJvtMomentAcc.reset(new SG::AuxElement::ConstAccessor<float>(m_jetJvtMomentName));
  if (!m_passJvtDecName.empty()){
    m_passJvtAcc.reset(new SG::AuxElement::ConstAccessor<char>(m_passJvtDecName));
  }
  m_jetEtaAcc.reset(new SG::AuxElement::ConstAccessor<float>(m_jetEtaName));
  m_sfDec.reset(new SG::AuxElement::Decorator< float>(m_sf_decoration_name));
  m_isHSDec.reset(new SG::AuxElement::Decorator<char>(m_isHS_decoration_name));
  m_isHSAcc.reset(new SG::AuxElement::ConstAccessor<char>(m_isHS_decoration_name));
  if (!m_ORdec.empty()) {
    m_passORAcc.reset(new SG::AuxElement::ConstAccessor<char>(m_ORdec));
  }

  if (!m_doTruthRequirement) ATH_MSG_WARNING ( "No truth requirement will be performed, which is not recommended.");

  if (m_file.empty() || m_useDummySFs) return StatusCode::SUCCESS;

  std::string filename = PathResolverFindCalibFile(m_file);
  if (filename.empty()){
    ATH_MSG_WARNING ( "Could NOT resolve file name " << m_file);
  }  else{
    ATH_MSG_INFO(" Path found = "<<filename);
  }

  std::unique_ptr<TFile> infile(TFile::Open( filename.c_str(), "READ" ));

  std::string histname = "Jvt" + m_wp;

  // Retrieve histogram containing SFs and their uncertainties
  m_h_JvtHist.reset( dynamic_cast<TH2*>(infile->Get(histname.c_str())) );
  if(!m_h_JvtHist){
    ATH_MSG_ERROR("SF histogram named " << histname  << " for WP " << m_wp << " does not exist! Please check recommendatations for supported WPs.");
    return StatusCode::FAILURE;
  }
  m_h_JvtHist->SetDirectory(0);

  // Retrieve histogram containing efficiencies and their uncertainties
  histname.replace(0,3,"Eff");
  m_h_EffHist.reset( dynamic_cast<TH2*>(infile->Get(histname.c_str())) );
  if(!m_h_EffHist){
    ATH_MSG_ERROR("Efficiency histogram named " << histname << " for WP " << m_wp << " does not exist! Please check recommendatations for supported WPs.");
    return StatusCode::FAILURE;
  }
  m_h_EffHist->SetDirectory(0);

  if(m_h_JvtHist.get()==nullptr || m_h_EffHist.get()==nullptr) {
    ATH_MSG_ERROR("Failed to retrieve histograms.");
    return StatusCode::FAILURE;
  }

  // Check consistency between calibration file and tool configuration
  if (m_h_JvtHist->GetXaxis()->GetBinUpEdge(m_h_JvtHist->GetNbinsX()) < m_maxPtForJvt) {
    ATH_MSG_WARNING("Supplied calibration file does not extend up to " << m_maxPtForJvt/1e3 << " GeV, please check configuration.");
  }

  return StatusCode::SUCCESS;
}

CorrectionCode JetJvtEfficiency::getEfficiencyScaleFactor( const xAOD::Jet& jet,float& sf ){
    if (!isInRange(jet)) {
      sf = 1;
      return CorrectionCode::OutOfValidityRange;
    }
    if (m_doTruthRequirement) {
        if(!m_isHSAcc->isAvailable(jet)) {
            ATH_MSG_ERROR("Truth tagging required but decoration not available. Please call JetJvtEfficiency::tagTruth(...) first.");
            return CorrectionCode::Error;
        } else {
            if (!(*m_isHSAcc)(jet)) {
                sf = 1;
                return CorrectionCode::Ok;
            }
        }
    }

    if (m_useDummySFs){
      float baseFactor = 1.0;
      float errorTerm = 0.1;

      if      (m_appliedSystEnum == NNJVT_EFFICIENCY_UP   || m_appliedSystEnum == FJVT_EFFICIENCY_UP   ) baseFactor += errorTerm;
      else if (m_appliedSystEnum == NNJVT_EFFICIENCY_DOWN || m_appliedSystEnum == FJVT_EFFICIENCY_DOWN ) baseFactor -= errorTerm;

      sf = baseFactor;
      return CorrectionCode::Ok;
    }

    int jetbin = 0;
    if( m_useMuBinsSF ){ // fJVT SFs use(pT,mu) binning
      const xAOD::EventInfo *eventInfo = nullptr;
      if ( evtStore()->retrieve(eventInfo, "EventInfo").isFailure() )
	{
	  ATH_MSG_ERROR(" Could not retrieve EventInfo ");
	  return CorrectionCode::Error;
	}
      jetbin = m_h_JvtHist->FindBin(jet.pt(),eventInfo->actualInteractionsPerCrossing());
    } else {
      jetbin = m_h_JvtHist->FindBin(jet.pt(),std::abs((*m_jetEtaAcc)(jet)));
    }

    float baseFactor = m_h_JvtHist->GetBinContent(jetbin);
    float errorTerm  = m_h_JvtHist->GetBinError(jetbin);

    if      (m_appliedSystEnum == NNJVT_EFFICIENCY_UP   || m_appliedSystEnum == FJVT_EFFICIENCY_UP   ) baseFactor += errorTerm;
    else if (m_appliedSystEnum == NNJVT_EFFICIENCY_DOWN || m_appliedSystEnum == FJVT_EFFICIENCY_DOWN ) baseFactor -= errorTerm;

    sf = baseFactor;
    return CorrectionCode::Ok;
}

CorrectionCode JetJvtEfficiency::getInefficiencyScaleFactor( const xAOD::Jet& jet,float& sf ){
    if (!isInRange(jet)) {
      sf = 1;
      return CorrectionCode::OutOfValidityRange;
    }
    if (m_doTruthRequirement) {
        if(!m_isHSAcc->isAvailable(jet)) {
            ATH_MSG_ERROR("Truth tagging required but decoration not available. Please call JetJvtEfficiency::tagTruth(...) first.");
            return CorrectionCode::Error;
        } else {
            if(!(*m_isHSAcc)(jet)) {
                sf = 1;
                return CorrectionCode::Ok;
            }
        }
    }

    if (m_useDummySFs){
      float baseFactor = 1.0;
      float errorTerm = 0.1;

      if      (m_appliedSystEnum == NNJVT_EFFICIENCY_UP   || m_appliedSystEnum == FJVT_EFFICIENCY_UP   ) baseFactor += errorTerm;
      else if (m_appliedSystEnum == NNJVT_EFFICIENCY_DOWN || m_appliedSystEnum == FJVT_EFFICIENCY_DOWN ) baseFactor -= errorTerm;

      sf = baseFactor;
      return CorrectionCode::Ok;
    }

    int jetbin = 0;
    if( m_useMuBinsSF ){
      const xAOD::EventInfo *eventInfo = nullptr;
      if ( evtStore()->retrieve(eventInfo, "EventInfo").isFailure() )
	{
	  ATH_MSG_ERROR(" Could not retrieve EventInfo ");
	  return CorrectionCode::Error;
	}
      jetbin = m_h_JvtHist->FindBin(jet.pt(),eventInfo->actualInteractionsPerCrossing());
    } else {
      jetbin = m_h_JvtHist->FindBin(jet.pt(),std::abs((*m_jetEtaAcc)(jet)));
    }

    float baseFactor = m_h_JvtHist->GetBinContent(jetbin);
    float effFactor = m_h_EffHist->GetBinContent(jetbin);
    float errorTerm  = m_h_JvtHist->GetBinError(jetbin);
    float errorEffTerm  = m_h_EffHist->GetBinError(jetbin);

    if      (m_appliedSystEnum == NNJVT_EFFICIENCY_UP   || m_appliedSystEnum == FJVT_EFFICIENCY_UP   ) baseFactor += errorTerm;
    else if (m_appliedSystEnum == NNJVT_EFFICIENCY_DOWN || m_appliedSystEnum == FJVT_EFFICIENCY_DOWN ) baseFactor -= errorTerm;

    if      (m_appliedSystEnum == NNJVT_EFFICIENCY_UP   || m_appliedSystEnum == FJVT_EFFICIENCY_UP   ) effFactor += errorEffTerm;
    else if (m_appliedSystEnum == NNJVT_EFFICIENCY_DOWN || m_appliedSystEnum == FJVT_EFFICIENCY_DOWN ) effFactor -= errorEffTerm;

    sf = (1-baseFactor*effFactor)/(1-effFactor);
    return CorrectionCode::Ok;
}

CorrectionCode JetJvtEfficiency::applyEfficiencyScaleFactor(const xAOD::Jet& jet) {
    float sf = 0;
    CorrectionCode result = this->getEfficiencyScaleFactor(jet,sf);
    (*m_sfDec)(jet) = sf;
    return result;
}

CorrectionCode JetJvtEfficiency::applyInefficiencyScaleFactor(const xAOD::Jet& jet) {
    float sf = 0;
    CorrectionCode result = this->getInefficiencyScaleFactor(jet,sf);
    (*m_sfDec)(jet) = sf;
    return result;
}

CorrectionCode JetJvtEfficiency::applyAllEfficiencyScaleFactor(const xAOD::IParticleContainer *jets,float& sf) {
  sf = 1;
  const xAOD::JetContainer *truthJets = nullptr;
  if( evtStore()->retrieve(truthJets, m_truthJetContName).isFailure()) {
    ATH_MSG_ERROR("Unable to retrieve truth jet container with name " << m_truthJetContName);
    return CP::CorrectionCode::Error;

  }
  if(!truthJets || tagTruth(jets,truthJets).isFailure()) {
    ATH_MSG_ERROR("Unable to match truthJets to jets in tagTruth() method");
    return CP::CorrectionCode::Error;
  }
  for(const auto ipart : *jets) {
    if (ipart->type()!=xAOD::Type::Jet) {
      ATH_MSG_ERROR("Input is not a jet");
      return CP::CorrectionCode::Error;
    }
    const xAOD::Jet *jet = static_cast<const xAOD::Jet*>(ipart);
    float current_sf = 0;

    CorrectionCode result;
    if (passesJvtCut(*jet)) {
      result = this->getEfficiencyScaleFactor(*jet,current_sf);
    }
    else {
      result = this->getInefficiencyScaleFactor(*jet,current_sf);
    }

    if (result == CP::CorrectionCode::Error) {
      ATH_MSG_ERROR("Inexplicably failed JVT calibration" );
      return result;
    }
    (*m_sfDec)(*jet) = current_sf;
    sf *= current_sf;
  }
  return CorrectionCode::Ok;
}

StatusCode JetJvtEfficiency::decorate(const xAOD::JetContainer& jets) const{
  SG::WriteDecorHandle<xAOD::JetContainer, char> passJvtHandle(m_passJvtKey);
  for(const xAOD::Jet* jet : jets)
    passJvtHandle(*jet) = passesJvtCut(*jet);
  return StatusCode::SUCCESS;
}

StatusCode JetJvtEfficiency::recalculateScores(const xAOD::JetContainer& jets) const{
  // recalculate NNJvt scores and passNNJvt decorations
  if (m_taggingAlg == JvtTagger::NNJvt) {
    ATH_CHECK(m_NNJvtTool_handle->decorate(jets));
    return StatusCode::SUCCESS;
  }
  // not required for other taggers, so return gently
  else {
    return StatusCode::SUCCESS;
  }
}

bool JetJvtEfficiency::passesJvtCut(const xAOD::Jet& jet) const {
  ATH_MSG_DEBUG("In JetJvtEfficiency::passesJvtCut ()");
  if (!isInRange(jet)) return true;
  // Jvt (deprecated)
  if (m_taggingAlg == JvtTagger::Jvt) {
    if (std::abs((*m_jetEtaAcc)(jet))>2.4 && std::abs((*m_jetEtaAcc)(jet))<2.5) return (*m_jetJvtMomentAcc)(jet)>m_jvtCutBorder;
    return (*m_jetJvtMomentAcc)(jet)>m_jvtCut;
  }
  // NNJvt
  else if (m_taggingAlg == JvtTagger::NNJvt) {
    return bool((*m_passJvtAcc)(jet));
  }
  // fJvt
  else {
    return ((*m_jetJvtMomentAcc)(jet) <= m_jvtCut) && (std::abs(acc_jetTiming(jet)) <= 10.0);
  }

}

bool JetJvtEfficiency::isInRange(const xAOD::Jet& jet) const {
  // if defined, require jet to pass OR flag
  if (m_passORAcc && !(*m_passORAcc)(jet)) return false;
  if ( m_useMuBinsSF ){
    if (std::abs((*m_jetEtaAcc)(jet))<2.5) return false;
    if (std::abs((*m_jetEtaAcc)(jet))>4.5) return false;
  } else {
    // in case a SF file has been specified look up if eta of jet is covered by the file
    if (!m_useDummySFs) {
      if (std::abs((*m_jetEtaAcc)(jet))<m_h_JvtHist->GetYaxis()->GetBinLowEdge(1)) return false;
      if (std::abs((*m_jetEtaAcc)(jet))>m_h_JvtHist->GetYaxis()->GetBinUpEdge(m_h_JvtHist->GetNbinsY())) return false;
    }
    // in case dummy SFs are used (NNJvt only), manually restrict pile-up jets to central region
    else {
      if (std::abs((*m_jetEtaAcc)(jet))>2.5) return false;
    }
  }
  // skip check of histograms when using dummy SFs as no histograms have been loaded
  if (!m_useDummySFs){
    if (jet.pt()<m_h_JvtHist->GetXaxis()->GetBinLowEdge(1)) return false;
    if (jet.pt()>m_h_JvtHist->GetXaxis()->GetBinUpEdge(m_h_JvtHist->GetNbinsX())) return false;
  }
  else {
    if (jet.pt() < 20e3) return false;
  }
  if (jet.pt()>m_maxPtForJvt) return false;
  return true;
}

StatusCode JetJvtEfficiency::sysApplySystematicVariation(const CP::SystematicSet& systSet){
  m_appliedSystEnum = NONE;
  if (systSet.size()==0) {
    ATH_MSG_DEBUG("No affecting systematics received.");
    return StatusCode::SUCCESS;
  } else if (systSet.size()>1) {
    ATH_MSG_WARNING("Tool does not support multiple systematics, returning unsupported" );
    return StatusCode::FAILURE;
  }
  SystematicVariation systVar = *systSet.begin();
  if (systVar == SystematicVariation("")) m_appliedSystEnum = NONE;
  else if (m_taggingAlg == JvtTagger::NNJvt && systVar == NNJvtEfficiencyUp   ) m_appliedSystEnum = NNJVT_EFFICIENCY_UP;
  else if (m_taggingAlg == JvtTagger::NNJvt && systVar == NNJvtEfficiencyDown ) m_appliedSystEnum = NNJVT_EFFICIENCY_DOWN;
  else if (m_taggingAlg == JvtTagger::fJvt  && systVar == fJvtEfficiencyUp    ) m_appliedSystEnum = FJVT_EFFICIENCY_UP;
  else if (m_taggingAlg == JvtTagger::fJvt  && systVar == fJvtEfficiencyDown  ) m_appliedSystEnum = FJVT_EFFICIENCY_DOWN;
  else m_appliedSystEnum = NONE;

  ATH_MSG_DEBUG("applied systematic is " << m_appliedSystEnum);
  return StatusCode::SUCCESS;
}

StatusCode JetJvtEfficiency::tagTruth(const xAOD::IParticleContainer *jets,const xAOD::IParticleContainer *truthJets) {
    for(const auto jet : *jets) {
      bool ishs = false;
      bool ispu = true;
      for(const auto tjet : *truthJets) {
        if (tjet->p4().DeltaR(jet->p4())<0.3 && tjet->pt()>10e3) ishs = true;
        if (tjet->p4().DeltaR(jet->p4())<0.6) ispu = false;
      }
      (*m_isHSDec)(*jet)=ishs;
      isPUDec(*jet)=ispu;
    }
    return StatusCode::SUCCESS;
  }

} /* namespace CP */
