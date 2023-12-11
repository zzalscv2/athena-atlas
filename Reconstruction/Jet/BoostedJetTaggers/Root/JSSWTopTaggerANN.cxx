/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "BoostedJetTaggers/JSSWTopTaggerANN.h"

#include <fstream>
#include <memory>

JSSWTopTaggerANN::JSSWTopTaggerANN( const std::string& name ) :
  JSSTaggerBase( name ),
  m_lwnn(nullptr)
{

}

/// Initialize the tagger
StatusCode JSSWTopTaggerANN::initialize() {

  ATH_MSG_INFO( "Initializing JSSWTopTaggerANN tool" );

  /// Pt values are defined in GeV
  m_ptGeV = true;

  /// Use mass cut
  m_useMassCut = true;

  /// Use discriminant score cut
  m_useScoreCut = true;

  if( ! m_configFile.empty() ) {

    /// Get configReader
    ATH_CHECK( getConfigReader() );

    /// Get tagger type
    m_tagType = m_configReader.GetValue("TaggerType" ,"");

    /// Get the CVMFS calib area where stuff is stored
    /// If this is set to "Local" then it will look for the config file in the share space
    m_kerasCalibArea = m_configReader.GetValue("CalibAreaKeras" ,"");

    /// Get the name/path of the JSON config
    m_kerasConfigFileName = m_configReader.GetValue("KerasConfigFile" ,"");

    /// Get the name of the Keras output node
    m_kerasConfigOutputName = m_configReader.GetValue("KerasOutput" ,"");

    /// Get min and max jet mass. The unit is GeV now. Need to be consistent with ATLAS convention in the future
    m_strMassCutLow  = m_configReader.GetValue("MassCutLow_in_GeV" ,"");
    m_strMassCutHigh = m_configReader.GetValue("MassCutHigh_in_GeV" ,"");

    /// Get min and max jet pt. The unit is GeV now. Need to be consistent with ATLAS convention in the future
    m_jetPtMin = m_configReader.GetValue("pTCutLow_in_GeV", 200.0);
    m_jetPtMax = m_configReader.GetValue("pTCutHigh_in_GeV", 2000.0);

    /// Get cut for ANN score
    m_strScoreCut = m_configReader.GetValue("ScoreCut" ,"");

    /// Get the decoration name
    m_decorationName = m_configReader.GetValue("DecorationName" ,"");

    /// Get the scale factor configuration
    m_calcSF = m_configReader.GetValue("CalcSF", false);
    if ( m_calcSF ) {
      m_weightDecorationName = m_configReader.GetValue("WeightDecorationName", "");
      m_weightFileName = m_configReader.GetValue("WeightFile", "");
      m_weightHistogramName = m_configReader.GetValue("WeightHistogramName", "");
      m_efficiencyHistogramName = m_configReader.GetValue("EfficiencyHistogramName", "");
      m_weightFlavors = m_configReader.GetValue("WeightFlavors", "");

      /// Get truth label name information
      m_truthLabelName = m_configReader.GetValue("TruthLabelName" , "R10TruthLabel_R21Consolidated");
    }

    /// Print out the configuration parameters for viewing
    ATH_MSG_INFO( "Configurations Loaded :");
    ATH_MSG_INFO( "tagType               : " << m_tagType );
    ATH_MSG_INFO( "calibarea_keras       : " << m_kerasCalibArea );
    ATH_MSG_INFO( "kerasConfigFileName   : " << m_kerasConfigFileName );
    ATH_MSG_INFO( "kerasConfigOutputName : " << m_kerasConfigOutputName );
    ATH_MSG_INFO( "strMassCutLow         : " << m_strMassCutLow );
    ATH_MSG_INFO( "strMassCutHigh        : " << m_strMassCutHigh );
    ATH_MSG_INFO( "pTCutLow              : " << m_jetPtMin );
    ATH_MSG_INFO( "pTCutHigh             : " << m_jetPtMax );
    ATH_MSG_INFO( "strScoreCut           : " << m_strScoreCut );
    ATH_MSG_INFO( "decorationName        : " << m_decorationName );
    if ( m_calcSF ) {
      ATH_MSG_INFO( "weightDecorationName  : " << m_weightDecorationName );
      ATH_MSG_INFO( "weightFile            : " << m_weightFileName );
      ATH_MSG_INFO( "weightHistogramName   : " << m_weightHistogramName );
      ATH_MSG_INFO( "efficiencyHistogramName : "<<m_efficiencyHistogramName );
      ATH_MSG_INFO( "weightFlavors         : " << m_weightFlavors );
      ATH_MSG_INFO( "TruthLabelName        : " << m_truthLabelName );
    }
  }
  else { /// No config file
    /// Assume the cut functions have been set through properties.
    /// Check they are non empty
    if ( (m_kerasConfigFileName.empty() ||
          m_kerasConfigOutputName.empty() ||
          m_strScoreCut.empty() ||
          m_strMassCutLow.empty() ||
          m_strMassCutHigh.empty() ||
          m_decorationName.empty() || 
          m_weightFileName.empty()) ||
        ((m_weightDecorationName.empty() ||
          m_weightHistogramName.empty() ||
          m_weightFlavors.empty()) && m_calcSF) ) {
      ATH_MSG_ERROR( "No config file provided OR you haven't manually specified all needed parameters" ) ;
      ATH_MSG_ERROR( "Please read the TWiki for this tool" );
      return StatusCode::FAILURE;
    }

  }

  ATH_MSG_INFO( "Mass cut low   : " << m_strMassCutLow );
  ATH_MSG_INFO( "Mass cut High  : " << m_strMassCutHigh );
  ATH_MSG_INFO( "Score cut low  : " << m_strScoreCut );

  /// If the calibarea is specified to be "Local" then it looks in the same place as the top level configs
  if ( m_kerasCalibArea.empty() ) {
    ATH_MSG_ERROR( "You need to specify where the calibarea is as either being Local or on CVMFS" );
    return StatusCode::FAILURE;
  }
  else if( !m_kerasCalibArea.compare("Local") ) {
    std::string localCalibArea = "BoostedJetTaggers/JSSWTopTaggerANN/";
    ATH_MSG_INFO( "Using Local calibarea " << localCalibArea );
    /// Convert the JSON config file name to the full path
    m_kerasConfigFilePath = PathResolverFindCalibFile(localCalibArea+m_kerasConfigFileName);
    if ( m_calcSF )
      m_weightConfigPath = PathResolverFindCalibFile(localCalibArea+m_weightFileName);
  }
  else {
    ATH_MSG_INFO( "Using CVMFS calibarea" );
    /// Get the config file from CVMFS
    /// Necessary because xml files are too large to house on the data space
    m_kerasConfigFilePath = PathResolverFindCalibFile( (m_kerasCalibArea+m_kerasConfigFileName).c_str() );
    if ( m_calcSF )
      m_weightConfigPath = PathResolverFindCalibFile( (m_kerasCalibArea+m_weightFileName).c_str());
  }

  /// Read json file for ANN weights
  ATH_MSG_INFO( "ANN Tagger configured with: " << m_kerasConfigFilePath );

  std::ifstream input_cfg( m_kerasConfigFilePath.c_str() );

  if ( !input_cfg.is_open() ) {
    ATH_MSG_ERROR( "Error openning config file: " << m_kerasConfigFilePath );
    ATH_MSG_ERROR( "Are you sure that the file exists at this path?" );
    return StatusCode::FAILURE;
  }

  lwt::GraphConfig config = lwt::parse_json_graph( input_cfg );

  for ( auto& input_node: config.inputs ) {
    ATH_MSG_INFO( " input node: " << input_node.name );
    for ( auto& input: input_node.variables ) {
      ATH_MSG_INFO( "  " << input );
    }
  }

  auto output_node_name = config.outputs.begin()->first;
  m_out_names = config.outputs.at(output_node_name).labels;

  ATH_MSG_INFO( "Keras Network NLayers: " << config.layers.size() );

  m_lwnn = std::make_unique< lwt::LightweightGraph >(config, output_node_name);

  /// Build the network
  try {
    m_lwnn = std::make_unique<lwt::LightweightGraph>(config, output_node_name);
  } catch (lwt::NNConfigurationException& exc) {
    ATH_MSG_ERROR( "NN configuration problem: " << exc.what() );
    return StatusCode::FAILURE;
  }

  /// Set internal tagger type
  if ( !m_tagType.compare("TopQuark") ) {
    ATH_MSG_DEBUG( "This is a top quark tagger" );
    m_tagClass = TAGCLASS::TopQuark;
  }
  else if ( !m_tagType.compare("WBoson") ) {
    ATH_MSG_DEBUG( "This is a W boson tagger" );
    m_tagClass = TAGCLASS::WBoson;
  }
  else if ( !m_tagType.compare("ZBoson") ) {
    ATH_MSG_DEBUG( "This is a Z boson tagger" );
    m_tagClass = TAGCLASS::ZBoson;
  }
  else {
    ATH_MSG_ERROR( "I can't tell what kind of tagger your configuration is for." );
    return StatusCode::FAILURE;
  }

  /// Set the possible states that the tagger can be left in after the JSSTaggerBase::tag() function is called
  m_acceptInfo.addCut( "PassMassLow" , "mJet > mCutLow" );
  m_acceptInfo.addCut( "PassScore"   , "ScoreJet > ScoreCut" );
  if ( m_tagClass == TAGCLASS::WBoson || m_tagClass == TAGCLASS::ZBoson ) {
    m_acceptInfo.addCut( "PassMassHigh", "mJet < mCutHigh" );
  }

  /// Loop over and print out the cuts that have been configured
  printCuts();

  /// Call base class initialize
  ATH_CHECK( JSSTaggerBase::initialize() );

  ATH_MSG_INFO( "ANN Tagger tool initialized" );

  return StatusCode::SUCCESS;

}

