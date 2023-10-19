/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRampBuilder.h"
#include "LArRawEvent/LArFebErrorSummary.h"
#include "LArRawConditions/LArRampComplete.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "CaloIdentifier/CaloCell_ID.h"

#include <Eigen/Dense>

#include <fstream>

#include "AthenaKernel/ClassID_traits.h"

StatusCode LArRampBuilder::initialize()
{
  StatusCode sc;
  if ( m_isSC ) {
    ATH_MSG_DEBUG("==== LArRampBuilder - looking at SuperCells ====");
    const LArOnline_SuperCellID* ll;
    sc = detStore()->retrieve(ll, "LArOnline_SuperCellID");
    if (sc.isFailure()) {
      msg(MSG::ERROR) << "Could not get LArOnlineID helper !" << endmsg;
      return StatusCode::FAILURE;
    }
    else {
      m_onlineHelper = (const LArOnlineID_Base*)ll;
      ATH_MSG_DEBUG("Found the LArOnlineID helper");
    }
    
  } else { // m_isSC
    const LArOnlineID* ll;
    sc = detStore()->retrieve(ll, "LArOnlineID");
    if (sc.isFailure()) {
      msg(MSG::ERROR) << "Could not get LArOnlineID helper !" << endmsg;
      return StatusCode::FAILURE;
    }
    else {
      m_onlineHelper = (const LArOnlineID_Base*)ll;
      ATH_MSG_DEBUG(" Found the LArOnlineID helper. ");
    }
    
  }

  ATH_CHECK( m_cablingKey.initialize() );
  ATH_CHECK( m_cablingKeySC.initialize(m_isSC) );

  ATH_CHECK(m_bcContKey.initialize(m_doBadChannelMask));
  ATH_CHECK(m_bcMask.buildBitMask(m_problemsToMask,msg()));

  //Intermediate ramp object (DAC/ADC pairs)
  m_ramps=std::make_unique<LArConditionsContainer<ACCRAMP> >();
  ATH_CHECK(m_ramps->setGroupingType(m_groupingType,msg())); 
  ATH_CHECK(m_ramps->initialize()); 
  
  chooseRecoMode() ;
  m_event_counter=0;
  
  unsigned int online_id_max = m_onlineHelper->channelHashMax() ; 
  m_thePedestal.resize(online_id_max,-1); 
  
  return StatusCode::SUCCESS;
}


void LArRampBuilder::chooseRecoMode()  {
  
  // choose reconstructiom mode
  if ( m_recoTypeProp == std::string("Parabola") ) {
    m_recoType=PARABOLA;
    StatusCode sc=m_peakParabolaTool.retrieve();
    if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "Can't get LArParabolaPeakRecoTool" );
	return;
      }
    ATH_MSG_DEBUG("LArParabolaPeakRecoTool retrieved with success!");
    
    if(m_correctBias){
      // if using parabola, get offlineID helper to obtain the layer (needed for correction) 
      const CaloCell_ID* idHelper = nullptr;
      if ( detStore()->retrieve (idHelper, "CaloCell_ID").isSuccess() ) {
        m_emId = idHelper->em_idHelper();
      }
      if (!m_emId) {
	ATH_MSG_ERROR( "Could not access lar EM ID helper" );
	return ;
      }
      
    }
    m_peakShapeTool.disable();
    m_peakOFTool.disable();
    // Shape reconstruction
  } else if (m_recoTypeProp == std::string("Shape") ) {
    m_recoType=SHAPE;
    ATH_MSG_INFO( "ShapePeakReco mode is ON ! ");
    if (m_peakShapeTool.retrieve().isFailure()) {
      ATH_MSG_ERROR( "Can't get LArShapePeakRecoTool");
      return;
    }
    ATH_MSG_DEBUG("LArShapePeakRecoTool retrieved with success!");
    m_peakParabolaTool.disable();
    m_peakOFTool.disable();
    // OFC recontruction 
  } else if ( m_recoTypeProp == std::string("OF") ) {
    m_recoType=OF;
    if (m_peakOFTool.retrieve().isFailure()) {
      ATH_MSG_ERROR( "Can't get LArOFPeakRecoTool");
      return;
    }
    ATH_MSG_DEBUG("LArOFPeakRecoTool retrieved with success!");
    m_peakShapeTool.disable();
    m_peakParabolaTool.disable();
  }
}

