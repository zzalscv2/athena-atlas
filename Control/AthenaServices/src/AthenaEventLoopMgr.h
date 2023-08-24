// -*- C++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASERVICES_ATHENAEVENTLOOPMGR_H
#define ATHENASERVICES_ATHENAEVENTLOOPMGR_H
/** @file AthenaEventLoopMgr.h
    @brief The default ATLAS batch event loop manager.

*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;  // non-MT EventLoopMgr

// Base class headers
#include "AthenaKernel/IEventSeek.h"
#include "AthenaKernel/ICollectionSize.h"
#include "GaudiKernel/IIncidentListener.h"
#include "AthenaKernel/Timeout.h"
#include "GaudiKernel/MinimalEventLoopMgr.h"

// Athena headers
#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaKernel/IAthenaEvtLoopPreSelectTool.h"
#include "AthenaKernel/IEvtSelectorSeek.h"
#include "AthenaKernel/IConditionsCleanerSvc.h"
#ifndef EVENTINFO_EVENTID_H
# include "EventInfo/EventID.h"  /* number_type */
#endif

// Gaudi headers
#include <string>
#include <vector>
#include "GaudiKernel/IEvtSelector.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IAlgExecStateSvc.h"

// Forward declarations
class IConversionSvc;
struct IDataManagerSvc;
class IIncidentSvc;
class StoreGateSvc;
class EventContext;
class ISvcLocator;
class IEvtIdModifierSvc;

