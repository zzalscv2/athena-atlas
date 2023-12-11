/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
  @class AsgElectronSelectorTool
  @brief Electron selector tool to select signal electrons using the ElectronDNNCalculator
         retrieve a score based on a DNN. This includes preprocessing/transformations of the
         input variables and the actual calculation of the score using lwtnn.

  @author Lukas Ehrke, Daniel Nielsen, Troels Petersen
  @note   heavily adapted from AsgElectronLikelihoodTool
*/

// Include this class's header
#include "ElectronPhotonSelectorTools/AsgElectronSelectorTool.h"
#include "ElectronPhotonSelectorTools/ElectronSelectorHelpers.h"
#include "EgammaAnalysisHelpers/AsgEGammaConfigHelper.h"
#include "EGSelectorConfigurationMapping.h"
#include "ElectronDNNCalculator.h"
// STL includes
#include <string>
#include <cstdint>
#include <cmath>

//EDM includes
#include "xAODEgamma/Electron.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "TEnv.h"

#include "AsgTools/CurrentContext.h"
#include "PathResolver/PathResolver.h"


//=============================================================================
// Standard constructor
//=============================================================================
AsgElectronSelectorTool::AsgElectronSelectorTool( const std::string& myname ) :
  AsgTool(myname),
  m_configFile{""},
  m_mvaTool(nullptr)
{

  // Declare the needed properties
  declareProperty("WorkingPoint", m_workingPoint="", "The Working Point");
  declareProperty("ConfigFile", m_configFile="", "The config file to use");

  // model file name. Managed in the ElectronDNNCalculator.
  declareProperty("inputModelFileName", m_modelFileName="", "The input file name that holds the model" );
  // QuantileTransformer file name ( required for preprocessing ). Managed in the ElectronDNNCalculator.
  declareProperty("quantileFileName", m_quantileFileName="", "The input file name that holds the QuantileTransformer");
  // Model used is a multiclass or a binary model
  declareProperty("multiClass", m_multiClass, "Whether the given model is multiclass or not");
  // DNN menu used is CF+ID or regular ID
  declareProperty("CFReject", m_CFReject, "Whether we use the DNN CF+ID approach or the regular DNN ID");
  // If multiclass, how to treat the chargeflip output node when combining into one discriminant
  declareProperty("cfSignal", m_cfSignal, "Whether to include the CF fraction in the numerator or denominator");
  // If multiclass, fractions with which the different output nodes get multiplied before combining them
  declareProperty("Fractions", m_fractions, "Fractions to combine the single outputs into one discriminant");
  // Variable list
  declareProperty("Variables", m_variables, "Variables used in the MVA tool");
  // The mva cut values
  declareProperty("CutSelector", m_cutSelector, "Cut on prompt electrons MVA discriminant");
  declareProperty("CutSelectorCF", m_cutSelectorCF, "Cut on CF electrons MVA discriminant");
  // do the ambiguity cut
  declareProperty("CutAmbiguity" , m_cutAmbiguity, "Apply a cut on the ambiguity bit");
  // cut on b-layer
  declareProperty("CutBL", m_cutBL, "Apply a cut on b-layer");
  // cut on pixel hits
  declareProperty("CutPi", m_cutPi, "Apply a cut on pixel hits");
  // cut on precision hits
  declareProperty("CutSCT", m_cutSCT, "Apply a cut on SCT hits");
  // use smooth interpolation between discriminant bins
  declareProperty("doSmoothBinInterpolation", m_doSmoothBinInterpolation, "use smooth interpolation between discriminant bins");
  // especially for  trigger electron
  declareProperty("skipDeltaPoverP",m_skipDeltaPoverP = false,"If true, it will skip the check of deltaPoverP");

  declareProperty("skipAmbiguityCut",m_skipAmbiguityCut = false,"If true, it will skip the ambiguity cut");
}


//=============================================================================
// Standard destructor
//=============================================================================
AsgElectronSelectorTool::~AsgElectronSelectorTool()
= default;


