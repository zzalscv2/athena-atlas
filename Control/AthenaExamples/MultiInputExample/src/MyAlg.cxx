/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MultiInputExample/MyAlg.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h"
#include "PileUpTools/PileUpMergeSvc.h"

#include "EventInfo/PileUpEventInfo.h"  
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "EventInfo/EventType.h"
#include "EventInfo/TagInfo.h"

#include "AthenaPoolExampleData/ExampleClass.h"
#include "AthenaPoolExampleData/ExampleHitContainer.h"

/////////////////////////////////////////////////////////////////////////////

MyAlg::MyAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  Algorithm(name, pSvcLocator),
  m_mergeSvc(0)
{

// Part 2: Properties go here


}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MyAlg::initialize(){

  // Get the messaging service, print where you are
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "initialize()" << endreq;

  /*
  // set up the SG service:
  if ( !(p_overStore.retrieve()).isSuccess() )  {
    log << MSG::ERROR 
	<< "Could not locate default store"
	<< endreq;
    return StatusCode::FAILURE;
  }
  
  */

  //locate the PileUpMergeSvc and initialize our local ptr
  const bool CREATEIF(true);
  if (!(service("PileUpMergeSvc", m_mergeSvc, CREATEIF)).isSuccess() || 
      0 == m_mergeSvc) {
    log << MSG::ERROR << "Could not find PileUpMergeSvc" << endreq;
    return StatusCode::FAILURE;
  }
  
  
  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MyAlg::execute() {

  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "execute()" << endreq;

  //Put all available ExampleClass with key "MyData" in alist
  typedef PileUpMergeSvc::TimedList<ExampleClass>::type ExampleClassList;
  ExampleClassList alist;

  if (!(m_mergeSvc->retrieveSubEvtsData("MyData", alist).isSuccess()) && alist.size()==0) {
    log << MSG::ERROR << "Could not fill ExampleClassList" << endreq;
    return StatusCode::FAILURE;
  } else {
    log << MSG::INFO << alist.size() 
	<< " ExampleClassList with key " << "MyData"
	<< " found" << endreq;
  }

  //Now alist.begin() is the first input "SimpleRootFile1.root"
  //Now alist.end() is the second (in this example only) input "SimpleRootFile2.root"
  //ExampleClassList::iterator has type pair
  ExampleClassList::iterator icls(alist.begin());
  ExampleClassList::iterator endcls(alist.end());
  int k=1;
  while (icls != endcls) {
    //Get a pointer to access 
    const ExampleClass* pcls(icls->second);
    log<<MSG::INFO<<"MyData in Input File #"<<k<<" contains getRun()="<<pcls->getRun()<<" getEvent()="
       <<pcls->getEvent()<<" getText()="
       <<pcls->getText()<<" "
       <<endreq;
    ++icls;++k;
  }


  //Put all available ExampleHitContainer with key "MyHits" in blist
  typedef PileUpMergeSvc::TimedList<ExampleHitContainer>::type ExampleHitContainerList;
  ExampleHitContainerList blist;

  if (!(m_mergeSvc->retrieveSubEvtsData("MyHits", blist).isSuccess()) && blist.size()==0) {
    log << MSG::ERROR << "Could not fill ExampleHitContainerList" << endreq;
    return StatusCode::FAILURE;
  } else {
    log << MSG::INFO << blist.size() 
	<< " ExampleHitContainerList with key " << "MyHits"
	<< " found" << endreq;
  }

  //Now blist.begin() is the first input "SimpleRootFile1.root"
  //Now blist.end() is the second (in this example only) input "SimpleRootFile2.root"
  //ExampleHitContainerList::iterator has type pair
  ExampleHitContainerList::iterator ihit(blist.begin());
  ExampleHitContainerList::iterator endhit(blist.end());
  k=1;
  while (ihit != endhit) {
    //Get a pointer to access 
    const ExampleHitContainer* cont(ihit->second);
    log<<MSG::INFO<<"MyHits in Input File #"<<k<<" contains:"<<endreq;
    
    for (ExampleHitContainer::const_iterator obj = cont->begin(); obj != cont->end(); obj++) {
      log << MSG::INFO << "Hit x = " << (*obj)->getX() << " y = " << (*obj)->getY() << " z = " << (*obj)->getZ() << " detector = " << (*obj)->getDetector() << endreq;
    }
    
    ++ihit;++k;
  }

  /*
  const PileUpEventInfo* pEvent;
  if (p_overStore->contains<PileUpEventInfo>("OverlayEvent")) p_overStore->retrieve(pEvent);
  
  
  if (pEvent) {
    // access the sub events DATA objects...
    PileUpEventInfo::SubEvent::const_iterator iEvt = pEvent->beginSubEvt();
    PileUpEventInfo::SubEvent::const_iterator endEvt = pEvent->endSubEvt();
    int i=0;


    //    while (iEvt != endEvt) {
    assert(iEvt->pSubEvtSG);
    assert(iEvt->pSubEvt);
    
    assert(endEvt->pSubEvtSG);
    assert(endEvt->pSubEvt);
    

    StatusCode sc;
    log<<MSG::INFO<<"HAHAHAHA1"<<endreq;
    if (iEvt->pSubEvtSG->contains<ExampleClass>("MyData")) {
      log<<MSG::INFO<<"HAHAHAHA2"<<endreq;
      const DataHandle<ExampleClass> ec1;
      log<<MSG::INFO<<"HAHAHAHA3"<<endreq;
      sc = iEvt->pSubEvtSG->retrieve(ec1, "MyData");
      log<<MSG::INFO<<"HAHAHAHA4"<<endreq;
      if (!sc.isSuccess()) {
	log<<MSG::INFO<<"HAHAHAHA5"<<endreq;
	log << MSG::ERROR << "Could not find ExampleClass/MyData" << endreq;
	return(StatusCode::FAILURE);
      }
      log << MSG::INFO<<"HAHAHAHA6"<<endreq;
      log << MSG::INFO << "Run = " << ec1->getRun() << " event = " << ec1->getEvent() << " :: " << ec1->getText() << endreq;
    }

    log<<MSG::INFO<<"GOGOHAHAHAHA1"<<endreq;    
    if (endEvt->pSubEvtSG->contains<ExampleClass>("MyData")) {
      log<<MSG::INFO<<"GOGOHAHAHAHA2"<<endreq;    
      const DataHandle<ExampleClass> ec2;
      log<<MSG::INFO<<"GOGOHAHAHAHA3"<<endreq;    
      sc = endEvt->pSubEvtSG->retrieve(ec2, "MyData");
      log<<MSG::INFO<<"GOGOHAHAHAHA4"<<endreq;    
      if (!sc.isSuccess()) {
	log<<MSG::INFO<<"GOGOHAHAHAHA5"<<endreq;    
	log << MSG::ERROR << "Could not find ExampleClass/MyData" << endreq;
	return(StatusCode::FAILURE);
      }
    log<<MSG::INFO<<"GOGOHAHAHAHA6"<<endreq;    
    log << MSG::INFO << "Run = " << ec2->getRun() << " event = " << ec2->getEvent() << " :: " << ec2->getText() << endreq;
    }
    
  *//*      
      log << MSG::INFO << "DUMPDUMPDUMP\t"<<i<<"\n"<<iEvt->pSubEvtSG->dump() <<endreq;

      const EventInfo * evt = 0;
      StatusCode sc = iEvt->pSubEvtSG->retrieve( evt );
      if ( sc.isFailure() ) {
	log << MSG::ERROR << "  Could not get event info" << endreq;      
	return StatusCode::FAILURE;
      }
      else {
	log << MSG::DEBUG << "Event ID: ["
	    << evt->event_ID()->run_number()   << ","
	    << evt->event_ID()->event_number() << ":"
	    << evt->event_ID()->time_stamp() << "] "
	    << endreq;
	log << MSG::DEBUG << "Event type: user type "
	    << evt->event_type()->user_type() << " weight "
	    << evt->event_type()->mc_event_weight() 
	    << endreq;
      }
      
      iEvt++;i++;
      
    }
    */
  //  }

  
  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MyAlg::finalize() {

// Part 1: Get the messaging service, print where you are
MsgStream log(msgSvc(), name());
log << MSG::INFO << "finalize()" << endreq;

return StatusCode::SUCCESS;
}
 
