/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "BoostedJetTaggers/JSSWTopTaggerDNN.h"

#include <fstream>

JSSWTopTaggerDNN::JSSWTopTaggerDNN( const std::string& name ) :
  JSSTaggerBase( name ),
  m_lwnn(nullptr),
  m_lwnnGraph(nullptr)
{

}

/// Initialize the tagger
StatusCode JSSWTopTaggerDNN::initialize() {

  ATH_MSG_INFO( "Initializing JSSWTopTaggerDNN tool" );
 
  /// Pt values are defined in GeV
  m_ptGeV = true;

  /// Use mass cut
  m_useMassCut = true;

  /// Use discriminant score cut
  m_useScoreCut = true;

  if ( ! m_configFile.empty() ) {

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

    /// Get the configured cut values
    m_strMassCutLow  = m_configReader.GetValue("MassCutLow" ,"");
    m_strMassCutHigh = m_configReader.GetValue("MassCutHigh" ,"");
    m_strScoreCut    = m_configReader.GetValue("ScoreCut" ,"");

    /// Get min and max jet pt
    m_jetPtMin = m_configReader.GetValue("pTCutLow", 350.0);
    m_jetPtMax = m_configReader.GetValue("pTCutHigh", 4000.0);

    /// Get the decoration name
    m_decorationName = m_configReader.GetValue("DecorationName" ,"");

    /// Get the scale factor configuration
    m_calcSF = m_configReader.GetValue("CalcSF", false);
    if ( m_calcSF ) {
      m_weightdecorationName = m_configReader.GetValue("WeightDecorationName", "");
      m_weightFileName = m_configReader.GetValue("WeightFile", "");
      m_weightHistogramName = m_configReader.GetValue("WeightHistogramName", "");
      m_efficiencyHistogramName = m_configReader.GetValue("EfficiencyHistogramName", "");
      m_weightFlavors = m_configReader.GetValue("WeightFlavors", "");

      /// Get truth label name information
      m_truthLabelName = m_configReader.GetValue("TruthLabelName" , "R10TruthLabel_R21Consolidated");
    }

    /// print out the configuration parameters for viewing
    ATH_MSG_INFO( "Configurations Loaded :");
    ATH_MSG_INFO( "tagType                 : " << m_tagType );
    ATH_MSG_INFO( "calibarea_keras         : " << m_kerasCalibArea );
    ATH_MSG_INFO( "kerasConfigFileName     : " << m_kerasConfigFileName );
    ATH_MSG_INFO( "kerasConfigOutputName   : " << m_kerasConfigOutputName );
    ATH_MSG_INFO( "strMassCutLow           : " << m_strMassCutLow );
    ATH_MSG_INFO( "strMassCutHigh          : " << m_strMassCutHigh );
    ATH_MSG_INFO( "pTCutLow                : " << m_jetPtMin );
    ATH_MSG_INFO( "pTCutHigh               : " << m_jetPtMax );
    ATH_MSG_INFO( "strScoreCut             : " << m_strScoreCut );
    ATH_MSG_INFO( "decorationName          : " << m_decorationName );
    if ( m_calcSF ) {
      ATH_MSG_INFO( "weightdecorationName    : " << m_weightdecorationName );
      ATH_MSG_INFO( "weightFile              : " << m_weightFileName );
      ATH_MSG_INFO( "weightHistogramName     : " << m_weightHistogramName );
      ATH_MSG_INFO( "efficiencyHistogramName : " << m_efficiencyHistogramName );
      ATH_MSG_INFO( "weightFlavors           : " << m_weightFlavors );
      ATH_MSG_INFO( "TruthLabelName          : " << m_truthLabelName );
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
        ((m_weightdecorationName.empty() ||
          m_weightHistogramName.empty() ||
          m_weightFlavors.empty()) && m_calcSF)
       )
    {
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
  else if ( !m_kerasCalibArea.compare("Local") ){
    std::string localCalibArea = "BoostedJetTaggers/JSSWTopTaggerDNN/";
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

  /// Read json file for DNN weights
  ATH_MSG_INFO( "DNN Tagger configured with: " << m_kerasConfigFilePath );

  std::ifstream input_cfg( m_kerasConfigFilePath.c_str() );

  if ( !input_cfg.is_open() ) {
    ATH_MSG_ERROR( "Error openning config file: " << m_kerasConfigFilePath );
    ATH_MSG_ERROR( "Are you sure that the file exists at this path?" );
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

  if(m_tagClass == TAGCLASS::TopQuark){
    lwt::JSONConfig cfg = lwt::parse_json( input_cfg );
    ATH_MSG_INFO( "Keras Network NLayers: " << cfg.layers.size() );
    m_lwnn = std::make_unique<lwt::LightweightNeuralNetwork>(cfg.inputs, cfg.layers, cfg.outputs);
  }else if (m_tagClass== TAGCLASS::WBoson || m_tagClass== TAGCLASS::ZBoson){
    lwt::GraphConfig config = lwt::parse_json_graph( input_cfg );
    auto output_node_name = config.outputs.begin()->first;
    m_out_names = config.outputs.at(output_node_name).labels;
    ATH_MSG_INFO( "Keras Network NLayers: " << config.layers.size() );
    m_lwnnGraph = std::make_unique< lwt::LightweightGraph >(config, output_node_name);
    /// Build the network
    try {
      m_lwnnGraph.reset(new lwt::LightweightGraph(config, output_node_name));
    } catch (lwt::NNConfigurationException& exc) {
      ATH_MSG_ERROR( "NN configuration problem: " << exc.what() );
      return StatusCode::FAILURE;
    }
  }
  
  /// Set the possible states that the tagger can be left in after the JSSTaggerBase::tag() function is called
  m_accept.addCut( "PassMassLow" , "mJet > mCutLow" );
  m_accept.addCut( "PassScore"   , "ScoreJet > ScoreCut" );
  if ( m_tagClass == TAGCLASS::WBoson || m_tagClass == TAGCLASS::ZBoson ) {
    m_accept.addCut( "PassMassHigh", "mJet < mCutHigh" );
  }

  /// Loop over and print out the cuts that have been configured
  printCuts();

  /// Call base class initialize
  ATH_CHECK( JSSTaggerBase::initialize() );
  
  ATH_MSG_INFO( "DNN Tagger tool initialized" );

  return StatusCode::SUCCESS;

}

Root::TAccept& JSSWTopTaggerDNN::tag( const xAOD::Jet& jet ) const {

  ATH_MSG_DEBUG( "Obtaining DNN result" );

  /// Reset the TAccept cut results
  resetCuts();

  /// Check basic kinematic selection
  checkKinRange(jet);

  /// Get the relevant attributes of the jet
  /// Mass and pt - note that this will depend on the configuration of the calibration used
  float jet_pt   = jet.pt()/1000.0;
  float jet_mass = jet.m()/1000.0;

  /// Get DNN score for the jet
  float jet_score = getScore(jet);

  /// Evaluate the values of the upper and lower mass bounds and the d2 cut
  float cut_mass_low  = m_funcMassCutLow ->Eval(jet_pt);
  float cut_mass_high = m_funcMassCutHigh->Eval(jet_pt);
  float cut_score     = m_funcScoreCut   ->Eval(jet_pt);

  /// Decorate truth label for SF provider
  if ( m_calcSF && (!m_acc_truthLabel->isAvailable(jet) || LargeRJetTruthLabel::intToEnum((*m_acc_truthLabel)(jet))==LargeRJetTruthLabel::UNKNOWN) ) {
    if ( m_isMC ) {
      if ( m_JetTruthLabelingTool->modifyJet(jet) == StatusCode::FAILURE ) {
        ATH_MSG_FATAL( "Failed to decorate jet truth label. Please check truth container names" );
      }
    }
  }
  /// Print cut criteria and jet values
  ATH_MSG_VERBOSE( "Cut values : Mass window = [" << cut_mass_low << "," << cut_mass_high << "], score cut = " << cut_score );
  ATH_MSG_VERBOSE( "Jet values : Mass = " << jet_mass << ", score = " << jet_score );

  /// Get SF weight
  if(m_calcSF)
    getWeight( jet, jet_score > cut_score );

  /// Decorate cut information if needed
  if ( m_decorate ) {
    ATH_MSG_DEBUG( "Decorating with score" );
    (*m_dec_mcutH)(jet)      = cut_mass_high;
    (*m_dec_mcutL)(jet)      = cut_mass_low;
    (*m_dec_scoreCut)(jet)   = cut_score;
    (*m_dec_scoreValue)(jet) = jet_score;
  }

  
  /// Set the TAccept depending on whether it is a W/Z or a top tagger
  if ( m_tagClass == TAGCLASS::WBoson || m_tagClass == TAGCLASS::ZBoson ) {
    ATH_MSG_VERBOSE( "Determining WZ tag return" );
    if ( jet_mass > cut_mass_low )
      m_accept.setCutResult( "PassMassLow", true );
    if ( jet_mass < cut_mass_high )
      m_accept.setCutResult( "PassMassHigh", true );
    if ( jet_score > cut_score )
      m_accept.setCutResult( "PassScore", true );
  }
  else if ( m_tagClass == TAGCLASS::TopQuark ) {
    ATH_MSG_VERBOSE( "Determining TopQuark tag return" );
    if ( jet_mass>cut_mass_low )
      m_accept.setCutResult( "PassMassLow", true );
    if ( jet_score > cut_score )
      m_accept.setCutResult( "PassScore", true );
  }

  if ( m_decorate && m_calcSF ) {
    TagResult::TypeEnum myCutResultForSF = TagResult::UNKNOWN;
    if (m_accept) myCutResultForSF = TagResult::passAll;
    else myCutResultForSF = TagResult::fail;
    (*m_dec_accept)(jet) = myCutResultForSF;
  }

  /// Return the TAccept object that was created and filled
  return m_accept;

}

double JSSWTopTaggerDNN::getScore( const xAOD::Jet& jet ) const {

  /// Create input dictionary map<string,double> for argument to lwtnn
  std::map<std::string,double> DNN_inputValues = getJetProperties(jet);

  /// Evaluate the network response
  lwt::ValueMap discriminant;
  if (m_tagClass== TAGCLASS::TopQuark)
    discriminant= m_lwnn->compute(DNN_inputValues);
  else if(m_tagClass== TAGCLASS::WBoson || m_tagClass== TAGCLASS::ZBoson){
    std::map< std::string, std::map<std::string, double> > DNN_inputs;
    DNN_inputs["node_0"] = DNN_inputValues;
    discriminant= m_lwnnGraph->compute(DNN_inputs);
  }
  /// Obtain the output associated with the single output node
  double DNNscore = -666.;

  /// Check that input variables are valid
  bool validVars = true;
  if ( acc_Tau21_wta(jet) < 0.0 ) validVars = false;
  if ( m_tagClass == TAGCLASS::TopQuark && acc_Tau32_wta(jet) < 0.0 ) validVars = false;

  if ( !validVars ) {
    if ( m_nWarnVar++ < m_nWarnMax ) ATH_MSG_WARNING( "One (or more) tagger input variable has an out-of-range value, setting score to -666" );
    else ATH_MSG_DEBUG( "One (or more) tagger input variable has an out-of-range value, setting score to -666" );
    return DNNscore;
  }

  if(m_tagClass== TAGCLASS::WBoson || m_tagClass== TAGCLASS::ZBoson)
    DNNscore = discriminant.at(m_out_names.at(0));
  else if (m_tagClass== TAGCLASS::TopQuark)
    DNNscore = discriminant[m_kerasConfigOutputName];

  return DNNscore;

}

std::map<std::string,double> JSSWTopTaggerDNN::getJetProperties( const xAOD::Jet& jet ) const {

  /// Map to store inputs
  std::map<std::string,double> DNN_inputValues;

  /// Calculate NSubjettiness and ECF ratios
  calculateJSSRatios(jet);

  ATH_MSG_DEBUG( "Loading variables for common DNN tagger" );
  
  /// Mass and pT
  /// It is assumed that these are the combined and calibrated mass and pT
  DNN_inputValues["CaloTACombinedMassUncorrelated"] = jet.m();
  DNN_inputValues["JetpTCorrByCombinedMass"] = jet.pt();

  /// Splitting scales
  DNN_inputValues["Split12"] = acc_Split12(jet);

  /// Energy Correlation Functions
  DNN_inputValues["C2"] = acc_C2(jet);
  DNN_inputValues["D2"] = acc_D2(jet);

  /// Tau21 WTA
  DNN_inputValues["Tau21_wta"] = acc_Tau21_wta(jet);

  if ( m_tagClass == TAGCLASS::WBoson ) {

    ATH_MSG_DEBUG( "Loading variables for W boson tagger" );

    /// Other moments
    DNN_inputValues["FoxWolfram20"] = jet.getAttribute<float>("FoxWolfram2") / jet.getAttribute<float>("FoxWolfram0");
    DNN_inputValues["PlanarFlow"] = jet.getAttribute<float>("PlanarFlow");
    DNN_inputValues["Angularity"] = jet.getAttribute<float>("Angularity");
    DNN_inputValues["Aplanarity"] = jet.getAttribute<float>("Aplanarity");
    DNN_inputValues["ZCut12"] = jet.getAttribute<float>("ZCut12");
    DNN_inputValues["KtDR"] = jet.getAttribute<float>("KtDR");
  
    //need pvLocation for getting Ntrk                                                                                                          
    int pvLocation=0;
    const xAOD::VertexContainer* vertexcontainer=0;
    if(evtStore()->retrieve( vertexcontainer, "PrimaryVertices" ) != StatusCode::SUCCESS){
      ATH_MSG_ERROR("No primary vertex container with name 'PrimaryVertices' was found");
      DNN_inputValues["Ntrk500"] = -999;
      pvLocation=-1;
    }else{
      for( auto vtx_itr : *vertexcontainer ){
	if(vtx_itr->vertexType() == xAOD::VxType::VertexType::PriVtx) {
	  break;
        }
        pvLocation++;
      }
    }
    //should get this from derivations, but if not available try to reconstruct it                                                              
    if (acc_NumTrkPt500.isAvailable( jet)){
      DNN_inputValues["Ntrk500"] = acc_NumTrkPt500(jet)[pvLocation];
    }else{
      ATH_MSG_DEBUG("'NumTrkPt500' is not available as a jet attribute. Try retrieving it from ungroomed parent jet.");
      if ( acc_parent(jet)  ) {
        ElementLink<xAOD::JetContainer> largeRJetParentLink=acc_parent(jet);
        if (largeRJetParentLink.isValid()){
          const xAOD::Jet* largeRJetParent {*largeRJetParentLink};
          if ( acc_NumTrkPt500.isAvailable( *largeRJetParent) ) {
	    DNN_inputValues["Ntrk500"] = (acc_NumTrkPt500( *largeRJetParent) )[pvLocation];
          }else{
            ATH_MSG_ERROR("The ungroomed parent jet doesn't have 'NumTrkPt500' as an attribute.");
            DNN_inputValues["Ntrk500"]=-999;
          }
        }else{
          ATH_MSG_ERROR("The link to the ungroomed parent jet is not valid.");
          DNN_inputValues["Ntrk500"]=-999;
        }
      }else{
        ATH_MSG_ERROR("No ungroomed parent jet available.");
	DNN_inputValues["Ntrk500"]=-999;
      }
    }

  }

  else if ( m_tagClass == TAGCLASS::TopQuark ) {
  
    ATH_MSG_DEBUG("Loading variables for top quark tagger");

    /// Mass and pT again
    DNN_inputValues["m"] = jet.m();
    DNN_inputValues["pt"] = jet.pt();

    /// Additional splitting scales
    DNN_inputValues["Split23"] = acc_Split23(jet);

    /// e3 := normalized ECF3/ECF1**3
    DNN_inputValues["e3"] = acc_e3(jet);

    /// N-subjettiness
    DNN_inputValues["Tau1_wta"] = acc_Tau1_wta(jet);
    DNN_inputValues["Tau2_wta"] = acc_Tau2_wta(jet);
    DNN_inputValues["Tau3_wta"] = acc_Tau3_wta(jet);
    if(acc_Tau4_wta.isAvailable(jet))
      DNN_inputValues["Tau4_wta"] = acc_Tau4_wta(jet);

    DNN_inputValues["Tau32_wta"] = acc_Tau32_wta(jet);
    if(acc_Tau42_wta.isAvailable(jet)){
      DNN_inputValues["Tau42_wta"] = acc_Tau42_wta(jet);
    }

    /// Qw observable for top tagging
    DNN_inputValues["Qw"] = acc_Qw(jet);

    if(acc_ThrustMaj.isAvailable(jet)){
      DNN_inputValues["ThrustMaj"] = acc_ThrustMaj(jet);
    }

    if(acc_L2.isAvailable(jet)){
      DNN_inputValues["L2"] = acc_L2(jet);
    }

    if(acc_L3.isAvailable(jet)){
      DNN_inputValues["L3"] = acc_L3(jet);
    }
  }
  
  else {
    ATH_MSG_ERROR( "Loading variables failed because the tagger type is not supported" );
  }

  return DNN_inputValues;

}