StatusCode JSSWTopTaggerANN::tag( const xAOD::Jet& jet ) const {

  ATH_MSG_DEBUG( "Obtaining ANN result" );

  /// Create asg::AcceptData object
  asg::AcceptData acceptData( &m_acceptInfo );

  /// Reset the AcceptData cut results
  ATH_CHECK( resetCuts( acceptData ) );

  /// Check basic kinematic selection
  ATH_CHECK( checkKinRange( jet, acceptData ) );

  /// Get the relevant attributes of the jet
  /// Mass and pt - note that this will depend on the configuration of the calibration used
  float jet_pt   = jet.pt()/1000.0;
  float jet_mass = jet.m()/1000.0;

  /// Get ANN score for the jet
  float jet_score = getScore(jet);

  /// Evaluate the values of the upper and lower mass bounds and the d2 cut
  float cut_mass_low  = m_funcMassCutLow ->Eval(jet_pt);
  float cut_mass_high = m_funcMassCutHigh->Eval(jet_pt);
  float cut_score     = m_funcScoreCut   ->Eval(jet_pt);

  /// Print cut criteria and jet values
  ATH_MSG_VERBOSE( "Cut values : Mass window = [" << cut_mass_low << "," << cut_mass_high << "], score cut = " << cut_score );
  ATH_MSG_VERBOSE( "Jet values : Mass = " << jet_mass << ", score = " << jet_score );

  /// Get SF weight
  ATH_CHECK( getWeight( jet, jet_score > cut_score, acceptData ) );

  /// Decorate cut information if needed
  ATH_MSG_DEBUG( "Decorating with score" );

  /// Create WriteDecorHandles
  SG::WriteDecorHandle<xAOD::JetContainer, bool> decPassMass(m_decPassMassKey);
  SG::WriteDecorHandle<xAOD::JetContainer, bool> decPassScore(m_decPassScoreKey);
  SG::WriteDecorHandle<xAOD::JetContainer, bool> decTagged(m_decTaggedKey);
  SG::WriteDecorHandle<xAOD::JetContainer, float> decCutMLow(m_decCutMLowKey);
  SG::WriteDecorHandle<xAOD::JetContainer, float> decCutMHigh(m_decCutMHighKey);
  SG::WriteDecorHandle<xAOD::JetContainer, float> decScoreCut(m_decScoreCutKey);
  SG::WriteDecorHandle<xAOD::JetContainer, float> decScoreValue(m_decScoreValueKey);

  /// Decorate values
  decCutMLow(jet) = cut_mass_low;
  decCutMHigh(jet) = cut_mass_high;
  decScoreCut(jet) = cut_score;
  decScoreValue(jet) = jet_score;

  /// Cut summary
  bool passCuts = true;

  /// Set the AcceptData depending on whether it is a W/Z or a top tagger
  if ( m_tagClass == TAGCLASS::WBoson || m_tagClass == TAGCLASS::ZBoson ) {
    ATH_MSG_VERBOSE( "Determining WZ tag return" );
    if ( jet_mass > cut_mass_low ) acceptData.setCutResult( "PassMassLow", true );
    if ( jet_mass < cut_mass_high ) acceptData.setCutResult( "PassMassHigh", true );
    if ( jet_score > cut_score ) acceptData.setCutResult( "PassScore", true );
    decPassMass(jet) = acceptData.getCutResult( "PassMassLow" ) && acceptData.getCutResult( "PassMassHigh" );
    passCuts = passCuts && acceptData.getCutResult( "PassMassLow" ) && acceptData.getCutResult( "PassMassHigh" );
  }
  else if ( m_tagClass == TAGCLASS::TopQuark ) {
    ATH_MSG_VERBOSE( "Determining TopQuark tag return" );
    if( jet_mass > cut_mass_low ) acceptData.setCutResult( "PassMassLow", true );
    if( jet_score > cut_score ) acceptData.setCutResult( "PassScore", true );
    decPassMass(jet) = acceptData.getCutResult( "PassMassLow" );
    passCuts = passCuts && acceptData.getCutResult( "PassMassLow" );
  }

  decPassScore(jet) = acceptData.getCutResult( "PassScore" );

  passCuts = passCuts && acceptData.getCutResult( "PassScore" );

  decTagged(jet) = passCuts;

  return StatusCode::SUCCESS;

}

