/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "LArDigitsAccumulator.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "LArIdentifier/LArOnlineID.h"

#include "CLHEP/Units/SystemOfUnits.h"
#include <cmath>
#include <cstdint>

LArDigitsAccumulator::LArDigitsAccumulator (const std::string& name, ISvcLocator* pSvcLocator):
  AthAlgorithm(name, pSvcLocator),
  m_onlineHelper(nullptr),
  m_AccuDigitContainerName("LArAccumulatedDigits"),
  m_NtriggersPerStep(100),
  m_nStepTrigger(30)
{
  declareProperty("LArAccuDigitContainerName",m_AccuDigitContainerName);
  declareProperty("KeyList",m_keylist);
  declareProperty("NTriggersPerStep",m_NtriggersPerStep);
  declareProperty("StepOfTriggers",m_nStepTrigger);
  declareProperty("DropPercentTrig",m_DropPercentTrig=10);
  declareProperty("isSC",m_isSC=false);

  m_event_counter=0;
}


StatusCode LArDigitsAccumulator::initialize()
{
  StatusCode sc;
  if(m_isSC){
     const LArOnline_SuperCellID *scid;
     sc = detStore()->retrieve(scid, "LArOnline_SuperCellID");
     if (sc.isFailure()) {
        ATH_MSG_ERROR( "Could not get LArOnline_SuperCellID helper !" );
        return sc;
     } else {
        m_onlineHelper = (const LArOnlineID_Base*)scid;
        ATH_MSG_DEBUG("Found the LArOnlineID helper");
     }
  } else { 
     const LArOnlineID* ll;
     sc = detStore()->retrieve(ll, "LArOnlineID");
     if (sc.isFailure()) {
        ATH_MSG_ERROR( "Could not get LArOnlineID helper !" );
        return sc;
     } else {
        m_onlineHelper = (const LArOnlineID_Base*)ll;
        ATH_MSG_DEBUG(" Found the LArOnlineID helper. ");
     }
  } //m_isSC

  m_Accumulated.resize(m_onlineHelper->channelHashMax());
  return StatusCode::SUCCESS;
}



