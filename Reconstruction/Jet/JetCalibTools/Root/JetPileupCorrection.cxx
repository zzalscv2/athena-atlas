/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <TEnv.h>
#include "JetCalibTools/IJetCalibrationTool.h"

#include "JetCalibTools/CalibrationMethods/JetPileupCorrection.h"
#include "JetCalibTools/CalibrationMethods/ResidualOffsetCorrection.h"

#include "PathResolver/PathResolver.h"
#include "PUResidual3DCorrection.h"

JetPileupCorrection::JetPileupCorrection()
  : JetCalibrationStep::JetCalibrationStep(),
    m_config(NULL), m_jetAlgo(""), m_calibAreaTag(""), m_dev(false), m_doResidual(false),
    m_doJetArea(false), m_doOrigin(false), m_isData(false),
    m_useFull4vectorArea(false), m_residualOffsetCorr(NULL), m_originScale("JetOriginConstitScaleMomentum")
{}

JetPileupCorrection::JetPileupCorrection(const std::string& name, TEnv* config, TString jetAlgo, TString calibAreaTag,
                                         bool doResidual, bool doJetArea, bool doOrigin, const std::string& originScale,
                                         bool isData, bool dev)
  : JetCalibrationStep::JetCalibrationStep(name.c_str()),
    m_config(config), m_jetAlgo(jetAlgo), m_calibAreaTag(calibAreaTag), m_dev(dev),
    m_doResidual(doResidual), m_doJetArea(doJetArea), m_doOrigin(doOrigin), m_isData(isData),
    m_useFull4vectorArea(false), m_residualOffsetCorr(NULL), m_originScale(originScale)
{}

JetPileupCorrection::~JetPileupCorrection() {

  if(m_residualOffsetCorr) delete m_residualOffsetCorr;

}