//=============================================================================
// Asgena initialize method
//=============================================================================
StatusCode AsgElectronSelectorTool::initialize()
{
  if (!m_workingPoint.empty()){
    m_configFile = AsgConfigHelper::findConfigFile(m_workingPoint, EgammaSelectors::ElectronDNNPointToConfFile);
    ATH_MSG_INFO("operating point : " << this->getOperatingPointName());
  }

  if (!m_configFile.empty()){
    std::string configFile = PathResolverFindCalibFile(m_configFile);
    if (configFile.empty()){
      ATH_MSG_ERROR("Could not locate " << m_configFile);
      return StatusCode::FAILURE;
    }


    ATH_MSG_DEBUG("Configfile to use: " << m_configFile);
    TEnv env;
    env.ReadFile(configFile.c_str(), kEnvLocal);

    std::string modelFilename("");
    std::string quantileFilename("");

    // Get the input model in the tool.
    ATH_MSG_DEBUG("Get the input model in the tool.");

    if (!m_modelFileName.empty()){  // If the property was set by the user, take that.
      ATH_MSG_INFO("Setting user specified Model file: " << m_modelFileName);
      modelFilename = m_modelFileName;
    }
    else {
      modelFilename = env.GetValue("inputModelFileName", "ElectronPhotonSelectorTools/offline/mc16_20210204/ElectronDNNNetwork.json");
      ATH_MSG_DEBUG("Getting the input Model from: " << modelFilename );
    }
    std::string filename = PathResolverFindCalibFile(modelFilename);
    if (filename.empty()){
      ATH_MSG_ERROR("Could not find model file " << modelFilename);
      return StatusCode::FAILURE;
    }

    // Get the input transformer in the tool.
    ATH_MSG_DEBUG("Get the input transformer in the tool.");

    if (!m_quantileFileName.empty()){  // If the property was set by the user, take that.
      ATH_MSG_INFO("Setting user specified QuantileTransformer file: " << m_quantileFileName);
      quantileFilename = m_quantileFileName;
    }
    else {
      quantileFilename = env.GetValue("inputQuantileFileName", "ElectronPhotonSelectorTools/offline/mc16_20210204/ElectronDNNQuantileTransformer.root");
      ATH_MSG_DEBUG("Getting the input QuantileTransformer from: " << quantileFilename);
    }
    std::string qfilename = PathResolverFindCalibFile(quantileFilename);
    if (qfilename.empty()){
        ATH_MSG_ERROR("Could not find QuantileTransformer file " << quantileFilename);
        return StatusCode::FAILURE;
    }

    // Variables used in the MVA tool as comma separated string;
    std::stringstream vars(env.GetValue("Variables", ""));
    // parse variables string into vector
    while(vars.good()){
      std::string substr;
      std::getline(vars, substr, ',');
      m_variables.push_back( substr );
    }

    // Model is multiclass or not, default is binary model
    m_multiClass = env.GetValue("multiClass", false);
    // Include cf node in numerator or denominator when combining different outputs
    m_cfSignal = env.GetValue("cfSignal", true);
    // Fractions to multiply different outputs with before combining
    m_fractions = AsgConfigHelper::HelperDouble("Fractions", env);

    // cut on MVA discriminant
    m_cutSelector = AsgConfigHelper::HelperDouble("CutSelector", env);
    m_cutSelectorCF = AsgConfigHelper::HelperDouble("CutSelectorCF", env);
    
    // cut on ambiuity bit
    m_cutAmbiguity = AsgConfigHelper::HelperInt("CutAmbiguity", env);
    // cut on b-layer
    m_cutBL = AsgConfigHelper::HelperInt("CutBL", env);
    // cut on pixel hits
    m_cutPi = AsgConfigHelper::HelperInt("CutPi", env);
    // cut on precision hits
    m_cutSCT = AsgConfigHelper::HelperInt("CutSCT", env);
    // do smooth interpolation between bins
    m_doSmoothBinInterpolation = env.GetValue("doSmoothBinInterpolation", false);



    unsigned int numberOfExpectedBinCombinedMVA ;
    numberOfExpectedBinCombinedMVA = s_fnDiscEtBins * s_fnDiscEtaBins;
    unsigned int numberOfExpectedEtaBins = s_fnDiscEtBins;

    if (m_cutSelector.size() != numberOfExpectedBinCombinedMVA){
      ATH_MSG_ERROR("Configuration issue :  cutSelector expected size " << numberOfExpectedBinCombinedMVA <<
                    " input size " << m_cutSelector.size());
      return StatusCode::FAILURE;
    }

    if (!m_cutSelectorCF.empty()){
      m_CFReject = true;
      if (m_cutSelectorCF.size() != numberOfExpectedBinCombinedMVA){
        ATH_MSG_ERROR("Configuration issue :  cutSelectorCF expected size " << numberOfExpectedBinCombinedMVA <<
                      " input size " << m_cutSelectorCF.size());
        return StatusCode::FAILURE;
      }
    }
    else {
      m_CFReject = false;
    }

    // Create an instance of the class calculating the DNN score
    m_mvaTool = std::make_unique<ElectronDNNCalculator>(this, filename.c_str(), qfilename.c_str(), m_variables, m_multiClass, m_CFReject);

    if (m_multiClass){
      // Fractions are only needed if multiclass model is used
      // There are five fractions for the combination, the signal fraction is either one (cfSignal == false) or 1 - cf fraction (cfSignal == true)
      if (m_fractions.size() != numberOfExpectedEtaBins * 5){
        ATH_MSG_ERROR("Configuration issue : multiclass but not the right amount of fractions." << m_fractions.size());
        return StatusCode::FAILURE;
      }
    }


    if (!m_cutSCT.empty()){
      if (m_cutSCT.size() != numberOfExpectedEtaBins){
        ATH_MSG_ERROR("Configuration issue :  cutSCT expected size " << numberOfExpectedEtaBins <<
                      " input size " << m_cutSCT.size());
        return StatusCode::FAILURE;
      }
    }

    if (!m_cutPi.empty()){
      if (m_cutPi.size() != numberOfExpectedEtaBins){
        ATH_MSG_ERROR("Configuration issue :  cutPi expected size " << numberOfExpectedEtaBins <<
                      " input size " << m_cutPi.size());
        return StatusCode::FAILURE;
      }
    }

    if (!m_cutBL.empty()){
      if (m_cutBL.size() != numberOfExpectedEtaBins){
        ATH_MSG_ERROR("Configuration issue :  cutBL expected size " << numberOfExpectedEtaBins <<
                      " input size " << m_cutBL.size());
        return StatusCode::FAILURE;
      }
    }

    if (!m_cutAmbiguity.empty()){
      if (m_cutAmbiguity.size() != numberOfExpectedEtaBins){
        ATH_MSG_ERROR("Configuration issue :  cutAmbiguity expected size " << numberOfExpectedEtaBins <<
                      " input size " << m_cutAmbiguity.size());
        return StatusCode::FAILURE;
      }
    }

    // --------------------------------------------------------------------------
    // Register the cuts and check that the registration worked:
    // NOTE: THE ORDER IS IMPORTANT!!! Cut0 corresponds to bit 0, Cut1 to bit 1,...
    // use an int as a StatusCode
    int sc(1);

    // Cut position for the kineatic pre-selection
    m_cutPosition_kinematic = m_acceptMVA.addCut("kinematic", "pass kinematic");
    if (m_cutPosition_kinematic < 0) sc = 0;

    // NSilicon
    m_cutPosition_NSilicon = m_acceptMVA.addCut("NSCT", "pass NSCT");
    if (m_cutPosition_NSilicon < 0) sc = 0;

    // NPixel
    m_cutPosition_NPixel = m_acceptMVA.addCut("NPixel", "pass NPixel");
    if (m_cutPosition_NPixel < 0) sc = 0;

    // NBlayer
    m_cutPosition_NBlayer = m_acceptMVA.addCut("NBlayer", "pass NBlayer");
    if (m_cutPosition_NBlayer < 0) sc = 0;

    // Ambiguity
    m_cutPosition_ambiguity = m_acceptMVA.addCut("ambiguity", "pass ambiguity");
    if (m_cutPosition_ambiguity < 0) sc = 0;


    // Cut position for the likelihood selection - DO NOT CHANGE ORDER!
    m_cutPosition_MVA = m_acceptMVA.addCut("passMVA", "pass MVA");
    if (m_cutPosition_MVA < 0) sc = 0;

    // Check that we got everything OK
    if (sc == 0){
      ATH_MSG_ERROR("ERROR: Something went wrong with the setup of the decision objects...");
      return StatusCode::FAILURE;
    }

  }
  else {  //Error if it cant find the conf
    ATH_MSG_ERROR("Could not find configuration file " << m_configFile);
    return StatusCode::FAILURE;
  }
  ///-----------End of text config----------------------------

  // define a default vector to return in the calculateMultipleOutputs methods
  // depending on the number of expected outputs
  if (m_multiClass){
    m_defaultVector = {-999., -999., -999., -999., -999., -999.};
  }
  else{
    m_defaultVector = {-999.};
  }

  return StatusCode::SUCCESS;
}


