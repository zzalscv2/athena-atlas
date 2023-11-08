/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************

 NAME:     LArPedestalAutoCorrBuilder.cxx
 PACKAGE:  offline/LArCalorimeter/LArCalibUtils

 AUTHORS:  W. Lampl
 CREATED:  Aug 17th, 2009, merging LArPedestalBuilder & LArAutoCorrBuilder

 PURPOSE:  Create LArPedestalComplete and LArAutoCorrComplete objects
           out of pre-accmulated LArAccumulatedDigits

********************************************************************/

// Include files
#include "LArCalibUtils/LArPedestalAutoCorrBuilder.h"

#include "LArRawEvent/LArFebErrorSummary.h"
#include "LArRawConditions/LArPedestalComplete.h"
#include "LArRawConditions/LArAutoCorrComplete.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"


LArPedestalAutoCorrBuilder::LArPedestalAutoCorrBuilder(const std::string& name, ISvcLocator* pSvcLocator) 
  : AthAlgorithm(name, pSvcLocator),
    m_onlineHelper(nullptr),
    m_event_counter(0),
    m_fatalFebErrorPattern(0xffff)
{
  declareProperty("KeyList",         m_keylist);
  declareProperty("PedestalKey",     m_pedContName="LArPedestal");
  declareProperty("normalize",       m_normalize=1); 
  declareProperty("AutoCorrKey",     m_acContName="LArAutoCorr");
  declareProperty("GroupingType",    m_groupingType="ExtendedFeedThrough"); 
  declareProperty("doPedestal",      m_doPedestal=true);
  declareProperty("doAutoCorr",      m_doAutoCorr=true);
  declareProperty("sample_min",      m_sample_min=-1);
  declareProperty("sample_max",      m_sample_max=-1);
}

LArPedestalAutoCorrBuilder::~LArPedestalAutoCorrBuilder()
= default;

StatusCode LArPedestalAutoCorrBuilder::initialize()
{
  if (m_groupingType == "SuperCells") {
    ATH_MSG_INFO("Processing LAr SuperCells");
    const LArOnline_SuperCellID* onlID;
    ATH_CHECK(detStore()->retrieve(onlID,"LArOnline_SuperCellID"));
    m_onlineHelper=onlID; // cast to base-class
  }
  else {
    ATH_MSG_INFO("Processing regular LArCells");
    const LArOnlineID* onlID;
    ATH_CHECK(detStore()->retrieve(onlID, "LArOnlineID"));
    m_onlineHelper=onlID; // cast to base-class
  }

  
  
  if (!m_doPedestal && !m_doAutoCorr) {
    ATH_MSG_ERROR( "Configuration Problem: Neither doPedstal nor doAutoCorr set!" );
    return StatusCode::FAILURE;
  }

 if (m_keylist.empty()) // Not key list given
   {m_keylist.emplace_back("HIGH");
    m_keylist.emplace_back("MEDIUM");
    m_keylist.emplace_back("LOW");
    m_keylist.emplace_back("FREE"); //For H6....
   }

 //Container for internal accumulation
 m_accu.setGroupingType(m_groupingType == "SuperCells" ? LArConditionsContainerBase::SuperCells : LArConditionsContainerBase::SingleGroup);
 StatusCode sc=m_accu.initialize(); 
 if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed initialize LArConditionsContainer 'm_accu'" );
    return sc;
  }
 return StatusCode::SUCCESS;
}


//---------------------------------------------------------------------------
StatusCode LArPedestalAutoCorrBuilder::execute()
//---------------------------------------------------------------------------
{

  StatusCode sc;
  ++m_event_counter;
  if (m_keylist.empty()) {
    ATH_MSG_ERROR( "Key list is empty! No containers processed!" );
    return StatusCode::FAILURE;
  } 
  
  
  const LArFebErrorSummary* febErrSum=nullptr;
  if (evtStore()->contains<LArFebErrorSummary>("LArFebErrorSummary")) {
    sc=evtStore()->retrieve(febErrSum);
    if (sc.isFailure()) {
      ATH_MSG_ERROR( "Failed to retrieve FebErrorSummary object!" );
      return sc;
    }
  }
  else
    if (m_event_counter==1)
      ATH_MSG_WARNING( "No FebErrorSummaryObject found! Feb errors not checked!" );


  const LArAccumulatedDigitContainer* container = nullptr;
  //Outermost loop goes over all gains (different containers).
  int foundkey = 0;

  for (const std::string& key : m_keylist) {
    sc= evtStore()->retrieve(container,key);
    if (sc.isFailure() || !container) {
      ATH_MSG_DEBUG("Cannot read LArAccumulatedDigitContainer from StoreGate! key=" << key);
      if ( ( (&key == &m_keylist.back()) ) && foundkey==0 ){
	ATH_MSG_ERROR("None of the provided LArAccumulatedDigitContainer keys could be read");
	return StatusCode::FAILURE;
      }else{
	continue;
      }
    }
    foundkey+=1;

    // check that container is not empty
    if(container->empty() ) {
      ATH_MSG_DEBUG("LArAccumulatedDigitContainer (key=" << key << ") is empty ");
      continue;
    }else{
      ATH_MSG_DEBUG("LArAccumulatedDigitContainer (key=" << key << ") has length " << container->size());
    }

    HWIdentifier  lastFailedFEB(0);
    //Inner loop goes over the cells.
    for (const LArAccumulatedDigit* dg : *container) {  //Loop over all cells
      if (dg->nTrigger()==0) continue; //Don't care about empty digits
      const HWIdentifier chid=dg->hardwareID();
      const HWIdentifier febid=m_onlineHelper->feb_Id(chid);
      if (febErrSum) {
	const uint16_t febErrs=febErrSum->feb_error(febid);
	if (febErrs & m_fatalFebErrorPattern) {
	  if (febid!=lastFailedFEB) {
	    lastFailedFEB=febid;
	    ATH_MSG_ERROR( "Event " << m_event_counter << " Feb " <<  m_onlineHelper->channel_name(febid) 
		<< " reports error(s):" << febErrSum->error_to_string(febErrs) << ". Data ignored." );
	  }
	  continue;
	} //end if fatal feb error
      }//end if check feb error summary

      const CaloGain::CaloGain gain=dg->gain();

      LArAccumulatedDigit& accDg=m_accu.get(chid,gain);
      if (!accDg.setAddSubStep(*dg)) 
	ATH_MSG_ERROR( "Failed to accumulate sub-steps! Inconsistent number of ADC samples" );
    } //end loop over input container
  }//end loop over keys
  return StatusCode::SUCCESS;
}



