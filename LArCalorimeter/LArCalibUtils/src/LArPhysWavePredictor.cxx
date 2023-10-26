/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibUtils/LArPhysWavePredictor.h"

#include "GaudiKernel/ToolHandle.h"
#include "AthenaKernel/ClassID_traits.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"

#include "LArRawConditions/LArCaliWave.h"
#include "LArRawConditions/LArCaliWaveContainer.h"
#include "LArRawConditions/LArPhysWave.h"
#include "LArRawConditions/LArPhysWaveContainer.h"

#include "LArRawConditions/LArFEBTimeOffset.h"
#include "LArRawConditions/LArWFParams.h"

#include "LArElecCalib/ILArCaliPulseParams.h"
#include "LArElecCalib/ILArDetCellParams.h"
#include "LArElecCalib/ILArPhysCaliTdiff.h"
#include "LArElecCalib/ILArTdrift.h"
#include "LArElecCalib/ILArMphysOverMcal.h"

#include "LArRawConditions/LArCaliPulseParamsComplete.h"
#include "LArRawConditions/LArDetCellParamsComplete.h"
#include "LArRawConditions/LArPhysCaliTdiffComplete.h"
#include "LArRawConditions/LArTdriftComplete.h"
#include "LArRawConditions/LArMphysOverMcalComplete.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"

#include "LArCalibUtils/LArPhysWaveTool.h"     // added by FT
#include "LArCalibUtils/LArPhysWaveHECTool.h"  // added by FT

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>


typedef LArPhysWaveContainer::ConstConditionsMapIterator PhysWaveIt;

LArPhysWavePredictor::LArPhysWavePredictor (const std::string& name, ISvcLocator* pSvcLocator) 
 : AthAlgorithm(name, pSvcLocator),
   m_groupingType("FeedThrough") // SubDetector, Single, FeedThrough
{
  declareProperty("TestMode",         m_testmode   = false);
  declareProperty("StoreEmpty",       m_storeEmpty = false);
  declareProperty("isSC", m_isSC = false);

  m_keyCali.clear() ;
  declareProperty("KeyCaliList",      m_keyCali);                  // Keys of LArCaliWaveContainers
  declareProperty("KeyPhys",          m_keyPhys = "LArPhysWave") ; // Key of LArPhysWaveContainer
  declareProperty("KeyIdealPhys",     m_keyIdealPhys   = "LArPhysWaveHECIdeal") ; // added by FT    
  declareProperty("KeyMphysMcali",    m_keyMphysMcali = "LArMphysOverMcal") ; // Key of LArMphysOverMcalComplete
  declareProperty("DumpMphysMcali",   m_dumpMphysMcali = false ) ; // for debugging
  declareProperty("NormalizeCali",    m_normalizeCali = false ) ; // for debugging
  declareProperty("isHEC",            m_isHEC = false ) ; 

  //
  // jobOptions for calibration pulse paramers (Tcali, Fstep)
  //
  declareProperty("UseCaliPulseParamsFromJO", m_useJOCaliPulseParams = false );
  declareProperty("Tcali", m_Tcali = 450. ); 
  declareProperty("Fstep", m_Fstep = 0.07 ); 
  
  declareProperty("UseDetCellParamsFromJO"  , m_useJODetCellParams   = false );
  declareProperty("Omega0", m_Omega0 = 0. ); // Omega0 == 0 will skip injection point correction 
  declareProperty("Taur",   m_Taur   = 0. ); 
  
  //
  // jobOptions for drift time of predicted pulses (Tdrift)
  //
  declareProperty("UseTdriftFromJO", m_useJOTdrift = false );
  m_Tdrift.clear();
  double default_Tdrift[4] = { 420 , 475 , 475 , 475 } ;
  for ( unsigned i=0;i<4;i++) m_Tdrift.push_back(default_Tdrift[i]);
  declareProperty("Tdrift", m_Tdrift) ;
  m_Tdrift2.clear();
  double default_Tdrift2[4] = { 1200. , 1200. , 1200. , 1200. } ;
  for ( unsigned i=0;i<4;i++) m_Tdrift2.push_back(default_Tdrift2[i]);
  declareProperty("Tdrift2", m_Tdrift2) ;
  m_wTriangle2.clear();
  double default_wTriangle2[4] = { 0.01 , 0.01 , 0.01 , 0.01 } ;
  for ( unsigned i=0;i<4;i++) m_wTriangle2.push_back(default_wTriangle2[i]);
  declareProperty("WeightTriangle2", m_wTriangle2) ;
  declareProperty("UseDoubleTriangle", m_doubleTriangle = false );
  
  //
  // jobOptions for time shift of predicted pulses
  // 
  declareProperty("UseTimeShiftFromJO",   m_useJOPhysCaliTdiff   = false );
  declareProperty("TimeShiftByIndex" ,    m_timeShiftByIndex     = 0);
  declareProperty("TimeShiftByHelper",    m_timeShiftByHelper    = false);
  declareProperty("TimeShiftByLayer",     m_timeShiftByLayer     = false) ;
  declareProperty("TimeShiftByFEB",       m_timeShiftByFEB       = false) ;
  declareProperty("TimeShiftGuardRegion", m_timeShiftGuardRegion = 0 ) ;  
  m_TshiftLayer.clear();
  unsigned int default_TshiftLayer[4] = { 0 , 0 , 0 , 0 } ;
  for ( unsigned i=0;i<4;i++)  m_TshiftLayer.push_back(default_TshiftLayer[i]);
  declareProperty("Tshift",m_TshiftLayer) ;
  
  // Grouping type
  declareProperty("GroupingType",      m_groupingType);  
}