// ********************** EXECUTE ****************************
StatusCode LArRampBuilder::execute()
{ 

  StatusCode sc;
  if ( m_event_counter < 100 || m_event_counter%100==0 )
    ATH_MSG_INFO( "Processing event " << m_event_counter);
  ++m_event_counter;
  
  if (m_keylist.size()==0) {
    ATH_MSG_ERROR( "Key list is empty! No containers to process!");
    return StatusCode::FAILURE;
  }
  
  const LArFebErrorSummary* febErrSum=NULL;
  if (evtStore()->contains<LArFebErrorSummary>("LArFebErrorSummary")) {
    sc=evtStore()->retrieve(febErrSum);
    if (sc.isFailure()) {
      ATH_MSG_ERROR( "Failed to retrieve FebErrorSummary object!");
      return sc;
    }
  }
  else
    if (m_event_counter==1)
      ATH_MSG_WARNING("No FebErrorSummaryObject found! Feb errors not checked!");
 
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
  

  std::vector<std::string>::const_iterator key_it=m_keylist.begin();
  std::vector<std::string>::const_iterator key_it_e=m_keylist.end();
  
  const LArAccumulatedCalibDigitContainer* larAccumulatedCalibDigitContainer = nullptr;
  
  
  // if using Shape Reco method, retrieve caliWaveContainer (only once !)
  // FT remove the bitwise &&->&
  if ( m_recoType == SHAPE && m_ipassShape==0) {
    
    m_ipassShape = 1;
    
    // keep only 8 samples of wave - dont need all samples for pseudo-OF reco
    std::vector<double> tempWave;
    int NSamplesKeep = 8;
    tempWave.resize(24*NSamplesKeep);
    
    m_CaliWaves.resize(3);
    m_CaliDACs.resize(3);
    m_IndexDAC0.resize(3);
    m_IndexHighestDAC.resize(3);
    
    // retrieve cali wave container
    
    const LArCaliWaveContainer* caliWaveContainer = NULL;
    ATH_MSG_WARNING("Will retrieve LArCaliWaveContainer ");
    sc= detStore()->retrieve(caliWaveContainer,"CaliWave");
    if (sc.isFailure()) {
      ATH_MSG_WARNING("Cannot read LArCaliWaveContainer from StoreGate for key 'CaliWave' ! ");
      return StatusCode::FAILURE;
    }
	ATH_MSG_DEBUG("Succefully retrieved LArCaliWaveContainer from StoreGate!");
    for (;key_it!=key_it_e;++key_it) { //Loop over all containers that are to be processed (e.g. different gains)
      
      // first, set reference DAC (dirty hardcoding for now...)
      CaloGain::CaloGain gainref = CaloGain::LARLOWGAIN;
      if(*key_it == "HIGH") {
	gainref=CaloGain::LARHIGHGAIN;
      }else if(*key_it == "MEDIUM") {
	gainref=CaloGain::LARMEDIUMGAIN;
      }else if(*key_it == "LOW") {
	gainref=CaloGain::LARLOWGAIN;
      }
      
      m_CaliWaves[gainref].resize(m_onlineHelper->channelHashMax());
      m_CaliDACs[gainref].resize(m_onlineHelper->channelHashMax());
      m_IndexDAC0[gainref].resize(m_onlineHelper->channelHashMax());
      m_IndexHighestDAC[gainref].resize(m_onlineHelper->channelHashMax());
      
      // Set gain from key value
      int gain = CaloGain::LARHIGHGAIN;
      if      ((*key_it) == "MEDIUM") gain = CaloGain::LARMEDIUMGAIN;
      else if ((*key_it) == "LOW")    gain = CaloGain::LARLOWGAIN;
      
      // extract from all the waves the ones we are interested in (a given DAC value)
      // and order them by hash ID in a vector
      typedef LArCaliWaveContainer::ConstConditionsMapIterator const_iterator;
      const_iterator itVec   = caliWaveContainer->begin(gain);
      const_iterator itVec_e = caliWaveContainer->end(gain);
      
      for (; itVec != itVec_e; ++itVec) {
	
	for (const LArCaliWave& larCaliWave : *itVec) {  //Loop over all cells
	  unsigned int DAC = larCaliWave.getDAC(); 
	  IdentifierHash chidwave_hash = m_onlineHelper->channel_Hash(itVec.channelId());

	  bool IsBad = false;
	  for(int i=0;i<24*NSamplesKeep;i++){
	    tempWave[i] = larCaliWave.getSample(i);
	    if(tempWave[i]<-500) { // check that this wave is not corrupted
	      IsBad = true; 
	      break;
	    }
	  }
	  
	  if(IsBad) continue; // if corrupted wave, skip it;
	  
	  m_CaliWaves[gainref][chidwave_hash].push_back(tempWave);
	  m_CaliDACs[gainref][chidwave_hash].push_back(DAC);
	  
	  // remember index of highest DAC value for this cell (i.e. non-saturating)
	  if(DAC > m_CaliDACs[gainref][chidwave_hash][m_IndexHighestDAC[gainref][chidwave_hash]]) m_IndexHighestDAC[gainref][chidwave_hash]=m_CaliDACs[gainref][chidwave_hash].size()-1;
	  
	  // remember which index corresponds to DAC0
	  if(m_dac0sub && DAC == m_DAC0) {
	    m_IndexDAC0[gainref][chidwave_hash] = m_CaliDACs[gainref][chidwave_hash].size()-1;
	    ATH_MSG_DEBUG("Cell " << chidwave_hash << ": DAC0 is at index = " << m_IndexDAC0[gainref][chidwave_hash]);
	  }
	} // loop over dac values
      } // loop over cells
    } // Loop over gains

    sc= detStore()->remove(caliWaveContainer);
    if (sc.isFailure()) {
      ATH_MSG_WARNING("Cannot remove LArCaliWaveContainer from StoreGate ! ");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Successfully removed LArCaliWaveContainer from StoreGate ");
    
  } // m_ipassShape
  


  
  // now start to deal with digits   
  int foundkey = 0;
  for (;key_it!=key_it_e;++key_it) { //Loop over all containers that are to be processed (e.g. different gains)
    
    sc= evtStore()->retrieve(larAccumulatedCalibDigitContainer,*key_it);
    if (sc.isFailure()) {
      ATH_MSG_WARNING("Cannot read LArAccumulatedCalibDigitContainer from StoreGate! key=" << *key_it);
      if ( (std::next(key_it) == key_it_e) && foundkey==0 ){
	ATH_MSG_ERROR("None of the provided LArAccumulatedDigitContainer keys could be read");
	return StatusCode::FAILURE;
      }else{
	continue;
      }
    }
    ++foundkey;
    HWIdentifier  lastFailedFEB(0);
    
    if(larAccumulatedCalibDigitContainer->empty()) {
       ATH_MSG_DEBUG("LArAccumulatedCalibDigitContainer with key=" << *key_it << " is empty ");
    } else {
       ATH_MSG_DEBUG("LArAccumulatedCalibDigitContainer with key=" << *key_it << " has size " << larAccumulatedCalibDigitContainer->size());
    }

    for (const LArAccumulatedCalibDigit* digit : *larAccumulatedCalibDigitContainer) {  //Loop over all cells
    
      if (!(digit->isPulsed())){  //Check if cell is pulsed
	continue; //Cell not pulsed -> ignore
      }

      HWIdentifier chid=digit->hardwareID();
      HWIdentifier febid=m_onlineHelper->feb_Id(chid);
      if (febErrSum) {
	const uint16_t febErrs=febErrSum->feb_error(febid);
	if (febErrs & m_fatalFebErrorPattern) {
	  if (febid!=lastFailedFEB) {
	    lastFailedFEB=febid;
	    ATH_MSG_ERROR( "Event " << m_event_counter << " Feb " <<  m_onlineHelper->channel_name(febid) 
			   << " reports error(s):" << febErrSum->error_to_string(febErrs) << ". Data ignored.");
	  }
	  continue;
	}
      }


      if (m_delay==-1) { //First (pulsed) cell to be processed:
	m_delay=digit->delay();
      }
      else
	if (m_delay!=digit->delay()) {
	  ATH_MSG_ERROR( "Delay does not match! Found " << digit->delay() << " expected: " << m_delay);
	  continue; //Ignore this cell
	}
      
      CaloGain::CaloGain gain=digit->gain();
      if (gain<0 || gain>CaloGain::LARNGAIN)
	{ATH_MSG_ERROR( "Found not-matching gain number ("<< (int)gain <<")");
	  return StatusCode::FAILURE;
	}
      
      // if using bias-corrected Parabola tool or OFC Tool, get the pedestal
      if ( (m_recoType == PARABOLA && m_correctBias )  || m_recoType == OF) {

	//GU, try to get only once the pedestal per channel...
        IdentifierHash chid_hash = m_onlineHelper->channel_Hash(chid);
        if (m_thePedestal[chid_hash] < 0) {
	  
	  //Pointer to conditions data objects 
	  const ILArPedestal* larPedestal=NULL;
	  sc=detStore()->retrieve(larPedestal);
	  if (sc.isFailure()) {
	    ATH_MSG_FATAL( "No pedestals found in database. Aborting executiong." );
	    return sc;
	  }
	  
	  if (larPedestal) {
	    float DBpedestal = larPedestal->pedestal(chid,gain);
	    if (DBpedestal >= (1.0+LArElecCalib::ERRORCODE) ) {
	      m_thePedestal[chid_hash]=DBpedestal;
	    } else {
	      ATH_MSG_WARNING("No pedestal value found for cell hash ID = " 
		  << chid_hash << " " << m_onlineHelper->channel_name(chid) 
		  << ".  Skipping channel." 
		 );
	      continue;
	    }
	  } else {
	    ATH_MSG_WARNING("No pedestal value found for cell hash ID = " 
		<< chid_hash << " " << m_onlineHelper->channel_name(chid) 
		<< ".  Skipping channel." 
		);
	    continue;
	  }

	  ATH_MSG_DEBUG(" channel,pedestal " <<  m_onlineHelper->channel_name(chid) << " " 
	      << m_thePedestal[chid_hash]);

	} // m_ipassPedestal 	 
      }
      
      LArCalibTriggerAccumulator& accpoints=(m_ramps->get(chid,gain))[digit->DAC()];
      LArCalibTriggerAccumulator::ERRTYPE ec=accpoints.add(digit->sampleSum(),digit->sample2Sum(),digit->nTriggers());
      if (ec==LArCalibTriggerAccumulator::WrongNSamples) {
	ATH_MSG_ERROR( "Failed to accumulate sub-steps: Inconsistent number of ADC samples");
      }
      if (ec==LArCalibTriggerAccumulator::NumericOverflow) {
	ATH_MSG_ERROR( "Failed to accumulate sub-steps: Numeric Overflow");
      }
    }//End loop over all cells
  } //End loop over all containers
  
  return StatusCode::SUCCESS;
} 

// ********************** FINALIZE ****************************
StatusCode LArRampBuilder::stop()
{ 
  ATH_MSG_INFO( "in stop."); 

  //retrieve BadChannel info:
  const LArBadChannelCont* bcCont=nullptr;
  if (m_doBadChannelMask) {
    SG::ReadCondHandle<LArBadChannelCont> bcContHdl{m_bcContKey};
    bcCont=(*bcContHdl);
  }

  StatusCode sc;
  //Create transient ramp object (to be filled later) (one object for all gains)
  std::unique_ptr<LArRampComplete> larRampComplete;
  if (m_saveRecRamp){
    larRampComplete=std::make_unique<LArRampComplete>();
    ATH_CHECK(larRampComplete->setGroupingType(m_groupingType,msg()));
    ATH_CHECK(larRampComplete->initialize());
  }
  
  const LArOnOffIdMapping* cabling(0);
  if( m_isSC ){
    ATH_MSG_INFO("setting up SC cabling");
    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKeySC};
    cabling = {*cablingHdl};
    if(!cabling) {
      ATH_MSG_ERROR("Do not have mapping object " << m_cablingKeySC.key());
      return StatusCode::FAILURE;
    }    
  }else{
    ATH_MSG_INFO("setting up calo cabling");
    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
    cabling = {*cablingHdl};
    if(!cabling) {
      ATH_MSG_ERROR("Do not have mapping object " << m_cablingKey.key());
      return StatusCode::FAILURE;
    }   
  }

  
  const ILArRinj* rinj=nullptr;
  if(m_ishec) {
    ATH_CHECK(detStore()->retrieve(rinj,m_hec_key));
  }

  int containerCounter=0;

  int NRamp=0;

  for (unsigned k=0;k<(int)CaloGain::LARNGAIN;k++) {
    CaloGain::CaloGain gain=(CaloGain::CaloGain)k;
    LArConditionsContainer<ACCRAMP>::ConditionsMapIterator cell_it=m_ramps->begin(gain);
    LArConditionsContainer<ACCRAMP>::ConditionsMapIterator cell_it_e=m_ramps->end(gain);
    if (cell_it==cell_it_e) {
      ATH_MSG_INFO( "No ramp points found for gain " << gain);
      continue; //No data for this gain
    }
    //Create transient object for raw ramp (one container per gain)
    std::unique_ptr<LArRawRampContainer> larRawRampContainer;
    if (m_saveRawRamp) {
      larRawRampContainer=std::make_unique<LArRawRampContainer>();
    }
    
    //Inner loop goes over the cells.
    for (;cell_it!=cell_it_e;cell_it++){
      
      const HWIdentifier chid = cell_it.channelId();

      ACCRAMP::const_iterator dac_it=cell_it->begin();
      ACCRAMP::const_iterator dac_it_e=cell_it->end();
      auto rawramp=std::make_unique<LArRawRamp>(chid,gain);
      
      std::vector<float> peak;
      float adcpeak, timepeak;
      std::vector<float> adc0v;
      bool isADCsat = false;




      for (;dac_it!=dac_it_e;++dac_it) {

        LArRawRamp::RAMPPOINT_t ramppoint;

	adcpeak  = -999.;
	timepeak = -999.;
	// prepare samples
	float MaxADC = 0;
	int iMaxADC = 0;
	// if DAC0, fill adc0v vector
	if(m_dac0sub && dac_it->first== m_DAC0){
	  // check that DAC0 is the first DAC of list
	  	  
	  if(dac_it!=cell_it->begin()) 
	    ATH_MSG_ERROR( "DAC0 is not the first DAC ? This might be a problem... " );
	  adc0v = dac_it->second.mean();
	  ramppoint.Samples   = adc0v;
	  ramppoint.RMS       = dac_it->second.RMS();
	  ramppoint.NTriggers = dac_it->second.nTriggers();
	}else{// if not DAC0, substract DAC0 to current DAC
	  const size_t nS=dac_it->second.nsamples();
	  adc0v.resize(nS,0.0);
	  ramppoint.Samples.resize(nS);
	  ramppoint.RMS.resize(nS);
	  ramppoint.NTriggers = dac_it->second.nTriggers();
	  for(size_t k=0;k<nS;++k){
	    ramppoint.Samples[k]=dac_it->second.mean(k) - adc0v[k];
	    ramppoint.RMS[k]=dac_it->second.RMS(k);
	    // find sample max and its index
	    if(ramppoint.Samples[k]>MaxADC){
	      MaxADC = ramppoint.Samples[k];
	      iMaxADC = k;
	    }
	  }
	}
      
  	// reconstruct
	if ( m_recoType == OF) {
          if (!m_dac0sub) {
            IdentifierHash chid_hash = m_onlineHelper->channel_Hash(chid);
            for (size_t k=0;k<ramppoint.Samples.size();++k) {
              ramppoint.Samples[k] = ramppoint.Samples[k] - m_thePedestal[chid_hash];
            }
          }

	  //The following lines have been moved here from LArOFPeakReco tool:
	  float delay=m_delay;
	  unsigned kMax = max_element(ramppoint.Samples.begin(),ramppoint.Samples.end()) - ramppoint.Samples.begin() ;
          unsigned kLow, kUp;
          //unsigned nIter=0;
          if(!m_iterate) { // No iteration, original code
	   if ( kMax < 2 || kMax+2 >= ramppoint.Samples.size() ) {
	    // probably wrong max for small signal = noise artifact 
	    kMax=2;
            bool isgood=true;
            if(m_doBadChannelMask && m_bcMask.cellShouldBeMasked(bcCont,chid)) isgood=false;
	    if (cabling->isOnlineConnected(chid) && isgood) {
	      ATH_MSG_WARNING( "Not enough samples around the maximum! Use kMax=2 ("
				<< m_onlineHelper->channel_name(chid) <<", DAC=" << dac_it->first 
				<< ", Amp[2]=" << ramppoint.Samples[2] <<   " )" );
	      if (msgLvl(MSG::VERBOSE)) {
		msg(MSG::VERBOSE) <<  " Samples: ";
		for (unsigned k=0;k<ramppoint.Samples.size();k++) 
		  msg() << ramppoint.Samples[k] << " "; 
		msg() << endmsg;
	      }//end if verbose message
	    }//end if bad or disconnected channel
	   }//end if kmax out-of-range
	  
	   // convention delay=0 OFC use samp 0-1-2-3-4
	   //            delay=24 OFC use samp 1-2-3-4-5
	   // => if kmax=2 : choose OFC with delay + delayShift
	   //    if kmax=3 : choose OFC with delay+ delayShift+24
	   //    if kmax=4 : stick to kmax=3 
	   //GU temporary hardcoded number. To move to jobOptions

	   if (kMax==4) kMax=3;
	   if (kMax==3) delay += (m_delayShift+24);
	   if (kMax==2) delay += m_delayShift;
	   ATH_MSG_VERBOSE("kMax " << kMax << " delay " << delay);
	   delay=(delay-0.5)*(25./24.);
	   //Call OF peak reco tool with no iteration and peak-sample forced to kMax
           kLow = kUp = kMax;
          } else { // code with iteration
           kLow = kMax - 2;
           kUp = kMax + 2;
           delay = 0.;
           //nIter = 10;
          }
	  const LArOFPeakRecoTool::Result results=m_peakOFTool->peak(ramppoint.Samples,chid,gain,delay,0,kMax,kLow,kUp);
	  if (results.getValid()) {
	    adcpeak  = results.getAmplitude();
	    timepeak = results.getTau();
	  }
	  else 
	    ATH_MSG_ERROR( "LArOFPeak reco tool returns invalid result.");


	} else if ( m_recoType == SHAPE) {
	  
	  IdentifierHash chid_hash = m_onlineHelper->channel_Hash(chid);
	  
	  // reconstruct for non-DAC0 values and non-saturating waves
	  if(dac_it->first!= m_DAC0 && dac_it->first <= m_CaliDACs[gain][chid_hash][m_IndexHighestDAC[gain][chid_hash]]){
	    
	    // find appropriate wave
	    unsigned int GoodIndex = 9999;
	    for(unsigned int i=0;i<m_CaliDACs[gain][chid_hash].size();i++){
	      if(dac_it->first == m_CaliDACs[gain][chid_hash][i]) GoodIndex=i;
	    }
	    if(GoodIndex == 9999) {
	      ATH_MSG_WARNING("No wave found for cell = " << chid_hash << ", DAC = " << dac_it->first);
	      float min = 9999999;
	      for(unsigned int i=0;i<m_CaliDACs[gain][chid_hash].size();i++){
                int dacdiff = dac_it->first - m_CaliDACs[gain][chid_hash][i]; 
		if(abs(dacdiff)<min) 
		  {
		    min=abs(dacdiff);
		    GoodIndex=i;
		  }
	      } 
	      ATH_MSG_WARNING("Replace with DAC = " << m_CaliDACs[gain][chid_hash][GoodIndex]);
	    }
	    // substract DAC0 wave
	    for(unsigned int k=0;k<m_CaliWaves[gain][chid_hash][GoodIndex].size();k++){
	      m_CaliWaves[gain][chid_hash][GoodIndex][k] -= m_CaliWaves[gain][chid_hash][m_IndexDAC0[gain][chid_hash]][k];
	    }
	    
	    // apply reconstruction 
	    if(m_CaliWaves[gain][chid_hash][GoodIndex].size() > 0){
	      peak=m_peakShapeTool->peak(ramppoint.Samples,m_CaliWaves[gain][chid_hash][GoodIndex]);  
	      ATH_MSG_DEBUG("cell chid=" << chid.get_compact() << ",peak= " << peak[0]);
	    }else{
	      ATH_MSG_ERROR( "No wave for this cell chid=" << chid.get_compact() << ",hash= " << chid_hash);
	      peak.push_back(-999);
	      peak.push_back(-999);
	    }
	    
	    if(peak.size()>1){
	      adcpeak = peak[0];
	      timepeak = peak[1];
	    }
	  }
        } else if ( m_recoType == PARABOLA ) {
	  
	  if(m_correctBias){
	    
	    IdentifierHash chid_hash = m_onlineHelper->channel_Hash(chid);
	    
	    // get layer for correction
	    Identifier id=cabling->cnvToIdentifier(chid);
	    int layer=m_emId->sampling(id);
	    peak=m_peakParabolaTool->peak(ramppoint.Samples,layer,m_thePedestal[chid_hash]);
	    
	  }else{
	    // call peak reco without layer --> no bias correction 
	    peak=m_peakParabolaTool->peak(ramppoint.Samples);
	  }
	  adcpeak = peak[0];
	  timepeak = peak[1];
	  
	  
	} else {
	  ATH_MSG_ERROR( "Both OF and Parabola reconstruction modes not available!" ) ;
	  return StatusCode::FAILURE ;
	} 
	
	ramppoint.ADC        = adcpeak;
	ramppoint.DAC        = dac_it->first; 

        if(m_ishec && m_onlineHelper->isHECchannel(chid)) {
           if(rinj) {
              const float rinjval = rinj->Rinj(chid);
              if(rinjval < 4) ramppoint.DAC /= 2;
           }
        }

	ramppoint.iMaxSample = iMaxADC;
	ramppoint.TimeMax    = timepeak;
	
	
	// only add to rawramp non saturing points (using rawdata information)
	
	if( (dac_it->first>= m_minDAC) &&  ramppoint.ADC > -998 
	    && ((m_maxADC <= 0) || (MaxADC < m_maxADC)) ) {
	  rawramp->add(ramppoint);
	}
	else if ((m_maxADC > 0)&&(MaxADC >= m_maxADC)) { 
	  isADCsat = true; // if ADC saturated at least once, it should be notified
	  ATH_MSG_DEBUG("Saturated: "<<m_onlineHelper->channel_name(chid)<<" at DAC "<<dac_it->first<<" ADC "<< MaxADC);
	}else{
	  ATH_MSG_DEBUG("Fail ramp selection: "<<chid<<" "<<dac_it->first<<" "<<m_minDAC<<" "<<ramppoint.ADC<<" "<<MaxADC<<" "<<m_maxADC);
	} 
      }
      
      //Build ramp object..........
      if (larRampComplete) {
	std::vector<LArRawRamp::RAMPPOINT_t>& data=rawramp->theRamp();
	sort(data.begin(),data.end()); //Sort vector of raw data (necessary to cut off nonlinar high ADC-values)
	std::vector<float> rampCoeffs;
	std::vector<int> vSat;
	StatusCode sc=rampfit(m_degree+1,data,rampCoeffs,vSat,chid,cabling,bcCont);
	if (sc!=StatusCode::SUCCESS){
	  if (!cabling->isOnlineConnected(chid))
	      ATH_MSG_DEBUG("Failed to produce ramp for disconnected channel " << m_onlineHelper->channel_name(chid));
	  else if (m_doBadChannelMask && m_bcMask.cellShouldBeMasked(bcCont,chid))
	    ATH_MSG_INFO( "Failed to produce ramp for known bad channel " << m_onlineHelper->channel_name(chid));
	  else
	    ATH_MSG_ERROR( "Failed to produce ramp for channel " << m_onlineHelper->channel_name(chid));
	}
	else{
	  if(rampCoeffs[1]<0) 
	    ATH_MSG_ERROR(  "Negative 1rst order coef for ramp = " << rampCoeffs[1] << " for channel " 
			     << m_onlineHelper->channel_name(chid) );

	  if (vSat[0] != -1) { rawramp->setsat(vSat[0]); } 	// if a saturation point was found in rampfit, record it 
	  else {
	    if (isADCsat) { rawramp->setsat(data.size()-1); }	// if no saturation point was found, and ADC saturation happened, record the last ramp point 	
	    if (!isADCsat) { rawramp->setsat(data.size()); }	// if no saturation point was found, and ADC saturation did not happen, record the ramp size
	  }
	   
          //Produce transient object
          larRampComplete->set(chid,(int)gain,rampCoeffs);
          NRamp++;
	}// end else (rampfitting suceeded)
      }// end if (build ramp object)
      //Save raw ramp for this cell, if requested by jobOpts
      if (larRawRampContainer){
	larRawRampContainer->push_back(std::move(rawramp));
      }
    }//end loop cells

    if (larRawRampContainer) {
      std::string key;
      switch (gain) {
      case CaloGain::LARHIGHGAIN:
	key="HIGH";
	break;
      case  CaloGain::LARMEDIUMGAIN:
	key="MEDIUM";
	break;
      case  CaloGain::LARLOWGAIN:
	key="LOW";
	break;
      default:
	key="UNKNOWN";
	break;
      }
      key = m_keyoutput + key;
      ATH_MSG_INFO( "Recording LArRawRampContainer for gain " << (int)gain << " key=" << key);
      sc=detStore()->record(std::move(larRawRampContainer),key);
      if (sc.isFailure()) {
	ATH_MSG_ERROR( "Failed to record LArRawRamp object");
      }
    }// end if larRawRampContainer
    ++containerCounter;
  }//end loop over containers

  if (containerCounter==0) {
    ATH_MSG_WARNING("No Ramps have been produced. No data found.");
    return StatusCode::FAILURE;
  }

  if (larRampComplete){  //Save the transient  Ramp object. 

    ATH_MSG_INFO( " Summary : Number of cells with a ramp value computed : " << NRamp );
    ATH_MSG_INFO( " Summary : Number of Barrel PS cells side A or C (connected+unconnected):   3904+ 192 =  4096 ");
    ATH_MSG_INFO( " Summary : Number of Barrel    cells side A or C (connected+unconnected):  50944+2304 = 53248 ");
    ATH_MSG_INFO( " Summary : Number of EMEC      cells side A or C (connected+unconnected):  31872+3456 = 35328 ");
    ATH_MSG_INFO( " Summary : Number of HEC       cells side A or C (connected+unconnected):   2816+ 256 =  3072 ");
    ATH_MSG_INFO( " Summary : Number of FCAL      cells side A or C (connected+unconnected):   1762+  30 =  1792 ");


    sc=detStore()->record(std::move(larRampComplete),m_keyoutput);
    if (sc.isFailure()) {
      ATH_MSG_ERROR( "Failed to record LArRampComplete object");
    }
    sc=detStore()->symLink(ClassID_traits<LArRampComplete>::ID(),m_keyoutput,ClassID_traits<ILArRamp>::ID());
    if (sc.isFailure()) {
      ATH_MSG_ERROR( "Failed to symlink LArRawRamp object");
    }
  }
  m_ramps.reset();//Not needed any more. Free memory.
  ATH_MSG_INFO( "LArRampBuilder has finished.");
  return StatusCode::SUCCESS;
}// end finalize-method.

 
StatusCode LArRampBuilder::rampfit(unsigned deg, const std::vector<LArRawRamp::RAMPPOINT_t>& data, 
				   std::vector<float>& rampCoeffs, std::vector<int>& vSat, 
                                   const HWIdentifier chid, const LArOnOffIdMapping* cabling,
                                   const LArBadChannelCont* bcCont) {
  unsigned linRange=data.size();
  if (linRange<2) {
    bool isgood=true;
    if(m_doBadChannelMask && m_bcMask.cellShouldBeMasked(bcCont,chid)) isgood=false; 
    if (cabling->isOnlineConnected(chid) && isgood ) {
      ATH_MSG_ERROR( "Not enough datapoints (" << linRange << ") to fit a polynom!" );
      return StatusCode::FAILURE;
    }
    else {
      ATH_MSG_DEBUG("Not enough datapoints (" << linRange << ") to fit a polynom for a disconnected or known bad channel!" );
      return StatusCode::FAILURE;
    }
  }
  int satpoint = -1;
  if (m_satSlope) {
  
    float thisslope = 0., meanslope = 0.;
    std::vector<float> accslope;
    accslope.push_back(0);
    for (unsigned int DACIndex=1;DACIndex<linRange;DACIndex++){
      thisslope = (data[DACIndex].ADC - data[DACIndex-1].ADC)/(data[DACIndex].DAC - data[DACIndex-1].DAC);

      //FIXME: this causes some HEC channels to have rampfrom 2 points only !!!!
      if ( (satpoint == -1) && ((meanslope-thisslope) > meanslope/10.) ) { satpoint = DACIndex; } // saturation was reached

      meanslope = ( thisslope + (DACIndex-1)*(accslope[DACIndex-1]) )/DACIndex;
      accslope.push_back(meanslope);

    }
  
    if (satpoint != -1) { linRange = satpoint; } // if a saturation was found, linRange becomes the saturation index
  
  }
  vSat.push_back(satpoint);
  
  if (!m_withIntercept) {
    deg--;
  }
  bool isgood=true;
  if(m_doBadChannelMask && m_bcMask.cellShouldBeMasked(bcCont,chid)) isgood=false;
  if (deg>linRange) {
    if (cabling->isOnlineConnected(chid) && isgood ) 
      ATH_MSG_ERROR( "Not enough datapoints before saturation (" << linRange << ") to fit a polynom of degree " << deg );
    else
      ATH_MSG_DEBUG("Not enough datapoints before saturation (" << linRange << ") to fit a polynom of degree " << deg 
		    << " (channel disconnected or known to be bad)");
    
    return StatusCode::FAILURE;
  }
  
  if (data[linRange-1].DAC>0 && data[linRange-1].ADC<m_deadChannelCut && data[linRange-1].ADC!=-999.) {
    ATH_MSG_ERROR( "DAC= " << data[linRange-1].DAC << " yields ADC= " << data[linRange-1].ADC 
	   << ". Dead channel?" );
    return StatusCode::FAILURE;
  }

  int begin = 0;
  if(data[0].DAC == m_DAC0) begin = 1; // starts at 1 to skip DAC=0

  Eigen::MatrixXd alpha(deg,deg);
  Eigen::VectorXd beta(deg);
  float sigma2 = 1.;
  for (unsigned k=0;k<deg;k++)
    for (unsigned j=0;j<=k;j++)
      {
	alpha(k,j)=0;
	for (unsigned i=begin;i<linRange;i++) 
	  {
	    // we are not storing any error on the reconstructed
	    // peaks, but we can simply use the error on the sample
	    // means (RMS/sqrt(NTriggers)) to account for any
	    // potential variation on the number of accumulated
	    // triggers. BTW, this would be proportional to the ADC
	    // uncertainly but used as *DAC* uncertainty: in the limit
	    // in which the ramp is linear this is still correct, and
	    // anyway better than nothing. -- M.D. 13/7/2009	    
	    sigma2 = 1.;
	    if ( data[i].NTriggers ) {
	      //float sigma2 = (data[i].RMS[0]*data[i].RMS[0])/data[i].NTriggers;
	      sigma2 = 100./data[i].NTriggers;
	    }
	      // just use trigger number, assume RMS is constant for
	      // all DAC points (same noise). The 100. scale factor is
	      // there to guarantee the same results with respect to
	      // previous fits withour errors (having usually 100
	      // triggers), because of potential numerical
	      // differences when inverting the fit matrix even if
	      // errors are all the same.
	    if (m_withIntercept) {    
	      alpha(k,j)+=(std::pow(data[i].ADC,(int)k)*std::pow(data[i].ADC,(int)j))/sigma2;
	    } else {
	      alpha(k,j)+=(std::pow(data[i].ADC,(int)k+1)*std::pow(data[i].ADC,(int)j+1))/sigma2;
	    }
	    alpha(j,k)=alpha(k,j); //Use symmetry
	  }
      }
  
  for (unsigned k=0;k<deg;k++)
    {
      beta[k]=0;
      for (unsigned i=begin;i<linRange;i++) {
	sigma2 = 1.;
	if ( data[i].NTriggers ) {
	  sigma2 = 100./data[i].NTriggers;
	}
	if (m_withIntercept) {
	  beta[k]+=(data[i].DAC*pow(data[i].ADC,(int)k))/sigma2; 
	} else {
	  beta[k]+=(data[i].DAC*pow(data[i].ADC,(int)k+1))/sigma2;
	}
      }
    }
  
  //HepVector comp=solve(alpha,beta);
  const Eigen::VectorXd comp=alpha.colPivHouseholderQr().solve(beta);

  //Fill RampDB object
  if (!m_withIntercept)
    rampCoeffs.push_back(0);
    
  for (int l=0;l<comp.size() ;l++)
    rampCoeffs.push_back(comp[l]);
  
#ifdef LARRAMPBUILDER_DEBUGOUTPUT
  // ****************************************
  // Output for Dugging:
  for (unsigned i=1;i<data.size();i++)
    std::cout << data[i].DAC << " " << data[i].ADC << " " << std::endl;
  std::cout << "LinRange= " << linRange << " satpoint= " << satpoint<<std::endl;
  for (unsigned k=0;k<deg;k++) {
     std::cout<<"Beta "<<k<<" "<<beta[k]<<std::endl;
     for (unsigned j=0;j<=k;j++) {
            std::cout<<"Alpha "<<j<<" "<<alpha(k,j)<<std::endl;
     }
  }
  
  //Calculate error:
  double sigma=0;
  for (unsigned k=0;k<linRange;k++) //Run over all data points
    {double DACcalc=comp[0];
      for (int i=1;i<comp.size();i++) //Apply polynom
	DACcalc+=comp[i]*pow(data[k].ADC,i);
      sigma+=(DACcalc-data[k].DAC)*(DACcalc-data[k].DAC);
    }
  sigma=sqrt(sigma);
  if (linRange>1)
    sigma=sigma/(linRange-1);
  
  std::cout << "Components: ";
  for (int i=0;i<comp.size();i++)
    std::cout << comp[i] << " ";
  std::cout << "sigma=" << sigma << std::endl;
#undef LARRAMPBUILDER_DEBUGOUTPUT
#endif 
  
  return StatusCode::SUCCESS;
}