/** @class AthenaEventLoopMgr
    @brief The default ATLAS batch event loop manager.
    
    @details It loops over input events according to
    job configuration. Among the main user-settable properties
    "FailureMode" controls behaviour of event loop depending on return code of Algorithms. 
    - 0: all non-SUCCESSes terminate job. 
    - 1: (DEFAULT) RECOVERABLE skips to next event, FAILURE terminates job.
    - 2: RECOVERABLE and FAILURE skip to next events
*/
class AthenaEventLoopMgr 
  : virtual public IEventSeek,
    virtual public ICollectionSize,
    virtual public IIncidentListener,
    public MinimalEventLoopMgr,
    public Athena::TimeoutMaster,
    public AthMessaging
{
public:
  typedef IEvtSelector::Context   EvtContext;

protected:
  typedef ServiceHandle<IIncidentSvc> IIncidentSvc_t;
  /// Reference to the incident service
  IIncidentSvc_t     m_incidentSvc;

  typedef ServiceHandle<StoreGateSvc> StoreGateSvc_t;
  /// Reference to StoreGateSvc;
  StoreGateSvc_t  m_eventStore;  ///< Property

  /// Reference to the Event Selector
  IEvtSelector*     m_evtSelector;
  /// Gaudi EventSelector Context (may be used as a cursor by the evt selector)
  IEvtSelector::Context* m_evtSelCtxt;
  /// @property Event selector Name. If empty string (default) take value from ApplicationMgr
  StringProperty    m_evtsel;

  typedef ServiceHandle<IDataManagerSvc> IDataManagerSvc_t;
  /// Reference to the Histogram Data Service
  IDataManagerSvc_t  m_histoDataMgrSvc;

  typedef ServiceHandle<IConversionSvc> IConversionSvc_t;
  /// @property Reference to the Histogram Persistency Service
  IConversionSvc_t   m_histoPersSvc;

  typedef ServiceHandle<IEvtIdModifierSvc> IEvtIdModifierSvc_t;
  /// @property Reference to the EventID modifier Service
  IEvtIdModifierSvc_t m_evtIdModSvc;

  /// @property list of algorithms/sequences to execute during  PreFork
  StringArrayProperty m_execAtPreFork;

  /// @property histogram persistency technology to use: "ROOT", "HBOOK", "NONE". By default ("") get property value from ApplicationMgr
  StringProperty    m_histPersName;

  typedef EventID::number_type number_type;
  /// current run number
  number_type m_currentRun;
  bool m_firstRun;

  /// @property Failure mode
  IntegerProperty m_failureMode;

  /// @property Print event heartbeat printouts every m_eventPrintoutInterval events
  UnsignedIntegerProperty m_eventPrintoutInterval;

  ///@property list of AthenaEventLoopPreselectTools
  typedef IAthenaEvtLoopPreSelectTool          tool_type;
  typedef ToolHandleArray< tool_type >         tool_store;
  typedef tool_store::const_iterator           tool_iterator;
  typedef std::vector<unsigned int>            tool_stats;
  typedef tool_stats::const_iterator           tool_stats_iterator;

  tool_stats m_toolInvoke; ///< tool called counter
  tool_stats m_toolReject; ///< tool returns StatusCode::FAILURE counter
  tool_stats m_toolAccept; ///< tool returns StatusCode::SUCCESS counter
  tool_store m_tools;         ///< internal tool store

  /// require input attribute list
  bool m_requireInputAttributeList{};

  /// read event number from secondary input
  bool m_useSecondaryEventNumber{};

  /// property update handler:sets up the Pre-selection tools
  void setupPreSelectTools(Gaudi::Details::PropertyBase&);

  /// @property configure the policy wrt handling of when 'clear-event-store'
  /// has to happen: BeginEvent xor EndEvent.
  /// Default is to clear the store at the end of the event
  StringProperty m_clearStorePolicy;

  /// property update handler:set the clear-store policy value and check its
  /// value.
  void setClearStorePolicy(Gaudi::Details::PropertyBase& clearStorePolicy);

  /// Dump out histograms as needed
  virtual StatusCode writeHistograms(bool force=false);

  /// Run the algorithms for the current event
  virtual StatusCode executeAlgorithms(const EventContext&);

  /// Initialize all algorithms and output streams
  StatusCode initializeAlgorithms();

protected:
  /// Reference to the Algorithm Execution State Svc
  SmartIF<IAlgExecStateSvc>  m_aess;

public:
  /// Standard Constructor
  AthenaEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  virtual ~AthenaEventLoopMgr();
  /// implementation of IAppMgrUI::initalize
  virtual StatusCode initialize();
  /// implementation of IAppMgrUI::finalize
  virtual StatusCode finalize();
  /// implementation of IAppMgrUI::nextEvent. maxevt==0 returns immediately
  virtual StatusCode nextEvent(int maxevt);
  /// implementation of IEventProcessor::executeEvent(EventContext&& ctx)
  virtual StatusCode executeEvent( EventContext && ctx );
  /// implementation of IEventProcessor::executeRun(int maxevt)
  virtual StatusCode executeRun(int maxevt);
  /// Seek to a given event.
  virtual StatusCode seek(int evt);
  /// Return the current event count.
  virtual int curEvent() const;
  /// Return the size of the collection.
  virtual int size();
  /// IIncidentListenet interfaces
  void handle(const Incident& inc);
  /// Execute certain algorithms/sequences in PreFork
  StatusCode execAtPreFork(const EventContext& ctx) const;

  /// interface dispatcher
  virtual StatusCode queryInterface( const InterfaceID& riid, 
                                     void** ppvInterface );

  using AthMessaging::msg;
  using AthMessaging::msgLvl;

  //FIXME hack to workaround pylcgdict problem...
  virtual const std::string& name() const { return Service::name(); } //FIXME 

  virtual void modifyEventContext(EventContext& ctx, const EventID& eID, bool consume_modifier_stream);

private:
  AthenaEventLoopMgr(); ///< no implementation
  AthenaEventLoopMgr(const AthenaEventLoopMgr&); ///< no implementation
  AthenaEventLoopMgr& operator= (const AthenaEventLoopMgr&); ///< no implementation

  StatusCode installEventContext(EventContext& ctx);

  int m_nevt{};
  /// @property histogram write/update interval
  IntegerProperty m_writeInterval;
  bool m_writeHists{};

  /// events processed
  unsigned int m_nev;
  unsigned int m_proc;
  bool m_useTools;

  unsigned int m_lastNev{};
  unsigned int m_intervalInSeconds;
  time_t m_lastTime{};

  bool m_liteLoop;

  StoreGateSvc* eventStore() const;

  bool m_doChrono = false;
  ServiceHandle<IChronoStatSvc> m_chronoStatSvc;
  ServiceHandle<Athena::IConditionsCleanerSvc> m_conditionsCleaner;
};

#endif // STOREGATE_ATHENAEVENTLOOPMGR_H