//=============================================================================
// return the accept info object
//=============================================================================

const asg::AcceptInfo& AsgElectronSelectorTool::getAcceptInfo() const
{
  return m_acceptMVA;
}

//=============================================================================
// The main accept method: the actual cuts are applied here
//=============================================================================
asg::AcceptData AsgElectronSelectorTool::accept( const EventContext& ctx, const xAOD::Electron* eg, double mu ) const
{
  ATH_MSG_VERBOSE("\t AsgElectronSelectorTool::accept( &ctx, *eg, mu= "<<(&ctx)<<", "<<eg<<", "<<mu<<" )");

  // Setup return accept with AcceptInfo
  asg::AcceptData acceptData(&m_acceptMVA);

  if (!eg){
    throw std::runtime_error("AsgElectronSelectorTool: Failed, no electron object was passed");
  }

  const xAOD::CaloCluster* cluster = eg->caloCluster();
  if (!cluster){
    ATH_MSG_DEBUG("exiting because cluster is NULL " << cluster);
    return acceptData;
  }

  if(!cluster->hasSampling(CaloSampling::CaloSample::EMB2) && !cluster->hasSampling(CaloSampling::CaloSample::EME2)){
    ATH_MSG_DEBUG("Failed, cluster is missing samplings EMB2 and EME2");
    return acceptData;
  }

  const double energy = cluster->e();
  const float eta = cluster->etaBE(2);

  if(isForwardElectron(eg, eta)){
    ATH_MSG_DEBUG("Failed, this is a forward electron! The AsgElectronSelectorTool is only suitable for central electrons!");
    return acceptData;
  }

  const xAOD::TrackParticle* track = eg->trackParticle();
  if (!track){
    ATH_MSG_DEBUG("exiting because track is NULL " << track);
    return acceptData;
  }

  // transverse energy of the electron (using the track eta)
  double et = (std::cosh(track->eta()) != 0.) ? energy / std::cosh(track->eta()) : 0.;

  // number of track hits
  uint8_t nSiHitsPlusDeadSensors(0);
  uint8_t nPixHitsPlusDeadSensors(0);
  bool passBLayerRequirement(false);
  uint8_t ambiguityBit(0);

  bool allFound = true;
  std::string notFoundList = "";

  // get the ambiguity type from the decoration
  if (!m_skipAmbiguityCut){
    if (eg->isAvailable<uint8_t>("ambiguityType")){
      static const SG::AuxElement::Accessor<uint8_t> acc("ambiguityType");
      ambiguityBit = acc(*eg);
    }
    else {
      allFound = false;
      notFoundList += "ambiguityType ";
    }
  } 

  nSiHitsPlusDeadSensors = ElectronSelectorHelpers::numberOfSiliconHitsAndDeadSensors(*track);
  nPixHitsPlusDeadSensors = ElectronSelectorHelpers::numberOfPixelHitsAndDeadSensors(*track);
  passBLayerRequirement = ElectronSelectorHelpers::passBLayerRequirement(*track);

  // calculate the output of the selector tool
  double mvaScore = calculate(ctx, eg, mu);
  ATH_MSG_VERBOSE(Form("PassVars: MVA=%8.5f, eta=%8.5f, et=%8.5f, nSiHitsPlusDeadSensors=%i, nHitsPlusPixDeadSensors=%i, passBLayerRequirement=%i, ambiguityBit=%i, mu=%8.5f",
                       mvaScore, eta, et,
                       nSiHitsPlusDeadSensors, nPixHitsPlusDeadSensors,
                       passBLayerRequirement,
                       ambiguityBit, mu));
  double mvaScoreCF = 0;
  if (m_CFReject){
    mvaScoreCF = calculateCF(ctx, eg, mu);
    ATH_MSG_VERBOSE(Form("PassVars: MVA=%8.5f, eta=%8.5f, et=%8.5f, nSiHitsPlusDeadSensors=%i, nHitsPlusPixDeadSensors=%i, passBLayerRequirement=%i, ambiguityBit=%i, mu=%8.5f",
                        mvaScoreCF, eta, et,
                        nSiHitsPlusDeadSensors, nPixHitsPlusDeadSensors,
                        passBLayerRequirement,
                        ambiguityBit, mu));
  }
  
  if (!allFound){
    throw std::runtime_error("AsgElectronSelectorTool: Not all variables needed for the decision are found. The following variables are missing: " + notFoundList );
  }

  // Set up the individual cuts
  bool passKine(true);
  bool passNSilicon(true);
  bool passNPixel(true);
  bool passNBlayer(true);
  bool passAmbiguity(true);
  bool passMVA(true);

  if (std::abs(eta) > 2.47){
    ATH_MSG_DEBUG("This electron is fabs(eta)>2.47 Returning False.");
    passKine = false;
  }

  unsigned int etBin = getDiscEtBin(et);
  unsigned int etaBin = getDiscEtaBin(eta);

  // sanity
  if (etBin >= s_fnDiscEtBins){
    ATH_MSG_DEBUG("Cannot evaluate model for Et " << et << ". Returning false..");
    passKine = false;
  }

  // Return if the kinematic requirements are not fulfilled
  acceptData.setCutResult(m_cutPosition_kinematic, passKine);
  if (!passKine){return acceptData;}

  // ambiguity bit
  if (!m_cutAmbiguity.empty()){
    if (!ElectronSelectorHelpers::passAmbiguity((xAOD::AmbiguityTool::AmbiguityType)ambiguityBit, m_cutAmbiguity[etaBin])){
      ATH_MSG_DEBUG("MVA macro: ambiguity Bit Failed.");
      passAmbiguity = false;
    }
  }

  // blayer cut
  if (!m_cutBL.empty()) {
    if(m_cutBL[etaBin] == 1 && !passBLayerRequirement){
      ATH_MSG_DEBUG("MVA macro: Blayer cut failed.");
      passNBlayer = false;
    }
  }
  // pixel cut
  if (!m_cutPi.empty()){
    if (nPixHitsPlusDeadSensors < m_cutPi[etaBin]){
      ATH_MSG_DEBUG("MVA macro: Pixels Failed.");
      passNPixel = false;
    }
  }
  // SCT cut
  if (!m_cutSCT.empty()){
    if (nSiHitsPlusDeadSensors < m_cutSCT[etaBin]){
      ATH_MSG_DEBUG( "MVA macro: Silicon Failed.");
      passNSilicon = false;
    }
  }

  unsigned int ibin_combinedMVA = etBin*s_fnDiscEtaBins+etaBin; // Must change if number of eta bins changes!.

  // First cut on the CF discriminant
  // If empty, continue only with prompt ID
  if (!m_cutSelectorCF.empty()){
    double cutDiscriminantCF;
    // To protect against a binning mismatch, which should never happen
    if (ibin_combinedMVA >= m_cutSelectorCF.size()){
      throw std::runtime_error("AsgElectronSelectorTool: The desired eta/pt bin is outside of the range specified by the input. This should never happen! This indicates a mismatch between the binning in the configuration file and the tool implementation." );
    }
    if (m_doSmoothBinInterpolation){
      cutDiscriminantCF = interpolateCuts(m_cutSelectorCF, et, eta);
    }
    else{
      cutDiscriminantCF = m_cutSelectorCF.at(ibin_combinedMVA);
    }
    // Determine if the calculated mva score value passes the combined cut
    ATH_MSG_DEBUG("MVA macro: CF Discriminant: ");
    if (mvaScoreCF < cutDiscriminantCF){
      ATH_MSG_DEBUG("MVA macro: CF cut failed.");
      passMVA = false;
    }
  }
  
// (Second) cut on prompt discriminant
  if (!m_cutSelector.empty()){
    double cutDiscriminant;
    // To protect against a binning mismatch, which should never happen
    if (ibin_combinedMVA >= m_cutSelector.size()){
      throw std::runtime_error("AsgElectronSelectorTool: The desired eta/pt bin is outside of the range specified by the input. This should never happen! This indicates a mismatch between the binning in the configuration file and the tool implementation." );
    }
    if (m_doSmoothBinInterpolation){
      cutDiscriminant = interpolateCuts(m_cutSelector, et, eta);
    }
    else{
      cutDiscriminant = m_cutSelector.at(ibin_combinedMVA);
    }
    // Determine if the calculated mva score value passes the combined cut
    ATH_MSG_DEBUG("MVA macro: Prompt Discriminant: "); 
    if (mvaScore < cutDiscriminant){
      ATH_MSG_DEBUG("MVA macro: Prompt cut failed.");
      passMVA = false;
    }
  }

  // Set the individual cut bits in the return object
  acceptData.setCutResult(m_cutPosition_NSilicon, passNSilicon);
  acceptData.setCutResult(m_cutPosition_NPixel, passNPixel);
  acceptData.setCutResult(m_cutPosition_NBlayer, passNBlayer);
  acceptData.setCutResult(m_cutPosition_ambiguity, passAmbiguity);
  acceptData.setCutResult(m_cutPosition_MVA, passMVA);

  return acceptData;

}

