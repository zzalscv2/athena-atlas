/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


// Class header
#include "PileUpEventLoopMgr.h"

// Athena includes
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "AthenaKernel/EventContextClid.h"
#include "AthenaKernel/IEvtIdModifierSvc.h"

#include "EventInfo/EventID.h"         // OLD EDM
#include "EventInfo/EventType.h"       // OLD EDM
#include "EventInfo/EventInfo.h"       // OLD EDM

#include "PileUpTools/IBeamIntensity.h"
#include "PileUpTools/IBeamLuminosity.h"
#include "PileUpTools/PileUpMergeSvc.h"
#include "PileUpTools/IBkgStreamsCache.h"
#include "PileUpTools/PileUpMisc.h"
#include "PileUpTools/PileUpHashHelper.h"

#include "StoreGate/StoreGateSvc.h"

#include "xAODEventInfo/EventInfo.h"             // NEW EDM
#include "xAODEventInfo/EventAuxInfo.h"          // NEW EDM
#include "xAODEventInfo/EventInfoContainer.h"    // NEW EDM
#include "xAODEventInfo/EventInfoAuxContainer.h" // NEW EDM

// xAOD include(s):
#include "EventInfoUtils/EventInfoFromxAOD.h"

// Gaudi headers
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/DataIncident.h" // For ContextIncident
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "GaudiKernel/Algorithm.h"

// std library headers
#include <cmath>

//=========================================================================
// Standard Constructor
//=========================================================================
PileUpEventLoopMgr::PileUpEventLoopMgr(const std::string& name,
                                       ISvcLocator* svcLoc)
  : MinimalEventLoopMgr(name, svcLoc)
  , AthMessaging (name)
  , m_incidentSvc("IncidentSvc", name) //FIXME should this be configurable?
  , m_mergeSvc("PileUpMergeSvc", name)
  , m_evtStore("StoreGateSvc/StoreGateSvc",  name)
  , m_evtIdModSvc("", name)
  , m_origSel("EventSelector", name)
  , m_signalSel("", name)
  , m_caches(this)
  , m_maxCollPerXing(23.0)
  , m_xingFreq(25)
  , m_firstXing(-2)
  , m_lastXing(+1)
  , m_allowSubEvtsEOF(true)
  , m_xingByXing(false)
  , m_failureMode(1)
  , m_evinfName( c_pileUpEventInfoObjName )
  , m_evinfContName( c_pileUpEventInfoContName )
  , m_beamInt("FlatBM", name)
  , m_beamLumi("LumiProfileSvc", name)
  , m_currentRun(0)
  , m_firstRun(true)
  , m_maxBunchCrossingPerOrbit(3564)
  , m_nevt(0)
  , m_ncurevt(0)
  , m_skipExecAlgs(false)
  , m_loadProxies(true)
  , m_allowSerialAndMPToDiffer(true)
{
  declareProperty("MaxBunchCrossingPerOrbit", m_maxBunchCrossingPerOrbit, "The number of slots in each LHC beam. Default: 3564.");
  declareProperty("OrigSelector", m_origSel, "EventSelector for original (physics) events stream" );
  declareProperty("SignalSelector", m_signalSel, "EventSelector for signal (hard-scatter) events stream" );
  declareProperty("XingFrequency", m_xingFreq, "ns");
  declareProperty("firstXing", m_firstXing, "time of first xing / XingFrequency (0th xing is 1st after trigger)");
  declareProperty("lastXing", m_lastXing, "time of last xing / XingFrequency (0th xing is 1st after trigger)");
  declareProperty("MaxMinBiasCollPerXing", m_maxCollPerXing, "Set to digitization numberOfCollisions prop. for variable-mu and RunDMC jobs.");
  declareProperty("bkgCaches", m_caches, "list of tools managing bkg events");
  declareProperty("AllowSubEvtsEOF", m_allowSubEvtsEOF, "if true(default) an EOF condition in the BkgStreamsCaches is not considered to be an error IF maxevt=-1 (loop over all available events)");
  declareProperty("XingByXing", m_xingByXing, "if set to true we will not cache bkg events from one xing to then next. This greatly increases the amount of I/O and greatly reduces the memory required to run a job");
  declareProperty("FailureMode", m_failureMode,
                  "Controls behaviour of event loop depending on return code of"
                  " Algorithms. 0: all non-SUCCESSes terminate job. "
                  "1: RECOVERABLE skips to next event, FAILURE terminates job "
                  "(DEFAULT). 2: RECOVERABLE and FAILURE skip to next events");

  declareProperty("BeamInt", m_beamInt,
                  "The service providing the beam intensity distribution");
  declareProperty("BeamLuminosity", m_beamLumi,
                  "The service providing the beam luminosity distribution vs. run");
  declareProperty("PileUpMergeSvc", m_mergeSvc, "PileUp Merge Service");
  declareProperty("EvtIdModifierSvc", m_evtIdModSvc, "ServiceHandle for EvtIdModifierSvc");
  declareProperty("AllowSerialAndMPToDiffer", m_allowSerialAndMPToDiffer, "When set to False, this will allow the code to reproduce serial output in an AthenaMP job, albeit with a significant performance penalty.");
  declareProperty("EventInfoName", m_evinfName, "SG key for the EventInfo object");
  declareProperty("EventInfoContName", m_evinfContName, "SG key for the EventInfoContainer object");
  //  m_caches.push_back("BkgStreamsCache/MinBiasCache");
}

