/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibDataQuality/LArRampValidationAlg.h"
#include <cmath>

LArRampValidationAlg::LArRampValidationAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  LArRampValidationBase(name,pSvcLocator),
  m_hasRawRampContainer(false),
  m_rawRampContainer(0),
  m_rampGlobalVal(0),
  m_rmsGlobalVal(0),
  m_rampGlobalRef(0),
  m_rmsGlobalRef(0),
  m_nEntriesGlobal(0)
{
  declareProperty("KeyList", m_contKey);
  declareProperty("RawRampTimeMinADC",m_rawrampTimeADC=100,
		  "Minimal ADC, where the RampTime is computed");
  declareProperty("RampTolerance",m_toleranceInit,
		  "Allowed deviation of ramp (%)");
  declareProperty("RampToleranceFEB",m_toleranceInitFEB,
		  "Allowed deviation of ramp (average over one FEB,");
  declareProperty("RawRampTimeTolerance",m_rawrampTimeTolerance=1.,
		  "Allowed deviation of the reconstructed time (ns)");
}


StatusCode LArRampValidationAlg::preLoop() {

  //Initialize CellGroup object storing the threhsolds:
  if (m_tolerance.setDefinition(m_caloId,m_toleranceInit,msg())==false) {
    ATH_MSG_ERROR ( "Failed to initialize CaloCellGroup of thresholds!" ) ;
    return StatusCode::FAILURE;
  }

  if (m_tolerance.getDefaults().size()!=3) {
    ATH_MSG_ERROR ( "Expected three values in CaloCellGroup of thresholds for three gains!" ) ;
     return StatusCode::FAILURE;
  }
  if (this->msgLvl(MSG::DEBUG)) m_tolerance.printDef();//for debugging....


  //Initialize CellGroup object storing the threhsolds:
  if (m_toleranceFEB.setDefinition(m_caloId,m_toleranceInitFEB,msg())==false) {
    ATH_MSG_ERROR ( "Failed to initialize CaloCellGroup of thresholds!" ) ;
    return StatusCode::FAILURE;
  }

  if (m_toleranceFEB.getDefaults().size()!=3) {
    ATH_MSG_ERROR ( "Expected three values in CaloCellGroup of thresholds for three gains!" ) ;
     return StatusCode::FAILURE;
  }
  if (this->msgLvl(MSG::DEBUG)) m_toleranceFEB.printDef();//for debugging....


  m_rampGlobalRef=0.;
  m_rampGlobalVal=0.;
  m_nEntriesGlobal=0;
  m_rawRampContainer=NULL;

  // Retrieve Raw Ramps Container
  m_hasRawRampContainer = false;
  for (const std::string& key_it : m_contKey) {
    StatusCode sc=detStore()->retrieve(m_rawRampContainer,key_it);
    if (sc!=StatusCode::SUCCESS || !m_rawRampContainer) {
      ATH_MSG_WARNING ( "Unable to retrieve LArRawRampContainer with key " << key_it ) ;
    } 
    else {
      ATH_MSG_DEBUG ( "Got LArRawRampContainer with key " << key_it ) ;
      m_hasRawRampContainer = true;
    }
  }
  if (!m_hasRawRampContainer) 
    ATH_MSG_WARNING ( "No LArRawRampContainer found. Only fitted ramp will be tested " ) ;
  
  // Check Raw Ramps
  if(m_hasRawRampContainer){

    LArRawRampContainer::const_iterator cont_it=m_rawRampContainer->begin();
    LArRawRampContainer::const_iterator cont_it_e=m_rawRampContainer->end();
    for (;cont_it!=cont_it_e;++cont_it) {
      const std::vector<LArRawRamp::RAMPPOINT_t>& singleRamp=(*cont_it)->theRamp(); 
      for (unsigned int DACIndex=0; DACIndex<singleRamp.size(); DACIndex++) {

        ATH_MSG_DEBUG ( "DAC Index:" << DACIndex 
                        << " DAC value : " << singleRamp[DACIndex].DAC 
                        << " ADC value : " << singleRamp[DACIndex].ADC 
                        << " Time " << singleRamp[DACIndex].TimeMax )  ;

	// Check point with DAC > m_rawrampTimeDAC where DeltaT is meaningful
	if(singleRamp[DACIndex].ADC > m_rawrampTimeADC){
	  if (fabs(singleRamp[DACIndex].TimeMax) > m_rawrampTimeTolerance){ 
	    msg().setf(std::ios::fixed,std::ios::floatfield); 
	    msg().precision(2);
	    msg() << m_myMsgLvl << "Deviating! chan= " << (*cont_it)->channelID()<< " gain= "<<(*cont_it)->gain() << " DeltaT=" << singleRamp[DACIndex].TimeMax << " DAC = " << singleRamp[DACIndex].DAC << endmsg;
	  }
	  break; //Stop loop after testing the m_rawrampTimeDAC DAC point
	}

      } //end DAC points loop
    } // end channels loop
  }
  return StatusCode::SUCCESS;
}

