/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "EvtRangeProcessor.h"
#include "copy_file_icc_hack.h"
#include "AthenaInterprocess/ProcessGroup.h"

#include "AthenaKernel/IEvtSelectorSeek.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/Timing.h"

#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdexcept>
#include <queue>
#include <signal.h>
#include <filesystem>

#include "yampl/SocketFactory.h"


EvtRangeProcessor::EvtRangeProcessor(const std::string& type
				     , const std::string& name
				     , const IInterface* parent)
  : AthenaMPToolBase(type,name,parent)
  , m_rankId(-1)
  , m_nEventsBeforeFork(0)
  , m_activeWorkers(0)
  , m_inpFile("")
  , m_chronoStatSvc("ChronoStatSvc", name)
  , m_incidentSvc("IncidentSvc", name)
  , m_evtSeek(nullptr)
  , m_channel2Scatterer("")
  , m_channel2EvtSel("")
  , m_sharedRankQueue(0)
  , m_sharedFailedPidQueue(0)
  , m_debug(false)
{
  declareInterface<IAthenaMPTool>(this);

  declareProperty("EventsBeforeFork",m_nEventsBeforeFork);
  declareProperty("Channel2Scatterer", m_channel2Scatterer);
  declareProperty("Channel2EvtSel", m_channel2EvtSel);
  declareProperty("Debug", m_debug);

  m_subprocDirPrefix = "worker_";
}

EvtRangeProcessor::~EvtRangeProcessor()
{
}

StatusCode EvtRangeProcessor::initialize()
{
  ATH_MSG_DEBUG("In initialize");

  ATH_CHECK(AthenaMPToolBase::initialize());
  ATH_CHECK(serviceLocator()->service(m_evtSelName,m_evtSeek));
  ATH_CHECK(m_chronoStatSvc.retrieve());
  ATH_CHECK(m_incidentSvc.retrieve());
  
  return StatusCode::SUCCESS;
}

StatusCode EvtRangeProcessor::finalize()
{
  delete m_sharedRankQueue;
  return StatusCode::SUCCESS;
}

int EvtRangeProcessor::makePool(int, int nprocs, const std::string& topdir)
{
  ATH_MSG_DEBUG("In makePool " << getpid());

  if(nprocs==0 || nprocs<-1) {
    ATH_MSG_ERROR("Invalid value for the nprocs parameter: " << nprocs);
    return -1;
  }

  if(topdir.empty()) {
    ATH_MSG_ERROR("Empty name for the top directory!");
    return -1;
  }

  m_nprocs = (nprocs==-1?sysconf(_SC_NPROCESSORS_ONLN):nprocs);
  m_activeWorkers = m_nprocs;
  m_subprocTopDir = topdir;

  // Create rank queue and fill it
  std::ostringstream rankQueueName;
  rankQueueName << "EvtRangeProcessor_RankQueue_" << getpid() << "_" << m_randStr;
  m_sharedRankQueue = new AthenaInterprocess::SharedQueue(rankQueueName.str(),m_nprocs,sizeof(int));
  for(int i=0; i<m_nprocs; ++i)
    if(!m_sharedRankQueue->send_basic<int>(i)) {
      ATH_MSG_ERROR("Unable to send int to the ranks queue!");
      return -1;
    }

  // Create the process group and map_async bootstrap
  m_processGroup = new AthenaInterprocess::ProcessGroup(m_nprocs);
  ATH_MSG_INFO("Created Pool of " << m_nprocs << " worker processes");
  if(mapAsyncFlag(AthenaMPToolBase::FUNC_BOOTSTRAP)) {
    return -1;
  }
  ATH_MSG_INFO("Workers bootstraped"); 

  // Populate the m_procStates map
  for(const AthenaInterprocess::Process& process : m_processGroup->getChildren()) {
    m_procStates[process.getProcessID()] = PROC_STATE_INIT;
  }

  return m_nprocs;
}

StatusCode EvtRangeProcessor::exec()
{
  ATH_MSG_DEBUG("In exec " << getpid());

  // Do nothing here. The exec will be mapped on workers one at a time ...

  return StatusCode::SUCCESS;
}