//=============================================================================
// The main result method: the actual mvaScore is calculated here
//=============================================================================
double AsgElectronSelectorTool::calculate( const EventContext& ctx, const xAOD::Electron* eg, double mu ) const
{
  // Get all outputs of the mva tool
  std::vector<float> mvaOutputs = calculateMultipleOutputs(ctx, eg, mu);

  double discriminant = 0;
  // If a binary model is used, vector will have one entry, if multiclass is used vector will have six entries
  if (!m_multiClass){
    discriminant = transformMLOutput(mvaOutputs.at(0));
  }
  else{
    const xAOD::CaloCluster* cluster = eg->caloCluster();
    const float eta = cluster->etaBE(2);
    // combine the six output nodes into one discriminant to cut on, any necessary transformation is applied within combineOutputs()
    discriminant = combineOutputs(mvaOutputs, eta);
  }

  return discriminant;
}

double AsgElectronSelectorTool::calculateCF( const EventContext& ctx, const xAOD::Electron* eg, double mu ) const
{
  // Get all outputs of the mva tool
  std::vector<float> mvaOutputs = calculateMultipleOutputs(ctx, eg, mu);

  double discriminant = 0;
  // If a binary model is used, vector will have one entry, if multiclass is used vector will have six entries
  if (!m_multiClass){
    discriminant = transformMLOutput(mvaOutputs.at(0));
  }
  else{
    // combine the six output nodes into one discriminant to cut on, any necessary transformation is applied within combineOutputs()
    discriminant = combineOutputsCF(mvaOutputs);
  }

  return discriminant;
}