//=========================================================================
// Standard Destructor
//=========================================================================
PileUpEventLoopMgr::~PileUpEventLoopMgr() {}

//=========================================================================
// implementation of IAppMgrUI::initialize
//=========================================================================
StatusCode PileUpEventLoopMgr::initialize()
{
  ATH_MSG_INFO ( "Initializing " << this->name() ) ;
  if (!m_allowSerialAndMPToDiffer) {
    ATH_MSG_WARNING ( "AllowSerialAndMPToDiffer=False! This will incur serious performance penalties! But Serial and MP output will be the same." );
  }

  //locate the StoreGateSvc and initialize our local ptr
  CHECK(m_evtStore.retrieve());

  //reset output store proxy provider: we don't want to have one!
  m_evtStore->setProxyProviderSvc(nullptr);

  //locate the IncidentSvc and initialize our local ptr
  CHECK(m_incidentSvc.retrieve());

  //locate the PileUpMergeSvc and initialize our local ptr
  CHECK(m_mergeSvc.retrieve());

  //-------------------------------------------------------------------------
  // Setup Event Selector
  //-------------------------------------------------------------------------
  CHECK(this->setupStreams());

  // Get the value of SkipEvents. It is needed for seeking
  SmartIF<IProperty> prpMgr(serviceLocator());
  IEvtSelector* evtSelector{};
  if (prpMgr.isValid()) {
    // Get event selector name. Retrieve EventSelector
    std::string evtSelName = prpMgr->getProperty("EvtSel").toString();
    CHECK(serviceLocator()->service(evtSelName,evtSelector));
  }
  else {
    ATH_MSG_ERROR ( "IProperty interface not found in ApplicationMgr" );
    return StatusCode::FAILURE;
  }

  SmartIF<IProperty> prpMgr1(evtSelector);
  if (prpMgr1.isValid()) {
    std::string skipEventsStr = prpMgr1->getProperty("SkipEvents").toString();
    m_ncurevt = std::atoi(skipEventsStr.c_str());
  }
  else {
    ATH_MSG_ERROR ( "IProperty interface not found on the event selector" );
    return StatusCode::FAILURE;
  }

  // Get the AlgExecStateSvc
  m_aess = serviceLocator()->service("AlgExecStateSvc");
  if (!m_aess.isValid()) {
    ATH_MSG_FATAL ( "Error retrieving AlgExecStateSvc" );
    return StatusCode::FAILURE;
  }

  //--------------------------------------------------------------------------
  // Set up the EventID modifier Service
  //--------------------------------------------------------------------------
  if( m_evtIdModSvc.empty() ) {
    ATH_MSG_DEBUG ( "EventID modifier Service not set. No run number, ... overrides will be applied." );
  }
  else if ( !m_evtIdModSvc.retrieve().isSuccess() ) {
    ATH_MSG_INFO ( "Could not find EventID modifier Service. No run number, ... overrides will be applied." );
  }

  //-------------------------------------------------------------------------
  // Base class initialize (done at the end to allow algs to access stores)
  //-------------------------------------------------------------------------

  return MinimalEventLoopMgr::initialize();
}

