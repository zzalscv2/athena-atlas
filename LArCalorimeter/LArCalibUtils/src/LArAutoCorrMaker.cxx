/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
 
 NAME:     LArAutoCorrMaker.cxx
 PACKAGE:  offline/LArCalorimeter/LArCalibUtils
 
 AUTHORS:  M. AHARROUCHE
 CREATED:  Dec. 16, 2003
  
 PURPOSE:  Selects the good events and computes the autocorrelation 
           matrix for each cell. It processes all the gains
           simultaneously.
           In fact only the last (m_nsamples-1) elements of the 
           first line (or column) of autocorrelation matrix are
           computed and stored in TDS, for these reasons:
           - symetrie of autocorrelation matrix
           - equivalence of autocorrelation elements: 
             B(n,n+i)\eq B(m,m+i) (eg B12 \eq B23).

  HISTORY:
          - Dec. 16, 2003: M. Aharrouche: creation
          - March 1st, 2004: S. Laplace: write result into DB instead of ASCII file
             
********************************************************************/

// Include files
#include "LArCalibUtils/LArAutoCorrMaker.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawConditions/LArAutoCorrComplete.h"

#include <cmath>
#include <unistd.h>

#include "xAODEventInfo/EventInfo.h"


LArAutoCorrMaker::LArAutoCorrMaker(const std::string& name, ISvcLocator* pSvcLocator) 
  : AthAlgorithm(name, pSvcLocator), 
    m_groupingType("ExtendedSubDetector"), // SubDetector, Single, FeedThrough  
    m_nEvents(0)
{
  declareProperty("KeyList",      m_keylistproperty);
  declareProperty("KeyOutput",    m_keyoutput="LArAutoCorr");
  declareProperty("events_ref",   m_nref=50);
  declareProperty("nsigma",       m_rms_cut=5);
  declareProperty("normalize",    m_normalize=1); 
  declareProperty("physics",      m_physics=0); 
  declareProperty("GroupingType", m_groupingType); 
  declareProperty("MinBCFromFront",m_bunchCrossingsFromFront=0);
}


LArAutoCorrMaker::~LArAutoCorrMaker()
= default;

StatusCode LArAutoCorrMaker::initialize() {

  ATH_MSG_INFO( ">>> Initialize" );
  
  if (m_keylistproperty.empty()) // Not key list given
    {m_keylistproperty.emplace_back("HIGH");
    m_keylistproperty.emplace_back("MEDIUM");
    m_keylistproperty.emplace_back("LOW");
    m_keylistproperty.emplace_back("FREE"); // For H6...
    }

  m_keylist=m_keylistproperty;
  if (m_keylist.empty()) {
    ATH_MSG_ERROR( "Key list is empty!" );
    return StatusCode::FAILURE;
  }

  m_autocorr.setGroupingType(LArConditionsContainerBase::SingleGroup);
  StatusCode sc=m_autocorr.initialize();
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed initialize intermediate AutoCorr object" );
    return sc;
  }

  return StatusCode::SUCCESS;
}


//---------------------------------------------------------------------------
StatusCode LArAutoCorrMaker::execute()
  //---------------------------------------------------------------------------
{
  StatusCode sc;
  if (m_bunchCrossingsFromFront>0) {
    const xAOD::EventInfo* eventInfo;
    sc=evtStore()->retrieve( eventInfo ); 
    if (sc.isFailure()) {
      ATH_MSG_ERROR( "Failed to retrieve EventInfo object!" );
      return sc;
    }

    SG::ReadCondHandle<BunchCrossingCondData> bccd (m_bcDataKey);
    const BunchCrossingCondData* bunchCrossing=*bccd;
    if (!bunchCrossing) {
      ATH_MSG_ERROR("Failed to retrieve Bunch Crossing obj");
      return StatusCode::FAILURE;
    }

    uint32_t bcid = eventInfo->bcid();
    const int nBCsFromFront=bunchCrossing->distanceFromFront(bcid,BunchCrossingCondData:: BunchCrossings);
    if (nBCsFromFront < m_bunchCrossingsFromFront) {
      ATH_MSG_DEBUG("BCID " << bcid << " only " << nBCsFromFront << " BCs from front of BunchTrain. Event ignored. (min=" <<m_bunchCrossingsFromFront 
		    << ", type= " << bunchCrossing->bcType(bcid) << ")" );
      return StatusCode::SUCCESS; //Ignore this event
    }
    else
     ATH_MSG_DEBUG("BCID " << bcid << " is " << nBCsFromFront << " BCs from front of BunchTrain. Event accepted.(min=" <<m_bunchCrossingsFromFront << ")");
  }

  const LArDigitContainer* larDigitContainer = nullptr;

  for (const std::string& key : m_keylist) {
    ATH_MSG_DEBUG("Reading LArDigitContainer from StoreGate! key=" << key);
    sc= evtStore()->retrieve(larDigitContainer,key);
    if (sc.isFailure() || !larDigitContainer) {
      ATH_MSG_DEBUG("Cannot read LArDigitContainer from StoreGate! key=" << key);
      continue;
    }
    if(larDigitContainer->empty()) {
      ATH_MSG_DEBUG("Got empty LArDigitContainer (key=" << key << ").");
      continue;
    }
    ATH_MSG_DEBUG("Got LArDigitContainer with key " << key <<", size="  << larDigitContainer->size());
    ++m_nEvents;
    m_nsamples = (*larDigitContainer->begin())->nsamples();
    ATH_MSG_DEBUG("NSAMPLES (from digit container) = " << m_nsamples );

    for (const LArDigit* digit : *larDigitContainer) {
      const HWIdentifier chid=digit->hardwareID();
      const CaloGain::CaloGain gain=digit->gain();
      if (gain<0 || gain>CaloGain::LARNGAIN) {
	ATH_MSG_ERROR( "Found odd gain number ("<< (int)gain <<")" );
	return StatusCode::FAILURE;
      }
      const std::vector<short> & samples = digit->samples();
      //      LArAutoCorr& thisAC=m_autocorr[gain][chid];
      LArAutoCorr& thisAC=m_autocorr.get(chid,gain);

      if(thisAC.get_max()!=-1){ //Have already boundaries set
	  std::vector<short>::const_iterator s_it=samples.begin();
	  std::vector<short>::const_iterator s_it_e=samples.end();
	  const short &  min = thisAC.get_min();
	  const short &  max = thisAC.get_max();
	  
	  for (;s_it!=s_it_e && *s_it>=min && *s_it<=max;++s_it)
            ;
	  if (s_it==s_it_e) 
	    thisAC.add(samples,m_nsamples);
      }
      else {
	  thisAC.add(samples,m_nsamples);
	  if (thisAC.get_nentries()==m_nref && m_nref>0) { //Set window size
	    // Define the window (min, max)according to pedestal and noise
	    // computed for a number of events = m_nref  
	    const double mean  = thisAC.get_mean();
	    const double noise = thisAC.get_rms();
	    const short min = (short)floor(mean - m_rms_cut*noise);
	    const short max = (short)ceil(mean + m_rms_cut*noise);
	    thisAC.set_min(min);
	    thisAC.set_max(max);
	    thisAC.correl_zero();
	  } //end if nentries==m_nref
	} // end else
    }//End loop over all cells
  }// End loop over all containers
  return StatusCode::SUCCESS;
}