StatusCode EvtRangeProcessor::wait_once(pid_t& pid)
{
  // This method performs two tasks:
  // 1. Checks if any of the workers has changed its state, and if so performs appropriate actions
  // 2. Tries to pull one result from the workers results queue, and if there is one, then decodes it

  // First make sure we have a valid pointer to the Failed PID Queue
  if(m_sharedFailedPidQueue==0) {
    if(detStore()->retrieve(m_sharedFailedPidQueue,"AthenaMPFailedPidQueue_"+m_randStr).isFailure()) {
      ATH_MSG_ERROR("Unable to retrieve the pointer to Shared Failed PID Queue");
      return StatusCode::FAILURE;
    }
  }

  // ____________________ Step 1: check for state changes in the workers ______________________________
  StatusCode sc = AthenaMPToolBase::wait_once(pid);
  if(pid>0) {
    // One of the workers finished. We need to figure out whether or not it finished abnormally

    auto itProcState = m_procStates.find(pid);
    if(itProcState==m_procStates.end()) {
      // Untracked subprocess?? Something's wrong. Exit
      // To Do: how to report this error to the pilot?
      sc.ignore();
      ATH_MSG_ERROR("Detected untracked process ID=" << pid);
      return StatusCode::FAILURE;
    }

    // Deal with failed workers
    if(sc.isFailure()) {

      switch(itProcState->second) {
      case PROC_STATE_INIT:
	// If the failed process was in INIT state, exit immediately
	ATH_MSG_ERROR("Worker with process ID=" << pid << " failed at initialization!");
	return StatusCode::FAILURE;
      case PROC_STATE_EXEC:
	// If the failed process was in EXEC state, report pid to EvtRangeScatterer and attempt to start new worker

	// Report pid to Event Range Scatterer
	if(!m_sharedFailedPidQueue->send_basic<pid_t>(pid)) {
	  // To Do: how to report this error to the pilot?
	  ATH_MSG_ERROR("Failed to report the crashed pid to the Event Range Scatterer");
	  return StatusCode::FAILURE;
	}

	// Start new worker
	if(startProcess().isSuccess()) {
	  ATH_MSG_INFO("Successfully started new process");
	  pid=0;
	}
	else {
	  // To Do: how to report this error to the pilot?
	  ATH_MSG_ERROR("Failed to start new process");
	  return StatusCode::FAILURE;
	}
	break;
      case PROC_STATE_FIN:
	// If the failed process was in FIN state, remove pid from the finQueue and schedule finalization of the next worker
	m_finQueue.pop_front();

	if(m_finQueue.size()) {
	  if(mapAsyncFlag(AthenaMPToolBase::FUNC_FIN,m_finQueue.front())) {
	    // To Do: how to report this error to the pilot?
	    ATH_MSG_ERROR("Problem scheduling finalization on PID=" << m_finQueue.front());
	    return StatusCode::FAILURE;
	  }
	  else  {
	    ATH_MSG_INFO("Scheduled finalization of PID=" << m_finQueue.front());
	  }
	}
	break;
      case PROC_STATE_STOP:
	break;
      default:
	ATH_MSG_ERROR("Detected unexpected state " << itProcState->second << " of failed worker with PID=" << pid);
	return StatusCode::FAILURE;
      }
    }
    else {
      // The worker finished successfully and it was the last worker. Release the Event Range Scatterer
      if(--m_activeWorkers==0
	 && !m_sharedFailedPidQueue->send_basic<pid_t>(-1)) {
	// To Do: how to report this error to the pilot?
	ATH_MSG_ERROR("Failed to release the Event Range Scatterer");
	return StatusCode::FAILURE;
      }
    }

    // Erase the pid from m_procStates map
    m_procStates.erase(itProcState);
  }
  else {
    sc.ignore();
    if(pid<0) {
      // Here we failed to wait on the group. Exit immediately
      // To Do: how to report this error to the pilot?
      ATH_MSG_ERROR("Failed to wait on the process group!");
      return StatusCode::FAILURE;
    }
  }
  // ____________________ ______________________________________________ ______________________________


  // ____________________ Step 2: decode worker result (if any) ______________________________
  AthenaInterprocess::ProcessResult* presult = m_processGroup->pullOneResult();
  if(presult) {
    int res{0};
    if((unsigned)(presult->output.size)>=sizeof(int)) {
      // Decode result
      const AthenaInterprocess::ScheduledWork& output = presult->output;

      // First extract pid from the ProcessResult and check its validity
      pid_t childPid = presult->pid;
      auto itChildState = m_procStates.find(childPid);
      if(itChildState==m_procStates.end()) {
	ATH_MSG_ERROR("Unable to find PID=" << childPid << " in the Proc States map!");
	return StatusCode::FAILURE;
      }

      ATH_MSG_DEBUG("Decoding the output of PID=" << childPid << " with the size=" << output.size);

      if(output.size!=2*sizeof(int)+sizeof(AthenaMPToolBase::Func_Flag)) {
	// We are dealing with the bootstrap function.
	// Schedule exec_func()
	if(mapAsyncFlag(AthenaMPToolBase::FUNC_EXEC,childPid)) {
	  ATH_MSG_ERROR("Problem scheduling execution on PID=" << childPid);
	  return StatusCode::FAILURE;
	}

	// Update process state in the m_procStates map
	itChildState->second=PROC_STATE_EXEC;
      }

      AthenaMPToolBase::Func_Flag func;
      memcpy(&func,(char*)output.data+sizeof(int),sizeof(func));

      if(func==AthenaMPToolBase::FUNC_EXEC) {
	// Store the number of processed events
	int nevt(0);
	memcpy(&nevt,(char*)output.data+sizeof(int)+sizeof(func),sizeof(int));
	m_nProcessedEvents[childPid]=nevt;
	ATH_MSG_DEBUG("PID=" << childPid << " processed " << nevt << " events");

	// Add PID to the finalization queue
	m_finQueue.push_back(childPid);
	ATH_MSG_DEBUG("Added PID=" << childPid << " to the finalization queue");

	// If this is the only element in the queue then start its finalization
	// Otherwise it has to wait its turn until all previous processes have been finalized
	if(m_finQueue.size()==1) {
	  if(mapAsyncFlag(AthenaMPToolBase::FUNC_FIN,childPid)) {
	    ATH_MSG_ERROR("Problem scheduling finalization on PID=" << childPid);
	    return StatusCode::FAILURE;
	  }
	  else {
	    ATH_MSG_INFO("Scheduled finalization of PID=" << childPid);
	  }
	}

	// Update process state in the m_procStates map
	itChildState->second=PROC_STATE_FIN;
      }
      else if(func==AthenaMPToolBase::FUNC_FIN) {
	ATH_MSG_DEBUG("Finished finalization of PID=" << childPid);
	pid_t pidFront = m_finQueue.front();
	if(pidFront==childPid) {
	  // pid received as expected

	  // Set the process free
	  if(m_processGroup->map_async(0,0,pidFront)) {
	    ATH_MSG_ERROR("Failed to set the process PID=" << pidFront << " free");
	    return StatusCode::FAILURE;
	  }

	  // Remove it from the queue
	  m_finQueue.pop_front();
	  ATH_MSG_DEBUG("PID=" << childPid << " removed from the queue");
	  // Schedule finalization of the next process in the queue
	  if(m_finQueue.size()) {
	    if(mapAsyncFlag(AthenaMPToolBase::FUNC_FIN,m_finQueue.front())) {
	      ATH_MSG_ERROR("Problem scheduling finalization on PID=" << m_finQueue.front());
	      return StatusCode::FAILURE;
	    }
	    else  {
	      ATH_MSG_INFO("Scheduled finalization of PID=" << m_finQueue.front());
	    }
	  }
	}
	else {
	  // Error: unexpected pid received from presult
	  ATH_MSG_ERROR("Finalized PID=" << childPid << " while PID=" << pid << " was expected");
	  return StatusCode::FAILURE;
	}

	// Update process state in the m_procStates map
	itChildState->second=PROC_STATE_STOP;
      }
    }
    free(presult->output.data);
    delete presult;
    if(res) return StatusCode::FAILURE;
  }
  // ____________________ ______________________________________________ ______________________________

  return StatusCode::SUCCESS;
}