std::vector<float> AsgElectronSelectorTool::calculateMultipleOutputs(const EventContext &ctx, const xAOD::Electron *eg, double mu) const
{
  ATH_MSG_VERBOSE("\t AsgElectronSelectorTool::calculateMultipleOutputs( &ctx, *eg, mu= "<<(&ctx)<<", "<<eg<<", "<<mu<<" )");
  if (!eg){
    throw std::runtime_error("AsgElectronSelectorTool: Failed, no electron object was passed" );
  }

  const xAOD::CaloCluster* cluster = eg->caloCluster();
  if (!cluster){
    ATH_MSG_DEBUG("Failed, no cluster.");
    // Return a default value
    return m_defaultVector;
  }

  if (!cluster->hasSampling(CaloSampling::CaloSample::EMB2) && !cluster->hasSampling(CaloSampling::CaloSample::EME2)){
    ATH_MSG_DEBUG("Failed, cluster is missing samplings EMB2 and EME2.");
    // Return a default value
    return m_defaultVector;
  }

  const double energy = cluster->e();
  const float eta = cluster->etaBE(2);

  if (isForwardElectron(eg, eta)){
    ATH_MSG_DEBUG("Failed, this is a forward electron! The AsgElectronSelectorTool is only suitable for central electrons!");
    // Return a default value
    return m_defaultVector;
  }

  const xAOD::TrackParticle* track = eg->trackParticle();
  if (!track){
    ATH_MSG_DEBUG("Failed, no track.");
    // Return a default value
    return m_defaultVector;
  }

  // transverse energy of the electron (using the track eta)
  const double et = energy / std::cosh(track->eta());

  // Variables used in the ML model
  // track quantities
  double  SCTWeightedCharge(0.0);
  uint8_t nSCTHitsPlusDeadSensors(0);
  uint8_t nPixHitsPlusDeadSensors(0);
  float d0(0.0), d0sigma(0.0), d0significance(0.0), qd0(0.0);
  float trackqoverp(0.0);
  double dPOverP(0.0);
  float TRT_PID(0.0);
  double trans_TRTPID(0.0);

  // Track Cluster matching
  float deltaEta1(0), deltaPhiRescaled2(0), EoverP(0);

  // Calorimeter
  float Reta(0), Rphi(0), Rhad1(0), Rhad(0), w2(0), f1(0), Eratio(0), f3(0), wtots1(0);

  bool allFound = true;
  std::string notFoundList = "";

  // retrieve track variables
  trackqoverp = track->qOverP();
  d0 = track->d0();
  qd0 = (eg->charge())*track->d0();
  float vard0 = track->definingParametersCovMatrix()(0, 0);
  if (vard0 > 0){
    d0sigma = std::sqrt(vard0);
  }
  d0significance = std::abs(d0 / d0sigma);

  const static SG::AuxElement::Accessor<float> trans_TRT_PID_acc("transformed_e_probability_ht");
  if (!trans_TRT_PID_acc.isAvailable(*eg)) {
    // most probable case, need to compute the variable

    if (!track->summaryValue(TRT_PID, xAOD::eProbabilityHT)) {
      allFound = false;
      notFoundList += "eProbabilityHT ";
    }

    // Transform the TRT PID output for use in the LH tool.
    const double tau = 15.0;
    const double fEpsilon = 1.0e-30; // to avoid zero division
    double pid_tmp = TRT_PID;
    if (pid_tmp >= 1.0)
      pid_tmp = 1.0 - 1.0e-15; // this number comes from TMVA
    else if (pid_tmp <= fEpsilon)
      pid_tmp = fEpsilon;
    trans_TRTPID = -std::log(1.0 / pid_tmp - 1.0) * (1. / tau);
  }
  else
  {
    // it means the variable have been already computed by another tool
    // usually this is the EGammaVariableCorrection, which means that
    // it is also fudged (only MC)
    trans_TRTPID = trans_TRT_PID_acc(*eg);
  }

  //Change default value of TRT PID to 0.15 instead of 0 when there is no information from the TRT
  if ((std::abs(trans_TRTPID) < 1.0e-6) && (std::abs(eta) > 2.01)){
      trans_TRTPID = 0.15;
  }

  unsigned int index;
  if (track->indexOfParameterAtPosition(index, xAOD::LastMeasurement)){
    double refittedTrack_LMqoverp = track->charge() / std::sqrt(std::pow(track->parameterPX(index), 2) +
                                                                std::pow(track->parameterPY(index), 2) +
                                                                std::pow(track->parameterPZ(index), 2));

    dPOverP = 1 - trackqoverp / (refittedTrack_LMqoverp);
  }
  else if (!m_skipDeltaPoverP) {
    allFound = false;
    notFoundList += "deltaPoverP ";
  }

  EoverP =  energy * std::abs(trackqoverp);

  nPixHitsPlusDeadSensors = ElectronSelectorHelpers::numberOfPixelHitsAndDeadSensors(*track);
  nSCTHitsPlusDeadSensors = ElectronSelectorHelpers::numberOfSCTHitsAndDeadSensors(*track);

  float charge = 0;
  uint8_t SCT = 0;
  for (unsigned TPit = 0; TPit < eg->nTrackParticles(); TPit++) {
    uint8_t temp_NSCTHits = 0;
    if (eg->trackParticle(TPit)) {
      eg->trackParticle(TPit)->summaryValue(temp_NSCTHits, xAOD::numberOfSCTHits);
      SCT += temp_NSCTHits;
      charge += temp_NSCTHits*(eg->trackParticle(TPit)->charge());
    }
  }
  if (SCT)
    SCTWeightedCharge = (eg->charge()*charge/SCT);
  else {
    ATH_MSG_WARNING("No SCT hit for any track associated to electron ! nTP = " << eg->nTrackParticles());
  } 
  
  // retrieve Calorimeter variables
  // reta = e237/e277
  if (!eg->showerShapeValue(Reta, xAOD::EgammaParameters::Reta)){
    allFound = false;
    notFoundList += "Reta ";
  }
  // rphi e233/e237
  if (!eg->showerShapeValue(Rphi, xAOD::EgammaParameters::Rphi)){
    allFound = false;
    notFoundList += "Rphi ";
  }
  // rhad1 = ethad1/et
  if (!eg->showerShapeValue(Rhad1, xAOD::EgammaParameters::Rhad1)){
    allFound = false;
    notFoundList += "Rhad1 ";
  }
  // rhad = ethad/et
  if (!eg->showerShapeValue(Rhad, xAOD::EgammaParameters::Rhad)){
    allFound = false;
    notFoundList += "Rhad ";
  }
  // shower width in 2nd sampling
  if (!eg->showerShapeValue(w2, xAOD::EgammaParameters::weta2)){
    allFound = false;
    notFoundList += "weta2 ";
  }
  // fraction of energy reconstructed in the 1st sampling
  if (!eg->showerShapeValue(f1, xAOD::EgammaParameters::f1)){
    allFound = false;
    notFoundList += "f1 ";
  }
  // E of 2nd max between max and min in strips
  if (!eg->showerShapeValue(Eratio, xAOD::EgammaParameters::Eratio)){
    allFound = false;
    notFoundList += "Eratio ";
  }
  // fraction of energy reconstructed in the 3rd sampling
  if (!eg->showerShapeValue(f3, xAOD::EgammaParameters::f3)){
    allFound = false;
    notFoundList += "f3 ";
  }

  // Set f3 to default value in eta region where it is poorly modelled
  if (std::abs(eta) > 2.01) {
    f3 = 0.05;
  }

  // Shower width in first sampling of the calorimeter
  if (!eg->showerShapeValue(wtots1, xAOD::EgammaParameters::wtots1)){
    allFound = false;
    notFoundList += "wtots1 ";
  }

  // retrieve Track Cluster matching variables
  // difference between cluster eta (sampling 1) and the eta of the track
  if (!eg->trackCaloMatchValue(deltaEta1, xAOD::EgammaParameters::deltaEta1)){
    allFound = false;
    notFoundList += "deltaEta1 ";
  }
  // difference between the cluster phi (sampling 2) and the phi of the track extrapolated from the last measurement point.
  if (!eg->trackCaloMatchValue(deltaPhiRescaled2, xAOD::EgammaParameters::deltaPhiRescaled2)){
    allFound = false;
    notFoundList += "deltaPhiRescaled2 ";
  }


  ATH_MSG_VERBOSE(Form("Vars: eta=%8.5f, et=%8.5f, f3=%8.5f, rHad==%8.5f, rHad1=%8.5f, Reta=%8.5f, w2=%8.5f, f1=%8.5f, Emaxs1=%8.5f, deltaEta1=%8.5f, d0=%8.5f, qd0=%8.5f, d0significance=%8.5f, Rphi=%8.5f, dPOverP=%8.5f, deltaPhiRescaled2=%8.5f, TRT_PID=%8.5f, trans_TRTPID=%8.5f, mu=%8.5f, wtots1=%8.5f, EoverP=%8.5f, nPixHitsPlusDeadSensors=%2df, nSCTHitsPlusDeadSensors=%2df, SCTWeightedCharge=%8.5f",
                       eta, et, f3, Rhad, Rhad1, Reta,
                       w2, f1, Eratio,
                       deltaEta1, d0, qd0,
                       d0significance,
                       Rphi, dPOverP, deltaPhiRescaled2,
                       TRT_PID, trans_TRTPID,
                       mu,
                       wtots1, EoverP, int(nPixHitsPlusDeadSensors), int(nSCTHitsPlusDeadSensors), SCTWeightedCharge));

  if (!allFound){
    throw std::runtime_error("AsgElectronSelectorTool: Not all variables needed for MVA calculation are found. The following variables are missing: " + notFoundList );
  }


  MVAEnum::MVACalcVars vars{};
  vars.eta = std::abs(eta);
  vars.et = et;
  vars.f3 = f3;
  vars.Rhad = Rhad;
  vars.Rhad1 = Rhad1;
  vars.Reta = Reta;
  vars.weta2 = w2;
  vars.f1 = f1;
  vars.Eratio = Eratio;
  vars.deltaEta1 = deltaEta1;
  if (m_CFReject){
    vars.qd0 = qd0;
    vars.SCTWeightedCharge = SCTWeightedCharge;
  }
  else {
    vars.d0 = d0;
  }
  vars.d0significance = d0significance;
  vars.Rphi = Rphi;
  vars.dPOverP = dPOverP;
  vars.deltaPhiRescaled2 = deltaPhiRescaled2;
  vars.trans_TRTPID = trans_TRTPID;
  vars.wtots1 = wtots1;
  vars.EoverP = EoverP;
  vars.nPixHitsPlusDeadSensors = nPixHitsPlusDeadSensors;
  vars.nSCTHitsPlusDeadSensors = nSCTHitsPlusDeadSensors;

  Eigen::Matrix<float, -1, 1> mvaScores = m_mvaTool->calculate(vars);

  // Return a vector of all outputs of the MVA
  std::vector<float> mvaOutputs;
  mvaOutputs.reserve(mvaScores.rows());
  for (int i = 0; i < mvaScores.rows(); i++) {
    mvaOutputs.push_back(mvaScores(i, 0));
  }

  return mvaOutputs;
}

