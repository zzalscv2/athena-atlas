/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PileUpTools/PileUpStream.h"

#include <cassert>
#include <stdexcept>
#include <string>

#include "GaudiKernel/Service.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "AthenaKernel/CloneService.h"
#include "AthenaKernel/IAddressProvider.h"
#include "StoreGate/StoreGateSvc.h"

#include "SGTools/DataProxy.h"


#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "PileUpTools/PileUpMergeSvc.h"

class IOpaqueAddress;

/// Structors
PileUpStream::PileUpStream():
  AthMessaging ("PileUpStream"),
  m_name("INVALID"), p_svcLoc(0), p_sel(0), p_SG(0), p_iter(0), 
  p_mergeSvc(nullptr), m_ownEvtIterator(false),
  m_neverLoaded(true), m_ownStore(false),
  m_used(false), m_hasRing(false), m_iOriginalRing(0)
{ 
}

PileUpStream::PileUpStream(PileUpStream&& rhs):
  AthMessaging (rhs.m_name),
  m_name(rhs.m_name), p_svcLoc(rhs.p_svcLoc), p_sel(rhs.p_sel), 
  p_SG(rhs.p_SG), p_iter(rhs.p_iter), p_mergeSvc(rhs.p_mergeSvc), m_ownEvtIterator(rhs.m_ownEvtIterator),
  m_neverLoaded(rhs.m_neverLoaded), m_ownStore(rhs.m_ownStore),
  m_used(rhs.m_used), m_hasRing(rhs.m_hasRing), m_iOriginalRing(rhs.m_iOriginalRing)
{ 
  //transferred ownership
  rhs.m_ownEvtIterator=false;
  rhs.m_ownStore=false;
}

PileUpStream&
PileUpStream::operator=(PileUpStream&& rhs) {
  if (this != &rhs) {
    m_name=rhs.m_name;
    p_svcLoc=rhs.p_svcLoc;
    p_sel=rhs.p_sel; 
    p_SG=rhs.p_SG; 
    p_iter=rhs.p_iter;
    p_mergeSvc = rhs.p_mergeSvc;
    m_ownEvtIterator=rhs.m_ownEvtIterator;
    rhs.m_ownEvtIterator=false;
    m_neverLoaded=rhs.m_neverLoaded;
    m_ownStore=rhs.m_ownStore;
    rhs.m_ownStore=false;
    m_used=rhs.m_used;
    m_hasRing=rhs.m_hasRing;
    m_iOriginalRing=rhs.m_iOriginalRing;
  }  
  return *this;
}
 
PileUpStream::PileUpStream(const std::string& name, 
			   ISvcLocator* svcLoc,
			   IEvtSelector* sel):
  AthMessaging (name),
  m_name(name), p_svcLoc(svcLoc), p_sel(sel), p_SG(0), p_iter(0), 
  m_ownEvtIterator(false), 
  m_neverLoaded(true), m_ownStore(false),
  m_used(false), m_hasRing(false), m_iOriginalRing(0)
{ 
  assert(p_sel);
  assert(p_svcLoc);
  if( !( p_sel->createContext(p_iter).isSuccess() &&
         serviceLocator()->service("PileUpMergeSvc", p_mergeSvc, true).isSuccess() ) ) {

    const std::string errMsg("PileUpStream:: can not create stream");
    ATH_MSG_ERROR ( errMsg );
    throw std::runtime_error(errMsg);
  } else  m_ownEvtIterator=true;
}

PileUpStream::PileUpStream(const std::string& name, 
			   ISvcLocator* svcLoc,
			   const std::string& selecName):
  AthMessaging (name),
  m_name(name), p_svcLoc(svcLoc), p_sel(0), p_SG(0), p_iter(0),
  m_ownEvtIterator(false), 
  m_neverLoaded(true), m_ownStore(false),
  m_used(false), m_hasRing(false), m_iOriginalRing(0)

{
  assert(p_svcLoc);
  if (!(serviceLocator()->service(selecName, p_sel).isSuccess() &&
        serviceLocator()->service("PileUpMergeSvc", p_mergeSvc, true).isSuccess() &&
	p_sel->createContext(p_iter).isSuccess() )) {
    const std::string errMsg("PileUpStream: can not create stream");
    ATH_MSG_ERROR ( errMsg );
    throw std::runtime_error(errMsg);
  } else  m_ownEvtIterator=true;
}

PileUpStream::~PileUpStream() 
{ 
}