void EvtRangeProcessor::reportSubprocessStatuses()
{
  ATH_MSG_INFO("Statuses of event processors");
  const std::vector<AthenaInterprocess::ProcessStatus>& statuses = m_processGroup->getStatuses();
  for(size_t i=0; i<statuses.size(); ++i) {
    // Get the number of events processed by this worker
    std::map<pid_t,int>::const_iterator it = m_nProcessedEvents.find(statuses[i].pid);
    std::ostringstream ostr;
    if(it==m_nProcessedEvents.end())
      ostr << "N/A";
    else
      ostr << it->second;
    ATH_MSG_INFO("*** Process PID=" << statuses[i].pid 
		 << ". Status " << ((statuses[i].exitcode)?"FAILURE":"SUCCESS") 
		 << ". Number of events processed: " << ostr.str());
  }
}

void EvtRangeProcessor::subProcessLogs(std::vector<std::string>& filenames)
{
  filenames.clear();
  for(int i=0; i<m_nprocs; ++i) {
    std::ostringstream workerIndex;
    workerIndex << i;
    std::filesystem::path worker_rundir(m_subprocTopDir);
    worker_rundir /= std::filesystem::path(m_subprocDirPrefix+workerIndex.str());
    filenames.push_back(worker_rundir.string()+std::string("/AthenaMP.log"));
  }
}

