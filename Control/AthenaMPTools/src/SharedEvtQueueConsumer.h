/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAMPTOOLS_SHAREDEVTQUEUECONSUMER_H
#define ATHENAMPTOOLS_SHAREDEVTQUEUECONSUMER_H

#include "AthenaMPToolBase.h"

#include "AthenaInterprocess/SharedQueue.h"
#include "GaudiKernel/Timing.h"
#include "GaudiKernel/IEvtSelector.h"
#include <queue>

class IEventSeek;
class IEvtSelectorSeek;
class IEventShare;
class IDataShare;
class IChronoStatSvc;

class SharedEvtQueueConsumer final : public AthenaMPToolBase
{
 public:
  SharedEvtQueueConsumer(const std::string& type
			 , const std::string& name
			 , const IInterface* parent);

  virtual ~SharedEvtQueueConsumer() override;
  
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  // _________IAthenaMPTool_________   
  virtual int makePool ATLAS_NOT_THREAD_SAFE (int maxevt, int nprocs, const std::string& topdir) override;
  virtual StatusCode exec ATLAS_NOT_THREAD_SAFE () override;
  virtual StatusCode wait_once ATLAS_NOT_THREAD_SAFE (pid_t& pid) override;

  virtual void reportSubprocessStatuses() override;
  virtual void subProcessLogs(std::vector<std::string>&) override;

  // _____ Actual working horses ________
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> bootstrap_func() override;
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> exec_func() override;
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> fin_func() override;

 private:
  SharedEvtQueueConsumer();
  SharedEvtQueueConsumer(const SharedEvtQueueConsumer&);
  SharedEvtQueueConsumer& operator= (const SharedEvtQueueConsumer&);

  // Decode process results
  // 1. Store number of processed events for FUNC_EXEC
  // 2. If doFinalize flag is set then serialize process finalizations
  int decodeProcessResult ATLAS_NOT_THREAD_SAFE (const AthenaInterprocess::ProcessResult* presult, bool doFinalize);

  // Properties
  bool m_useSharedReader; // Work in pair with a SharedReader
  bool m_useSharedWriter; // Work in pair with a SharedWriter
  bool m_isRoundRobin;    // Are we running in the "reproducible mode"?
  int  m_nEventsBeforeFork;
  int  m_nSkipEvents;
  bool m_debug;

  int  m_rankId;          // Each worker has its own unique RankID from the range (0,...,m_nprocs-1) 

  ServiceHandle<IChronoStatSvc>  m_chronoStatSvc;
  IEventSeek*                    m_evtSeek;
  IEvtSelectorSeek*              m_evtSelSeek;
  IEvtSelector::Context*         m_evtContext;
  IEventShare*                   m_evtShare;
  IDataShare*                    m_dataShare;

  AthenaInterprocess::SharedQueue*  m_sharedEventQueue;          
  AthenaInterprocess::SharedQueue*  m_sharedRankQueue;          

  typedef System::ProcessTime::TimeValueType TimeValType;
  std::map<pid_t,std::pair<int,TimeValType>> m_eventStat; // Number of processed events by PID
  std::queue<pid_t>                          m_finQueue;         // PIDs of processes queued for finalization

  // "Persistent" event orders for reproducibility
  bool                           m_readEventOrders;
  std::string                    m_eventOrdersFile;
  std::vector<int>               m_eventOrders;
  pid_t                          m_masterPid;  // In finalize() of the master process merge workers' saved orders into one
};

#endif