double JSSWTopTaggerANN::getScore( const xAOD::Jet& jet ) const {

    /// Create input dictionary map<string,double> for argument to lwtnn
    std::map<std::string, std::map<std::string,double>> ANN_inputs = getJetProperties(jet);

    /// Evaluate the network response
    auto discriminant = m_lwnn->compute(ANN_inputs);

    /// Obtain the output associated with the single output node
    double ANNscore = -666.;

    /// Check that input variables are valid
    bool validVars = true;
    SG::ReadDecorHandle<xAOD::JetContainer, float> readTau21WTA(m_readTau21WTAKey);
    if ( readTau21WTA(jet) < 0.0 ) validVars = false;
    if ( m_tagClass == TAGCLASS::TopQuark ) {
      SG::ReadDecorHandle<xAOD::JetContainer, float> readTau32WTA(m_readTau32WTAKey);
      if ( readTau32WTA(jet) < 0.0 ) validVars = false;
    }

    if ( !validVars ) {

      if ( m_nWarnVar++ < m_nWarnMax ) ATH_MSG_WARNING( "One (or more) tagger input variable has an out-of-range value, setting score to -666" );
      else ATH_MSG_WARNING( "One (or more) tagger input variable has an out-of-range value, setting score to -666" );

      return ANNscore;

    }

    ANNscore = discriminant.at(m_out_names.at(0));

    return ANNscore;
}