LArPhysWavePredictor::~LArPhysWavePredictor() 
{}


StatusCode LArPhysWavePredictor::initialize() 
{
  if ( ! m_doubleTriangle ) {
      ATH_MSG_INFO( "Using standard triangular ionization pulse" ) ;
  } else {
    ATH_MSG_INFO( "Using refined ionization pulse (double triangle)" ) ;
  }

  StatusCode sc;
  if ( m_isSC ) {
    const LArOnline_SuperCellID* ll;
    ATH_CHECK(detStore()->retrieve(ll, "LArOnline_SuperCellID"));
    m_onlineHelper = (const LArOnlineID_Base*)ll;
    ATH_MSG_DEBUG("Found the LArOnlineID helper");
    const CaloCell_SuperCell_ID* scid;
    ATH_CHECK(detStore()->retrieve(scid, "CaloCell_SuperCell_ID" ));
    m_caloCellId= (const CaloCell_Base_ID*)scid;

  } else { // m_isSC
    const LArOnlineID* ll;
    ATH_CHECK(detStore()->retrieve(ll, "LArOnlineID") );
    m_onlineHelper = (const LArOnlineID_Base*)ll;
    ATH_MSG_DEBUG(" Found the LArOnlineID helper. ");
     const CaloCell_ID* cid;
    ATH_CHECK(detStore()->retrieve(cid, "CaloCell_ID" ));
    m_caloCellId= (const CaloCell_Base_ID*)cid;
  }

  ATH_CHECK( m_BCKey.initialize() );
  ATH_CHECK( m_cablingKey.initialize() );
  ATH_CHECK( m_cablingKeySC.initialize(m_isSC) );
  ATH_CHECK( m_bcMask.buildBitMask(m_problemsToMask,msg()));

  return StatusCode::SUCCESS ;
}


namespace {
struct FileCloser
{
  FileCloser (FILE* the_f): f (the_f) {}
  ~FileCloser() { if (f) fclose(f); }
  FILE* f;

  FileCloser (const FileCloser&) = delete;
  FileCloser& operator= (const FileCloser&) = delete;
};
} // anonymous namespace