//=============================================================================
/// Get the name of the current operating point
//=============================================================================
std::string AsgElectronSelectorTool::getOperatingPointName() const
{
  return m_workingPoint;
}

//=============================================================================
asg::AcceptData AsgElectronSelectorTool::accept( const xAOD::IParticle* part ) const
{
  return accept(Gaudi::Hive::currentContext(), part);
}
asg::AcceptData AsgElectronSelectorTool::accept( const EventContext& ctx, const xAOD::IParticle* part ) const
{
  ATH_MSG_VERBOSE("\t AsgElectronSelectorTool::accept( &ctx, *part= "<<(&ctx)<<", "<<part<<" )");
  const xAOD::Electron* eg = dynamic_cast<const xAOD::Electron*>(part);
  if (eg){
    return accept(ctx, eg);
  }
  else {
    ATH_MSG_DEBUG("AsgElectronSelectorTool::could not cast to const Electron");
    // Setup return accept with AcceptInfo
    asg::AcceptData acceptData(&m_acceptMVA);
    return acceptData;
  }
}

double AsgElectronSelectorTool::calculate( const xAOD::IParticle* part ) const
{
  return calculate(Gaudi::Hive::currentContext(), part);
}

double AsgElectronSelectorTool::calculate( const EventContext& ctx, const xAOD::IParticle* part ) const
{
  ATH_MSG_VERBOSE("\t AsgElectronSelectorTool::calculate( &ctx, *part"<<(&ctx)<<", "<<part<<" )");
  const xAOD::Electron* eg = dynamic_cast<const xAOD::Electron*>(part);
  if (eg){
    return calculate(ctx, eg);
  }
  else {
    ATH_MSG_DEBUG("AsgElectronSelectorTool::could not cast to const Electron");
    // Return a default value
    return -999.;
  }
}

