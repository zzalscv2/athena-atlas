/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonEventInfoMonTool.h"
#include "MuonDQAEvent.h"

#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadHandle.h"
#include "AthenaMonitoring/LogFileMsgStream.h"


#include "AthenaMonitoring/AthenaMonManager.h"

// trigger includes:
#include "TrigT1Result/CTP_RDO.h"
#include "TrigT1Result/CTP_RIO.h"
#include "TrigT1Result/CTP_Decoder.h"


#include "TH1F.h"

namespace MuonDQA { 

  /////////////////////////////////////////////////////////////////////////////
  // *********************************************************************
  // Public Methods
  // ********************************************************************* 

  MuonEventInfoMonTool::MuonEventInfoMonTool( const std::string & type, const std::string & name, const IInterface* parent )
    : ManagedMonitorToolBase( type, name, parent ), m_eventStore(nullptr),
      m_hTriggerType(nullptr)
  {
    /*---------------------------------------------------------*/ 
    declareProperty("LastEvent",     m_lastEvent=0);
    declareProperty("TriggerTagAdd", m_TriggerTagAdd = true);
  }
  /*---------------------------------------------------------*/

  /*---------------------------------------------------------*/
  StatusCode MuonEventInfoMonTool:: initialize()
    /*---------------------------------------------------------*/
  {
    ATH_CHECK(ManagedMonitorToolBase::initialize());

    ATH_MSG_INFO( "initialize MuonEventInfoMonTool" );
 
    // The StoreGateSvc is where event-by-event information is stored.
    ATH_CHECK(service( "StoreGateSvc", m_eventStore));

    ATH_CHECK( m_eventInfoKey.initialize() );

    return StatusCode::SUCCESS;
  }
 
  /*----------------------------------------------------------------------------------*/
  StatusCode MuonEventInfoMonTool::bookHistograms()
    /*----------------------------------------------------------------------------------*/
  {
  
    ATH_MSG_DEBUG( "MuonEventInfoMonTool Histograms being filled" );
    StatusCode sc = StatusCode::SUCCESS;

    std::string generic_path_muonmonitoring = "Muon/MuonEventInfo";
   
    //declare a group of histograms
    MonGroup muonevt_shift( this, generic_path_muonmonitoring, run, ATTRIB_MANAGED );
      
    if(newEventsBlockFlag()){}
    if(newLumiBlockFlag()){}
    if(newRunFlag())
      {      
	ATH_MSG_DEBUG( "MuonEventInfoMonTool : isNewRun" );
        
	// Trigger types

 	m_hTriggerType = new TH1F("TriggerType","Number_of_Events_per_L1_TriggerType(8 bits)",256, -0.5, 255.5);
        m_hTriggerType->SetFillColor(42);

     
	m_hTriggerType->GetXaxis()->SetTitle("L1 trigger word");
	m_hTriggerType->GetYaxis()->SetTitle("Number of Events");        
	

	sc=muonevt_shift.regHist(m_hTriggerType);
	if(sc.isFailure()){
	  ATH_MSG_FATAL( "m_hTriggerType Failed to register histogram " );
	  return StatusCode::FAILURE;	
	}
     
	ATH_MSG_DEBUG( "exiting bookHistograms for trigger type " << m_hTriggerType << " end of run : " << run );
	
      }// isEndOfRun
 
    return sc;

  
  }

  /*----------------------------------------------------------------------------------*/
  StatusCode MuonEventInfoMonTool::fillHistograms()
    /*----------------------------------------------------------------------------------*/ 
  {
 
    StatusCode sc = StatusCode::SUCCESS;
   
    ATH_MSG_DEBUG( "MuonEventInfoMonTool::EventInfo Monitoring Histograms being filled" );

    //Retrieve all ingredients needed to build an MuonDQAMonitoringEvent
    MuonDQA::MuonDQAEventInfo eventInfo = retrieveEventInfo();
    return sc;
  }
  /*---------------------------------------------------------*/
  MuonDQA::MuonDQAEventInfo MuonEventInfoMonTool::retrieveEventInfo()
  { 
  
    StatusCode sc = StatusCode::SUCCESS;  
    ATH_MSG_VERBOSE( "MuonEventInfoMonTool::retrieveEventInfo() called" );


    MuonDQAEventInfo MuonDQAeventInfo;

    SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey);
 
    //Cast eventID into MuonDQAEventInfo class:
    
