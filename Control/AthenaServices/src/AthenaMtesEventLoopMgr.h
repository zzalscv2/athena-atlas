// -*- C++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASERVICES_ATHENAMTESEVENTLOOPMGR_H
#define ATHENASERVICES_ATHENAMTESEVENTLOOPMGR_H

#include "GaudiKernel/IEvtSelector.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/MinimalEventLoopMgr.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/IAlgExecStateSvc.h"

#include "AthenaKernel/Timeout.h"
#include "AthenaKernel/IAthenaEvtLoopPreSelectTool.h"
#include "AthenaKernel/IEventSeek.h"
#include "AthenaKernel/ICollectionSize.h"
#include "AthenaKernel/IConditionsCleanerSvc.h"
#include "AthenaKernel/IHybridProcessorHelper.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>

#ifndef EVENTINFO_EVENTID_H
# include "EventInfo/EventID.h"  /* number_type */
#endif

// Forward declarations
class IConversionSvc;
struct IDataManagerSvc;
class IDataProviderSvc;
class IIncidentSvc;
class StoreGateSvc;
class ISvcLocator;
class OutputStreamSequencerSvc;

namespace yampl {
  class ISocket;
}

class ATLAS_NOT_THREAD_SAFE AthenaMtesEventLoopMgr
  : virtual public IEventSeek,
    virtual public ICollectionSize,
    virtual public IIncidentListener,
    virtual public IHybridProcessorHelper,
            public MinimalEventLoopMgr,
            public Athena::TimeoutMaster
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
  /// Gaudi event selector Context (may be used as a cursor by the evt selector)
  EvtContext*       m_evtContext;
  /// @property Event selector Name. If empty string (default) take value from ApplicationMgr
  StringProperty    m_evtsel;

  typedef ServiceHandle<IDataManagerSvc> IDataManagerSvc_t;
  /// Reference to the Histogram Data Service
  IDataManagerSvc_t  m_histoDataMgrSvc;

  typedef ServiceHandle<IConversionSvc> IConversionSvc_t;
  /// @property Reference to the Histogram Persistency Service
  IConversionSvc_t   m_histoPersSvc;

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

  /// property update handler:sets up the Pre-selection tools
  void setupPreSelectTools(Gaudi::Details::PropertyBase&);

  /// @property configure the policy wrt handling of when 'clear-event-store'
  /// has to happen: BeginEvent xor EndEvent.
  /// Default is to clear the store at the end of the event
  StringProperty m_clearStorePolicy;

  /// read event number from secondary input
  bool m_useSecondaryEventNumber;

  /// property update handler:set the clear-store policy value and check its
  /// value.
  void setClearStorePolicy(Gaudi::Details::PropertyBase& clearStorePolicy);

  /// Dump out histograms as needed
  virtual StatusCode writeHistograms(bool force=false);

  /// Run the algorithms for the current event
  virtual StatusCode executeAlgorithms();

  /// Initialize all algorithms and output streams
  StatusCode initializeAlgorithms();


  //***********************************************************//
  // for Hive
protected:

  /// Reference to the Whiteboard interface
  SmartIF<IHiveWhiteBoard>  m_whiteboard;

  /// Reference to the Algorithm resource pool
  SmartIF<IAlgResourcePool>  m_algResourcePool;

  /// Reference to the Algorithm Execution State Svc
  SmartIF<IAlgExecStateSvc>  m_aess;

  /// Property interface of ApplicationMgr
  SmartIF<IProperty>        m_appMgrProperty;

  /// A shortcut for the scheduler
  SmartIF<IScheduler> m_schedulerSvc;
  /// Clear a slot in the WB 
  StatusCode clearWBSlot(int evtSlot);
  /// Declare the root address of the event
  int declareEventRootAddress(EventContext&);
  /// Instance of the incident listener waiting for AbortEvent. 
  SmartIF< IIncidentListener >  m_abortEventListener;
  /// Name of the scheduler to be used
  std::string m_schedulerName;
  /// Name of the Whiteboard to be used
  std::string m_whiteboardName;
  /// Scheduled stop of event processing
  bool                m_scheduledStop;

  int m_currentEvntNum{-1};

public:
  /// Create event address using event selector
  StatusCode getEventRoot(IOpaqueAddress*& refpAddr);    