bool LArRampValidationAlg::validateChannel(const LArCondObj& ref, const LArCondObj& val, const HWIdentifier chid, const int gain, const LArOnOffIdMapping *cabling,const LArBadChannelCont *bcCont) {


  HWIdentifier febid=m_onlineHelper->feb_Id(chid);
  Identifier offlineID = cabling->cnvToIdentifier(chid);

  ++m_nEntriesGlobal;

  // Store average Ramp value per FEB
  DataPerRegion& dataPerFEB=m_vDataPerFEB[febid];
  dataPerFEB.rampVal+=val.m_vRamp[1];
  dataPerFEB.rampRef+=ref.m_vRamp[1];
  ++(dataPerFEB.nEntries);

  m_rampGlobalVal+=val.m_vRamp[1];
  m_rampGlobalRef+=ref.m_vRamp[1];

  const Identifier region=m_caloId->region_id(offlineID);
  int eta = m_caloId->eta(offlineID); 
  //Build fake-identifier with phi=0 to identify sector:
   //Identifier cell_id (const int subCalo, const int barec_or_posneg, const int sampling_or_fcalmodule,const int region_or_dummy,const int eta,  const int phi ) const;
   //mis-used phi for gain:
  const Identifier sectorId=m_caloId->cell_id(region,eta,gain);

  // Store avg Ramp value per sector : pos_neg/region/layer/eta (average over phi)
  DataPerRegion& dataPerSector=m_vDataPerSector[sectorId];
  dataPerSector.rampVal+=val.m_vRamp[1];
  ++(dataPerSector.nEntries);

  // Check individual channel for Ramp deviation
  const float& tolerance=m_tolerance.valuesForCell(offlineID)[gain];


  if (fabs(val.m_vRamp[1]-ref.m_vRamp[1])/ref.m_vRamp[1] > tolerance){
    if (m_nFailedValidation<m_maxmessages) {
      std::stringstream devMsg;
      devMsg.setf(std::ios::fixed,std::ios::floatfield); 
      devMsg.precision(3);
      devMsg << "Deviating! " << channelDescription(chid,cabling,bcCont,gain) << " Ramp: " << val.m_vRamp[1] << " (" << ref.m_vRamp[1] << ", ";
      devMsg.precision(2);
      devMsg << 100*(val.m_vRamp[1]-ref.m_vRamp[1])/ref.m_vRamp[1] << "%)";
      msg() << this->m_myMsgLvl << devMsg.str() << endmsg;
      ATH_MSG_DEBUG ( "Ramp Tolerance: " << tolerance ) ;
    }
    else if ( m_nFailedValidation==m_maxmessages)
      msg() <<  this->m_myMsgLvl << "Channel deviation message has already been printed " << m_maxmessages << " times. Now silent..." << endmsg;
    
    return false;
  } 
  else 
    return true;

  
}