    MuonDQAeventInfo.setRunNumber( eventInfo->runNumber() ) ;
    MuonDQAeventInfo.setEventNumber( eventInfo->eventNumber() );
    MuonDQAeventInfo.setTimeStamp( eventInfo->timeStamp() );
    // Number of days since 1/1/1970 
    MuonDQAeventInfo.setOffset( eventInfo->timeStamp()/(24*3600));

    MuonDQAeventInfo.setTrigType(eventInfo->level1TriggerType());
    
    // Get time of the day for the event and convert from seconds to hours
    MuonDQAeventInfo.setRunTime( float( eventInfo->timeStamp() - ( ((eventInfo->timeStamp()/(24*3600))*24*3600)/3600. ) ) );         
    MuonDQAeventInfo.setLumiBlock(eventInfo->lumiBlock() );  
     
    std::string eventTag=m_eventTag;
    MuonDQAeventInfo.setTag( eventTag );
    
    ATH_MSG_DEBUG( "MuonDQAeventInfo" << MuonDQAeventInfo );
 
    uint l1Trig = (uint) ( eventInfo->level1TriggerType() );
    m_hTriggerType->Fill(l1Trig);

    // Get number of events per Trigger type : 0001 Tile | 0010 RPC | 0100 TGC | 1000 CTP
    //for(int idx = 0; idx < 0; idx++) if(l1Trig.test(idx)) m_hTriggerType->Fill(idx);

    // Fill CTP bin for exclusive CTP events only
    //if( l1Trig == 8) m_hTriggerType->Fill(3);

    if ( m_TriggerTagAdd ) {

      if(!m_eventStore->contains<CTP_RDO>("CTP_RDO") || ! m_eventStore->contains<CTP_RIO>("CTP_RIO"))
	{
	  return MuonDQAeventInfo;
	}

      const CTP_RDO* ctpRDO = nullptr;
      sc = m_eventStore->retrieve( ctpRDO, "CTP_RDO" );
      if ( sc.isFailure() ) {
	ATH_MSG_WARNING( "CTP_RDO trigger info missing, not added to EventTag" );
	return MuonDQAeventInfo;
      }

      const CTP_RIO* ctpRIO = nullptr;
      sc = m_eventStore->retrieve( ctpRIO, "CTP_RIO" );
      if ( sc.isFailure() ) {
	ATH_MSG_WARNING( "CTP_RIO trigger info missing, not added to EventTag" );
	return MuonDQAeventInfo;
      }

      CTP_Decoder ctp;
      ctp.setRDO(ctpRDO);
      const std::vector<CTP_BC>& BCs = ctp.getBunchCrossings();
      
      // now get the data
      //uint16_t l1aPos = ctpRIO->getDetectorEventType() >> 16;
      unsigned int storeBunch = ctpRDO->getL1AcceptBunchPosition();
      
      //const CTP_BC& bunch = ctp.getBunchCrossings().at(l1aPos);
      const CTP_BC& bunch = BCs.at(storeBunch);
      
      MuonDQAeventInfo.setNumberOfTriggerBits(bunch.getTAV().size() + bunch.getTAP().size());
      int nth_bit(0);
      for(unsigned int i=0; i<bunch.getTAV().size(); i++)
	{
	  MuonDQAeventInfo.setTriggerBit(nth_bit, bunch.getTAV().test(i));
	  nth_bit++;
	}
      for(unsigned int i=0; i<bunch.getTAP().size(); i++)
	{
	  MuonDQAeventInfo.setTriggerBit(nth_bit, bunch.getTAP().test(i));
	  nth_bit++;
	}

    }

    return MuonDQAeventInfo;  
    
  }

  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

  StatusCode MuonEventInfoMonTool::procHistograms()
  {
    StatusCode sc = StatusCode::SUCCESS;
 
    ATH_MSG_DEBUG(  "MuonEventInfoMonTool procHist()" );
   
    if(endOfEventsBlockFlag()){}
    if(endOfLumiBlockFlag()){}
    if(endOfRunFlag()){ 
    } // isEndOfRun
 
    return sc;
  } 
  const MuonDQAEvent* MuonEventInfoMonTool::retrieveEvent() {
     
    //Retrieve all ingredients needed to build an MuonDQAEvent
    MuonDQAEventInfo eventInfo = retrieveEventInfo();
    MuonDQAEvent* event = new MuonDQAEvent();
    event->setMuonDQAEventInfo( eventInfo );
    return event;
  }
}//end namespace MuonDQA
