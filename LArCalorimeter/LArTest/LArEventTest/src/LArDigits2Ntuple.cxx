/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "LArEventTest/LArDigits2Ntuple.h"
#include "CaloIdentifier/CaloGain.h"
#include "xAODEventInfo/EventInfo.h"

#include "GaudiKernel/SmartDataPtr.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArFebHeaderContainer.h"
#include "TBEvent/TBPhase.h"
#include "TBEvent/TBTriggerPatternUnit.h"
#include "TBEvent/TBScintillatorCont.h"

#include "LArElecCalib/ILArPedestal.h"
#include <fstream>


LArDigits2Ntuple::LArDigits2Ntuple(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator),
    m_onlineHelper(0),
    m_larCablingSvc(0),
    m_emId(0),
    m_hecId(0),
    m_fcalId(0),
    m_nt(0),
    m_nsamples(0),
    m_accept(0),
    m_reject(0),
    m_phase(0),
    m_scint(0),
    m_trigger(0),
    m_sca(0),
    m_ped(0)
{ 
  declareProperty("ContainerKey",m_contKey);
  declareProperty("NSamples",m_nsamples=5);
  // By default to get only pedestal events, we need to set m_reject=1
  // But to reject non flaged events (physics events occuring out of
  // the beam signal...) we need also to set m_accept=14.
  declareProperty("accept", m_accept=14);
  declareProperty("reject", m_reject=1);
  declareProperty("ReadPhase", m_phase=1);
  declareProperty("ReadScint", m_scint=1);
  declareProperty("ReadTrigger", m_trigger=1);
  declareProperty("ReadSCA", m_sca=1);
  declareProperty("ReadPedestal", m_ped=1);
}

LArDigits2Ntuple::~LArDigits2Ntuple() 
{}

StatusCode LArDigits2Ntuple::initialize()
{
  //Use CaloIdManager to access detector info
  const CaloIdManager *caloIdMgr=CaloIdManager::instance() ;
  m_emId=caloIdMgr->getEM_ID();
  m_fcalId=caloIdMgr->getFCAL_ID();
  m_hecId=caloIdMgr->getHEC_ID();

  if (!m_emId) {
    ATH_MSG_ERROR ( "Could not access lar EM ID helper" );
    return StatusCode::FAILURE;
  }
  if (!m_fcalId) {
    ATH_MSG_ERROR ( "Could not access lar FCAL ID helper" );
    return StatusCode::FAILURE;
  }
  if (!m_hecId) {
    ATH_MSG_ERROR ( "Could not access lar HEC ID helper" );
    return StatusCode::FAILURE;
  }

  ATH_CHECK( toolSvc()->retrieveTool("LArCablingService",m_larCablingSvc) );

  ATH_CHECK( detStore()->retrieve(m_onlineHelper, "LArOnlineID") );
  ATH_MSG_DEBUG ( " Found the LArOnlineID helper. " );

  // Book ntuple
  NTupleFilePtr file1(ntupleSvc(),"/NTUPLES/FILE1");
  if (!file1) {
    ATH_MSG_ERROR ( "Booking of NTuple failed" );
    return StatusCode::FAILURE;
  }
  NTuplePtr nt(ntupleSvc(),"/NTUPLES/FILE1/DIGITS");
  if (!nt) {
    nt=ntupleSvc()->book("/NTUPLES/FILE1/DIGITS",CLID_ColumnWiseTuple,"Digits");
  }
  if (!nt) {
    ATH_MSG_ERROR ( "Booking of NTuple failed" );
    return StatusCode::FAILURE;
  }

  ATH_CHECK( nt->addItem("event",event) );
  ATH_CHECK( nt->addItem("layer",layer,0,4) );
  ATH_CHECK( nt->addItem("ieta",eta,0,510) );
  ATH_CHECK( nt->addItem("iphi",phi,0,1023) );
  ATH_CHECK( nt->addItem("region",region,0,1) );
  ATH_CHECK( nt->addItem("barrel_ec",barrel_ec,0,1) );
  ATH_CHECK( nt->addItem("pos_neg",pos_neg,0,1) );
  ATH_CHECK( nt->addItem("detector",detector,0,2) );
  ATH_CHECK( nt->addItem("FT",FT,0,32) );
  ATH_CHECK( nt->addItem("slot",slot,1,15) );
  ATH_CHECK( nt->addItem("channel",channel,0,127) );
  ATH_CHECK( nt->addItem("gain",gain,0,3) );
  ATH_CHECK( nt->addItem("samples",m_nsamples,samples) );

  if(m_ped)     ATH_CHECK( nt->addItem("ped",ped) );
  if(m_sca)     ATH_CHECK( nt->addItem("sca",m_nsamples,sca) );
  if(m_phase)   ATH_CHECK( nt->addItem("tdc",tdc) );
  if(m_trigger) ATH_CHECK( nt->addItem("trigger",trigger) );
  if(m_scint)   ATH_CHECK( nt->addItem("S1",S1) );

  m_nt=nt;

  return StatusCode::SUCCESS;
}  