//=========================================================================
// implementation of IAppMgrUI::finalize
//=========================================================================
StatusCode PileUpEventLoopMgr::finalize()
{
  ATH_MSG_INFO ( "Finalizing " << this->name() );

  //we need to release all our BkgStreamCaches
  ToolHandleArray<IBkgStreamsCache>::iterator cacheIterator(m_caches.begin());
  ToolHandleArray<IBkgStreamsCache>::iterator endOfCaches(m_caches.end());
  while (cacheIterator != endOfCaches) {
    (*cacheIterator++)->release();
  }
  m_caches.clear();

  //and to clean up the store the stream owns
  CHECK(m_origStream.finalize());

  return MinimalEventLoopMgr::finalize();
}

//=========================================================================
// implementation of IAppMgrUI::terminate
//=========================================================================
//  StatusCode PileUpEventLoopMgr::terminate()
//  {
//     m_histoDataMgrSvc = releaseInterface(m_histoDataMgrSvc);
//     m_histoPersSvc    = releaseInterface(m_histoPersSvc);
//     return MinimalEventLoopMgr::terminate();
//  }


//---------------------------------------------------------------------------
void PileUpEventLoopMgr::modifyEventContext(EventContext& ctx, const EventID& eID, bool consume_modifier_stream) {

  if (m_evtIdModSvc.isSet()) {
    EventID* new_eID=new EventID(eID);
    // interface to m_evtIdModSvc->modify_evtid wants to be able to
    // update the pointer itself, but in reality function doesn't need
    // it. And cannot obviously use a smart pointer here, as the
    // pointer itself can get updated by modify_evtid, so plain
    // pointer for now.
    // CHECK: Update evtIdModSvc method modify_evtid interface to
    // pointer or reference?
    // m_nevt - 1 because the PileUpEventLoopMgr increments it rather early
    m_evtIdModSvc->modify_evtid(new_eID, m_ncurevt - 1, consume_modifier_stream);
    if (msgLevel(MSG::DEBUG)) {
      unsigned int oldrunnr=eID.run_number();
      unsigned int oldLB=eID.lumi_block();
      unsigned int oldTS=eID.time_stamp();
      unsigned int oldTSno=eID.time_stamp_ns_offset();
      ATH_MSG_DEBUG ( "modifyEventContext: use evtIdModSvc runnr=" << oldrunnr << " -> " << new_eID->run_number() );
      ATH_MSG_DEBUG ( "modifyEventContext: use evtIdModSvc LB=" << oldLB << " -> " << new_eID->lumi_block() );
      ATH_MSG_DEBUG ( "modifyEventContext: use evtIdModSvc TimeStamp=" << oldTS << " -> " << new_eID->time_stamp() );
      ATH_MSG_DEBUG ( "modifyEventContext: use evtIdModSvc TimeStamp ns Offset=" << oldTSno << " -> " << new_eID->time_stamp_ns_offset() );
    }
    ctx.setEventID( *new_eID );
    delete new_eID;
    return;
  }

  ctx.setEventID( eID );
}