StatusCode LArPhysWavePredictor::stop()
{
  ATH_MSG_INFO( "... in stop()" ) ;
  
  LArWaveHelper larWaveHelper;

  // Get access to the Detector Store
  //StoreGateSvc* detStore; 
  //StatusCode sc = service("DetectorStore",detStore);
  //if (sc!=StatusCode::SUCCESS) {
  //  ATH_MSG_ERROR( "Cannot get DetectorStore!" );
  //  return sc;
  //}
  
  // Retrieve LArPhysWaveTool
  ToolHandle<LArPhysWaveTool> larPhysWaveTool("LArPhysWaveTool");
  StatusCode sc=larPhysWaveTool.retrieve();
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( " Can't get LArPhysWaveTool " );
    return sc;
  }

  // Retrieve LArPhysWaveHECTool   // added by FT
  ToolHandle<LArPhysWaveHECTool> larPhysWaveHECTool("LArPhysWaveHECTool");
  if(m_isHEC){
    sc=larPhysWaveHECTool.retrieve();
    if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( " Can't get LArPhysWaveHECTool " );
      return sc;
    }}

  
  // Retrieve cabling
  const LArOnOffIdMapping* cabling(0);
  if( m_isSC ){
    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKeySC};
    cabling = {*cablingHdl};
    if(!cabling) {
	ATH_MSG_ERROR("Do not have mapping object " << m_cablingKeySC.key());
        return StatusCode::FAILURE;
    }
  }else{
    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
    cabling = {*cablingHdl};
    if(!cabling) {
       ATH_MSG_ERROR("Do not have mapping object " << m_cablingKey.key());
       return StatusCode::FAILURE;
    }
  }

  // Get parameters from detStore (access through abtract interfaces)
  const ILArCaliPulseParams* larCaliPulseParams = nullptr;
  const ILArDetCellParams*   larDetCellParams = nullptr;  
  const ILArTdrift*          larTdrift = nullptr;
  const ILArPhysCaliTdiff*   larPhysCaliTdiff = nullptr;

  if ( !m_useJOCaliPulseParams ) {
    sc = detStore()->retrieve(larCaliPulseParams);
    if ( sc == StatusCode::FAILURE ) {
      ATH_MSG_WARNING( "Cannot retrieve LArCaliPulseParams" ) ;
      return sc;
    } else {
      ATH_MSG_INFO( "LArCaliPulseParams successfully retrieved" ) ;
    }
  }

  if ( !m_useJODetCellParams ) {
    if (!m_isHEC) {
      sc = detStore()->retrieve(larDetCellParams);
      if ( sc == StatusCode::FAILURE ) {
	ATH_MSG_WARNING( "Cannot retrieve LArDetCellParams" );
	return sc;
      }else {
	ATH_MSG_INFO( "LArDetCellParams successfully retrieved" );
      }
    }else{
      m_useJODetCellParams = true ;
    }
  }
  
  if ( !m_useJOTdrift ) {
    sc = detStore()->retrieve(larTdrift);
    if ( sc == StatusCode::FAILURE ) {
      ATH_MSG_WARNING( "Cannot retrieve LArTdriftComplete" );
      return sc;
    }else {
      ATH_MSG_INFO( "LArTdriftComplete successfully retrieved" );
    }
  }
  
  if ( !m_useJOPhysCaliTdiff ) {
    sc = detStore()->retrieve(larPhysCaliTdiff);
    if ( sc == StatusCode::FAILURE ) {
      ATH_MSG_WARNING( "Cannot retrieve LArPhysCaliTdiff" ); 
      return sc;
    }else {
      ATH_MSG_INFO( "LArPhysCaliTdiff successfully retrieved" );
    }
  }
  
  const ILArFEBTimeOffset* larFebTshift = NULL;  
  if ( m_useJOPhysCaliTdiff ) { // no LArPhysCaliTdiffComplete found, or manual time shift selected
    if ( m_timeShiftByHelper ) {
      ATH_MSG_INFO( "Will use helper class for start time." );
      m_timeShiftByIndex = -1 ;
      m_timeShiftByLayer = false ;
      m_timeShiftByFEB   = false ;
    }  
    if ( m_timeShiftByIndex != -1 ) {
      ATH_MSG_INFO( "Manually shifting pulses by time index " << m_timeShiftByIndex );
      m_timeShiftByLayer = false ;
      m_timeShiftByFEB   = false ;
    }
    if ( m_timeShiftByLayer ) {
      ATH_MSG_INFO( "Manually shifting pulses by *layer-dependent* time indexes." );
      m_timeShiftByFEB   = false ;
    }
    if ( m_timeShiftByFEB ) {
      ATH_MSG_INFO( "Manually shifting pulses by *FEB* time indexes." );
      sc = detStore()->retrieve(larFebTshift);
      if (sc.isFailure())
         larFebTshift = NULL;
    }
  }
  
  int nchannel = 0 ;
  
  // Create LArPhysWaveContainer for predicted physics waveforms
  std::unique_ptr<LArPhysWaveContainer> larPhysWaveContainer = std::make_unique<LArPhysWaveContainer>();

  sc=larPhysWaveContainer->setGroupingType(m_groupingType,msg());
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed to set groupingType for LArPhysWaveContainer object" );
    return sc;
  }

  sc=larPhysWaveContainer->initialize(); 
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed initialize LArPhysWaveContainer object" );
    return sc;
  }   

  // Create LArMphysOverMcalComplete for predicted Mphys/Mcali
  std::unique_ptr<LArMphysOverMcalComplete> MphysOverMcalComplete = std::make_unique<LArMphysOverMcalComplete>(); 
  sc=MphysOverMcalComplete->setGroupingType(m_groupingType,msg());
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed to set groupingType for LArMphysOverMcalComplete object" );
    return sc;
  }

  sc=MphysOverMcalComplete->initialize(); 
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed initialize LArMphysOverMcalComplete object" );
    return sc;
  }   
  
  FILE* f = NULL;
  if (m_dumpMphysMcali) {
     f = fopen("MphysOverMcali.dat","w");
     fprintf(f,"# Region Layer Eta Phi Gain  MphysMcali\n");
  }
  FileCloser fcloser (f);
  
  std::vector<int> nTotal;
  std::vector<int> noTcali;
  std::vector<int> noFstep;
  std::vector<int> noOmega0;
  std::vector<int> noTaur;
  std::vector<int> noTdrift;
  std::vector<int> noTdiff;
		  
  for ( unsigned i=0; i<CaloGain::LARNGAIN; ++i ) {
    nTotal.push_back(0);
    noTcali.push_back(0);
    noFstep.push_back(0);
    noOmega0.push_back(0);
    noTaur.push_back(0);
    noTdrift.push_back(0);
    noTdiff.push_back(0);
  }

  /////////////IDEAL PHYSWAVE/////////////////////////////
  // Get current LArPhysWaveContainer
  const LArPhysWaveContainer* larIdealPhysWaveContainer=0;
  if(m_isHEC){
    ATH_CHECK(detStore()->retrieve(larIdealPhysWaveContainer,m_keyIdealPhys));
    ATH_MSG_INFO("LArPhysWaveContainer with (key = " << m_keyIdealPhys << ") reading from StoreGate" );
  }
  /////////////IDEAL PHYSWAVE/////////////////////////////
    
  // get the calibration waveforms from the detector store 

  int NPhysWave=0;
  int NMPMC=0;  

  for (const std::string& key : m_keyCali) { // Loop over all LArCaliWave containers that are to be processed

    // Get current LArCaliWaveContainer
    const LArCaliWaveContainer* caliWaveContainer;
    sc = detStore()->retrieve(caliWaveContainer,key);
    if (sc.isFailure()) {
       //log << MSG::INF0 << "LArCaliWaveContainer (key = " << key << ") not found in StoreGate" );
       continue;   
    }
    if ( caliWaveContainer == NULL ) {
       ATH_MSG_INFO( "LArCaliWaveContainer (key = " << key << ") is empty" );
       continue;
    }
    
    ATH_MSG_INFO( "Processing LArCaliWaveContainer from StoreGate, key = " << key );
    
    for ( unsigned gain = CaloGain::LARHIGHGAIN ; gain < CaloGain::LARNGAIN ; ++ gain ) { // loop over gain in the current   LArCaliWAveContainer
      
      ATH_MSG_INFO( "Now processing gain = " << gain << " in LArCaliWaveContainer with key = " << key );
    

      // loop over current cali wave container
      typedef LArCaliWaveContainer::ConstConditionsMapIterator const_iterator;
      const_iterator itVec   = caliWaveContainer->begin(gain);
      const_iterator itVec_e = caliWaveContainer->end(gain);
      for (; itVec != itVec_e; ++itVec) { // loop over channels for a given gain
		
        for (const LArCaliWave& larCaliWave : *itVec) { // loop over DAC values for a given channel

          ATH_MSG_DEBUG((*itVec).size() << " LArCaliWaves found for channel " << m_onlineHelper->channel_name(itVec.channelId()) << " 0x" 
		       << std::hex << itVec.channelId().get_identifier32().get_compact() << std::dec);
	  const HWIdentifier chid = itVec.channelId();

          // Skip if it is FCAL
          if(m_onlineHelper->isFCALchannel(chid)) continue;

	  if ( nchannel < 100 || ( nchannel < 1000 && nchannel%100==0 ) || nchannel%1000==0 ) 
	     ATH_MSG_INFO( "Processing calibration waveform number " << nchannel );

	  if ( larCaliWave.getFlag() == LArWave::dac0 )  continue ; // skip dac0 waves          
	  // TODO: here we should add a DAC selection mechanism for TCM method

          nTotal[gain]++; // counter of processed pulse per gain 

          ATH_MSG_VERBOSE("Predicting physics waveform for channel 0x" << MSG::hex << chid << MSG::dec 
			  << " (gain = " << gain << " - DAC = " << larCaliWave.getDAC() << ")");

	  // calibration pulse copy (working around the const iterator to be able to manipulate it...)
	  LArCaliWave theLArCaliWave = larCaliWave;
	  
	  // region and layer information are needed
	  Identifier id;
	  try {
            id = cabling->cnvToIdentifier(chid);
          } catch (LArID_Exception & execpt) {
	    ATH_MSG_ERROR( "LArCabling exception caught for channel 0x" << MSG::hex << chid << MSG::dec 
	        << ". Skipping channel." ) ;
            continue ;
	  }

	  if ( !cabling->isOnlineConnected(chid)  ) { // unconnected channel : skipping ...          
	    ATH_MSG_VERBOSE("Unconnected channel 0x" << MSG::hex << chid << MSG::dec 
			    << ". Skipping channel.");
	    continue ; 	  
	  }

	  int region = m_caloCellId->region(id);
          int layer  = m_caloCellId->sampling(id);

  	  // Get the parameters corresponding to current LArCaliWave
	  float Tcali;
	  float Fstep;
	  float Omega0;
	  float Taur;
	  float Tdrift;
	  int   Tdiff;
	  
	  // LArCaliPulseParams: if not fould, will use jobOptions values
	  if ( !m_useJOCaliPulseParams ) {
             Tcali = larCaliPulseParams->Tcal(chid,gain) ;

	     if ( Tcali <= 1.0+LArCaliPulseParamsComplete::ERRORCODE ) {
	       notFoundMsg(chid,gain,"Tcali");
	       Tcali = m_Tcali;
	       noTcali[gain]++;
	     }
	     Fstep = larCaliPulseParams->Fstep(chid,gain) ;
	     if ( Fstep <= 1.0+LArCaliPulseParamsComplete::ERRORCODE ) {
	       notFoundMsg(chid,gain,"Fstep");
	       Fstep = m_Fstep;
	       noFstep[gain]++;
	     }
          } else {
	    Tcali = m_Tcali;
	    Fstep = m_Fstep;
	  }
	  
	  // LArDetCellParams: if not found, will not apply injection point correction
          if ( !m_useJODetCellParams ) {
	     Omega0 = larDetCellParams->Omega0(chid,gain) ;
	     if ( Omega0 <= 1.0+LArDetCellParamsComplete::ERRORCODE ) {
	       notFoundMsg(chid,gain,"Omega0");
	       Omega0 = m_Omega0;
	       noOmega0[gain]++;
	     }
	     Taur = larDetCellParams->Taur(chid,gain) ;
	     if ( Taur <= 1.0+LArDetCellParamsComplete::ERRORCODE ) {
	       notFoundMsg(chid,gain,"Taur");
	       Taur = m_Taur;
	       noTaur[gain]++;
	     }
	  } else {
	    Omega0 = m_Omega0;
	    Taur   = m_Taur;
	  }
	  
          // LArTdrift: if not found will be set to jobOptions settings (or defaults if none) according to layer
	  if ( !m_useJOTdrift ) {
	    Tdrift = larTdrift->Tdrift(chid) ;
	     if ( Tdrift <= 1.0+LArTdriftComplete::ERRORCODE ) {
	       notFoundMsg(chid,gain,"Tdrift");
	       //ATH_MSG_ERROR( "Cannot access Tdrift for channel 0x" << MSG::hex << chid << MSG::dec 
	       //   << ". Will use jobOption setting." ) ;
		if ( layer>=0 && layer<4 ) 
		  Tdrift = m_Tdrift[layer]; // Set drift time according to layer
		else 
		  Tdrift = 475.;  // very crude! 
		noTdrift[gain]++;
	     }
	  } else { // use jobOptions settings
	    if ( layer>=0 && layer<4 ) 
	      Tdrift = m_Tdrift[layer]; // Set drift time according to layer
	    else 
	      Tdrift = 475.;  // very crude! 
	  }

	  
          // LArPhysCaliTdiff: if not found, will use jobOptions settings
	  Tdiff = 0 ; // default value if everything else fails...
	  if ( !m_useJOPhysCaliTdiff ) {
	     Tdiff = (int)larPhysCaliTdiff->Tdiff(chid,gain) ;
	     if ( Tdiff <= 1.0+LArPhysCaliTdiffComplete::ERRORCODE ) {
	       notFoundMsg(chid,gain,"Tdiff");
	        Tdiff = 0 ;
		m_useJOPhysCaliTdiff = true ;
		noTdiff[gain]++;
	     }
          }
	  if ( m_useJOPhysCaliTdiff ) {
	    if ( m_timeShiftByHelper ) {
	      Tdiff  = larWaveHelper.getStart(theLArCaliWave) ;
	      Tdiff -= m_timeShiftGuardRegion ;
	    }
	    if ( m_timeShiftByIndex != 0 ) {
	      Tdiff = m_timeShiftByIndex;
	    }
	    if ( m_timeShiftByLayer ) {
	      Tdiff = m_TshiftLayer[layer] ;
	    }
	    if ( m_timeShiftByFEB && larFebTshift ) {
	      const HWIdentifier febid = m_onlineHelper->feb_Id(chid);
	      Tdiff  = (int)larFebTshift->TimeOffset(febid);
	      Tdiff -= m_timeShiftGuardRegion ;
	    }
          }

          // Fill the LArWFParams structure to be used by the LArPhysWaveTool
          float Tshaper  = 15. ;
	  float Amplitude = 1. ;
          LArWFParams wfParams(Tcali,Fstep,Tdrift,Omega0,Taur,Tshaper,Amplitude);
          wfParams.setFlag( 0 ) ;  // this should contain the method used to find parameters and the gain
          wfParams.setTdiff(Tdiff);
	  	  
	  ATH_MSG_VERBOSE( "wfParams: " << Tcali << " " <<Fstep<<" " <<Tdrift<<" "<<Omega0<<" "<<Taur<<" "<<Tdiff<<" "<<layer<<" "<<region );
          // calibration pulse normalization 
	  // (should be done here instead than in LArPhysWaveTool to get 
	  // the correct Mphys/Mcali in case of double triangle prediction)
	  if ( m_normalizeCali ) {
	    double peak = theLArCaliWave.getSample(larWaveHelper.getMax(theLArCaliWave));
            ATH_MSG_VERBOSE("Channel 0x" << MSG::hex << chid << MSG::dec << " -> Applying normalisation (CaliWave peak = " << peak << ")");
            if ( peak <=0 ) {
               ATH_MSG_WARNING( "Peak value <=0 , cannot normalize!" ) ;
            } else {
               theLArCaliWave = LArCaliWave( (theLArCaliWave*(1./peak)).getWave(),
					     theLArCaliWave.getErrors(),
					     theLArCaliWave.getTriggers(),
					     theLArCaliWave.getDt(), 
					     theLArCaliWave.getDAC(),
					     theLArCaliWave.getIsPulsedInt(),
					     theLArCaliWave.getFlag() );
            }
          } 
	  
	  //
	  // Predict the Physics Waveform
	  //      
	  LArPhysWave larPhysWave;
	  float MphysMcali ;	
	  if(larIdealPhysWaveContainer && m_caloCellId->is_lar_hec(id)) {
	    const LArPhysWave& laridealPhysWave = larIdealPhysWaveContainer -> get(chid,gain);
	    int LArWaveFlag=LArWave::predCali;    // 111 - for HEC Wave
	    //int LArIdealPhysWaveFlag=LArWave::predCali;    // 111 - for HEC Wave
	    ATH_MSG_DEBUG( "Using HEC tool to predict LArPhysWave for channel " <<  chid.get_identifier32().get_compact());
	    
	    sc = larPhysWaveHECTool->makeLArPhysWaveHEC(wfParams,theLArCaliWave,larPhysWave,laridealPhysWave,MphysMcali,chid,gain,LArWaveFlag);
	    
	    //laridealPhysWave.setFlag( LArIdealPhysWaveFlag ) ;
	  }
	  else {
	    ATH_MSG_DEBUG( "Using EM tool to predict LArPhysWave for channel 0x" << chid.get_identifier32().get_compact() );
	    sc = larPhysWaveTool->makeLArPhysWave(wfParams,theLArCaliWave,region,layer,larPhysWave,MphysMcali);
          }
	  if (sc.isFailure()) {
	    ATH_MSG_FATAL( "Cannot predict LArPhysWave for channel " << chid.get_identifier32().get_compact() );
	    continue;
	  }
	  larPhysWave.setFlag( LArWave::predCali ) ;

	  if ( m_doubleTriangle ) { // mix of Tdrift ...
	    ATH_MSG_DEBUG("Applying double triangle correction");
	    if ( region==0 && layer>=0 && layer<4 && m_wTriangle2[layer]>0 ) {  // EMB: set drift times and weight according to layer
	      LArWFParams wfParams2 = wfParams ;
	      wfParams2.setTdrift(m_Tdrift2[layer]);
	      LArPhysWave larPhysWave2;
	      float MphysMcali2 ;
	      sc = larPhysWaveTool->makeLArPhysWave(wfParams2,theLArCaliWave,region,layer,larPhysWave2,MphysMcali2);
	      if (sc.isFailure()) {
                ATH_MSG_FATAL( "Cannot predict LArPhysWave for channel 0x" << MSG::hex << chid << MSG::dec << "with double triangle."  );
                continue;
	      }
	      larPhysWave = LArPhysWave ( ( larPhysWave*(1.-m_wTriangle2[layer])+larPhysWave2*m_wTriangle2[layer] ).getWave() ,
			                    larPhysWave.getDt(), 
			                    larPhysWave.getFlag() ) ;
	      // Double-triagle predicion was used, Mphys/Mcali should be recomputed... 
	      MphysMcali = larPhysWave.getSample(larWaveHelper.getMax(larPhysWave))/theLArCaliWave.getSample(larWaveHelper.getMax(theLArCaliWave));
	    } else {
	      ATH_MSG_WARNING( "Double triangle implemented only for EMB, skip channel!" ) ;
	    }
	  } // end if 2nd triangle
          
	  // Apply time shift...
	  if ( Tdiff !=0 ) {
	    ATH_MSG_DEBUG("Time shift for channel " << (itVec.channelId()).get_compact() << " is " 
			  << Tdiff << " samples (" << Tdiff*larPhysWave.getDt() << " ns)");
	    larPhysWave = LArPhysWave( (larWaveHelper.translate(larPhysWave,-Tdiff,0)).getWave() ,
                                       larPhysWave.getDt(), 
				       larPhysWave.getFlag() ) ;
	  }
          
	  // Add predicted physics wave to container
          larPhysWaveContainer->setPdata(chid,larPhysWave,gain);
          NPhysWave++; 

	  // Add Mphys/Mcali to container
	  if (MphysMcali<=0.) {
	     MphysMcali = LArMphysOverMcalComplete::ERRORCODE ;
	  }
	  ATH_MSG_VERBOSE("Channel 0x" << MSG::hex << chid << MSG::dec << " -> Mphys/Mcali = " << MphysMcali);
	  MphysOverMcalComplete->set(chid,gain,MphysMcali);
	  NMPMC++;
	  if ( m_dumpMphysMcali ) { 
	     int eta = m_caloCellId->eta(id);
	     int phi = m_caloCellId->phi(id);
	     fprintf( f , "%2d %2d %3d %3d  %2u %8.3f \n", region, layer, eta, phi, gain, MphysMcali ) ;
	  } //end if m_dumpMphysMcal
	  
        } // end loop over DAC value for a given cell
	if ( m_testmode && nchannel>=1 ) break;
      }  // end loop over cells for a given gain
      if ( m_testmode && nchannel>=1 ) break;
    } // end loop over gains for a give container
    if ( m_testmode && nchannel>=1 ) break;
  }  // End loop over all CaliWave containers

  //ATH_MSG_INFO( " Summary : Number of cells with a PhysWave values computed : " << larPhysWaveContainer->totalNumberOfConditions()  );
  //ATH_MSG_INFO( " Summary : Number of cells with a MphysOverMcal values computed : " << MphysOverMcalComplete->totalNumberOfConditions()  );
  ATH_MSG_INFO( " Summary : Number of cells with a PhysWave values computed : " << NPhysWave  );
  ATH_MSG_INFO( " Summary : Number of cells with a MphysOverMcal values computed : " << NMPMC  );
  ATH_MSG_INFO( " Summary : Number of Barrel PS cells side A or C (connected+unconnected):  3904+ 192 " );
  ATH_MSG_INFO( " Summary : Number of Barrel    cells side A or C (connected+unconnected): 50944+2304 " );
  ATH_MSG_INFO( " Summary : Number of EMEC      cells side A or C (connected+unconnected): 31872+3456 " );
  ATH_MSG_INFO( " Summary : Number of HEC       cells side A or C (connected+unconnected):  2816+ 256 " );
  ATH_MSG_INFO( " Summary : Number of FCAL      cells side A or C (connected+unconnected):  1762+  30 " );

  ATH_MSG_DEBUG("LArPhysWaveContainer->totalNumberOfConditions()     = " <<  larPhysWaveContainer->totalNumberOfConditions());
  ATH_MSG_DEBUG("LArMphysOverMcalComplete->totalNumberOfConditions() = " <<  MphysOverMcalComplete->totalNumberOfConditions());

  // final report
 ATH_MSG_INFO( "\n Final report \n" );
  for ( unsigned theGain = CaloGain::LARHIGHGAIN ; theGain < CaloGain::LARNGAIN ; ++theGain ) { 

    ATH_MSG_INFO( " *** Gain = " << theGain << " ***" );
    if ( !m_useJOCaliPulseParams ) {
       ATH_MSG_INFO( "\t" << noTcali[theGain]  << " / " << nTotal[theGain] << " channel(s) missing Tcali"  );
       ATH_MSG_INFO( "\t" << noFstep[theGain]  << " / " << nTotal[theGain] << " channel(s) missing Fstep"  );
    }
    if ( !m_useJODetCellParams ) {  
       ATH_MSG_INFO( "\t" << noOmega0[theGain] << " / " << nTotal[theGain] << " channel(s) missing Omega0"  );
       ATH_MSG_INFO( "\t" << noTaur[theGain]   << " / " << nTotal[theGain] << " channel(s) missing Taur"    );
    }
    if ( !m_useJOTdrift )
       ATH_MSG_INFO( "\t" << noTdrift[theGain] << " / " << nTotal[theGain] << " channel(s) missing Tdrift" );
    if ( !m_useJOPhysCaliTdiff )
       ATH_MSG_INFO( "\t" << noTdiff[theGain]  << " / " << nTotal[theGain] << " channel(s) missing Tdiff"  );	    
  }
  ATH_MSG_INFO( "\n" );

  // Record LArPhysWaveContainer to DetectorStore
  sc = detStore()->record(std::move(larPhysWaveContainer),m_keyPhys);
  if (sc.isFailure()) {
    ATH_MSG_FATAL( "Cannot record LArPhysWaveContainer to StoreGate! key=" << m_keyPhys );
    return StatusCode::FAILURE;
  }

  // Record LArMphysOverMcalComplete to DetectorStore
  sc = detStore()->record(std::move(MphysOverMcalComplete),m_keyMphysMcali); 
  if (sc.isFailure()) {
    ATH_MSG_FATAL( "Cannot record LArMphysOverMcalComplete to StoreGate! key=" << m_keyMphysMcali );
    return StatusCode::FAILURE;
  }
  
  // Symlink LArMphysOverMcalComplete to ILArMphysOverMcal for further use
  ATH_MSG_DEBUG("Trying to symlink ILArMphysOverMcal with LArMphysOverMcalComplete...");
  sc = detStore()->symLink(ClassID_traits<LArMphysOverMcalComplete>::ID(),m_keyMphysMcali,ClassID_traits<ILArMphysOverMcal>::ID());
  if (sc.isFailure()) {
      ATH_MSG_FATAL( "Could not symlink ILArMphysOverMcal with LArMphysOverMcalComplete." );
      return StatusCode::FAILURE;
  } 
  ATH_MSG_INFO( "ILArMphysOverMcal symlink with LArMphysOverMcalComplete successfully" ) ;
  
  ATH_MSG_INFO( "LArPhysWavePredictor finalized!" );  
  
  return StatusCode::SUCCESS;
}

void LArPhysWavePredictor::notFoundMsg(const HWIdentifier chid, const int gain, const char* value) {
  SG::ReadCondHandle<LArBadChannelCont> bcContHdl{m_BCKey};
  const LArBadChannelCont* bcCont{*bcContHdl};

  if (m_bcMask.cellShouldBeMasked(bcCont,chid)) {
    ATH_MSG_WARNING( "Cannot access " << value << " for known bad channel channel " << m_onlineHelper->channel_name(chid)
		      << ", gain = " << gain << ". Will use jobO setting." ) ;
  }
  else {    
    LArBadChanBitPacking packer;
    const LArBadChannel bc = bcCont->status(chid);
    const std::string badChanStatus=packer.stringStatus(bc);

    ATH_MSG_ERROR( "Cannot access " << value << " for channel " << m_onlineHelper->channel_name(chid) 
		    << ", gain = " << gain << " BC status=[" << badChanStatus << "]. Will use jobO setting." );
  }
  return;
}