asg::AcceptData AsgElectronSelectorTool::accept( const EventContext& ctx, const xAOD::Egamma* eg, double mu ) const
{
  ATH_MSG_VERBOSE("\t AsgElectronSelectorTool::accept( &ctx, *eg, mu= "<<(&ctx)<<", "<<eg<<", "<<mu<<" )");
  const xAOD::Electron* ele = dynamic_cast<const xAOD::Electron*>(eg);
  if (ele){
    return accept(ctx, ele, mu);
  }
  else {
    ATH_MSG_DEBUG("AsgElectronSelectorTool::could not cast to const Electron");
    // Setup return accept with AcceptInfo
    asg::AcceptData acceptData(&m_acceptMVA);
    return acceptData;
  }
}

double AsgElectronSelectorTool::calculate( const EventContext& ctx, const xAOD::Egamma* eg, double mu ) const
{
  ATH_MSG_VERBOSE("\t AsgElectronSelectorTool::calculate( &ctx, *eg, mu= "<<(&ctx)<<", "<<eg<<", "<<mu<<" )");
  const xAOD::Electron* ele = dynamic_cast<const xAOD::Electron*>(eg);
  if (ele){
    return calculate(ctx, ele, mu);
  }
  else {
    ATH_MSG_DEBUG("AsgElectronSelectorTool::could not cast to const Electron");
    return -999.;
  }
}