//=========================================================================
// implementation of IAppMgrUI::nextEvent
//=========================================================================
StatusCode PileUpEventLoopMgr::nextEvent(int maxevt)
{
  // make nextEvent(0) a dummy call, as needed by athenaMP
  if (0 == maxevt) {
    return StatusCode::SUCCESS;
  }


  // These following two initialization loops are performed here in case new
  // Top level Algorithms or Output Streams have been created interactively
  // at run-time instead of configuration time. Note also that it is possible
  // that some newly created Algorithms are still not initialized as a result
  // of these loops (e.g. where the membership of an existing Sequencer is
  // changed at run-time. In this case, the Algorithm::sysExecute() function
  // will ensure that the Algorithm is correctly initialized. This mechanism
  // actually makes loops redundant, but they do  provide a well defined
  // location for the initialization to take place in the non-interactive case.

  // Initialize the list of Algorithms. Note that existing Algorithms
  // are protected against multiple initialization attempts.
  ListAlg::iterator ita;
  for ( ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ++ita ) {
    if( !((*ita)->sysInitialize()).isSuccess() ) {
      ATH_MSG_ERROR ( "Unable to initialize Algorithm: " <<
                      (*ita)->name() );
      return StatusCode::FAILURE;
    }
  }

  // Initialize the list of Output Streams. Note that existing Output Streams
  // are protected against multiple initialization attempts.
  for (ita = m_outStreamList.begin(); ita != m_outStreamList.end(); ++ita ) {
    if( !((*ita)->sysInitialize()).isSuccess() ) {
      ATH_MSG_ERROR ( "Unable to initialize Output Stream: " <<
                      (*ita)->name() );
      return StatusCode::FAILURE;
    }
  }

  const xAOD::EventInfo *inputEventInfo{};

  // loop over events if the maxevt (received as input) is different from -1.
  // if evtmax is -1 it means infinite loop
  while( (maxevt == -1 || m_nevt < maxevt) &&
         0 != (inputEventInfo = m_origStream.nextEventPre()) ) {
    // Check if there is a scheduled stop issued by some algorithm/sevice
    if ( m_scheduledStop ) {
      m_scheduledStop = false;
      ATH_MSG_ALWAYS ( "A stopRun was requested. Terminating event loop." );
      break;
    }
    ++m_nevt; ++m_ncurevt;

    //-----------------------------------------------------------------------
    // Setup overlaid event in the event store
    //-----------------------------------------------------------------------

    xAOD::EventInfo *pOverEvent = new xAOD::EventInfo();
    xAOD::EventAuxInfo *pOverEventAux = new xAOD::EventAuxInfo();
    pOverEvent->setStore( pOverEventAux );
    ATH_MSG_DEBUG(" #subevents in the signal event =" << inputEventInfo->subEvents().size());

    // Copy the eventInfo data from origStream event
    *pOverEvent = *inputEventInfo;
    pOverEvent->clearSubEvents();  // start clean without any subevents

    // Propagate MC metadata
    if (pOverEvent->mcChannelNumber() == 0) {
      if (m_mcChannelNumber.value() != 0) {
        ATH_MSG_WARNING("Input mcChannelNumber is 0, setting it to " << m_mcChannelNumber.value());
        pOverEvent->setMCChannelNumber(m_mcChannelNumber.value());
      } else {
        ATH_MSG_WARNING("Input mcChannelNumber is 0");
      }
    } else {
      if (m_mcChannelNumber.value() != 0 && pOverEvent->mcChannelNumber() != m_mcChannelNumber.value()) {
        ATH_MSG_WARNING("Input mcChannelNumber (" << pOverEvent->mcChannelNumber()
                        << ") and provided mcChannelNumber (" << m_mcChannelNumber.value() << ") do not match.");
      }
    }

    if (pOverEvent->mcEventNumber() == 0) {
      if (pOverEvent->eventNumber() != 0) {
        ATH_MSG_WARNING("Input mcEventNumber is 0, setting it to match the eventNumber (" << pOverEvent->eventNumber() << ")");
        pOverEvent->setMCEventNumber(pOverEvent->eventNumber());
      } else {
        ATH_MSG_ERROR("Input eventNumber and mcEventNumber are 0");
        return StatusCode::FAILURE;
      }
    }

    if (pOverEvent->mcEventWeights().empty()) {
      ATH_MSG_ERROR("Input mcEventWeights are empty. This should not happen.");
      return StatusCode::FAILURE;
    }

    // Record the xAOD object(s):
    CHECK( m_evtStore->record( pOverEventAux, m_evinfName + "Aux." ) );
    CHECK( m_evtStore->record( pOverEvent, m_evinfName ) );
    pOverEvent->setEvtStore( &*m_evtStore );
    // Create an EventInfoContainer for the pileup events:
    xAOD::EventInfoContainer *puei(new xAOD::EventInfoContainer());
    xAOD::EventInfoAuxContainer *puaux(new xAOD::EventInfoAuxContainer());
    puei->setStore( puaux );

    // Record the EI container object(s):
    CHECK( m_evtStore->record( puei, m_evinfContName ) );
    CHECK( m_evtStore->record( puaux, m_evinfContName+"Aux." ) );

    // Setup the EventContext.
    // Ensure that the overridden run number etc. is being used
    EventContext ctx;
    EventID   ev_id = eventIDFromxAOD( pOverEvent );
    bool consume_modifier_stream = false; // FIXME/CHECK: was true inside TP converter and checks for active storegate
    // kludge FIXME
    const AthenaAttributeList* pAttrList = m_origStream.store().tryConstRetrieve<AthenaAttributeList>("Input");
    if ( pAttrList != nullptr && pAttrList->size() > 6 ) {
      // Ideally we would create the initial EventID from the TAG
      // information, but not doing that (yet?).  For now using the
      // presence of TAG information in the HITS file to indicate that
      // the EvtIdModifierSvc has not already been called for this
      // Athena event.
      consume_modifier_stream = true;
    }
    ctx.set(m_nevt,0);
    modifyEventContext(ctx, ev_id, consume_modifier_stream);
    /// Is this correct, or should it be set to a pileup store?
    ctx.setExtension( Atlas::ExtendedEventContext(m_evtStore->hiveProxyDict(),
                                                  ctx.eventID().run_number()) );
    Gaudi::Hive::setCurrentContext( ctx );
    m_aess->reset(ctx);
    // make copy to std::move into store
    auto puctx =std::make_unique<EventContext> ( ctx );
    if (m_evtStore->record( std::move(puctx) ,"EventContext").isFailure()) {
      ATH_MSG_ERROR ( "Error recording event context object" );
      return StatusCode::FAILURE;
    }

    //Update run number, event number, lumi block and timestamp from EventContext
    pOverEvent->setRunNumber( ctx.eventID().run_number() );
    pOverEvent->setEventNumber( ctx.eventID().event_number() );
    pOverEvent->setLumiBlock( ctx.eventID().lumi_block() );
    pOverEvent->setTimeStamp( ctx.eventID().time_stamp() );
    ATH_MSG_DEBUG( "Updated pOverEvent " << *pOverEvent );

    ATH_MSG_INFO ( "nextEvent(): overlaying original event " <<
                   pOverEvent->runNumber() << '/' <<
                   pOverEvent->eventNumber() << '/' <<
                   pOverEvent->lumiBlock() );

    //ask the BeamIntensitySvc to choose (and remember)
    //in which xing this event will be wrto the beam int distribution
    m_beamInt->selectT0(ctx.eventID().run_number(), ctx.eventID().event_number());

    // Use the position in the beam intensity array to set a BCID-like quantity
    pOverEvent->setBCID( m_beamInt->getCurrentT0BunchCrossing() );

    unsigned int t0BCID = pOverEvent->bcid();
    ATH_MSG_VERBOSE ( "BCID =" << t0BCID );

    // pOvrEt->set_user_type("Overlaid"); //FIXME    ?? MN: ?? what to do with that

    // Overlay RDO files should be treated like data for reco
    // purposes, so only set this for SimHit level pile-up.
    pOverEvent->setEventTypeBitmask( inputEventInfo->eventTypeBitmask() | xAOD::EventInfo::IS_SIMULATION );

    //  register as sub event of the overlaid
    bool addpEvent(true);

    ATH_MSG_DEBUG( "inputEventInfo->evtStore()="<<inputEventInfo->evtStore()<<",  pOverEvent->evtStore()=" << pOverEvent->evtStore()<<", &m_origStream.store()="<<&m_origStream.store() );
    if ( addpEvent ) {
      xAOD::EventInfo* newEv=addSubEvent(pOverEvent, inputEventInfo, 0, xAOD::EventInfo::Signal, puei, m_evinfContName );
      if(newEv->evtStore()==nullptr) newEv->setEvtStore(&m_origStream.store());
      newEv->setRunNumber( ctx.eventID().run_number() );
      newEv->setEventNumber( ctx.eventID().event_number() );
      newEv->setLumiBlock( ctx.eventID().lumi_block() );
      newEv->setTimeStamp( ctx.eventID().time_stamp() );
      ATH_MSG_DEBUG( "Added inputEventInfo="<<inputEventInfo<<" as subEvent "<<newEv<<" to pOverEvent " << pOverEvent );
      ATH_MSG_DEBUG( "  afterwards: newEv->evtStore()="<<newEv->evtStore()<<",  pOverEvent->evtStore()=" << pOverEvent->evtStore() );
      ATH_MSG_DEBUG( "  pOverEvent->subEvents()[0].ptr()="<<pOverEvent->subEvents()[0].ptr()<<" content="<<*pOverEvent->subEvents()[0].ptr()<<" store="<< pOverEvent->subEvents()[0].ptr()->evtStore() );
    }

    ATH_MSG_INFO ( "set aliases" );
    //add an alias to "OverlayEvent" (backward compatibility)
    CHECK(m_evtStore->setAlias(pOverEvent, "OverlayEvent"));

    //FIXME at this point one may want to look into the original event
    //FIXME to decide whether to skip it or to do the pile-up

    ///ask the BeamLuminositySvc to check for a new scalefactor
    bool needupdate;
    float sf = m_beamLumi->scaleFactor( pOverEvent->runNumber(), pOverEvent->lumiBlock(), needupdate );
    float currentaveragemu(sf*m_maxCollPerXing);
    pOverEvent->setAverageInteractionsPerCrossing(currentaveragemu);
    //FIXME check whether actualInteractionsPerCrossing should be set
    //to mu for the central bunch crossing, as below, or whether it
    //should be set to the actual number of minbias piled-up in the
    //central bunch crossing for this particular event.
    pOverEvent->setActualInteractionsPerCrossing(m_beamInt->normFactor(0)*currentaveragemu);
    ATH_MSG_DEBUG ( "BCID = "<< t0BCID <<
                    ", mu =" << pOverEvent->actualInteractionsPerCrossing() <<
                    ", <mu> =" << pOverEvent->averageInteractionsPerCrossing() );
    ToolHandleArray<IBkgStreamsCache>::iterator cacheIterator(m_caches.begin());
    ToolHandleArray<IBkgStreamsCache>::iterator endOfCaches(m_caches.end());
    while (cacheIterator != endOfCaches) {
      if (needupdate) {
        (*cacheIterator)->resetEvtsPerXingScaleFactor(sf);//set cache scale factor. some caches should ignore it?
      }
      (*cacheIterator++)->newEvent(); //inform cache we overlay a new event
    }
    //-----------------------------------------------------------------------
    // loop over x-ings
    //-----------------------------------------------------------------------
    for (int iXing=m_firstXing; iXing<=m_lastXing; iXing++) {
      // if (m_xingByXing.value()) test that PileUpToolsAlg is there?
      //-----------------------------------------------------------------------
      // Read input events in bkg stores and link them to overlay store
      //-----------------------------------------------------------------------
      int t0BinCenter((int)m_xingFreq*iXing);
      unsigned int currentBCID = getBCID((t0BinCenter/25), t0BCID); //Should account for m_xingFreq>25. FIXME - hardcoded 25ns
      ATH_MSG_VERBOSE ( "Background BCID =" << currentBCID );
      cacheIterator = m_caches.begin();
      while (cacheIterator != endOfCaches) {
        // Now set the BCID for background events - also requires changes to PileUpEventInfo, SubEvent
        if (((*cacheIterator)->addSubEvts(iXing-m_firstXing, pOverEvent, t0BinCenter, m_loadProxies, currentBCID)).isFailure()) {
          //if (((*cacheIterator)->addSubEvts(iXing-m_firstXing, *pOverEvent, t0BinCenter)).isFailure()) {
          if (maxevt == -1 && m_allowSubEvtsEOF) {
            ATH_MSG_INFO ( "No more sub events for " << cacheIterator->name() );
            return StatusCode::SUCCESS;
          }
          else {
            ATH_MSG_FATAL ( "Error adding sub events to " << cacheIterator->name() );
            return StatusCode::FAILURE;
          }
        }
        ++cacheIterator;
      }
    }  //loop over xings

    // Calculate/copy pile-up hash
    PileUpHashHelper pileUpHashHelper;

    pileUpHashHelper.addToHashSource(pOverEvent);

    ATH_MSG_VERBOSE ( "Pile-up hash source:" << pileUpHashHelper.hashSource() );

    // Calculate and set hash
    uuid_t pileUpHash;
    pileUpHashHelper.calculateHash(pileUpHash);

    pOverEvent->setPileUpMixtureID( PileUpHashHelper::uuidToPileUpMixtureId(pileUpHash) );

    ATH_MSG_DEBUG("PileUpMixtureID = " << pOverEvent->pileUpMixtureID());

    //set active store back to the overlaid one
    m_evtStore->makeCurrent();

    // Execute event for all required algorithms. The overlaid event
    // (PileUpEventInfo) is considered to be current
    if (m_skipExecAlgs) {
      ATH_MSG_INFO ( "Firing reseed incident (" << pOverEvent->eventNumber() << "," << pOverEvent->runNumber() << ")" );
      m_incidentSvc->fireIncident(ContextIncident<std::pair<unsigned,unsigned> >(this->name(),"ReseedIncident",std::pair<unsigned,unsigned>(pOverEvent->eventNumber(),pOverEvent->runNumber())));
      m_incidentSvc->fireIncident(Incident(this->name(),"AfterReseedIncident"));
      ATH_MSG_INFO ( "Not going to process this event" );
    }
    else {
      if ( !(executeEvent( std::move(ctx) ).isSuccess()) ) {
        ATH_MSG_ERROR ( "Terminating event processing loop due to errors" );
        return StatusCode::FAILURE;
      }
      else {
        ATH_MSG_INFO ( "Event processing successful." );
      }
    }
    //if we are not doing overlap xing by xing clean up all stores at the end
    //the clean up is done object by object and controlled by
    //PileUpXingFolder.CacheRefreshFrequency property
    if (!m_xingByXing.value()) {
      // FIXME test that PileUpToolsAlg is NOT there?
      //clear all stores as configured in
      CHECK(m_mergeSvc->clearDataCaches());
      ATH_MSG_DEBUG ( "Cleared data caches" );
    }
    CHECK(m_evtStore->clearStore());
  }  //end of event loop

  if (m_skipExecAlgs) {
    m_nevt--;
  }

  return StatusCode::SUCCESS;

}

