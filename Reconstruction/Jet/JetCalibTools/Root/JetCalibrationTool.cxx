///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// JetCalibrationTool.cxx 
// Implementation file for class JetCalibrationTool
// Author: Joe Taenzer <joseph.taenzer@cern.ch>
/////////////////////////////////////////////////////////////////// 


// JetCalibTools includes
#include "JetCalibTools/JetCalibrationTool.h"
#include "PathResolver/PathResolver.h"

// Constructors
////////////////

JetCalibrationTool::JetCalibrationTool(const std::string& name)
  : JetCalibrationToolBase::JetCalibrationToolBase( name ),
    m_jetAlgo(""), m_config(""), m_calibSeq(""), m_calibAreaTag(""), m_originScale(""), m_shower(""), m_devMode(false), m_isData(true), m_timeDependentCalib(false), m_rhoKey("auto"), m_dir(""), m_eInfoName(""), m_globalConfig(nullptr),
    m_doBcid(true), m_doJetArea(true), m_doResidual(true), m_doOrigin(true), m_doGSC(true), m_doMC2MC(true),
    m_bcidCorr(nullptr), m_jetPileupCorr(nullptr), m_etaJESCorr(nullptr), m_globalSequentialCorr(nullptr), m_insituDataCorr(nullptr), m_jetMassCorr(nullptr), m_jetSmearCorr(nullptr), m_jetMC2MCCorr(nullptr), InsituCombMassCorr(nullptr), m_insituCombMassCorr(), m_genericScaleCorr(nullptr), m_useOriginVertex(false)
{ 
  declareProperty( "JetCollection", m_jetAlgo = "AntiKt4LCTopo" );
  declareProperty( "RhoKey", m_rhoKey = "auto" );
  declareProperty( "ConfigFile", m_config = "" );
  declareProperty( "CalibSequence", m_calibSeq = "JetArea_Offset_AbsoluteEtaJES_Insitu" );
  declareProperty( "IsData", m_isData = true );
  declareProperty( "ConfigDir", m_dir = "JetCalibTools/CalibrationConfigs/" );
  declareProperty( "EventInfoName", m_eInfoName = "EventInfo");
  declareProperty( "DEVmode", m_devMode = false);
  declareProperty( "OriginScale", m_originScale = "JetOriginConstitScaleMomentum");
  declareProperty( "CalibArea", m_calibAreaTag = "00-04-82");
  declareProperty( "ShowerModel", m_shower = "Py8" );
  declareProperty( "useOriginVertex", m_useOriginVertex = false);
}


// Destructor
///////////////
JetCalibrationTool::~JetCalibrationTool() {

  if (m_globalConfig) delete m_globalConfig;
  if (m_bcidCorr) delete m_bcidCorr;
  if (m_jetPileupCorr) delete m_jetPileupCorr;
  if (m_etaJESCorr) delete m_etaJESCorr;
  if (m_globalSequentialCorr) delete m_globalSequentialCorr;
  if (m_insituDataCorr) delete m_insituDataCorr;
  if (m_jetMassCorr) delete m_jetMassCorr;
  if (m_jetSmearCorr) delete m_jetSmearCorr;
  if (m_jetMC2MCCorr) delete m_jetMC2MCCorr;
  if (m_timeDependentCalib) delete InsituCombMassCorr;
  if (m_genericScaleCorr) delete m_genericScaleCorr;
}


/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

StatusCode JetCalibrationTool::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_CHECK( this->initializeTool( name() ) );
  return StatusCode::SUCCESS;
}

