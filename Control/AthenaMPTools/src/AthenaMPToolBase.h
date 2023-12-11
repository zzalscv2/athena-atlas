/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAMPTOOLS_ATHENAMPTOOLBASE_H
#define ATHENAMPTOOLS_ATHENAMPTOOLBASE_H

#include "AthenaMPTools/IAthenaMPTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/IIoComponentMgr.h"

#include "AthenaInterprocess/ProcessGroup.h"
#include "AthenaInterprocess/IMessageDecoder.h"

#include <filesystem>

class IEvtSelector;

class AthenaMPToolBase : public AthAlgTool
  , public IAthenaMPTool
  , public AthenaInterprocess::IMessageDecoder
{
 public:
  AthenaMPToolBase(const std::string& type
		   , const std::string& name
		   , const IInterface* parent);

  virtual ~AthenaMPToolBase() override;
  
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  // _________IAthenaMPTool_________   
  virtual StatusCode wait_once ATLAS_NOT_THREAD_SAFE (pid_t& pid) override;

  virtual void reportSubprocessStatuses() override;
  virtual AthenaMP::AllWorkerOutputs_ptr generateOutputReport() override;

  virtual void useFdsRegistry(std::shared_ptr<AthenaInterprocess::FdsRegistry>) override;
  virtual void setRandString(const std::string& randStr) override;

  virtual void killChildren() override;

  // _________IMessageDecoder_________
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> operator() ATLAS_NOT_THREAD_SAFE (const AthenaInterprocess::ScheduledWork&) override;

  // _____ Actual working horses ________
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> bootstrap_func() = 0;
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> exec_func() = 0;
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> fin_func() = 0;

 protected:
  enum ESRange_Status {
    ESRANGE_SUCCESS
    , ESRANGE_NOTFOUND
    , ESRANGE_SEEKFAILED
    , ESRANGE_PROCFAILED
    , ESRANGE_FILENOTMADE
    , ESRANGE_BADINPFILE
  };

  enum Func_Flag {
    FUNC_BOOTSTRAP
    , FUNC_EXEC
    , FUNC_FIN
  };

  int mapAsyncFlag ATLAS_NOT_THREAD_SAFE(Func_Flag flag, pid_t pid=0);
  int redirectLog(const std::string& rundir, bool addTimeStamp = true);
  int updateIoReg(const std::string& rundir);
  std::string fmterror(int errnum);

  int reopenFds();
  int handleSavedPfc(const std::filesystem::path& dest_path);

  void waitForSignal();

  IEvtSelector* evtSelector() { return m_evtSelector; }

  int         m_nprocs;           // Number of workers spawned by the master process
  std::string m_subprocTopDir;    // Top run directory for subprocesses
  std::string m_subprocDirPrefix; // For ex. "worker__"
  std::string m_evtSelName;       // Name of the event selector

  AthenaInterprocess::ProcessGroup* m_processGroup;

  ServiceHandle<IEventProcessor> m_evtProcessor;
  ServiceHandle<IAppMgrUI>       m_appMgr;
  ServiceHandle<IFileMgr>        m_fileMgr;
  ServiceHandle<IIoComponentMgr> m_ioMgr;
  IEvtSelector*                  m_evtSelector;
  std::string                    m_fileMgrLog;
  std::shared_ptr<AthenaInterprocess::FdsRegistry> m_fdsRegistry;
  std::string                    m_randStr;

  Gaudi::Property<bool> m_isPileup {this, "IsPileup", false, "Flag for configuring PileUpEventLoopMgr"};

 private:
  AthenaMPToolBase();
  AthenaMPToolBase(const AthenaMPToolBase&);
  AthenaMPToolBase& operator= (const AthenaMPToolBase&);
  int reopenFd(int fd, const std::string& name); // reopen individual descriptor

};

#endif