bool AsgElectronSelectorTool::isForwardElectron( const xAOD::Egamma* eg, const float eta ) const
{
  static const SG::AuxElement::ConstAccessor< uint16_t > accAuthor( "author" );

  if (accAuthor.isAvailable(*eg)){
    // cannot just do eg->author() because it isn't always filled
    // at trigger level
    if (accAuthor(*eg) == xAOD::EgammaParameters::AuthorFwdElectron){
      ATH_MSG_DEBUG("Failed, this is a forward electron! The AsgElectronSelectorTool is only suitable for central electrons!");
      return true;
    }
  }
  else{
    //Check for fwd via eta range the old logic
    if (std::abs(eta) > 2.5){
      ATH_MSG_DEBUG("Failed, cluster->etaBE(2) range due to " << eta << " seems like a fwd electron" );
      return true;
    }
  }

  return false;
}


double AsgElectronSelectorTool::transformMLOutput( float score ) const
{
  // returns transformed or non-transformed output
  constexpr double oneOverTau = 1. / 10;
  constexpr double fEpsilon = 1.0e-30; // to avoid zero division
  if (score >= 1.0) score = 1.0 - 1.0e-15; // this number comes from TMVA
  else if (score <= fEpsilon) score = fEpsilon;
  score = -std::log(1.0 / score - 1.0) * oneOverTau;
  ATH_MSG_DEBUG("score is " << score);
  return score;
}


double AsgElectronSelectorTool::combineOutputs( const std::vector<float>& mvaScores, double eta ) const
{
  unsigned int etaBin = getDiscEtaBin(eta);
  double disc = 0;

  if (m_cfSignal){
    // Put cf node into numerator

    disc = (mvaScores.at(0) * (1 - m_fractions.at(5 * etaBin + 0)) +
            (mvaScores.at(1) * m_fractions.at(5 * etaBin + 0))) /
           ((mvaScores.at(2) * m_fractions.at(5 * etaBin + 1)) +
            (mvaScores.at(3) * m_fractions.at(5 * etaBin + 2)) +
            (mvaScores.at(4) * m_fractions.at(5 * etaBin + 3)) +
            (mvaScores.at(5) * m_fractions.at(5 * etaBin + 4)));
  }
  else{
    // Put cf node in denominator
    disc = mvaScores.at(0) /
           ((mvaScores.at(1) * m_fractions.at(5 * etaBin + 0)) +
            (mvaScores.at(2) * m_fractions.at(5 * etaBin + 1)) +
            (mvaScores.at(3) * m_fractions.at(5 * etaBin + 2)) +
            (mvaScores.at(4) * m_fractions.at(5 * etaBin + 3)) +
            (mvaScores.at(5) * m_fractions.at(5 * etaBin + 4)));
  }

  // Log transform to have values in reasonable range
  return std::log(disc);
}

double AsgElectronSelectorTool::combineOutputsCF( const std::vector<float>& mvaScores ) 
{
  double disc = 0;
  disc = mvaScores.at(0) / mvaScores.at(1);
    
  return std::log(disc);
}


// Gets the Discriminant Eta bin [0,s_fnDiscEtaBins-1] given the eta
unsigned int AsgElectronSelectorTool::getDiscEtaBin( double eta ) 
{
  const unsigned int nEtaBins = s_fnDiscEtaBins;
  const double etaBins[nEtaBins] = {0.1, 0.6, 0.8, 1.15, 1.37, 1.52, 1.81, 2.01, 2.37, 2.47};
  for (unsigned int etaBin = 0; etaBin < nEtaBins; ++etaBin){
    if (std::abs(eta) < etaBins[etaBin]) return etaBin;
  }
  return (nEtaBins-1);
}

// Gets the Discriminant Et bin (MeV) [0,s_fnDiscEtBins-1]
unsigned int AsgElectronSelectorTool::getDiscEtBin( double et ) 
{
  static const double GeV = 1000;
  const unsigned int nEtBins = s_fnDiscEtBins;
  const double etBins[nEtBins] = {7*GeV,10*GeV,15*GeV,20*GeV,25*GeV,30*GeV,35*GeV,40*GeV,45*GeV,6000*GeV};
  for (unsigned int etBin = 0; etBin < nEtBins; ++etBin){
    if (et < etBins[etBin]) return etBin;
  }
  return (nEtBins-1);
}


// Note that this will only perform the cut interpolation up to ~45 GeV, so
// no smoothing is done above this for the high ET LH binning yet
double AsgElectronSelectorTool::interpolateCuts( const std::vector<double>& cuts,double et,double eta ) 
{
  const int etbin = getDiscEtBin(et);
  const int etabin = getDiscEtaBin(eta);
  unsigned int ibin_combinedML = etbin*s_fnDiscEtaBins+etabin;
  double cut = cuts.at(ibin_combinedML);
  const double GeV = 1000;
  const double eTBins[10] = {5.5*GeV,8.5*GeV,12.5*GeV,17.5*GeV,22.5*GeV,27.5*GeV,32.5*GeV,37.5*GeV,42.5*GeV,47.5*GeV};

  if (et >= eTBins[9]) return cut; // no interpolation for electrons above 47.5 GeV
  if (et <= eTBins[0]) return cut; // no interpolation for electrons below 5.5 GeV

  // find the bin where the value is smaller than the next bin
  // Start with bin = 1, since it always has to be at least in
  // bin 1 because of previous cut
  int bin = 1;
  while ( et > eTBins[bin] ) bin++;

  double etLow = eTBins[bin-1];
  double etUp = eTBins[bin];
  double discLow = cuts.at((bin-1) * s_fnDiscEtaBins+etabin);
  double discUp = cuts.at((bin) * s_fnDiscEtaBins+etabin);

  double gradient = ( discUp - discLow ) / ( etUp - etLow );

  return discLow + (et - etLow) * gradient;
}