StatusCode JetCalibrationTool::finalize() {
  ATH_MSG_INFO ("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}


StatusCode JetCalibrationTool::initializeTool(const std::string& name) {

  TString jetAlgo = m_jetAlgo;
  TString config = m_config;
  TString calibSeq = m_calibSeq;
  std::string dir = m_dir;

  ATH_MSG_INFO("===================================");
  ATH_MSG_INFO("Initializing the xAOD Jet Calibration Tool for " << jetAlgo << "jets");

  //Make sure the necessary properties were set via the constructor or python configuration
  if ( jetAlgo.EqualTo("") || calibSeq.EqualTo("") ) {
    ATH_MSG_FATAL("JetCalibrationTool::initializeTool : At least one of your constructor arguments is not set. Did you use the copy constructor?");
    return StatusCode::FAILURE;
  }

  if ( config.EqualTo("") || !config ) { ATH_MSG_FATAL("No configuration file specified."); return StatusCode::FAILURE; } 
  // The calibration area tag is a property of the tool
  const std::string calibPath = "CalibArea-" + m_calibAreaTag + "/";
  if(m_devMode){
    ATH_MSG_WARNING("Dev Mode is ON!!!");
    ATH_MSG_WARNING("Dev Mode is NOT RECOMMENDED!!!");
    dir = "JetCalibTools/";
  }
  else{dir.insert(14,calibPath);} // Obtaining the path of the configuration file
  std::string configPath=dir+m_config; // Full path
  TString fn =  PathResolverFindCalibFile(configPath);

  ATH_MSG_INFO("Reading global JES settings from: " << m_config);
  ATH_MSG_INFO("resolved in: " << fn);
  
  m_globalConfig = new TEnv();
  //int status=m_globalConfig->ReadFile(FindFile(fn),EEnvLevel(0));
  int status=m_globalConfig->ReadFile(fn ,EEnvLevel(0));
  if (status!=0) { ATH_MSG_FATAL("Cannot read config file " << fn ); return StatusCode::FAILURE; }

  //Make sure that one of the standard jet collections is being used
  if ( calibSeq.Contains("JetArea") ) {
    if ( jetAlgo.Contains("PFlow") ) m_jetScale = PFLOW;
    else if ( jetAlgo.Contains("EM") ) m_jetScale = EM;
    else if ( jetAlgo.Contains("LC") ) m_jetScale = LC;
    else { ATH_MSG_FATAL("jetAlgo " << jetAlgo << " not recognized."); return StatusCode::FAILURE; }
  }

  //Set the default units to MeV, user can override by calling setUnitsGeV(true)
  setUnitsGeV(false);

  // Settings for R21/2.5.X
  m_originCorrectedClusters = m_globalConfig->GetValue("OriginCorrectedClusters",false);
  m_doSetDetectorEta = m_globalConfig->GetValue("SetDetectorEta",true);

  // Rho key specified in the config file?
  m_rhoKey_config = m_globalConfig->GetValue("RhoKey", "None");

  // Get name of vertex container
  m_vertexContainerName = m_globalConfig->GetValue("VertexContainerName","PrimaryVertices");

  //Make sure the residual correction is turned on if requested

  if ( !calibSeq.Contains("JetArea") && !calibSeq.Contains("Residual") ) {
    m_doJetArea = false;
    m_doResidual = false;
  } else if ( calibSeq.Contains("JetArea") ) {
    if ( m_rhoKey.compare("auto") == 0 && m_rhoKey_config.compare("None") == 0) {
      if(!m_originCorrectedClusters){
        if ( m_jetScale == EM ) m_rhoKey = "Kt4EMTopoEventShape";
        else if ( m_jetScale == LC ) m_rhoKey = "Kt4LCTopoEventShape";
        else if ( m_jetScale == PFLOW ) m_rhoKey = "Kt4EMPFlowEventShape";
      } else{
        if ( m_jetScale == EM ) m_rhoKey = "Kt4EMTopoOriginEventShape";
        else if ( m_jetScale == LC ) m_rhoKey = "Kt4LCTopoOriginEventShape";
        else if ( m_jetScale == PFLOW ) m_rhoKey = "Kt4EMPFlowEventShape";
      }
    }
    else if(m_rhoKey_config.compare("None") != 0 && m_rhoKey.compare("auto") == 0){
      m_rhoKey = m_rhoKey_config;
    }
    if ( !calibSeq.Contains("Residual") ) m_doResidual = false;
  } else if ( !calibSeq.Contains("JetArea") && calibSeq.Contains("Residual") ) {
    m_doJetArea = false;
    ATH_MSG_INFO("ApplyOnlyResidual should be true if only Residual pile up correction wants to be applied. Need to specify pile up starting scale in the configuration file.");
  }
  // get nJet threshold and name
  m_useNjetInResidual = m_globalConfig->GetValue("OffsetCorrection.UseNjet", false);
  m_nJetPtThreshold = m_globalConfig->GetValue("OffsetCorrection.nJetPtThreshold", 20);
  m_nJetContainerName = m_globalConfig->GetValue("OffsetCorrection.nJetContainerName", "HLT_xAOD__JetContainer_a4tcemsubjesISFS");

  m_doMuOnlyInResidual = m_globalConfig->GetValue("ApplyOnlyMuResidual", false);

  // save the pt after area-based correction for jets in the jet container used to calculate nJet
  m_saveAreaCorrectedScaleMomentum = m_globalConfig->GetValue("SaveAreaCorrectedScaleMomentum",false);

  if ( !calibSeq.Contains("Origin") ) m_doOrigin = false;

  if ( !calibSeq.Contains("GSC") && !calibSeq.Contains("GNNC")) m_doGSC = false;

  if ( !calibSeq.Contains("Bcid") ) m_doBcid = false;

  if ( !calibSeq.Contains("MC2MC")) m_doMC2MC = false;

  //Protect against the in-situ calibration being requested when isData is false
  if ( calibSeq.Contains("Insitu") && !m_isData ) {
    ATH_MSG_FATAL("JetCalibrationTool::initializeTool : calibSeq string contains Insitu with isData set to false. Can't apply in-situ correction to MC!!");
    return StatusCode::FAILURE;
  }

  // Time-Dependent Insitu Calibration
  m_timeDependentCalib = m_globalConfig->GetValue("TimeDependentInsituCalibration",false);
  if(m_timeDependentCalib && calibSeq.Contains("Insitu")){ // Read Insitu Configs
    m_timeDependentInsituConfigs = JetCalibUtils::Vectorize( m_globalConfig->GetValue("InsituTimeDependentConfigs","") );
    if(m_timeDependentInsituConfigs.size()==0) ATH_MSG_ERROR("Please check there are at least two insitu configs");
    m_runBins = JetCalibUtils::VectorizeD( m_globalConfig->GetValue("InsituRunBins","") );
    if(m_runBins.size()!=m_timeDependentInsituConfigs.size()+1) ATH_MSG_ERROR("Please check the insitu run bins");
    for(unsigned int i=0;i<m_timeDependentInsituConfigs.size();++i){
      //InsituDataCorrection *insituTemp = NULL;
      //m_insituTimeDependentCorr.push_back(insituTemp);

      std::string configPath_insitu = dir+m_timeDependentInsituConfigs.at(i).Data(); // Full path
      TString fn_insitu =  PathResolverFindCalibFile(configPath_insitu);

      ATH_MSG_INFO("Reading time-dependent insitu settings from: " << m_timeDependentInsituConfigs.at(i));
      ATH_MSG_INFO("resolved in: " << fn_insitu);
  
      TEnv *globalConfig_insitu = new TEnv();
      int status = globalConfig_insitu->ReadFile(fn_insitu ,EEnvLevel(0));
      if (status!=0) { ATH_MSG_FATAL("Cannot read config file " << fn_insitu ); return StatusCode::FAILURE; }
      m_globalTimeDependentConfigs.push_back(globalConfig_insitu);
    }
  }

  // Combined Mass Calibration
  m_insituCombMassCalib = m_globalConfig->GetValue("InsituCombinedMassCorrection",false);
  if(m_insituCombMassCalib && calibSeq.Contains("InsituCombinedMass")){ // Read Combination Config
    m_insituCombMassConfig = JetCalibUtils::Vectorize( m_globalConfig->GetValue("InsituCombinedMassCorrectionFile","") );
    if(m_insituCombMassConfig.size()==0) ATH_MSG_ERROR("Please check there is a combination config");
    for(unsigned int i=0;i<m_insituCombMassConfig.size();++i){

      std::string configPath_comb = dir+m_insituCombMassConfig.at(i).Data(); // Full path
      TString fn_comb =  PathResolverFindCalibFile(configPath_comb);

      ATH_MSG_INFO("Reading combination settings from: " << m_insituCombMassConfig.at(i));
      ATH_MSG_INFO("resolved in: " << fn_comb);

      TEnv *globalInsituCombMass = new TEnv();
      int status = globalInsituCombMass->ReadFile(fn_comb ,EEnvLevel(0));
      if (status!=0) { ATH_MSG_FATAL("Cannot read config file " << fn_comb ); return StatusCode::FAILURE; }
      m_globalInsituCombMassConfig.push_back(globalInsituCombMass);
    }
  }

  //Loop over the request calib sequence
  //Initialize derived classes for applying the requested calibrations and add them to a vector
  std::vector<TString> vecCalibSeq = JetCalibUtils::Vectorize(calibSeq,"_");
  TString vecCalibSeqtmp;
  for ( unsigned int i=0; i<vecCalibSeq.size(); ++i) {
    if ( vecCalibSeq[i].EqualTo("Origin") || vecCalibSeq[i].EqualTo("DEV") ) continue;
    if ( vecCalibSeq[i].EqualTo("Residual") && m_doJetArea ) continue;
    ATH_CHECK( getCalibClass(name,vecCalibSeq[i] ));
  }

  ATH_MSG_INFO("===================================");

  return StatusCode::SUCCESS;
}

//Method for initializing the requested calibration derived classes
StatusCode JetCalibrationTool::getCalibClass(const std::string&name, TString calibration) {
  TString jetAlgo = m_jetAlgo;
  TString shower = m_shower;
  const TString calibPath = "CalibArea-" + m_calibAreaTag + "/";
  std::string suffix = "";
  //ATH_MSG_INFO("Initializing sub tools.");
  if ( calibration.EqualTo("Bcid") ){
    ATH_MSG_INFO("Initializing BCID correction for data.");
    suffix="_Bcid";
    if(m_devMode) suffix+="_DEV";
    m_globalConfig->SetValue("PileupStartingScale","JetBcidScaleMomentum");
    m_bcidCorr = new BcidOffsetCorrection(name+suffix,m_globalConfig,jetAlgo,calibPath,m_isData,m_devMode);
    if ( m_bcidCorr->initializeTool(name+suffix).isFailure() ) {
      ATH_MSG_FATAL("Couldn't initialize the BCID Offset correction. Aborting"); 
      return StatusCode::FAILURE; 
    } else { 
      m_calibClasses.push_back(m_bcidCorr); 
      return StatusCode::SUCCESS; 
    }
  } else if ( calibration.EqualTo("JetArea") || calibration.EqualTo("Residual") ) {
    ATH_MSG_INFO("Initializing pileup correction.");
    suffix="_Pileup";
    if(m_devMode) suffix+="_DEV";
    m_jetPileupCorr = new JetPileupCorrection(name+suffix,m_globalConfig,jetAlgo,calibPath,m_doResidual,m_doJetArea,m_doOrigin,m_isData,m_devMode);
    ATH_CHECK( m_jetPileupCorr->setProperty("OriginScale",m_originScale.c_str()) );
    m_jetPileupCorr->msg().setLevel( this->msg().level() );
    if( m_jetPileupCorr->initializeTool(name+suffix).isFailure() ) { 
      ATH_MSG_FATAL("Couldn't initialize the pileup correction. Aborting"); 
      return StatusCode::FAILURE; 
    } else { 
      m_calibClasses.push_back(m_jetPileupCorr); 
      return StatusCode::SUCCESS; 
    }
  } else if ( calibration.EqualTo("EtaJES") || calibration.EqualTo("AbsoluteEtaJES") ) {
    ATH_MSG_INFO("Initializing JES correction.");
    suffix="_EtaJES";
    if(m_devMode) suffix+="_DEV";
    m_etaJESCorr = new EtaJESCorrection(name+suffix,m_globalConfig,jetAlgo,calibPath,false,m_devMode);
    m_etaJESCorr->msg().setLevel( this->msg().level() );
    if ( m_etaJESCorr->initializeTool(name+suffix).isFailure() ) {
      ATH_MSG_FATAL("Couldn't initialize the Monte Carlo JES correction. Aborting"); 
      return StatusCode::FAILURE; 
    } else { 
      m_calibClasses.push_back(m_etaJESCorr); 
      return StatusCode::SUCCESS; 
    }
  } else if ( calibration.EqualTo("EtaMassJES") ) {
    ATH_MSG_INFO("Initializing JES correction.");
    suffix="_EtaMassJES";
    if(m_devMode) suffix+="_DEV";
    m_etaJESCorr = new EtaJESCorrection(name+suffix,m_globalConfig,jetAlgo,calibPath,true,m_devMode);
    m_etaJESCorr->msg().setLevel( this->msg().level() );
    if ( m_etaJESCorr->initializeTool(name+suffix).isFailure() ) {
      ATH_MSG_FATAL("Couldn't initialize the Monte Carlo JES correction. Aborting");
      return StatusCode::FAILURE;
    } else {
      m_calibClasses.push_back(m_etaJESCorr);
      return StatusCode::SUCCESS;
    }
  } else if ( calibration.EqualTo("GSC") ) {
    ATH_MSG_INFO("Initializing GSC correction.");
    suffix="_GSC";
    if(m_devMode) suffix+="_DEV";
    m_globalSequentialCorr = new GlobalSequentialCorrection(name+suffix,m_globalConfig,jetAlgo,calibPath,m_useOriginVertex,m_devMode);
    m_globalSequentialCorr->msg().setLevel( this->msg().level() );
    if ( m_globalSequentialCorr->initializeTool(name+suffix).isFailure() ) {
      ATH_MSG_FATAL("Couldn't initialize the Global Sequential Calibration. Aborting"); 
      return StatusCode::FAILURE; 
    } else { 
      m_calibClasses.push_back(m_globalSequentialCorr); 
      return StatusCode::SUCCESS; 
    }
  } else if ( calibration.EqualTo("GNNC") ) {
    ATH_MSG_INFO("Initializing GenNI correction.");
    suffix="_GenNI";
    if(m_devMode) suffix+="_DEV";
    m_globalNNCorr = new GlobalNNCalibration(name+suffix,m_globalConfig,jetAlgo,calibPath,m_devMode);
    m_globalNNCorr->msg().setLevel( this->msg().level() );
    if ( m_globalNNCorr->initializeTool(name+suffix).isFailure() ) {
      ATH_MSG_FATAL("Couldn't initialize the Global Sequential Calibration. Aborting");
      return StatusCode::FAILURE;
    } else {
      m_calibClasses.push_back(m_globalNNCorr);
      return StatusCode::SUCCESS;
    }
  } else if ( calibration.EqualTo("JMS") ) {
    ATH_MSG_INFO("Initializing JMS correction.");
    suffix="_JMS";
    if(m_devMode) suffix+="_DEV";
    m_jetMassCorr = new JMSCorrection(name+suffix,m_globalConfig,jetAlgo,calibPath,m_devMode);
    m_jetMassCorr->msg().setLevel( this->msg().level() );
    if ( m_jetMassCorr->initializeTool(name+suffix).isFailure() ) {
      ATH_MSG_FATAL("Couldn't initialize the JMS Calibration. Aborting");
      return StatusCode::FAILURE;
    } else {
      m_calibClasses.push_back(m_jetMassCorr);
      return StatusCode::SUCCESS;
    }
   } else if ( calibration.EqualTo("InsituCombinedMass") ){
      ATH_MSG_INFO("Initializing Combined Mass Correction"); 
      for(unsigned int i=0;i<m_insituCombMassConfig.size();++i){
        suffix="_InsituCombinedMass"; suffix += "_"; suffix += std::to_string(i);
        if(m_devMode) suffix+="_DEV";
        InsituCombMassCorr = new JMSCorrection(name+suffix,m_globalInsituCombMassConfig.at(i),jetAlgo,calibPath,m_devMode);
        InsituCombMassCorr->msg().setLevel( this->msg().level() );
        if ( InsituCombMassCorr->initializeTool(name+suffix).isFailure() ) {
          ATH_MSG_FATAL("Couldn't initialize the Combined Mass correction. Aborting");
          return StatusCode::FAILURE;
        } else {
          m_insituCombMassCorr.push_back(InsituCombMassCorr);
          return StatusCode::SUCCESS;
        }
      }
  } else if ( calibration.EqualTo("Insitu") ) {
    if(!m_timeDependentCalib){
      ATH_MSG_INFO("Initializing Insitu correction.");
      suffix="_Insitu";
      if(m_devMode) suffix+="_DEV";
      m_insituDataCorr = new InsituDataCorrection(name+suffix,m_globalConfig,jetAlgo,calibPath,m_devMode);
      m_insituDataCorr->msg().setLevel( this->msg().level() );
      if ( m_insituDataCorr->initializeTool(name+suffix).isFailure() ) {
        ATH_MSG_FATAL("Couldn't initialize the In-situ data correction. Aborting"); 
        return StatusCode::FAILURE; 
      } else { 
        m_calibClasses.push_back(m_insituDataCorr);
	m_relInsituPtMax.push_back(m_insituDataCorr->getRelHistoPtMax());
	m_absInsituPtMax.push_back(m_insituDataCorr->getAbsHistoPtMax());
        return StatusCode::SUCCESS; 
      }
    } else{
      ATH_MSG_INFO("Initializing Time-Dependent Insitu Corrections");
      for(unsigned int i=0;i<m_timeDependentInsituConfigs.size();++i){
        suffix="_Insitu"; suffix += "_"; suffix += std::to_string(i);
        if(m_devMode) suffix+="_DEV";
        InsituDataCorrection *insituTimeDependentCorr_Tmp = new InsituDataCorrection(name+suffix,m_globalTimeDependentConfigs.at(i),jetAlgo,calibPath,m_devMode);
        insituTimeDependentCorr_Tmp->msg().setLevel( this->msg().level() );
        if ( insituTimeDependentCorr_Tmp->initializeTool(name+suffix).isFailure() ) {
          ATH_MSG_FATAL("Couldn't initialize the In-situ data correction. Aborting"); 
          return StatusCode::FAILURE; 
        } else {     		
          m_insituTimeDependentCorr.push_back(insituTimeDependentCorr_Tmp);
	  m_relInsituPtMax.push_back(insituTimeDependentCorr_Tmp->getRelHistoPtMax());
	  m_absInsituPtMax.push_back(insituTimeDependentCorr_Tmp->getAbsHistoPtMax());
        }
      }
      return StatusCode::SUCCESS; 
    }
  } else if ( calibration.EqualTo("MC2MC") ) {
    if (m_isData)
    {
      ATH_MSG_FATAL("Asked for MC2MC of data, which is not supported.  Aborting.");
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Initializing MC2MC correction.");
    suffix="_MC2MC";
    m_jetMC2MCCorr = new MC2MCCorrection(name+suffix,m_globalConfig,jetAlgo,calibPath,shower,m_devMode);
    m_jetMC2MCCorr->msg().setLevel(this->msg().level());
    if ( m_jetMC2MCCorr->initializeTool(name+suffix).isFailure() ) {
      ATH_MSG_FATAL("Couldn't initialize the MC2MC correction. Aborting"); 
      return StatusCode::FAILURE; 
    } else { 
      m_calibClasses.push_back(m_jetMC2MCCorr);
      return StatusCode::SUCCESS; 
    }
  } else if ( calibration.EqualTo("Smear") ) {
    if (m_isData)
    {
      ATH_MSG_FATAL("Asked for smearing of data, which is not supported.  Aborting.");
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Initializing jet smearing correction");
    suffix = "_Smear";
    if (m_devMode) suffix += "_DEV";
    
    m_jetSmearCorr = new JetSmearingCorrection(name+suffix,m_globalConfig,jetAlgo,calibPath,m_devMode);
    m_jetSmearCorr->msg().setLevel(this->msg().level());

    if (m_jetSmearCorr->initializeTool(name+suffix).isFailure())
    {
      ATH_MSG_FATAL("Couldn't initialize the jet smearing correction.  Aborting.");
      return StatusCode::FAILURE;
    }
    m_calibClasses.push_back(m_jetSmearCorr);
    return StatusCode::SUCCESS;
  } else if ( calibration.EqualTo("GenericCorr") ) {
    ATH_MSG_INFO("Initializing a generic correction factor");
    suffix = "_GenericCorr";
    if (m_devMode) suffix += "_DEV";

    m_genericScaleCorr = new GenericHistScaleCorrection(name+suffix,m_globalConfig,jetAlgo,calibPath,m_devMode);
    m_genericScaleCorr->msg().setLevel(this->msg().level());

    if (m_genericScaleCorr->initializeTool(name+suffix).isFailure())
    {
        ATH_MSG_FATAL("Couldn't initialize the generic jet correction tool.  Aborting.");
        return StatusCode::FAILURE;
    }
    m_calibClasses.push_back(m_genericScaleCorr);
    return StatusCode::SUCCESS;
  }
  ATH_MSG_FATAL("Calibration string not recognized: " << calibration << ", aborting.");
  return StatusCode::FAILURE;
}

CP::CorrectionCode JetCalibrationTool::applyCorrection(xAOD::Jet& jet) {
  StatusCode sc = applyCalibration(jet);
  if ( sc != StatusCode::FAILURE ) return CP::CorrectionCode::Ok;
  return CP::CorrectionCode::Error;
}

StatusCode JetCalibrationTool::applyCalibration(xAOD::Jet& jet) const { 
  //Grab necessary event info for pile up correction and store it in a JetEventInfo class object
  JetEventInfo jetEventInfo;
  ATH_MSG_VERBOSE("Extracting event information.");
  ATH_CHECK( initializeEvent(jetEventInfo) );
  ATH_MSG_VERBOSE("Applying calibration.");
  ATH_CHECK( calibrateImpl(jet,jetEventInfo) );
  return StatusCode::SUCCESS; 
}


int JetCalibrationTool::modify(xAOD::JetContainer& jets) const {
  //Grab necessary event info for pile up correction and store it in a JetEventInfo class object
  ATH_MSG_VERBOSE("Modifying jet collection.");
  JetEventInfo jetEventInfo;
  ATH_CHECK( initializeEvent(jetEventInfo) );
  xAOD::JetContainer::iterator jet_itr = jets.begin();
  xAOD::JetContainer::iterator jet_end = jets.end(); 
  for ( ; jet_itr != jet_end; ++jet_itr )
    ATH_CHECK( calibrateImpl(**jet_itr,jetEventInfo) );
 return 0;
}

int JetCalibrationTool::modifyJet(xAOD::Jet& jet) const {
  ATH_MSG_VERBOSE("Modifying jet.");
  ATH_CHECK( applyCalibration(jet) );
  return 0;
}

// Private/Protected Methods
///////////////

StatusCode JetCalibrationTool::initializeEvent(JetEventInfo& jetEventInfo) const {

  // Check if the tool was initialized
  if( m_calibClasses.size() == 0 ){
    ATH_MSG_FATAL("   JetCalibrationTool::initializeEvent : The tool was not initialized.");
    return StatusCode::FAILURE;
  }

  // static accessor for PV index access
  static SG::AuxElement::ConstAccessor<int> PVIndexAccessor("PVIndex");
  
  ATH_MSG_VERBOSE("Initializing event.");

  //Determine the rho value to use for the jet area subtraction
  //Should be determined using EventShape object, use hard coded values if EventShape doesn't exist
  double rho=0;
  const xAOD::EventShape * eventShape = 0;
  if ( m_doJetArea && evtStore()->contains<xAOD::EventShape>(m_rhoKey) ) {
    ATH_MSG_VERBOSE("  Found event density container " << m_rhoKey);
    if ( evtStore()->retrieve(eventShape, m_rhoKey).isFailure() || !eventShape ) {
      ATH_MSG_VERBOSE("  Event shape container not found.");
      ATH_MSG_FATAL("Could not retrieve xAOD::EventShape from evtStore.");
      return StatusCode::FAILURE;
    } else if ( !eventShape->getDensity( xAOD::EventShape::Density, rho ) ) {
      ATH_MSG_VERBOSE("  Event density not found in container.");
      ATH_MSG_FATAL("Could not retrieve xAOD::EventShape::Density from xAOD::EventShape.");
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_VERBOSE("  Event density retrieved.");
    }
  } else if ( m_doJetArea && !evtStore()->contains<xAOD::EventShape>(m_rhoKey) ) {
    ATH_MSG_VERBOSE("  Rho container not found: " << m_rhoKey);
    ATH_MSG_FATAL("Could not retrieve xAOD::EventShape from evtStore.");
    return StatusCode::FAILURE;
  }
  jetEventInfo.setRho(rho);
  ATH_MSG_VERBOSE("  Rho = " << 0.001*rho << " GeV");

  // Necessary retrieval and calculation for use of nJetX instead of NPV
  if(m_useNjetInResidual || m_saveAreaCorrectedScaleMomentum) {
    // retrieve the container
    const xAOD::JetContainer * jets = 0;
    if (evtStore()->contains<xAOD::JetContainer>(m_nJetContainerName) ) {
      ATH_MSG_VERBOSE("  Found jet container " << m_nJetContainerName);
      if ( evtStore()->retrieve(jets, m_nJetContainerName).isFailure() || !jets ) {
        ATH_MSG_FATAL("Could not retrieve xAOD::JetContainer from evtStore.");
        return StatusCode::FAILURE;
      }
    } else {
      ATH_MSG_FATAL("Could not find jet container " << m_nJetContainerName);
      return StatusCode::FAILURE;
    }

    // count jets above threshold
    int nJets = 0;
    for (auto jet : *jets) {
      xAOD::JetFourMom_t trigjetconstitP4 = jet->getAttribute<xAOD::JetFourMom_t>("JetPileupScaleMomentum");

      // At the moment, the Njet-based pile-up residual calibration is only used to calibrate (HLT) trigger jets from data already taken.
      // Online trigger jets have no pile-up residual calibration applied, so jets at JetPileupScaleMomentum scale have only the area-based pile-up calibration.
      // The pT at this scale is saved as JetAreaCorrectedPt so it can be used (elsewhere) to calculate Njet even when an offline pile-up residual calibration is applied and hence JetPileupScaleMomentum scale is overwritten
      jet->auxdecor<float>("JetAreaCorrectedPt") = trigjetconstitP4.Pt();

      if(trigjetconstitP4.pt()/m_GeV > m_nJetPtThreshold)
        nJets += 1;
    }
    jetEventInfo.setNjet(nJets);

  }

  // Retrieve EventInfo object, which now has multiple uses
  if ( m_doResidual || m_doGSC || m_doBcid || m_doMC2MC) {
    const xAOD::EventInfo * eventObj = 0;
    static unsigned int eventInfoWarnings = 0;
    if ( evtStore()->retrieve(eventObj,m_eInfoName).isFailure() || !eventObj ) {
      ++eventInfoWarnings;
      if ( eventInfoWarnings < 20 )
        ATH_MSG_ERROR("   JetCalibrationTool::initializeEvent : Failed to retrieve event information.");
      jetEventInfo.setMu(0); //Hard coded value mu = 0 in case of failure (to prevent seg faults later).
      jetEventInfo.setPVIndex(0);
      return StatusCode::SUCCESS; //error is recoverable, so return SUCCESS
    }

    // If we are applying the reisdual, then store mu
    if (m_doResidual || m_doBcid)
      jetEventInfo.setMu( eventObj->averageInteractionsPerCrossing() );
    
    // If this is GSC, we need EventInfo to determine the PV to use
    // This is support for groups where PV0 is not the vertex of interest (H->gamgam, etc)
    if (m_doGSC)
    {
      // First retrieve the PVIndex if specified
      // Default is to not specify this, so no warning if it doesn't exist
      // However, if specified, it should be a sane value - fail if not
      if ( m_doGSC && PVIndexAccessor.isAvailable(*eventObj) )
        jetEventInfo.setPVIndex( PVIndexAccessor(*eventObj) );
      else
        jetEventInfo.setPVIndex(0);
      
    }

    if (m_doMC2MC)
    {
      jetEventInfo.setChannelNumber( eventObj->mcChannelNumber() );
    }

    // Extract the BCID information for the BCID correction
    if (m_doBcid)
    {
      jetEventInfo.setRunNumber( eventObj->runNumber() );
      jetEventInfo.setBcidDistanceFromFront( eventObj->auxdata<int>("DFCommonJets_BCIDDistanceFromFront") );
      jetEventInfo.setBcidGapBeforeTrain( eventObj->auxdata<int>("DFCommonJets_BCIDGapBeforeTrain") );
      jetEventInfo.setBcidGapBeforeTrainMinus12( eventObj->auxdata<int>("DFCommonJets_BCIDGapBeforeTrainMinus12") );
    }

    // If PV index is not zero, we need to confirm it's a reasonable value
    // To do this, we need the primary vertices
    // However, other users of the GSC may not have the PV collection (in particular: trigger GSC in 2016)
    // So only retrieve vertices if needed for NPV (residual) or a non-zero PV index was specified (GSC)
    if ((m_doResidual && !( m_useNjetInResidual || m_doMuOnlyInResidual )) || (m_doGSC && jetEventInfo.PVIndex()))
    {
      //Retrieve VertexContainer object, use it to obtain NPV for the residual correction or check validity of GSC non-PV0 usage
      const xAOD::VertexContainer * vertices = 0;
      static unsigned int vertexContainerWarnings = 0;
      if ( evtStore()->retrieve(vertices,m_vertexContainerName).isFailure() || !vertices ) {
        ++vertexContainerWarnings;
        if ( vertexContainerWarnings < 20 )
          ATH_MSG_ERROR("   JetCalibrationTool::initializeEvent : Failed to retrieve primary vertices.");
        jetEventInfo.setNPV(0); //Hard coded value NPV = 0 in case of failure (to prevent seg faults later).
        return StatusCode::SUCCESS; //error is recoverable, so return SUCCESS
      }

      // Calculate and set NPV if this is residual
      if (m_doResidual)
      {
        int eventNPV = 0;
        xAOD::VertexContainer::const_iterator vtx_itr = vertices->begin();
        xAOD::VertexContainer::const_iterator vtx_end = vertices->end(); 
        for ( ; vtx_itr != vtx_end; ++vtx_itr ) 
          if ( (*vtx_itr)->nTrackParticles() >= 2 ) ++eventNPV;
  
        jetEventInfo.setNPV(eventNPV);
      }
      
      // Validate value of non-standard PV index usage
      if (m_doGSC && jetEventInfo.PVIndex())
      {
        static unsigned int vertexIndexWarnings = 0;
        if (jetEventInfo.PVIndex() < 0 || static_cast<size_t>(jetEventInfo.PVIndex()) >= vertices->size())
        {
          ++vertexIndexWarnings;
          if (vertexIndexWarnings < 20)
            ATH_MSG_ERROR("   JetCalibrationTool::initializeEvent : PV index is out of bounds.");
          jetEventInfo.setPVIndex(0); // Hard coded value PVIndex = 0 in case of failure (to prevent seg faults later).
          return StatusCode::SUCCESS; // error is recoverable, so return SUCCESS
        }
      }
    }

    //Check if the input jets are coming from data or MC
    //if ( m_eventObj->eventType( xAOD::EventInfo::IS_SIMULATION ) ) {
    //     m_isData = false; // controls mu scaling in the pile up correction, no scaling for data
    //}

  }

    return StatusCode::SUCCESS;
}

StatusCode JetCalibrationTool::calibrateImpl(xAOD::Jet& jet, JetEventInfo& jetEventInfo) const {

  //Check for OriginCorrected and PileupCorrected attributes, assume they are false if not found
  int tmp = 0; //temporary int for checking getAttribute
  if ( !jet.getAttribute<int>("OriginCorrected",tmp) )
    jet.setAttribute<int>("OriginCorrected",false);
  if ( !jet.getAttribute<int>("PileupCorrected",tmp) )
    jet.setAttribute<int>("PileupCorrected",false);

  ATH_MSG_VERBOSE("Calibrating jet " << jet.index());
  if(m_doSetDetectorEta) {
    xAOD::JetFourMom_t jetconstitP4 = jet.getAttribute<xAOD::JetFourMom_t>("JetConstitScaleMomentum");
    jet.setAttribute<float>("DetectorEta",jetconstitP4.eta()); //saving constituent scale eta for later use
  }
  for (unsigned int i=0; i<m_calibClasses.size(); ++i) //Loop over requested calibations
    ATH_CHECK ( m_calibClasses[i]->calibrateImpl(jet,jetEventInfo) );
  TString CalibSeq = m_calibSeq;
  if(CalibSeq.Contains("Insitu") && m_timeDependentCalib){ // Insitu Time-Dependent Correction
    for(unsigned int i=0;i<m_timeDependentInsituConfigs.size();++i){
      // Retrive EventInfo container
      const xAOD::EventInfo* eventInfo(nullptr);
      if( evtStore()->retrieve(eventInfo,"EventInfo").isFailure() || !eventInfo ) {
          ATH_MSG_ERROR("   JetCalibrationTool::calibrateImpl : Failed to retrieve EventInfo.");
      }
      // Run Number Dependent Correction
      double runNumber = eventInfo->runNumber();
      if(runNumber>m_runBins.at(i) && runNumber<=m_runBins.at(i+1)){ ATH_CHECK ( m_insituTimeDependentCorr.at(i)->calibrateImpl(jet,jetEventInfo) );}
    }
  }
  if(CalibSeq.Contains("InsituCombinedMass") && m_insituCombMassCalib){
   for(unsigned int i=0;i<m_insituCombMassConfig.size();++i){
      //Retrive EventInfo container
      const xAOD::EventInfo* eventInfo(nullptr);
      if( evtStore()->retrieve(eventInfo,"EventInfo").isFailure() || !eventInfo ) {
       ATH_MSG_ERROR("   JetCalibrationTool::calibrateImpl : Failed to retrieve EventInfo.");
      }
      ATH_CHECK ( m_insituCombMassCorr.at(i)->calibrateImpl(jet,jetEventInfo) );
   }
  }
  return StatusCode::SUCCESS; 
}


StatusCode JetCalibrationTool::getNominalResolutionData(const xAOD::Jet& jet, double& resolution) const
{
    if (!m_jetSmearCorr)
    {
        ATH_MSG_ERROR("Cannot retrieve the nominal data resolution - smearing was not configured during initialization");
        return StatusCode::FAILURE;
    }
    return m_jetSmearCorr->getNominalResolutionData(jet,resolution);
}

StatusCode JetCalibrationTool::getNominalResolutionMC(const xAOD::Jet& jet, double& resolution) const
{
    if (!m_jetSmearCorr)
    {
        ATH_MSG_ERROR("Cannot retrieve the nominal MC resolution - smearing was not configured during initialization");
        return StatusCode::FAILURE;
    }
    return m_jetSmearCorr->getNominalResolutionMC(jet,resolution);
}