bool LArRampValidationAlg::febSummary(const LArOnOffIdMapping *cabling, const LArBadChannelCont *bcCont) {

  // FEBs
  unsigned nBadFebs=0;
  for(auto& dataPerFebPair : m_vDataPerFEB) {
    DataPerRegion& dataPerFeb=dataPerFebPair.second;
    const HWIdentifier febAndGainId=dataPerFebPair.first;
    const int gain=m_onlineHelper->channel(febAndGainId);
    const HWIdentifier febId=m_onlineHelper->feb_Id(febAndGainId);
    dataPerFeb.rampVal/=dataPerFeb.nEntries;
    dataPerFeb.rampRef/=dataPerFeb.nEntries;

    ATH_MSG_DEBUG ( " nb of channels = "  << dataPerFeb.nEntries 
                    << " for FEB " << channelDescription(febId,cabling,bcCont) ) ;  

    //Get offline identifier of channel 0 of this FEB, should be good enough ...
    const Identifier id=cabling->cnvToIdentifier(febId);
    const float& tolerance=m_toleranceFEB.valuesForCell(id)[gain];
    
    if (fabs(dataPerFeb.rampVal-dataPerFeb.rampRef)/dataPerFeb.rampRef > tolerance){
      msg().precision(3);
      msg().setf(std::ios::fixed,std::ios::floatfield); 
      msg() << m_myMsgLvl << "Deviating! " << channelDescription(febId,cabling,bcCont,gain,true) << "Average Ramp: " 
            << dataPerFeb.rampVal << " (reference: " << dataPerFeb.rampRef << ")" << endmsg;
      ATH_MSG_DEBUG ( "Ramp FEB average tolerance: " << tolerance ) ;
      ++nBadFebs;
    }
  }
  
  if (nBadFebs) {
    ATH_MSG_ERROR ( "Found " << nBadFebs << " out of " << m_vDataPerFEB.size() 
                    << " FEBs deviating from reference" ) ;
    return false;
  } else {
    ATH_MSG_INFO ( "All " << m_vDataPerFEB.size() 
                   << " FEBs withing given tolerance." ) ;
    return true;
  }
  /*
  // Sectors
  std::vector<DataPerSector>::iterator it2=m_vDataPerSector.begin();
  std::vector<DataPerSector>::iterator it2_e=m_vDataPerSector.end();
  for (;it2!=it2_e;++it2) {
    DataPerSector& dataPerSector=*it2;
    dataPerSector.rampVal/=dataPerSector.nEntries;
    //dataPerSector.rampRef/=dataPerSector.nEntries;

    (*m_log) << MSG::DEBUG << " nb of channels = "  << dataPerSector.nEntries 
	     << " for Sector : gain " <<  dataPerSector.gain 
	     << " pos_neg " <<  dataPerSector.pos_neg 
	     << " region " << dataPerSector.region
	     << " layer " << dataPerSector.layer 
	     << " eta " << dataPerSector.eta << endmsg;  
  }
  */
}

bool LArRampValidationAlg::deviateFromAvg(const LArCondObj& val, const HWIdentifier chid, const int gain, const LArOnOffIdMapping *cabling, const LArBadChannelCont *bcCont) {

  Identifier offlineID = cabling->cnvToIdentifier(chid);
  const Identifier regId=m_caloId->region_id(offlineID);
  const int eta = m_caloId->eta(offlineID); 
  const Identifier sectorId=m_caloId->cell_id(regId,eta,gain);

  const auto& dataPerSector=m_vDataPerSector[sectorId];
  if (dataPerSector.rampVal == 0 ){
    ATH_MSG_ERROR ( "Found Sector with Ramp Average equals to zero" ) ;
    ATH_MSG_ERROR ( "Sector : pos_neg " <<  m_caloId->side(regId) << " region " << m_caloId->region(regId)
		    << " layer " << m_caloId->sampling(regId) << " eta " << eta << " gain " << gain);     
    return false;
  }else{
    float ratio = val.m_vRamp[1]/dataPerSector.rampVal;      
    if ( ratio > 2.){
      msg() << m_myMsgLvl << "!!! Deviating Sector channel =  " <<channelDescription(chid,cabling,bcCont,gain) << "Ramp: " << val.m_vRamp[1] << " (Average Sector Ramp: " << dataPerSector.rampRef << ")" << endmsg;
      return false;
    }
  }
  return true;
}

StatusCode LArRampValidationAlg::summary(const LArOnOffIdMapping *cabling, const LArBadChannelCont *bcCont) {
  StatusCode sc=StatusCode::SUCCESS;
  //1nd step: Check the FEB-averages:
  if (m_doFebAverages && !febSummary(cabling, bcCont))
    sc=StatusCode::RECOVERABLE;
  //2nd step: Call the summary method from base-class (single-channel summary)
  if (!LArRampValidationBase::summary(cabling, bcCont).isSuccess())
    sc=StatusCode::RECOVERABLE;
  //3rd step: Check the gobal averages:
  if (m_nEntriesGlobal) {
    m_rampGlobalVal/=m_nEntriesGlobal;
    m_rampGlobalRef/=m_nEntriesGlobal;
    m_rmsGlobalVal/=m_nEntriesGlobal;
    m_rmsGlobalRef/=m_nEntriesGlobal;
  }
  ATH_MSG_INFO ( "Gobal ramp average: " << m_rampGlobalVal << " Reference: " << m_rampGlobalRef
                 << " Deviation: " << m_rampGlobalVal-m_rampGlobalRef ) ;

  return sc;
}


LArRampValidationAlg::LArCondObj LArRampValidationAlg::getRefObj(const HWIdentifier chid, const int gain) const{
  auto ramp=m_reference->ADC2DAC(chid,gain);
  return LArCondObj(std::vector<float>(ramp.begin(),ramp.end()));
}