StatusCode LArDigits2Ntuple::execute()
{
  int eventnumber,triggerword;
  double S1Adc,tdcphase;

  // Retrieve EventInfo
  const DataHandle<xAOD::EventInfo> thisEventInfo;
  StatusCode sc=evtStore()->retrieve(thisEventInfo);
  eventnumber=0;
  if (sc!=StatusCode::SUCCESS)
    ATH_MSG_WARNING ( "No EventInfo object found!" );
  else {
    eventnumber=thisEventInfo->eventNumber();
  }

  // Retrieve the TBScintillators
  if(m_scint) {
    S1Adc=-999.0;
    const TBScintillatorCont * theTBScint;
    sc = evtStore()->retrieve(theTBScint,"ScintillatorCont");
    if (sc.isFailure()) 
      {
	ATH_MSG_ERROR ( " Cannot read TBScintillatorCont from StoreGate! " );
	//return StatusCode::FAILURE;
      }
    else {
      TBScintillatorCont::const_iterator it_scint = theTBScint->begin();
      TBScintillatorCont::const_iterator last_scint = theTBScint->end();
      for(;it_scint!=last_scint;it_scint++) {
	const TBScintillator * scint = (*it_scint);
	const std::string name = scint->getDetectorName();
	if (name=="S1") {
	  S1Adc = scint->getSignal();
	  break;
	}
      } //end loop over scintillator-container
    }
  }
  else S1Adc=0; 

  //Retrieve the TBPhase
  if(m_phase) {
    const TBPhase* theTBPhase;
    sc = evtStore()->retrieve(theTBPhase, "TBPhase");
    
    if (sc.isFailure()) {
      tdcphase = -999.0;
      ATH_MSG_ERROR( "cannot allocate TBPhase " );
      //return StatusCode::FAILURE;
    } else {
      tdcphase = theTBPhase->getPhase();
    }
  }
  else tdcphase = 0;

  //Retrieve the TBTriggerPatternUnit
  if(m_trigger) {
    const TBTriggerPatternUnit* theTBTriggerPatternUnit;
    sc = evtStore()->retrieve(theTBTriggerPatternUnit, "TBTrigPat");

    if (sc.isFailure()) {
      triggerword = 0;
      ATH_MSG_ERROR( "cannot allocate TBTriggerPatternUnit" );
      //return StatusCode::FAILURE;
    } else {
      triggerword = theTBTriggerPatternUnit->getTriggerWord();
    }

    // As a trigger can be both physics and pedestal (Indeed!)
    // we need to make two checks to cover all possibilities
    // Check whether we should reject this trigger
    if(m_reject>0) if(triggerword&m_reject) return StatusCode::SUCCESS;
    // Check whether we should accept this trigger
    if(m_accept>0) if((triggerword&m_accept)==0) return StatusCode::SUCCESS;
  }
  else triggerword=0;

  // Retrieve the pedestal
  //Pointer to conditions data objects 
  const ILArPedestal* larPedestal=NULL;
  if (m_ped) {
    sc=detStore()->retrieve(larPedestal);
    if (sc.isFailure()) {
      larPedestal=NULL;
      ATH_MSG_INFO ( "No pedestal found in database. Use default values." );
    }
  }

  // Retrieve LArDigitContainer
  const DataHandle < LArDigitContainer > digit_cont;
  if (m_contKey.size())
    ATH_CHECK( evtStore()->retrieve(digit_cont,m_contKey) );
  else
    ATH_CHECK( evtStore()->retrieve(digit_cont) );
  ATH_MSG_INFO ( "Retrieved LArDigitContainer from StoreGate! key=" << m_contKey );

  // Retrieve LArFebHeaderContainer
  const LArFebHeaderContainer *larFebHeaderContainer;
  if(m_sca) {
    ATH_CHECK( evtStore()->retrieve(larFebHeaderContainer) );
  }

  // Fill ntuple
  LArDigitContainer::const_iterator it = digit_cont->begin(); 
  LArDigitContainer::const_iterator it_e = digit_cont->end(); 
  if(it==it_e) {
    ATH_MSG_DEBUG ( "LArDigitContainer is empty..." );
  }
  for(; it!=it_e; ++it){
    const HWIdentifier hwid=(*it)->channelID();//hardwareID();
    // Fill detector geometry information
    event   = eventnumber;
    if(m_phase)   tdc     = tdcphase;
    if(m_trigger) trigger = triggerword;
    if(m_scint)   S1      = S1Adc;
    try {
      Identifier id=m_larCablingSvc->cnvToIdentifier(hwid);
      if (m_emId->is_lar_em(id)) {
	eta       = m_emId->eta(id);
	phi       = m_emId->phi(id);
	layer     = m_emId->sampling(id);
	region    = m_emId->region(id);
	detector  = 0;
      }
      else if (m_hecId->is_lar_hec(id)) {
	eta       = m_hecId->eta(id);
	phi       = m_hecId->phi(id);
	layer     = m_hecId->sampling(id);
	region    = m_hecId->region(id);
	detector  = 1;
      }
      else if (m_fcalId->is_lar_fcal(id)) {
	eta       = m_fcalId->eta(id);
	phi       = m_fcalId->phi(id);
	layer     = m_fcalId->module(id);
	region    = 0;
	detector  = 2;
      }
    }
    catch (LArID_Exception & except) {
      eta       = -1;
      phi       = -1;
      layer     = -1;
      region    = -1;
      detector  = -1;
    }
    // Fill hardware information
    barrel_ec = m_onlineHelper->barrel_ec(hwid);
    pos_neg   = m_onlineHelper->pos_neg(hwid);
    FT        = m_onlineHelper->feedthrough(hwid);
    slot      = m_onlineHelper->slot(hwid);
    channel   = m_onlineHelper->channel(hwid);

    // Fill pedestal information
    float thePedestal=-1;    
    if (larPedestal) {
      float  DBpedestal=larPedestal->pedestal(hwid,(*it)->gain());
      if (DBpedestal >= (1.0+LArElecCalib::ERRORCODE))
	thePedestal=DBpedestal;
    }
    if (thePedestal<0) {
      thePedestal = -999;
      ATH_MSG_DEBUG ( "No pedestal found for this cell. Use default value " << thePedestal );
    }
    ped = thePedestal;

    // Fill raw data samples and gain
    for(unsigned int i=0;i<(*it)->samples().size();i++) {
      if((int)i>=m_nsamples) break;
      samples[i]=(*it)->samples()[i];
    }
    gain=(*it)->gain();

    // Fill SCA numbers
    if(m_sca) {
      const HWIdentifier febid=m_onlineHelper->feb_Id(hwid);
      LArFebHeaderContainer::const_iterator feb_it=larFebHeaderContainer->begin();
      LArFebHeaderContainer::const_iterator feb_it_e=larFebHeaderContainer->end();
      for (;feb_it!=feb_it_e;feb_it++) {
	const HWIdentifier this_febid=(*feb_it)->FEBId();
	
	if(this_febid!=febid) continue;
	for(unsigned int i=0;i<(*feb_it)->SCA().size();i++) {
	  if((int)i>=m_nsamples) break;
	  sca[i]=(*feb_it)->SCA()[i];
	}
	break;
      } // End FebHeader loop
    }
    sc=ntupleSvc()->writeRecord(m_nt);
    
    if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR ( "writeRecord failed" );
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode LArDigits2Ntuple::finalize()
{
  ATH_MSG_INFO ( "LArDigits2Ntuple has finished." );
  return StatusCode::SUCCESS;
}// end finalize-method.