AthenaMP::AllWorkerOutputs_ptr EvtRangeProcessor::generateOutputReport()
{
  AthenaMP::AllWorkerOutputs_ptr jobOutputs(new AthenaMP::AllWorkerOutputs());
  return jobOutputs;
}

std::unique_ptr<AthenaInterprocess::ScheduledWork> EvtRangeProcessor::bootstrap_func()
{
  if(m_debug) waitForSignal();

  std::unique_ptr<AthenaInterprocess::ScheduledWork> outwork(new AthenaInterprocess::ScheduledWork);
  outwork->data = malloc(sizeof(int));
  *(int*)(outwork->data) = 1; // Error code: for now use 0 success, 1 failure
  outwork->size = sizeof(int);
  // ...
  // (possible) TODO: extend outwork with some error message, which will be eventually
  // reported in the master proces
  // ...

  // ________________________ Get RankID ________________________
  //
  if(!m_sharedRankQueue->receive_basic<int>(m_rankId)) {
    ATH_MSG_ERROR("Unable to get rank ID!");
    return outwork;
  }
  std::ostringstream workindex;
  workindex<<m_rankId;

  // ________________________ Worker dir: mkdir ________________________
  std::filesystem::path worker_rundir(m_subprocTopDir);
  worker_rundir /= std::filesystem::path(m_subprocDirPrefix+workindex.str());
  // TODO: this "worker_" can be made configurable too

  if(mkdir(worker_rundir.string().c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)==-1) {
    ATH_MSG_ERROR("Unable to make worker run directory: " << worker_rundir.string() << ". " << fmterror(errno));
    return outwork;
  }

  // ________________________ Redirect logs ________________________
  if(!m_debug) {
    if(redirectLog(worker_rundir.string()))
      return outwork;
    
    ATH_MSG_INFO("Logs redirected in the AthenaMP event worker PID=" << getpid());
  }

  // ________________________ Update Io Registry ____________________________
  if(updateIoReg(worker_rundir.string()))
    return outwork;

  ATH_MSG_INFO("Io registry updated in the AthenaMP event worker PID=" << getpid());

  // ________________________ SimParams & DigiParams & PDGTABLE.MeV ____________________________
  std::filesystem::path abs_worker_rundir = std::filesystem::absolute(worker_rundir);
  if(std::filesystem::is_regular_file("SimParams.db"))
    COPY_FILE_HACK("SimParams.db", abs_worker_rundir.string()+"/SimParams.db");
  if(std::filesystem::is_regular_file("DigitParams.db"))
    COPY_FILE_HACK("DigitParams.db", abs_worker_rundir.string()+"/DigitParams.db");
  if(std::filesystem::is_regular_file("PDGTABLE.MeV"))
    COPY_FILE_HACK("PDGTABLE.MeV", abs_worker_rundir.string()+"/PDGTABLE.MeV");

  // _______________________ Handle saved PFC (if any) ______________________
  if(handleSavedPfc(abs_worker_rundir))
    return outwork;

  // ________________________  reopen descriptors ____________________________
  if(reopenFds())
    return outwork;

  ATH_MSG_INFO("File descriptors re-opened in the AthenaMP event worker PID=" << getpid());

  
  // ________________________ I/O reinit ________________________
  if(!m_ioMgr->io_reinitialize().isSuccess()) {
    ATH_MSG_ERROR("Failed to reinitialize I/O");
    return outwork;
  } else {
    ATH_MSG_DEBUG("Successfully reinitialized I/O");
  }

  // ________________________ Event selector restart ________________________
  IService* evtSelSvc = dynamic_cast<IService*>(evtSelector());
  if(!evtSelSvc) {
    ATH_MSG_ERROR("Failed to dyncast event selector to IService");
    return outwork;
  }
  if(!evtSelSvc->start().isSuccess()) {
    ATH_MSG_ERROR("Failed to restart the event selector");
    return outwork;
  } else {
    ATH_MSG_DEBUG("Successfully restarted the event selector");
  }

  // ________________________ Restart background event selectors in pileup jobs ________________________
  if(m_isPileup) {
    const std::list<IService*>& service_list = serviceLocator()->getServices();
    std::list<IService*>::const_iterator itSvc = service_list.begin(),
      itSvcLast = service_list.end();
    for(;itSvc!=itSvcLast;++itSvc) {
      IEvtSelector* evtsel = dynamic_cast<IEvtSelector*>(*itSvc);
      if(evtsel && (evtsel != evtSelector())) {
	if((*itSvc)->start().isSuccess())
	  ATH_MSG_DEBUG("Restarted event selector " << (*itSvc)->name());
	else {
	  ATH_MSG_ERROR("Failed to restart event selector " << (*itSvc)->name());
	  return outwork;
	}
      }
    }
  }

  // ________________________ Worker dir: chdir ________________________
  if(chdir(worker_rundir.string().c_str())==-1) {
    ATH_MSG_ERROR("Failed to chdir to " << worker_rundir.string());
    return outwork;
  }

  // Declare success and return
  *(int*)(outwork->data) = 0;
  return outwork;
}