//---------------------------------------------------------------------------
StatusCode LArAutoCorrMaker::stop()
  //---------------------------------------------------------------------------
{
  StatusCode sc;
  ATH_MSG_INFO( ">>> Stop()" );

  if (m_keylist.empty()) {
    ATH_MSG_ERROR( "Key list is empty! No containers processed!" );
    return StatusCode::FAILURE;
  }

  // Create the LArAutoCorrComplete object
  LArAutoCorrComplete* larAutoCorrComplete = new LArAutoCorrComplete();

  sc=larAutoCorrComplete->setGroupingType(m_groupingType,msg());
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed to set groupingType for LArAutoCorrComplete object" );
    return sc;
  }

  sc=larAutoCorrComplete->initialize(); 
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed initialize LArAutoCorrComplete object" );
    return sc;
  }

  for (int gain=0;gain<(int)CaloGain::LARNGAIN;gain++)  {
      LARACMAP::ConditionsMapIterator cell_it=m_autocorr.begin(gain);
      LARACMAP::ConditionsMapIterator cell_it_e=m_autocorr.end(gain);
      
      //Inner loop goes over the cells.
      for (;cell_it!=cell_it_e;cell_it++) {
	LArAutoCorr autocorr = *cell_it;
	// Check number of entries
	if(autocorr.get_nentries()==0) continue;
	    
	// Get the autocorrelation matrix


	//MGV implement normalization switch
	const  std::vector<double> & cov = autocorr.get_cov(m_normalize,m_physics);

	//The AutoCorr is stored as float -> convert
	std::vector<float> cov_flt;
	cov_flt.reserve(cov.size());
	std::vector<double>::const_iterator it=cov.begin();
	std::vector<double>::const_iterator it_e=cov.end();
	for (;it!=it_e;++it)
	  cov_flt.push_back((float)*it);
	HWIdentifier ch_id = cell_it.channelId();
      	
	// Fill the data class with autocorrelation elements
	if (ch_id!=0) {
	  larAutoCorrComplete->set(ch_id,gain,cov_flt);
	}
      }
    }
  
  ATH_MSG_INFO( "AutoCorrelation based on " << m_nEvents << " events." );
  ATH_MSG_INFO( " Summary : Number of cells with a autocorr value computed : " << larAutoCorrComplete->totalNumberOfConditions()  );
  ATH_MSG_INFO( " Summary : Number of Barrel PS cells side A or C (connected+unconnected):  4096 " );
  ATH_MSG_INFO( " Summary : Number of Barrel    cells side A or C (connected+unconnected): 53248 " );
  ATH_MSG_INFO( " Summary : Number of EMEC      cells side A or C (connected+unconnected): 35328 " );
  ATH_MSG_INFO( " Summary : Number of HEC       cells side A or C (connected+unconnected):  3072 ");
  ATH_MSG_INFO( " Summary : Number of FCAL      cells side A or C (connected+unconnected):  1792 " );
  
  // Record LArAutoCorrComplete
  sc = detStore()->record(larAutoCorrComplete,m_keyoutput);
  if (sc != StatusCode::SUCCESS) { 
      ATH_MSG_ERROR( " Cannot store LArAutoCorrComplete in DetectorStore " );
      return sc;
    }
  
  // Make symlink
  sc = detStore()->symLink(larAutoCorrComplete, (ILArAutoCorr*)larAutoCorrComplete);
  if (sc != StatusCode::SUCCESS)  {
      ATH_MSG_ERROR( " Cannot make link for Data Object " );
      return sc;
    }
  
  return StatusCode::SUCCESS;
}