StatusCode LArPedestalAutoCorrBuilder::stop() {

  std::unique_ptr<LArAutoCorrComplete> larAutoCorrComplete;
  std::unique_ptr<LArPedestalComplete> larPedestalComplete;

  if (m_doAutoCorr) { 
    //Book and initialize LArAutoCorrComplete object
    ATH_MSG_INFO("Creating LArAutoCorrComplete");
    larAutoCorrComplete = std::make_unique<LArAutoCorrComplete>();
    ATH_CHECK( larAutoCorrComplete->setGroupingType(m_groupingType,msg()) );
    ATH_CHECK( larAutoCorrComplete->initialize() );
  }

  if (m_doPedestal) {
    //Book and initialize LArPedestalComplete object
    ATH_MSG_INFO("Creating LArAutoPedestalComplete");
    larPedestalComplete = std::make_unique<LArPedestalComplete>();
    ATH_CHECK( larPedestalComplete->setGroupingType(m_groupingType,msg()) );
    ATH_CHECK( larPedestalComplete->initialize() );
  }
    
  //For the log output further down
  std::string objName;
  if (m_doPedestal)
    objName="pedestal";
  if (m_doAutoCorr)
    objName="autocorr";
  if (m_doAutoCorr && m_doPedestal)
    objName="pedestal & autocorr";

  int n_zero,n_min, n_max, n_cur;
  n_zero=0; n_max=n_min=-1;
  unsigned NCells=0;
  std::vector<float> cov;
  //Loop over gains
  for (unsigned k=0;k<(int)CaloGain::LARNGAIN;k++) {
    CaloGain::CaloGain gain=(CaloGain::CaloGain)k;
    //Loop over cells
    ACCU::ConditionsMapIterator cell_it=m_accu.begin(gain);
    ACCU::ConditionsMapIterator cell_it_e=m_accu.end(gain);
    
    if (cell_it==cell_it_e){
      continue; //No data for this gain
    }
    for (;cell_it!=cell_it_e;cell_it++) {
      const LArAccumulatedDigit& dg=*cell_it;
      n_cur = dg.nTrigger();
      if(n_cur==0) { n_zero++; continue; }

      HWIdentifier chid = cell_it.channelId();     
      
      if(n_cur<n_min || n_min<0) n_min=n_cur;
      if(n_cur>n_max || n_max<0) n_max=n_cur;
      
      // Fill the data class with pedestal and rms values
      if (larPedestalComplete)
	larPedestalComplete->set(chid,gain,dg.mean(m_sample_min, m_sample_max),dg.RMS(m_sample_min, m_sample_max));
      
      if (larAutoCorrComplete) {
	dg.getCov(cov,m_normalize);
	larAutoCorrComplete->set(chid,gain,cov);
      }
      NCells++;
    }//end loop over all cells	

    ATH_MSG_INFO( "Gain " << gain << " Number of cells with 0 events to compute "<<objName<< ": " << n_zero );
    ATH_MSG_INFO( "Gain " << gain << " Minimum number of events*samples to compute " <<objName<<": "<< n_min );
    ATH_MSG_INFO( "Gain " << gain << " Maximum number of events*samples to compute " <<objName<<": " <<n_max );
  }// End loop over all containers
  
  ATH_MSG_INFO( " Summary : Number of cells with " <<objName<<" value computed : " << NCells  );
  ATH_MSG_INFO( " Summary : Number of Barrel PS cells side A or C (connected+unconnected):   3904+ 192 =  4096 " );
  ATH_MSG_INFO( " Summary : Number of Barrel    cells side A or C (connected+unconnected):  50944+2304 = 53248 " );
  ATH_MSG_INFO( " Summary : Number of EMEC      cells side A or C (connected+unconnected):  31872+3456 = 35328 " );
  ATH_MSG_INFO( " Summary : Number of HEC       cells side A or C (connected+unconnected):   2816+ 256 =  3072 " );
  ATH_MSG_INFO( " Summary : Number of FCAL      cells side A or C (connected+unconnected):   1762+  30 =  1792 " );
    
  if (larPedestalComplete) {
    ATH_CHECK( detStore()->record(std::move(larPedestalComplete),m_pedContName) );
  }


  if (larAutoCorrComplete) {
    ATH_CHECK( detStore()->record(std::move(larAutoCorrComplete),m_acContName) );
  }

  return StatusCode::SUCCESS;
}