//***********************************************************//

public:
  /// Standard Constructor
  AthenaMtesEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  virtual ~AthenaMtesEventLoopMgr();
  /// implementation of IAppMgrUI::initalize
  virtual StatusCode initialize() override;
  /// implementation of IAppMgrUI::finalize
  virtual StatusCode finalize() override;
  /// implementation of IAppMgrUI::nextEvent. maxevt==0 returns immediately
  virtual StatusCode nextEvent(int maxevt) override;
  /// implementation of IEventProcessor::createEventContext()
  virtual EventContext createEventContext() override;
  /// implementation of IEventProcessor::executeEvent(void* par)
  virtual StatusCode executeEvent( EventContext&& ctx ) override;
  /// implementation of IEventProcessor::executeRun(int maxevt)
  virtual StatusCode executeRun(int maxevt) override;
  /// implementation of IEventProcessor::stopRun()
  virtual StatusCode stopRun() override;
  /// implementation of IService::stop
  virtual StatusCode stop() override;
 

  /// Seek to a given event.
  virtual StatusCode seek(int evt) override;
  /// Return the current event count.
  virtual int curEvent() const override;
  /// Return the size of the collection.
  virtual int size() override;
  /// IIncidentListenet interfaces
  virtual void handle(const Incident& inc) override;

  /// Reset the application return code
  virtual void resetAppReturnCode() override;
  
  virtual void setCurrentEventNum(int num) override;
  virtual bool terminateLoop() override;

  /// Drain the scheduler from all actions that may be queued
  virtual int drainScheduler(int& finishedEvents, bool report) override;

  /// interface dispatcher
  virtual StatusCode queryInterface( const InterfaceID& riid, 
                                     void** ppvInterface ) override;

  //FIXME hack to workaround pylcgdict problem...
  virtual const std::string& name() const  override { return Service::name(); } //FIXME 

private:
  AthenaMtesEventLoopMgr() = delete;
  AthenaMtesEventLoopMgr(const AthenaMtesEventLoopMgr&) = delete;
  AthenaMtesEventLoopMgr& operator= (const AthenaMtesEventLoopMgr&) = delete;

  unsigned int m_nevt;
  unsigned int m_timeStamp { 0 };
  /// @property histogram write/update interval
  UnsignedIntegerProperty m_writeInterval;
  bool m_writeHists;

  bool m_terminateLoop { false };

  /// events processed
  unsigned int m_nev;
  unsigned int m_proc;
  bool m_useTools;
  bool m_doEvtHeartbeat;

  unsigned int m_flmbi, m_timeStampInt;

  // from MinimalEventLoopMgr
public:
  typedef std::list<SmartIF<IAlgorithm> >  ListAlg;

  // Property to specify text messages with event ranges to simulate input from
  // EventService pilot - use for standalone tests
  StringArrayProperty m_testPilotMessages;
  bool m_inTestMode { false };
   
private:
  StoreGateSvc* eventStore() const;

  ServiceHandle<Athena::IConditionsCleanerSvc> m_conditionsCleaner;
  
  // Save a copy of the last event context to use
  // at the end of event processing.
  EventContext m_lastEventContext;

  // Event Service Specific stuff
  struct RangeStruct{
    RangeStruct()
      : eventRangeID{""}
      , pfn{""}
      , startEvent{-1}
      , lastEvent{-1} {}

    std::string eventRangeID;
    std::string pfn;
    int startEvent;
    int lastEvent;
  };

  std::unique_ptr<RangeStruct> getNextRange(yampl::ISocket* socket);
  void trimRangeStrings(std::string& str);

  ServiceHandle<OutputStreamSequencerSvc>  m_outSeqSvc;

  Gaudi::Property<std::string> m_eventRangeChannel{this
      , "EventRangeChannel"
      , "EventService_EventRanges"
      , "The name of the Yampl channel between AthenaMT and the Pilot"
      };

  // Hopefully a temporary measurement. For the time being we cannot
  // support event ranges from different input files.
  std::string m_pfn{""};

  // For the event service running:
  yampl::ISocket* m_socket{nullptr};
};

#endif // ATHENASERVICES_ATHENAHIVEEVENTLOOPMGR_H