std::unique_ptr<AthenaInterprocess::ScheduledWork> EvtRangeProcessor::exec_func()
{
  ATH_MSG_INFO("Exec function in the AthenaMP worker PID=" << getpid());

  int nEvt(1);
  int nEventsProcessed(0);

  std::queue<std::string> queueTokens;

  // Get the yampl connection channels
  yampl::ISocketFactory* socketFactory = new yampl::SocketFactory();
  std::string socket2ScattererName = m_channel2Scatterer.value() + std::string("_") + m_randStr;
  yampl::ISocket* socket2Scatterer = socketFactory->createClientSocket(yampl::Channel(socket2ScattererName,yampl::LOCAL),yampl::MOVE_DATA);
  ATH_MSG_INFO("Created CLIENT socket to the Scatterer: " << socket2ScattererName);
  std::ostringstream pidstr;
  pidstr << getpid();

  // Construct a "welcome" message to be sent to the EvtRangeScatterer
  std::string ping = pidstr.str() + std::string(" ready for event processing");

  while(true) {
    void* message2scatterer = malloc(ping.size());   
    memcpy(message2scatterer,ping.data(),ping.size());   
    socket2Scatterer->send(message2scatterer,ping.size());
    ATH_MSG_INFO("Sent a welcome message to the Scatterer");

    // Get the response - list of tokens - from the scatterer. 
    // The format of the response: | ResponseSize | RangeID, | evtEvtRange[,evtToken] |
    char *responseBuffer(0);
    std::string strPeerId;
    ssize_t responseSize = socket2Scatterer->recv(responseBuffer,strPeerId);
    // If response size is 0 then break the loop
    if(responseSize==1) {
      ATH_MSG_INFO("Empty range received. Terminating the loop");
      break;
    }

    std::string responseStr(responseBuffer,responseSize);
    ATH_MSG_INFO("Received response from the Scatterer : " << responseStr);

    // Start timing
    System::ProcessTime time_start = System::getProcessTime();

    size_t startpos(0);
    size_t endpos = responseStr.find(',');
    while(endpos!=std::string::npos) {
      queueTokens.push(responseStr.substr(startpos,endpos-startpos));
      startpos = endpos+1;
      endpos = responseStr.find(',',startpos);
    }
    queueTokens.push(responseStr.substr(startpos));
    // Actually the first element in the tokens queue is the RangeID. Get it
    std::string rangeID = queueTokens.front();
    queueTokens.pop();
    ATH_MSG_INFO("Received RangeID=" << rangeID);
    // Fire an incident
    m_incidentSvc->fireIncident(FileIncident(name(),"NextEventRange",rangeID));

    // Here we need to support two formats of the responseStr
    // Format 1. RangeID,startEvent,endEvent
    // Format 2. RangeID,fileName,startEvent,endEvent
    //
    // The difference between these two is that for Format 2 we first
    // need to update InputCollections property on the Event Selector
    // and only after that proceed with seeking
    //
    // The seeking part is identical for Format 1 and 2

    AthenaMPToolBase::ESRange_Status rangeStatus(AthenaMPToolBase::ESRANGE_SUCCESS);

    // Determine the format
    std::string filename("");
    if(queueTokens.front().find("PFN:")==0) {
      // We have Format 2
      // Update InputCollections property of the Event Selector with the file name from Event Range
      filename = queueTokens.front().substr(4);
      if(setNewInputFile(filename).isFailure()) {
	ATH_MSG_WARNING("Failed to set input file for the range: " << rangeID);
	rangeStatus = AthenaMPToolBase::ESRANGE_BADINPFILE;
	reportError(socket2Scatterer,rangeStatus);
	m_incidentSvc->fireIncident(FileIncident(name(),"NextEventRange","dummy"));
	continue;
      }
      queueTokens.pop();
    }

    // Get the number of events to process
    int startEvent = std::atoi(queueTokens.front().c_str());
    queueTokens.pop();
    int endEvent = std::atoi(queueTokens.front().c_str());
    queueTokens.pop();
    ATH_MSG_INFO("Range fields. File Name: " << (filename.empty()?"N/A":filename)
		 << ", First Event:" << startEvent
		 << ", Last Event:" << endEvent);

    // Process the events
    IEvtSelector::Context* ctx = nullptr;
    if (evtSelector()->createContext (ctx).isFailure()) {
      ATH_MSG_WARNING("Failed to create IEventSelector context.");
      rangeStatus = AthenaMPToolBase::ESRANGE_SEEKFAILED;
    }
    else {
      for(int i(startEvent-1); i<endEvent; ++i) {
        StatusCode sc = m_evtSeek->seek(*ctx, i);
        if(sc.isRecoverable()) {
          ATH_MSG_WARNING("Event " << i << " from range: " << rangeID << " not in the input file");
          rangeStatus = AthenaMPToolBase::ESRANGE_NOTFOUND;
          break;
        }
        else if(sc.isFailure()) {
          ATH_MSG_WARNING("Failed to seek to " << i << " in range: " << rangeID);
          rangeStatus = AthenaMPToolBase::ESRANGE_SEEKFAILED;
          break;
        }
        ATH_MSG_INFO("Seek to " << i << " succeeded");
        m_chronoStatSvc->chronoStart("AthenaMP_nextEvent");
        sc = m_evtProcessor->nextEvent(nEvt++);

        m_chronoStatSvc->chronoStop("AthenaMP_nextEvent");
        if(sc.isFailure()){
          ATH_MSG_WARNING("Failed to process the event " << i << " in range:" << rangeID);
          rangeStatus = AthenaMPToolBase::ESRANGE_PROCFAILED;
          break;
        }
        else {
          ATH_MSG_DEBUG("Event processed");
          nEventsProcessed++;
        }
      }
    }
    if (evtSelector()->releaseContext (ctx).isFailure()) {
      ATH_MSG_WARNING("Failed to release IEventSelector context.");
    }

    // Fire dummy NextEventRange incident in order to cut the previous output and report it
    m_incidentSvc->fireIncident(FileIncident(name(),"NextEventRange","dummy"));
    if(rangeStatus!=AthenaMPToolBase::ESRANGE_SUCCESS) {
      reportError(socket2Scatterer,rangeStatus);
      continue;
    }

    // Event range successfully processed
    std::string strOutpFile;
    // Get the full path of the event range output file
    for(std::filesystem::directory_iterator fdIt(std::filesystem::current_path()); fdIt!=std::filesystem::directory_iterator(); fdIt++) {
      if(fdIt->path().string().rfind(rangeID) == fdIt->path().string().size()-rangeID.size()) {
	if(strOutpFile.empty()) {
	  strOutpFile = fdIt->path().string();
	}
	else {
	  strOutpFile += (std::string(",")+fdIt->path().string());
	}	
      }
    }

    // Stop timing
    System::ProcessTime time_delta = System::getProcessTime() - time_start;
    
    // Prepare the output report
    if(!strOutpFile.empty()) {
      // We need to combine the output file name with
      // 1. RangeID (requested by JEDI)
      // 2. CPU time
      // 3. Wall time
      std::ostringstream outputReportStream;
      outputReportStream << strOutpFile
			 << ",ID:" << rangeID
			 << ",CPU:" << time_delta.cpuTime<System::Sec>()
			 << ",WALL:" << time_delta.elapsedTime<System::Sec>();
      std::string outputFileReport = outputReportStream.str();
      
      // Report the output
      message2scatterer = malloc(outputFileReport.size());
      memcpy(message2scatterer,outputFileReport.data(),outputFileReport.size());
      socket2Scatterer->send(message2scatterer,outputFileReport.size());
      ATH_MSG_INFO("Reported the output " << outputFileReport);
    }
    else {
      // This is an error: range successfully processed but no outputs were made
      ATH_MSG_WARNING("Failed to make an output file for range: " << rangeID);
      reportError(socket2Scatterer,AthenaMPToolBase::ESRANGE_FILENOTMADE);
    }
  } // Main "event loop"

  if(m_evtProcessor->executeRun(0).isFailure()) {
    ATH_MSG_WARNING("Could not finalize the Run");
  }

  std::unique_ptr<AthenaInterprocess::ScheduledWork> outwork(new AthenaInterprocess::ScheduledWork);

  // Return value: "ERRCODE|Func_Flag|NEvt"
  int outsize = 2*sizeof(int)+sizeof(AthenaMPToolBase::Func_Flag);
  void* outdata = malloc(outsize);
  *(int*)(outdata) = 0; // Error code: for now use 0 success, 1 failure
  AthenaMPToolBase::Func_Flag func = AthenaMPToolBase::FUNC_EXEC;
  memcpy((char*)outdata+sizeof(int),&func,sizeof(func));
  memcpy((char*)outdata+sizeof(int)+sizeof(func),&nEventsProcessed,sizeof(int));

  outwork->data = outdata;
  outwork->size = outsize;
  // ...
  // (possible) TODO: extend outwork with some error message, which will be eventually
  // reported in the master proces
  // ...

  delete socket2Scatterer;
  delete socketFactory;

  return outwork;
}