StatusCode LArDigitsAccumulator::execute() 
{
  if ( m_event_counter < 100 || m_event_counter%100==0 )
    ATH_MSG_INFO( "Processing event " << m_event_counter );
  ++m_event_counter;

  
  // pointer to input container
  const LArDigitContainer* DigitContainer=nullptr;


  // retrieve input Digits
  
  //Loop over all containers that are to be processed (e.g. different gains)
  for (const std::string& key : m_keylist) {
     
    ATH_CHECK( evtStore()->retrieve(DigitContainer,key) );

    // store LArAccumulatedDigits
    ACCUMDIGIT_VEC& vAccum = m_my_vec;
    if(vAccum.empty()) vAccum.resize(m_onlineHelper->channelHashMax());

    // Loop over DigitContainer

    if(DigitContainer->empty()) {
      ATH_MSG_DEBUG( "LArDigitContainer with key=" << key << " is empty "  );
    }else{
      ATH_MSG_DEBUG( "LArDigitContainer with key=" << key << " has size =  " << DigitContainer->size()  );
    }

    //unsigned int iStepTrigger=0;

    // output container
    auto larAccuDigitContainer = std::make_unique<LArAccumulatedDigitContainer>(SG::VIEW_ELEMENTS);
    //Loop over all cells
    for (const LArDigit* digit : *DigitContainer) {

      // identificators
      HWIdentifier chid=digit->hardwareID();      
      const IdentifierHash hashid = m_onlineHelper->channel_Hash(chid);
      
      if( m_isSC ){
	///first check if there is a -1 and continue
	bool hasInvalid=false;
	for(auto s : digit->samples()){
	  if(s<0){
	    hasInvalid=true;
	    break;
	  }
	}
	if(hasInvalid) continue;
      }


      CaloGain::CaloGain gain=digit->gain();
      if (gain<0 || gain>CaloGain::LARNGAIN)
      {
        ATH_MSG_ERROR( "Found not-matching gain number ("<< (int)gain <<")"  );
        return StatusCode::FAILURE;
      }

      // object to be filled for each cell
      LArAccumulated& cellAccumulated = m_Accumulated[hashid];
      cellAccumulated.m_onlineId=chid.get_identifier32().get_compact();
      
      // trigger counter for each cell
      cellAccumulated.m_ntrigger++;

      //ATH_MSG_DEBUG( "chid = " << chid << ", trigger = " << cellAccumulated.m_ntrigger << ", sample 0 = "<< digit->samples()[0]  );

      // at first trigger, initialize vectors
      unsigned int sizeSamples = digit->samples().size();
      //ATH_MSG_DEBUG( "# of samples = " << sizeSamples  );

      LArAccumulatedDigit* accuDigit;

      if(m_event_counter==1){
	accuDigit = new LArAccumulatedDigit(chid);
	vAccum[hashid]=accuDigit;	
      }
      
      if(cellAccumulated.m_ntrigger==1){
	//	std::cout << "resizing to 0 "<< std::endl;
	cellAccumulated.m_samplesum.clear();
	cellAccumulated.m_samplesum.resize(sizeSamples,0);
	cellAccumulated.m_matrix.clear();
	cellAccumulated.m_matrix.resize(sizeSamples,0);
      }
      accuDigit = m_my_vec[hashid];
      
      unsigned int l= 0;
      for(unsigned int j=0;j<sizeSamples;j++){
	cellAccumulated.m_samplesum[j] += digit->samples()[j];
	for(unsigned int k=j;k<sizeSamples;k++)
	  {
	    l=k-j;
	    cellAccumulated.m_matrix[l] += digit->samples()[j]*digit->samples()[k];
	    //	    std::cout << "accumulation, l =  "<< l << ", j = "<< j << ", k = "<< k << ", matrix = "<< cellAccumulated.m_matrix[l] <<", sum = "<< cellAccumulated.m_samplesum <<  std::endl;
	    l++;
	  }
      }

      //ATH_MSG_DEBUG( "Sum = " << cellAccumulated.m_samplesum  );
      //ATH_MSG_DEBUG( "Matrix = " << cellAccumulated.m_matrix[0]  );

      // when reached total number of triggers for this step, fill LArAccumulatedDigit and reset number of triggers
      
      unsigned int ntrigUsed = m_NtriggersPerStep;
      if ( m_isSC && m_DropPercentTrig != 0 ){
	ntrigUsed -= ntrigUsed*(m_DropPercentTrig/100);	
      }

      if(cellAccumulated.m_ntrigger==ntrigUsed){

	ATH_MSG_DEBUG( "filling LArAccumulatedDigit "  );
	ATH_MSG_DEBUG( "chid = " << chid << ", gain = " << gain << ", trigPerStep = " << m_NtriggersPerStep  );
	
	accuDigit->setAddSubStep(gain,chid,cellAccumulated.m_samplesum,cellAccumulated.m_matrix,ntrigUsed);
      
	larAccuDigitContainer->push_back(accuDigit);

	cellAccumulated.m_nused = cellAccumulated.m_ntrigger;
	cellAccumulated.m_ntrigger = 0;

      }
      
    }// loop over cells in container

    ATH_MSG_DEBUG( "Finished loop over channels "  );

    const std::string SGkey=key+m_AccuDigitContainerName;
    ATH_CHECK( evtStore()->record(std::move(larAccuDigitContainer),SGkey,false) );
    ATH_MSG_INFO( "Recorded succesfully LArAccumulatedDigitContainer with key " << SGkey   );
    

  } // loop over key container
  return StatusCode::SUCCESS;
}
  

StatusCode LArDigitsAccumulator::finalize(){

  if ( !m_isSC ) return StatusCode::SUCCESS;
  int ntrigUsed = m_NtriggersPerStep - m_NtriggersPerStep*(m_DropPercentTrig/100);
  for(auto &sc : m_Accumulated){
    if(sc.m_onlineId && sc.m_nused != ntrigUsed){
      ATH_MSG_WARNING("Not enough triggers for channel OnlineID " << sc.m_onlineId << " ntriggers " << sc.m_ntrigger );
    }
  }

  return StatusCode::SUCCESS;

}