bool PileUpStream::setupStore() 
{
  assert( p_iter );
  assert( p_sel );
  bool rc(true);
  std::string storeName(name() + "_SG");

  //start by looking for the store directly: in overlay jobs it may already be there
  const bool DONOTCREATE(false);
  rc = (serviceLocator()->service(storeName, p_SG, DONOTCREATE)).isSuccess();
  if (rc) {
    m_ownStore = false;
  } else {
    //not there, create one cloning the master store
    Service *child;
    //if the parent store is not there barf
    //remember the clone function also initializes the service if needed
    IService* pIS(0);
    rc = ((serviceLocator()->getService("StoreGateSvc", pIS)).isSuccess() &&
          CloneService::clone(pIS, storeName, child).isSuccess() &&
          0 != (p_SG = dynamic_cast<StoreGateSvc*>(child)));
    if ( rc )  {
      m_ownStore = true;
      // further initialization of the cloned service 
      rc = (p_SG->sysInitialize()).isSuccess();      
      p_SG->setStoreID(StoreID::PILEUP_STORE); //needed by ProxyProviderSvc
    } //clones
  }
  if (rc) {
    //if the selector is an address provider like the AthenaPool one, 
    //create a dedicated ProxyProviderSvc and associate it to the store
    IAddressProvider* pIAP(dynamic_cast<IAddressProvider*>(p_sel));
    if (0 != pIAP) {
      IProxyProviderSvc* pPPSvc(0);
      std::string PPSName(name() + "_PPS");
      ISvcManager* pISM(dynamic_cast<ISvcManager*>(serviceLocator()));
      if ( 0 != pISM &&  
	   (pISM->declareSvcType(PPSName, "ProxyProviderSvc")).isSuccess() &&
	   //check the service is not there then create it
	   (serviceLocator()->service(PPSName,
				      pPPSvc,
				      true)).isSuccess()) {
	pPPSvc->addProvider(pIAP);
        IService* pSAthPoolAddProv(0);
        IAddressProvider* pAthPoolAddProv(0);
        if (serviceLocator()->service("AthenaPoolAddressProviderSvc", pSAthPoolAddProv).isSuccess() &&
            0 != (pAthPoolAddProv = dynamic_cast<IAddressProvider*>(pSAthPoolAddProv))) {
          pPPSvc->addProvider(pAthPoolAddProv);
        } else {
          ATH_MSG_WARNING ( "could not add AthenaPoolAddressProviderSvc as AddresssProvider for "<< PPSName );
        }
	IService* pSAddrRemap(0);
	IAddressProvider* pAddrRemap(0);
	if (serviceLocator()->service("AddressRemappingSvc", pSAddrRemap).isSuccess() &&
	    0 != (pAddrRemap = dynamic_cast<IAddressProvider*>(pSAddrRemap))) {
	  pPPSvc->addProvider(pAddrRemap);
	} else {
	  ATH_MSG_WARNING ( "could not add AddressRemappingSvc as AddresssProvider for "<< PPSName );
	}
	p_SG->setProxyProviderSvc(pPPSvc);
      } 
    } //valid address provider
  }
  return rc;
}

void PileUpStream::setActiveStore()
{
  store().makeCurrent();
}

StatusCode PileUpStream::nextRecordPre_Passive()
{
  this->setActiveStore();
  // Clear the store, move to next event
  return (this->store().clearStore().isSuccess() && 
	  this->selector().next(iterator()).isSuccess() ) ?
    StatusCode::SUCCESS :
    StatusCode::FAILURE;
}

StatusCode PileUpStream::nextRecordPre()
{
  // Clear the store, move to next event and load the store
  return (this->nextRecordPre_Passive().isSuccess() && 
	  this->loadStore()) ?
    StatusCode::SUCCESS :
    StatusCode::FAILURE;
}

// bool PileUpStream::isNotEmpty() const
// {
//   //  std::cout << "isNotEmpty " << (0 != iterator() && *(iterator()) != *(selector().end()) ) << std::endl;
//   return (0 != iterator() && *(iterator()) != *(selector()->end()) );
// }

bool PileUpStream::loadStore()
{
  m_neverLoaded=false;

  IOpaqueAddress* paddr(0);
  bool rc = (this->selector().createAddress(iterator(), paddr)).isSuccess();
  if ( 0 != paddr) rc &= this->store().recordAddress(paddr).isSuccess();

  // load store proxies
  rc &= this->store().loadEventProxies().isSuccess();
  return rc;
}

//return next event, load store with next event
const xAOD::EventInfo* PileUpStream::nextEventPre(bool readRecord)
{
   if (m_neverLoaded) readRecord=true;
   else if (readRecord) {
      //do not reset these the first time we call nextEventPre
      this->resetUsed();
      m_hasRing=false;
   }
   //  if (isNotEmpty()) {
   if (readRecord && this->nextRecordPre().isFailure()) {
      ATH_MSG_INFO ( "nextEventPre(): end of the loop. No more events in the selection" );
      return nullptr;
   }

   const xAOD::EventInfo* xAODEventInfo = p_mergeSvc->getPileUpEvent( p_SG, "" );
   if (readRecord and xAODEventInfo) {
      ATH_MSG_DEBUG ( "nextEventPre(): read new event " 
		      <<  xAODEventInfo->eventNumber() 
		      << " run " << xAODEventInfo->runNumber()
		      << " into store " << this->store().name() );
   }

   return xAODEventInfo;
}

bool PileUpStream::nextEventPre_Passive(bool readRecord) {
  if (m_neverLoaded) readRecord=true;
  else if (readRecord) {
    //do not reset these the first time we call nextEventPre_Passive
    this->resetUsed();
    m_hasRing=false;
  }
  if (readRecord && this->nextRecordPre_Passive().isFailure()) {
    ATH_MSG_INFO ( "nextEventPre_Passive(): end of the loop. No more events in the selection" );
    return false;
  } 
  return true;
}    

StatusCode PileUpStream::finalize() {
  StatusCode sc(StatusCode::SUCCESS);
  if (m_ownEvtIterator) delete p_iter;
  //we own and manage our cloned SG instance
#ifdef GAUDIKERNEL_STATEMACHINE_H_
  if (m_ownStore && Gaudi::StateMachine::INITIALIZED == store().FSMState()) {
    sc = this->store().sysFinalize();
  }
#else
  if (m_ownStore && this->store().state() == IService::INITIALIZED) sc = this->store().sysFinalize();
#endif
  this->store().release();
  return sc;
}