std::unique_ptr<AthenaInterprocess::ScheduledWork> EvtRangeProcessor::fin_func()
{
  ATH_MSG_INFO("Fin function in the AthenaMP worker PID=" << getpid());

  if(m_appMgr->stop().isFailure()) {
    ATH_MSG_WARNING("Unable to stop AppMgr"); 
  }
  else { 
    if(m_appMgr->finalize().isFailure()) {
      std::cout << "Unable to finalize AppMgr" << std::endl;
    }
  }

  std::unique_ptr<AthenaInterprocess::ScheduledWork> outwork(new AthenaInterprocess::ScheduledWork);

  // Return value: "ERRCODE|Func_Flag|NEvt"  (Here NEvt=-1)
  int outsize = 2*sizeof(int)+sizeof(AthenaMPToolBase::Func_Flag);
  void* outdata = malloc(outsize);
  *(int*)(outdata) = 0; // Error code: for now use 0 success, 1 failure
  AthenaMPToolBase::Func_Flag func = AthenaMPToolBase::FUNC_FIN;
  memcpy((char*)outdata+sizeof(int),&func,sizeof(func));
  int nEvt = -1;
  memcpy((char*)outdata+sizeof(int)+sizeof(func),&nEvt,sizeof(int));

  outwork->data = outdata;
  outwork->size = outsize;

  return outwork;
}