StatusCode PileUpEventLoopMgr::seek(int evt)
{
  int nevtsToAdvance = evt-m_ncurevt;
  int nParam = m_nevt+1;
  ATH_MSG_INFO ( "In PileUpEventLoopMgr::seek. m_ncurevt=" << m_ncurevt <<
                 " nevtsToAdvance=" << nevtsToAdvance );

  m_skipExecAlgs = true;
  for (int i=0; i<nevtsToAdvance; ++i) {
    if (m_allowSerialAndMPToDiffer) {
      m_loadProxies = (nevtsToAdvance==(i+1));
    }
    if (this->nextEvent(nParam).isFailure()) {
      ATH_MSG_ERROR ( "Seek error! Failed to advance to evt=" << m_ncurevt+1 );
      return StatusCode::FAILURE;
    }
  }
  m_skipExecAlgs = false;
  return StatusCode::SUCCESS;
}

int PileUpEventLoopMgr::curEvent() const
{
  return m_nevt;
}

//=========================================================================
// setup input and overlay selctors and iterators
//=========================================================================
StatusCode
PileUpEventLoopMgr::setupStreams()
{
  //OK now we can run
  // retrieve selector and setup iterator (the selector has been setup as an ExtService)
  CHECK(m_origSel.retrieve());

  m_origStream = PileUpStream("OriginalEvent", serviceLocator(), &*m_origSel);
  if (!m_origStream.setupStore()) {
    ATH_MSG_ERROR ( "Can not setup original evt store " << m_origSel );
    return StatusCode::FAILURE;
  }

  //now get the bkg stream caches, and set them up
  CHECK(m_caches.retrieve());
  ATH_MSG_DEBUG ( "retrieved BkgStreamsCaches " << m_caches );

  ToolHandleArray<IBkgStreamsCache>::iterator cacheIterator(m_caches.begin());
  ToolHandleArray<IBkgStreamsCache>::iterator endOfCaches(m_caches.end());
  unsigned int nXings(m_lastXing-m_firstXing+1);
  unsigned int firstStore(0);
  StatusCode sc(StatusCode::SUCCESS);
  while ((cacheIterator != endOfCaches) && sc.isSuccess()) {
    sc = (*cacheIterator)->setup(m_firstXing, nXings, firstStore, &*m_beamInt);
    firstStore += (*cacheIterator)->nStores() ;
    ++cacheIterator;
  }
  return sc;
}