std::map<std::string, std::map<std::string, double>> JSSWTopTaggerANN::getJetProperties( const xAOD::Jet& jet ) const {

  /// Map to store inputs
  std::map< std::string, std::map<std::string, double> > ANN_inputs;
  std::map< std::string, double > ANN_inputValues;

  /// Calculate NSubjettiness and ECF ratios
  calculateJSSRatios(jet);

  ATH_MSG_DEBUG( "Loading variables for common ANN tagger" );

  /// Create common read decor handles
  SG::ReadDecorHandle<xAOD::JetContainer, float> readTau21WTA(m_readTau21WTAKey);
  SG::ReadDecorHandle<xAOD::JetContainer, float> readC2(m_readC2Key);
  SG::ReadDecorHandle<xAOD::JetContainer, float> readD2(m_readD2Key);
  SG::ReadDecorHandle<xAOD::JetContainer, float> readSplit12(m_readSplit12Key);

  /// Mass and pT
  /// It is assumed that these are the combined and calibrated mass and pT
  ANN_inputValues["CaloTACombinedMassUncorrelated"] = jet.m();
  ANN_inputValues["JetpTCorrByCombinedMass"] = jet.pt();

  /// Splitting scales
  ANN_inputValues["Split12"] = readSplit12(jet);

  /// Energy Correlation Functions
  ANN_inputValues["C2"] = readC2(jet);
  ANN_inputValues["D2"] = readD2(jet);

  /// Tau21 WTA
  ANN_inputValues["Tau21_wta"] = readTau21WTA(jet);

  if ( m_tagClass == TAGCLASS::WBoson ) {

    ATH_MSG_DEBUG( "Loading variables for W boson tagger" );

    /// Other moments
    ANN_inputValues["FoxWolfram20"] = jet.getAttribute<float>("FoxWolfram2") / jet.getAttribute<float>("FoxWolfram0");
    ANN_inputValues["PlanarFlow"] = jet.getAttribute<float>("PlanarFlow");
    ANN_inputValues["Angularity"] = jet.getAttribute<float>("Angularity");
    ANN_inputValues["Aplanarity"] = jet.getAttribute<float>("Aplanarity");
    ANN_inputValues["ZCut12"] = jet.getAttribute<float>("ZCut12");
    ANN_inputValues["KtDR"] = jet.getAttribute<float>("KtDR");

    int pv_location = findPV();

    if(pv_location != -1){
      if( GetUnGroomTracks(jet, pv_location).isSuccess()){

	SG::ReadDecorHandle<xAOD::JetContainer, int> readNtrk500(m_readNtrk500Key);

        ANN_inputValues["Ntrk500"] = readNtrk500(jet);
      }
      else{
	ATH_MSG_ERROR("Either the ungroomed parent jet doesn't have 'NumTrkPt500' as an attribute or the parent link is broken");
	ANN_inputValues["Ntrk500"] = -999;
      }
    }
    else {
      ATH_MSG_ERROR("Could not find a primary vertex");
      ANN_inputValues["Ntrk500"] = -999;
    }
  }

  else if ( m_tagClass == TAGCLASS::TopQuark ) {

    ATH_MSG_DEBUG( "Loading variables for top quark tagger" );

    /// Create top quark read decor handles
    SG::ReadDecorHandle<xAOD::JetContainer, float> readTau1WTA(m_readTau1WTAKey);
    SG::ReadDecorHandle<xAOD::JetContainer, float> readTau2WTA(m_readTau2WTAKey);
    SG::ReadDecorHandle<xAOD::JetContainer, float> readTau3WTA(m_readTau3WTAKey);
    SG::ReadDecorHandle<xAOD::JetContainer, float> readTau32WTA(m_readTau32WTAKey);
    SG::ReadDecorHandle<xAOD::JetContainer, float> readSplit23(m_readSplit23Key);
    SG::ReadDecorHandle<xAOD::JetContainer, float> readQw(m_readQwKey);
    SG::ReadDecorHandle<xAOD::JetContainer, float> readE3(m_readE3Key);

    /// Additional splitting Scales
    ANN_inputValues["Split23"] = readSplit23(jet);

    /// e3 := normalized ECF3/ECF1**3
    ANN_inputValues["e3"] = readE3(jet);

    /// N-subjettiness
    ANN_inputValues["Tau1_wta"] = readTau1WTA(jet);
    ANN_inputValues["Tau2_wta"] = readTau2WTA(jet);
    ANN_inputValues["Tau3_wta"] = readTau3WTA(jet);

    ANN_inputValues["Tau32_wta"] = readTau32WTA(jet);

    /// Qw observable for top tagging
    ANN_inputValues["Qw"] = readQw(jet);

  }

  else {
    ATH_MSG_ERROR( "Loading variables failed because the tagger type is not supported" );
  }

  ANN_inputs["node_0"] = ANN_inputValues;

  return ANN_inputs;

}