StatusCode EvtRangeProcessor::startProcess()
{
  m_nprocs++;

  // Create a rank for the new process
  if(!m_sharedRankQueue->send_basic<int>(m_nprocs-1)) {
    ATH_MSG_ERROR("Unable to send int to the ranks queue!");
    return StatusCode::FAILURE;
  }
  
  pid_t pid = m_processGroup->launchProcess();
  if(pid==0) {
    ATH_MSG_ERROR("Unable to start new process");
    return StatusCode::FAILURE;
  }
  
  if(mapAsyncFlag(AthenaMPToolBase::FUNC_BOOTSTRAP,pid)) {
    ATH_MSG_ERROR("Unable to bootstrap new process");
    return StatusCode::FAILURE;
  }

  m_procStates[pid] = PROC_STATE_INIT;
  return StatusCode::SUCCESS;
}

StatusCode EvtRangeProcessor::setNewInputFile(const std::string& newFile)
{
  if(m_inpFile == newFile) return StatusCode::SUCCESS;

  // Get Property Server
  IProperty* propertyServer = dynamic_cast<IProperty*>(evtSelector());
  if(!propertyServer) {
    ATH_MSG_ERROR("Unable to dyn-cast the event selector to IProperty");
    return StatusCode::FAILURE;
  }

  std::string propertyName("InputCollections");
  if(m_inpFile.empty()) {
    std::vector<std::string> vect;
    StringArrayProperty inputFileList(propertyName, vect);
    if(propertyServer->getProperty(&inputFileList).isFailure()) {
      ATH_MSG_ERROR("Failed to get InputCollections property value of the Event Selector");
      return StatusCode::FAILURE;
    }
    if(newFile==inputFileList.value()[0]) {
      m_inpFile = newFile;
      return StatusCode::SUCCESS;
    }
  }
  std::vector<std::string> vect{newFile,};
  StringArrayProperty newInputFileList(propertyName, vect);
  if(propertyServer->setProperty(newInputFileList).isFailure()) {
    ATH_MSG_ERROR("Unable to update " << newInputFileList.name() << " property on the Event Selector");
    return StatusCode::FAILURE;
  }
  m_inpFile=newFile;
  return StatusCode::SUCCESS;
}

void EvtRangeProcessor::reportError(yampl::ISocket* socket, AthenaMPToolBase::ESRange_Status status)
{
  pid_t pid = getpid();
  size_t messageSize = sizeof(pid_t)+sizeof(AthenaMPToolBase::ESRange_Status);
  void* message2scatterer = malloc(messageSize);
  memcpy(message2scatterer,&pid,sizeof(pid_t));
  memcpy((pid_t*)message2scatterer+1,&status,sizeof(AthenaMPToolBase::ESRange_Status));
  socket->send(message2scatterer,messageSize);
}