//=========================================================================
// Run the algorithms for the current event
//=========================================================================
StatusCode PileUpEventLoopMgr::executeAlgorithms(const EventContext& ctx)
{
  // Call the execute() method of all top algorithms
  for ( ListAlg::iterator ita = m_topAlgList.begin();
        ita != m_topAlgList.end();
        ++ita ) {
    StatusCode sc = (*ita)->sysExecute( ctx );
    // this duplicates what is already done in Algorithm::sysExecute, which
    // calls Algorithm::setExecuted, but eventually we plan to remove that
    // function
    m_aess->algExecState(*ita, ctx ).setState(AlgExecState::State::Done, sc);
    if ( !sc.isSuccess() ) {
      ATH_MSG_INFO ( "Execution of algorithm " <<
                     (*ita)->name() << " failed with StatusCode::" << sc );
      return sc;
    }
  }

  return StatusCode::SUCCESS;
}


//=========================================================================
// executeEvent(EventContext&& ctx)
//=========================================================================
StatusCode PileUpEventLoopMgr::executeEvent( EventContext&& ctx )
{
  /// Fire begin-Run incident if new run:
  if (m_firstRun || (m_currentRun != ctx.eventID().run_number())) {
    // Fire EndRun incident unless this is the first run
    if (!m_firstRun) {
      m_incidentSvc->fireIncident(Incident(this->name(),IncidentType::EndRun));
    }
    m_firstRun = false;
    m_currentRun = ctx.eventID().run_number();

    ATH_MSG_INFO ( "  ===>>>  start of run " << m_currentRun << "    <<<===" );

    // Fire BeginRun "Incident"
    m_incidentSvc->fireIncident(Incident(this->name(),IncidentType::BeginRun,ctx));
  }

  bool eventFailed(false);
  // Call the execute() method of all top algorithms
  StatusCode sc = executeAlgorithms( ctx );

  if (!sc.isSuccess()) {
    eventFailed = true;
    m_aess->setEventStatus( EventStatus::AlgFail, ctx );

    /// m_failureMode 1,
    /// RECOVERABLE: skip algorithms, but do not terminate job
    /// FAILURE: terminate job
    if (m_failureMode == 1 && sc.isRecoverable()) {
      ATH_MSG_WARNING ( "RECOVERABLE error returned by algorithm. " <<
                        "Skipping remaining algorithms." << std::endl <<
                        "\tNo output will be written for this event, " <<
                        "but job will continue to next event" );
      eventFailed = false;
    }

    /// m_failureMode 2: skip algorithms, but do not terminate job
    if (m_failureMode >= 2) {
      ATH_MSG_INFO ( "Skipping remaining algorithms." << std::endl <<
                     "\tNo output will be written for this event, " <<
                     "but job will continue to next event" );
      eventFailed = false;
    }
  }
  else {
    m_aess->setEventStatus( EventStatus::Success, ctx );

    // Call the execute() method of all output streams
    for (ListAlg::iterator ito = m_outStreamList.begin();
           ito != m_outStreamList.end(); ++ito ) {
      sc = (*ito)->sysExecute( ctx );
      if ( !sc.isSuccess() ) {
        eventFailed = true;
      }
    }
  }

  //------------------------------------------------------------------------
  // Check if there was an error processing current event
  //------------------------------------------------------------------------
  return eventFailed?StatusCode::FAILURE:StatusCode::SUCCESS;
}

StatusCode PileUpEventLoopMgr::queryInterface(const InterfaceID& riid,void** ppvInterface)
{
  if (IEventSeek::interfaceID().versionMatch(riid)) {
    *ppvInterface = dynamic_cast<IEventSeek*>(this);
  }
  else {
    // Interface is not directly available : try out a base class
    return MinimalEventLoopMgr::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}