StatusCode JetPileupCorrection::initialize() {

  ATH_MSG_INFO("Initializing pileup correction.");

  if (m_doOrigin) ATH_MSG_INFO("OriginScale: " << m_originScale);

  if(!m_config){
    ATH_MSG_ERROR("Jet pileup correction tool received a null config pointer.");
    return StatusCode::FAILURE;
  }

  m_doOnlyResidual = m_config->GetValue("ApplyOnlyResidual",false);
  if(m_doJetArea && m_doOnlyResidual){
    ATH_MSG_FATAL("If you're trying to apply only the Residual pile up correction, it needs to be specify in the Calibration Sequence. ApplyOnlyResidual should be true in the configuration file and the PileupStartScale should be specified.");
    return StatusCode::FAILURE;
  }

  m_doMuOnly             = m_config->GetValue("ApplyOnlyMuResidual",false);
  m_doNPVOnly            = m_config->GetValue("ApplyOnlyNPVResidual",false);
  m_doNJetOnly           = m_config->GetValue("ApplyOnlyNJetResidual",false);
  m_doSequentialResidual = m_config->GetValue("DoSequentialResidual",false); // first mu and then NPV/NJet corrections
  bool useNjet           = m_config->GetValue("OffsetCorrection.UseNjet", false);

  m_do3Dcorrection       = m_config->GetValue("Do3DCorrection", false);

  if(m_doSequentialResidual) ATH_MSG_DEBUG("The pileup residual calibrations will be applied sequentially.");
  else                       ATH_MSG_DEBUG("The pileup residual calibrations will be applied simultaneously (default).");
  if(m_doMuOnly)             ATH_MSG_INFO("Only the pileup mu-based calibration will be applied.");
  if(m_doNPVOnly)            ATH_MSG_INFO("Only the pileup NPV-based calibration will be applied.");
  if(m_doNJetOnly)           ATH_MSG_INFO("Only the pileup NJet-based calibration will be applied.");
  else if (useNjet)          ATH_MSG_DEBUG("NJet will be used instead of NPV in the pileup corrections.");

  // Protections
  if(m_doSequentialResidual && (m_doMuOnly || m_doNPVOnly || m_doNJetOnly)){
    ATH_MSG_FATAL("Sequential residual calibration can not be applied in doMuOnly or doNPVOnly or doNJetOnly cases.");
    return StatusCode::FAILURE;
  }
  if(useNjet && (m_doMuOnly || m_doNPVOnly)){
    ATH_MSG_FATAL("Conflicting configuation, UseNjet true but doMuOnly or doNPVOnly also true.");
    return StatusCode::FAILURE;
  }
  if(m_doMuOnly && m_doNPVOnly){
    ATH_MSG_FATAL("It was requested to apply only the mu-based AND the NPV-based calibrations.");
    return StatusCode::FAILURE;
  }
  if(m_doMuOnly && m_doNJetOnly){
    ATH_MSG_FATAL("It was requested to apply only the mu-based AND the NJet-based calibrations.");
    return StatusCode::FAILURE;
  }
  if(!useNjet && m_doNJetOnly){
    ATH_MSG_FATAL("It was requested to apply only the NJet-based calibration but not to use Njet instead of NPV.");
    return StatusCode::FAILURE;
  }
  if(m_doNJetOnly && m_doNPVOnly){
    ATH_MSG_FATAL("It was requested to apply NJet-based and NPV calibrations.");
    return StatusCode::FAILURE;
  }

  if(m_do3Dcorrection && (m_doSequentialResidual || m_doMuOnly || m_doNPVOnly || m_doNJetOnly ) ){
    ATH_MSG_FATAL("3D correction incompatible with any other PU correction. Please turn off any PU residual options.");
    return StatusCode::FAILURE;

  }
  
  m_jetStartScale = m_config->GetValue("PileupStartingScale","JetConstitScaleMomentum");
  ATH_MSG_INFO("JetPileupCorrection: Starting scale: " << m_jetStartScale);
  if ( m_jetStartScale.compare("DO_NOT_USE") == 0 ) {
    ATH_MSG_WARNING("Configuration file does not specify the jet starting scale!");
  }

  m_useFull4vectorArea = m_config->GetValue("ApplyFullJetArea4MomentumCorrection", false);
  if(m_doJetArea) ATH_MSG_INFO("Jet area pile up correction will be applied.");
  if ( m_useFull4vectorArea ) ATH_MSG_INFO("  Full 4-vector jet area correction is activated."); 
  //ATH_MSG_INFO(" \n");

  if(m_do3Dcorrection){
    m_residual3DCorr.reset( new PUCorrection::PU3DCorrectionHelper() ) ;

    TString PUCalibFile3D = m_config->GetValue("PU3DCorrection.constants", "pu3DResidualsConstants.root");

    if(m_dev){
      //Currently hard coded that we remove "JetCalibTools/CalibrationFactors/" from the string in dev mode
      //Same implementation as in other JetCalibTools classes for now, will be changed everywhere during major overhaul of package for r22
      PUCalibFile3D.Remove(0,33);
      PUCalibFile3D.Insert(0,"JetCalibTools/");
    }
    else{
      PUCalibFile3D.Insert(14,m_calibAreaTag);
    }

    const std::string calibFilePU = PathResolverFindCalibFile(PUCalibFile3D.Data());

    m_residual3DCorr->loadParameters(calibFilePU);
    m_residual3DCorr->m_rhoEnergyScale = m_config->GetValue("PU3DCorrection.rhoEnergyScale", 0.001);
    m_residual3DCorr->m_pTEnergyScale = m_config->GetValue("PU3DCorrection.pTEnergyScale", 0.001);
    m_residual3DCorr->m_applyDeltaPtTerm = m_config->GetValue("PU3DCorrection.applyDeltaPtTerm", true);
    ATH_MSG_INFO("Pile-up 3D correction. Configured with :");
    ATH_MSG_INFO("  calib constants file="<< m_config->GetValue("PU3DCorrection.constants", "pu3DResidualsConstants.root") );
    ATH_MSG_INFO("  rho scale ="<<m_residual3DCorr->m_rhoEnergyScale );
    ATH_MSG_INFO("  pT scale ="<<m_residual3DCorr->m_pTEnergyScale);
    ATH_MSG_INFO("  apply deltaPt term = " << m_residual3DCorr->m_applyDeltaPtTerm);
  } else if ( m_doResidual ) {
    std::string suffix = "_Residual";
    m_residualOffsetCorr = new ResidualOffsetCorrection(m_name+suffix,m_config,m_jetAlgo,m_calibAreaTag,m_isData,m_dev);
    m_residualOffsetCorr->msg().setLevel( this->msg().level() );
    ATH_CHECK( m_residualOffsetCorr->initialize() );
  }

  if ( m_doResidual && m_useFull4vectorArea ) { 
    ATH_MSG_WARNING("JetPileupCorrection::initializeTool : WARNING!! You have requested the 4 vector jet area correction and the residual offset correction. This configuration is not currently supported, the residual offset correction will be deactivated.");
     return StatusCode::SUCCESS;
  } else if ( !m_doResidual && !m_useFull4vectorArea ) {
    ATH_MSG_VERBOSE("JetPileupCorrection::initializeTool : You have requested the transverse jet area correction without the residual offset correction. This configuration is not recommended.");
    return StatusCode::SUCCESS;
  } else { 
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

StatusCode JetPileupCorrection::calibrate(xAOD::Jet& jet, JetEventInfo& jetEventInfo) const {
  ATH_MSG_VERBOSE("  Applying pileup calibration to jet " << jet.index());
  ATH_MSG_VERBOSE("    Initial jet pT = " << 0.001*jet.pt() << " GeV");

  xAOD::JetFourMom_t jetStartP4;
  ATH_CHECK( setStartP4(jet) );
  jetStartP4 = jet.jetP4();

  const double E_det = jetStartP4.e();
  const double pT_det = jetStartP4.pt();
  const double eta_det = jetStartP4.eta();
  const double mass_det = jetStartP4.mass();

  if ( E_det < mass_det ) {
    ATH_MSG_WARNING( "JetPileupCorrection::calibrateImpl : Current jet has mass=" << mass_det/m_GeV << " GeV, which is greater than it's energy=" << E_det/m_GeV << " GeV?? Aborting." );
    return StatusCode::FAILURE;
  }

  xAOD::JetFourMom_t jetareaP4 = jet.getAttribute<xAOD::JetFourMom_t>("ActiveArea4vec");
  ATH_MSG_VERBOSE("    Area = " << jetareaP4);
  const double rho = jetEventInfo.rho();
  ATH_MSG_VERBOSE("    Rho = " << rho);

  if(m_do3Dcorrection){
    int NPV = jetEventInfo.NPV();
    float mu  = jetEventInfo.mu();

    double pt_calib= m_residual3DCorr->correctedPt(pT_det,  eta_det, jetareaP4.Pt(), rho, mu, NPV ) ;
    double scaleF = pt_calib < 0 ? 0.01*m_GeV/pT_det : pt_calib/pT_det;
    xAOD::JetFourMom_t calibP4 = jetStartP4 * scaleF;
    jet.setAttribute<int>("PileupCorrected",true);
    jet.setAttribute<xAOD::JetFourMom_t>("JetPileupScaleMomentum",calibP4);
    jet.setJetP4( calibP4 );

  } else if (m_useFull4vectorArea) {
    ATH_MSG_VERBOSE("  Applying area-subtraction calibration to jet " << jet.index() << " with pT = " << 0.001*jet.pt() << " GeV");
    //subtract rho * the jet area from the jet
    xAOD::JetFourMom_t calibP4 = jetStartP4 - rho*jetareaP4;
    //Attribute to track if a jet has received the pileup subtraction (always true if this code was run)
    jet.setAttribute<int>("PileupCorrected",true);
    //Transfer calibrated jet properties to the Jet object
    jet.setAttribute<xAOD::JetFourMom_t>("JetPileupScaleMomentum",calibP4);
    jet.setJetP4( calibP4 );

  } else if ( m_residualOffsetCorr && !m_useFull4vectorArea ) {
    ATH_MSG_VERBOSE("  Applying residual pileup calibration to jet " << jet.index() << " with pT = " << 0.001*jet.pt() << " GeV");

    const double NPV = jetEventInfo.NPV();
    const double mu  = jetEventInfo.mu();
    const int nJet   = jetEventInfo.nJet();

    // Retrieve the offset correction from the residual correction class
    double offsetET  = 0;  // pT residual subtraction
    double pT_offset = pT_det; // pT difference before/after pileup corrections
    double pileup_SF = 1; // final calibration factor applied to the four vector

    xAOD::JetFourMom_t calibP4;
    if(!m_doSequentialResidual){ // Default, both corrections are applied simultaneously
      offsetET = m_residualOffsetCorr->GetResidualOffset(fabs(eta_det), mu, NPV, nJet, m_doMuOnly, m_doNPVOnly||m_doNJetOnly);

      // Calculate the pT after jet areas and residual offset
      pT_offset = m_doJetArea ? pT_det - rho*jetareaP4.pt() - offsetET : pT_det - offsetET;

      // Set the jet pT to 10 MeV if the pT is negative after the jet area and residual offset corrections
      pileup_SF = pT_offset >= 0 ? pT_offset / pT_det : 0.01*m_GeV/pT_det;

      if ( m_doOrigin ) {
        xAOD::JetFourMom_t jetOriginP4;
        static std::atomic<unsigned int> originWarnings = 0;
        if ( jet.getAttribute<xAOD::JetFourMom_t>(m_originScale.c_str(),jetOriginP4) )
          calibP4 = jetOriginP4*pileup_SF;
        else {
          if ( originWarnings < 20 ) {
            ATH_MSG_WARNING("Could not retrieve " << m_originScale << " jet attribute, origin correction will not be applied.");
            ++originWarnings;
          }
          calibP4 = jetStartP4*pileup_SF;
        }
      } else {
        calibP4 = jetStartP4*pileup_SF;
      }
    } else {

      // Calculate mu-based correction factor
      offsetET    = m_residualOffsetCorr->GetResidualOffset(fabs(eta_det), mu, NPV, nJet, true, false);
      pT_offset   = m_doJetArea ? pT_det - rho*jetareaP4.pt() - offsetET : pT_det - offsetET;
      double muSF = pT_offset >= 0 ? pT_offset / pT_det : 0.01*m_GeV/pT_det;

      // Apply mu-based calibration
      if ( m_doOrigin ) {
        xAOD::JetFourMom_t jetOriginP4;
        static std::atomic<unsigned int> originWarnings = 0;
        if ( jet.getAttribute<xAOD::JetFourMom_t>(m_originScale.c_str(),jetOriginP4) )
          calibP4 = jetOriginP4*muSF;
        else {
          if ( originWarnings < 20 ) {
            ATH_MSG_WARNING("Could not retrieve " << m_originScale << " jet attribute, origin correction will not be applied.");
            ++originWarnings;
          }
          calibP4 = jetStartP4*muSF;
        }
      } else {
        calibP4 = jetStartP4*muSF;
      }

      // Calculate and apply NPV/Njet-based calibration
      offsetET = m_residualOffsetCorr->GetResidualOffset(fabs(eta_det), mu, NPV, nJet, false, true);
      double pT_afterMuCalib = calibP4.pt();
      pT_offset = pT_afterMuCalib - offsetET;
      double SF = pT_offset >= 0 ? pT_offset / pT_afterMuCalib : 0.01*m_GeV/pT_afterMuCalib;
      calibP4   = calibP4*SF;

    }

    //Attribute to track if a jet has received the origin correction
    jet.setAttribute<int>("OriginCorrected",m_doOrigin);
    //Attribute to track if a jet has received the pileup subtraction (always true if this code was run)
    jet.setAttribute<int>("PileupCorrected",true);
    
    //Transfer calibrated jet properties to the Jet object
    jet.setAttribute<xAOD::JetFourMom_t>("JetPileupScaleMomentum",calibP4);
    jet.setJetP4( calibP4 );

  } else {
    ATH_MSG_VERBOSE("  Applying postive-only area-subtraction calibration to jet " << jet.index() << " with pT = " << 0.001*jet.pt() << " GeV");
    //Set the jet pT to 10 MeV if the pT or energy is negative after the jet area correction
    const double area_SF = (pT_det-rho*jetareaP4.pt()<=0 || E_det-rho*jetareaP4.e()<=0) ? 10/pT_det : (pT_det-rho*jetareaP4.pt())/pT_det;
    xAOD::JetFourMom_t calibP4;
    if ( m_doOrigin ) {
      xAOD::JetFourMom_t jetOriginP4;
      static std::atomic<unsigned int> originWarnings = 0;
      if ( jet.getAttribute<xAOD::JetFourMom_t>(m_originScale.c_str(),jetOriginP4) )
	calibP4 = jetOriginP4*area_SF;
      else {
	if ( originWarnings < 20 ) {
	  ATH_MSG_WARNING("Could not retrieve " << m_originScale << " jet attribute, origin correction will not be applied.");
	  ++originWarnings;
	}
	calibP4 = jetStartP4*area_SF;
      }
    } else calibP4 = jetStartP4*area_SF;

    //Attribute to track if a jet has received the origin correction
    jet.setAttribute<int>("OriginCorrected",m_doOrigin);
    //Attribute to track if a jet has received the pileup subtraction (always true if this code was run)
    jet.setAttribute<int>("PileupCorrected",true);

    //Transfer calibrated jet properties to the Jet object
    jet.setAttribute<xAOD::JetFourMom_t>("JetPileupScaleMomentum",calibP4);
    jet.setJetP4( calibP4 );
  }

  ATH_MSG_VERBOSE("    Calibrated jet pT = " << 0.001*jet.pt() << " GeV");
  return StatusCode::SUCCESS;
}
