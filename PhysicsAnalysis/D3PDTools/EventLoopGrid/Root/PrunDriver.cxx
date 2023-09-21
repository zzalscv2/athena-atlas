/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Alexander Madsen
/// @author Nils Krumnack



#include <EventLoopGrid/PrunDriver.h>
#include <EventLoopGrid/GridDriver.h>
#include <EventLoop/Algorithm.h>
#include <EventLoop/ManagerData.h>
#include <EventLoop/ManagerStep.h>
#include <EventLoop/Job.h>
#include <EventLoop/MessageCheck.h>
#include <EventLoop/OutputStream.h>
#include <PathResolver/PathResolver.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/hadd.h>
#include <RootCoreUtils/ExceptionMsg.h>
#include <RootCoreUtils/ShellExec.h>
#include <SampleHandler/MetaObject.h>
#include <SampleHandler/Sample.h>
#include <SampleHandler/SampleGrid.h>
#include <SampleHandler/SampleHandler.h>
#include <SampleHandler/GridTools.h>


#include <TList.h>
#include <TPython.h>
#include <TROOT.h>
#include <TFile.h>
#include <TSystem.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

#if __cplusplus >= 201103L
#include "pool.h"
#include <mutex>
#endif

ClassImp(EL::PrunDriver)

namespace {
  namespace JobState {  
    static const unsigned int NSTATES = 6;
    enum Enum { INIT=0, RUN=1, DOWNLOAD=2, MERGE=3, FINISHED=4, FAILED=5 };
    static const char* name[NSTATES] = 
      { "INIT", "RUNNING", "DOWNLOAD", "MERGE", "FINISHED", "FAILED" };
    Enum parse(const std::string& what)
    {
      for (unsigned int i = 0; i != NSTATES; ++i) {
	if (what == name[i]) { return static_cast<Enum>(i); }
      }
      RCU_ASSERT0("Failed to parse job state string");
      throw std::runtime_error("PrunDriver.cxx: Failed to parse job state string"); //compiler dummy
    }
  }

  // When changing the values in the enum make sure
  // corresponding values in `data/ELG_jediState.py` script
  // are changed accordingly
  namespace Status {
    //static const int NSTATES = 3;
    enum Enum { DONE=0, PENDING=1, FAIL=2 };
  }

  struct TransitionRule {
    JobState::Enum fromState; 
    Status::Enum status; 
    JobState::Enum toState; 
    TransitionRule(JobState::Enum fromState,
		   Status::Enum status,
		   JobState::Enum toState)
      : fromState(fromState)
      , status(status)
      , toState(toState)
    {
    }
  };

  struct TmpCd {
    const std::string origDir;
    TmpCd(const std::string & dir)
      : origDir(gSystem->pwd())
    {
      gSystem->cd(dir.c_str());
    }
    ~TmpCd()
    {
      gSystem->cd(origDir.c_str());
    }
  };
}

static JobState::Enum sampleState(SH::Sample* sample)
{
  RCU_REQUIRE(sample);
  static const std::string defaultState = JobState::name[JobState::INIT];
  std::string label = sample->meta()->castString ("nc_ELG_state", defaultState, SH::MetaObject::CAST_NOCAST_DEFAULT);
  return JobState::parse(label);
}

static JobState::Enum nextState(JobState::Enum state, Status::Enum status)  
{
  RCU_REQUIRE(state != JobState::FINISHED);
  RCU_REQUIRE(state != JobState::FAILED);
  static const TransitionRule TABLE[] =
    {
      TransitionRule(JobState::INIT,     Status::DONE,    JobState::RUN),
      TransitionRule(JobState::INIT,     Status::PENDING, JobState::INIT),
      TransitionRule(JobState::INIT,     Status::FAIL,    JobState::FAILED),
      TransitionRule(JobState::RUN,      Status::DONE,    JobState::DOWNLOAD),
      TransitionRule(JobState::RUN,      Status::PENDING, JobState::RUN),
      TransitionRule(JobState::RUN,      Status::FAIL,    JobState::FAILED),
      TransitionRule(JobState::DOWNLOAD, Status::DONE,    JobState::MERGE),
      TransitionRule(JobState::DOWNLOAD, Status::PENDING, JobState::DOWNLOAD),
      TransitionRule(JobState::DOWNLOAD, Status::FAIL,    JobState::FAILED),
      TransitionRule(JobState::MERGE,    Status::DONE,    JobState::FINISHED),
      TransitionRule(JobState::MERGE,    Status::PENDING, JobState::MERGE),
      TransitionRule(JobState::MERGE,    Status::FAIL,    JobState::DOWNLOAD)
    };
  static const unsigned int TABLE_SIZE = sizeof(TABLE) / sizeof(TABLE[0]);
  for (unsigned int i = 0; i != TABLE_SIZE; ++i) {
    if (TABLE[i].fromState == state && TABLE[i].status == status) {
      return TABLE[i].toState;
    }
  }
  RCU_ASSERT0("Missing state transition rule");
  throw std::logic_error("PrunDriver.cxx: Missing state transition rule"); 
}

static SH::MetaObject defaultOpts()
{
  SH::MetaObject o;
  o.setString("nc_nGBPerJob", "MAX");
  o.setString("nc_mergeOutput", "true");
  o.setString("nc_cmtConfig", gSystem->ExpandPathName("$AnalysisBase_PLATFORM"));
  o.setString("nc_useAthenaPackages", "true");
  const std::string mergestr = "elg_merge jobdef.root %OUT %IN";
  o.setString("nc_mergeScript", mergestr);
  return o;
} 

static bool downloadContainer(const std::string& name, 
			      const std::string& location)
{
  RCU_ASSERT(not name.empty()); 
  RCU_ASSERT(name[name.size()-1] == '/'); 
  RCU_ASSERT(not location.empty()); 

  try {
    gSystem->Exec(Form("mkdir -p %s", location.c_str()));

    std::vector<std::string> datasets;
    for (auto& entry : SH::rucioListDids (name))
    {
      if (entry.type == "CONTAINER" || entry.type == "DIDType.CONTAINER")
        datasets.push_back (entry.name);
    }

    auto downloadResult = SH::rucioDownloadList (location, datasets);
    for (const auto& result : downloadResult)
    {
      if (result.notDownloaded != 0)
        return false;
    }
  } catch (...) { 
    return false;
  }
  return true;
}

static Status::Enum submit(SH::Sample* const sample)
{
  RCU_REQUIRE(sample);

  std::cout << "Submitting " << sample->name() << "...\n";

  static bool loaded = false;
  if (not loaded) {
    // TString path = "$ROOTCOREBIN/python/EventLoopGrid/ELG_prun.py";
    // gSystem->ExpandPathName(path);
    // TPython::LoadMacro(path.Data());
    std::string path = PathResolverFindCalibFile("EventLoopGrid/ELG_prun.py");
    TPython::LoadMacro(path.c_str());
    loaded = true;
  }

  TPython::Bind(dynamic_cast<TObject*>(sample), "ELG_SAMPLE"); 
  int ret = TPython::Eval("ELG_prun(ELG_SAMPLE)");
  TPython::Bind(0, "ELG_SAMPLE");   

  if (ret < 100) {
    sample->meta()->setString("nc_ELG_state_details", 
                              "problem submitting"); 
    return Status::FAIL;
  }

  sample->meta()->setDouble("nc_jediTaskID", ret);

  return Status::DONE;
}

static Status::Enum checkPandaTask(SH::Sample* const sample)
{
  RCU_REQUIRE(sample);
  RCU_REQUIRE(static_cast<int>(sample->meta()->castDouble("nc_jediTaskID",0, SH::MetaObject::CAST_NOCAST_DEFAULT)) > 100);

  static bool loaded = false;
  if (not loaded) {
    std::string path = PathResolverFindCalibFile("EventLoopGrid/ELG_jediState.py");
    TPython::LoadMacro(path.c_str());
    loaded = true;
  }

  TPython::Bind(dynamic_cast<TObject*>(sample), "ELG_SAMPLE");
  int ret =  TPython::Eval("ELG_jediState(ELG_SAMPLE)");
  TPython::Bind(0, "ELG_SAMPLE");

  if (ret == Status::DONE) return Status::DONE;
  if (ret == Status::FAIL) return Status::FAIL;

  // Value 90 corresponds to `running` state of the job
  if (ret != 90) { sample->meta()->setString("nc_ELG_state_details", "task status other than done/finished/failed/running"); }
  // Value 99 is returned if there is error in the script (import, missing ID)
  if (ret == 99) {
    sample->meta()->setString("nc_ELG_state_details",
                              "problem checking jedi task status");
  }

  return Status::PENDING;
}

static Status::Enum download(SH::Sample* const sample)
{
  RCU_REQUIRE(sample);

  {
#if __cplusplus >= 201103L
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
#endif
    std::cout << "Downloading output from: " 
	      << sample->name() << "..." << std::endl;
  }

  std::string container = sample->meta()->castString("nc_outDS", "", SH::MetaObject::CAST_NOCAST_DEFAULT);
  RCU_ASSERT(not container.empty());
  if (container[container.size()-1] == '/') {
    container.resize(container.size() - 1); 
  }
  container += "_hist/";

  bool downloadOk = downloadContainer(container, "elg/download/" + container);

  if (not downloadOk) {
    std::cerr << "Failed to download one or more files" << std::endl;
    sample->meta()->setString("nc_ELG_state_details", 
                              "error, check log for details"); 
    return Status::PENDING;
  }

  return Status::DONE;
}

static Status::Enum merge(SH::Sample* const sample)
{
  RCU_REQUIRE(sample);

  std::string container = sample->meta()->castString("nc_outDS", "", SH::MetaObject::CAST_NOCAST_DEFAULT);
  RCU_ASSERT(not container.empty());
  if (container[container.size()-1] == '/') {
    container.resize(container.size() - 1); 
  }
  container += "_hist/";
  const std::string dir = "elg/download/" + container;

  const std::string fileName = "hist-output.root";

  const std::string target = Form("hist-%s.root", sample->name().c_str());
  
  const std::string findCmd(Form("find %s -name \"*.%s*\" | tr '\n' ' '", 
				 dir.c_str(), fileName.c_str()));
  std::istringstream input(gSystem->GetFromPipe(findCmd.c_str()).Data()); 
  std::vector<std::string> files((std::istream_iterator<std::string>(input)),
				 std::istream_iterator<std::string>());
  
  std::sort(files.begin(), files.end());
  RCU_ASSERT(std::unique(files.begin(), files.end()) == files.end());
  
  if (not files.size()) {
    std::cerr << "Found no input files for merging! "
	      << "Requeueing sample for download..." << std::endl;
    sample->meta()->setString("nc_ELG_state_details", "retry, files were lost"); 
    return Status::FAIL;
  }

  try {
    RCU::hadd(target.c_str(), files); 
  } catch (...) {
    sample->meta()->setString("nc_ELG_state_details", 
                              "error, check log for details"); 
    gSystem->Exec(Form("rm -f %s", target.c_str()));
    return Status::PENDING;
  }

  for (size_t i = 0; i != files.size(); ++i) {
    gSystem->Exec(Form("rm %s", files[i].c_str()));
  }  
  gSystem->Exec(Form("rmdir %s/*", dir.c_str()));
  gSystem->Exec(Form("rmdir %s", dir.c_str()));

  return Status::DONE;
}

static void processTask(SH::Sample* const sample)
{
  RCU_REQUIRE(sample);

  JobState::Enum state = sampleState(sample);
  
  sample->meta()->setString("nc_ELG_state_details", "");

  Status::Enum status = Status::PENDING;
  switch (state) {
  case JobState::INIT: 
    status = submit(sample);
    break;
  case JobState::RUN: 
    status = checkPandaTask(sample);
    break;
  case JobState::DOWNLOAD: 
    status = download(sample);
    break;
  case JobState::MERGE: 
    status = merge(sample);
    break;
  case JobState::FINISHED: 
  case JobState::FAILED:
    break;
  }

  state = nextState(state, status);
  sample->meta()->setString("nc_ELG_state", JobState::name[state]);
}

static void processAllInState(const SH::SampleHandler& sh, JobState::Enum state,
                              const size_t nThreads)
{
  RCU_REQUIRE(sh.size());

#if __cplusplus >= 201103L

  WorkList workList;
  for (SH::SampleHandler::iterator s = sh.begin(); s != sh.end(); ++s) {
    if (sampleState(*s) == state) {
      workList.push_back([s]()->void{ processTask(*s); });
    }
  }    
  process(workList, nThreads);

#else

  for (SH::SampleHandler::iterator s = sh.begin(); s != sh.end(); ++s) {
    if (sampleState(*s) == state) {
      processTask(*s);
    }
  }    

#endif

}

static std::string formatOutputName(const SH::MetaObject& sampleMeta,
				    const std::string & pattern)
{
  const std::string sampleName = sampleMeta.castString("sample_name");
  RCU_REQUIRE(not pattern.empty());

  static const std::string nickname = 
    gSystem->GetFromPipe(Form("python -c \"%s\" 2>/dev/null", 
			      "from pandatools import PsubUtils;"
			      "print PsubUtils.getNickname();")).Data();
    
  TString out = pattern.c_str();

  out.ReplaceAll("%nickname%", nickname);
  out.ReplaceAll("%in:name%", sampleName);

  std::stringstream ss(sampleName);
  std::string item;
  int field = 0;
  while(std::getline(ss, item, '.')) {
    std::stringstream sskey;
    sskey << "%in:name[" << ++field << "]%";
    out.ReplaceAll(sskey.str(), item);
  }
  while (out.Index("%in:") != -1) {
    int i1 = out.Index("%in:");
    int i2 = out.Index("%", i1+1);
    TString metaName = out(i1+4, i2-i1-4);
    out.ReplaceAll("%in:"+metaName+"%", 
		   sampleMeta.castString(std::string(metaName.Data())));  
  }
  out.ReplaceAll("/", "");
  return out.Data();
}

std::string outputFileNames(const EL::Job& job)
{
  TList outputs;
  for (EL::Job::outputIter out = job.outputBegin(),
	 end = job.outputEnd(); out != end; ++out) {
    outputs.Add(out->Clone());
  }      
  std::string out = "hist:hist-output.root";
  TIter itr(&outputs);
  TObject *obj = 0;
  while ((obj = itr())) {
    EL::OutputStream *os = dynamic_cast<EL::OutputStream*>(obj);
    const std::string name = os->label() + ".root";
    const std::string ds = 
      os->options()->castString(EL::OutputStream::optContainerSuffix);
    out += "," + (ds.empty() ? name : ds + ":" + name);
  }
  return out;
}

// Save algortihms and lists of inputs and outputs to a root file
static void saveJobDef(const std::string& fileName,
		       const EL::Job& job,
		       const SH::SampleHandler sh)
{    
  TFile file(fileName.c_str(), "RECREATE"); 
  TList outputs; 
  for (EL::Job::outputIter o = job.outputBegin(); o !=job.outputEnd(); ++o) 
    outputs.Add(o->Clone());
  file.WriteTObject(&job.jobConfig(), "jobConfig", "SingleKey");        
  file.WriteTObject(&outputs, "outputs", "SingleKey");        
  for (SH::SampleHandler::iterator s = sh.begin(); s != sh.end(); ++s) {
    const SH::MetaObject& meta = *((*s)->meta());
    file.WriteObject(&meta, meta.castString("sample_name").c_str());
  }
}  

// Create a sample handler with grid locations of outputs with given label
static SH::SampleHandler outputSH(const SH::SampleHandler& in,
				  const std::string& outputLabel)
{
  SH::SampleHandler out;
  const std::string outputFile = "*" + outputLabel + ".root*";
  const std::string outDSSuffix = '_' + outputLabel + ".root/"; 
  for (SH::SampleHandler::iterator s = in.begin(); s != in.end(); ++s) {
    SH::SampleGrid* outSample = new SH::SampleGrid((*s)->name());
    const std::string outputDS = (*s)->meta()->castString("nc_outDS", "", SH::MetaObject::CAST_NOCAST_DEFAULT) + outDSSuffix;
    outSample->meta()->setString("nc_grid", outputDS);
    outSample->meta()->setString("nc_grid_filter", outputFile);
    out.add(outSample);
  }
  out.fetch(in);
  return out;
}

void EL::PrunDriver::testInvariant() const 
{
  RCU_INVARIANT(this != 0);
}

EL::PrunDriver::PrunDriver() 
{
  RCU_NEW_INVARIANT(this);
}

::StatusCode EL::PrunDriver ::
doManagerStep (Detail::ManagerData& data) const
{
  using namespace msgEventLoop;
  ANA_CHECK (Driver::doManagerStep (data));
  switch (data.step)
  {
  case Detail::ManagerStep::submitJob:
    {
      const std::string jobELGDir = data.submitDir + "/elg";
      const std::string runShFile = jobELGDir + "/runjob.sh";
      //const std::string runShOrig = "$ROOTCOREBIN/data/EventLoopGrid/runjob.sh";
      const std::string mergeShFile = jobELGDir + "/elg_merge";
      //const std::string mergeShOrig = 
      //  "$ROOTCOREBIN/user_scripts/EventLoopGrid/elg_merge";
      const std::string runShOrig = PathResolverFindCalibFile("EventLoopGrid/runjob.sh");
      const std::string mergeShOrig = PathResolverFindCalibFile("EventLoopGrid/elg_merge");

      const std::string jobDefFile = jobELGDir + "/jobdef.root";
      gSystem->Exec(Form("mkdir -p %s", jobELGDir.c_str()));
      gSystem->Exec(Form("cp %s %s", runShOrig.c_str(), runShFile.c_str()));
      gSystem->Exec(Form("chmod +x %s", runShFile.c_str()));
      gSystem->Exec(Form("cp %s %s", mergeShOrig.c_str(), mergeShFile.c_str()));
      gSystem->Exec(Form("chmod +x %s", mergeShFile.c_str()));

      // create symbolic links for additionnal files/directories if any to ship to the grid 
      std::string listToShipToGrid = data.options.castString(EL::Job::optGridPrunShipAdditionalFilesOrDirs, ""); 
      // parse the list of comma separated files and/or directories to ship to the grid 
      if (listToShipToGrid.size()){
        ANA_MSG_INFO (
          "Creating symbolic links for additional files or directories to be sent to grid.\n"
          "For root or heavy files you should also add their name (not the full path) to EL::Job::optUserFiles.\n"
          "Otherwise prun ignores those files."
        );

        std::vector<std::string> vect_filesOrDirToShip;
        // split string based on comma separators
        boost::split(vect_filesOrDirToShip,listToShipToGrid,boost::is_any_of(","));
        
        // Create symbolic links of files or directories to the submission directory
        for (const std::string & fileOrDirToShip: vect_filesOrDirToShip){
          ANA_MSG_INFO (("Creating symbolic link for: " +fileOrDirToShip).c_str());
          RCU::Shell::exec("ln -sf " + fileOrDirToShip + " " + jobELGDir);
        }
        ANA_MSG_INFO ("Finished creation of symbolic links");
      }

      const SH::SampleHandler& sh = data.job->sampleHandler();

      for (SH::SampleHandler::iterator s = sh.begin(); s != sh.end(); ++s) {
        SH::MetaObject& meta = *(*s)->meta();
        meta.fetchDefaults(data.options);
        meta.fetchDefaults(defaultOpts());
        meta.setString("nc_outputs", outputFileNames(*data.job));
        std::string outputSampleName = meta.castString("nc_outputSampleName");
        if (outputSampleName.empty()) {
          outputSampleName = "user.%nickname%.%in:name%";
        }
        meta.setString("nc_outDS", formatOutputName(meta, outputSampleName));
        meta.setString("nc_inDS", meta.castString("nc_grid", (*s)->name()));
        meta.setString("nc_writeInputToTxt", "IN:input.txt");
        meta.setString("nc_match", meta.castString("nc_grid_filter"));
        const std::string execstr = "runjob.sh " + (*s)->name();
        meta.setString("nc_exec", execstr);
      }

      saveJobDef(jobDefFile, *data.job, sh);
  
      for (EL::Job::outputIter out = data.job->outputBegin();
           out != data.job->outputEnd(); ++out) {
        SH::SampleHandler shOut = outputSH(sh, out->label());
        shOut.save(data.submitDir + "/output-" + out->label());
      }
      SH::SampleHandler shHist = outputSH(sh, "hist-output");
      shHist.save(data.submitDir + "/output-hist");
 
      TmpCd keepDir(jobELGDir);

      processAllInState(sh, JobState::INIT, 0); 

      sh.save(data.submitDir + "/input");
      data.submitted = true;
    }
    break;

  case Detail::ManagerStep::doRetrieve:
    {
      ANA_CHECK (doRetrieve (data));
    }
    break;

  default:
    (void) true; // safe to do nothing
  }
  return ::StatusCode::SUCCESS;
}

::StatusCode EL::PrunDriver::doRetrieve (Detail::ManagerData& data) const 
{
  RCU_READ_INVARIANT(this);

  TmpCd tmpDir(data.submitDir);

  SH::SampleHandler sh;
  sh.load("input");
  RCU_ASSERT(sh.size());

  const size_t nRunThreads = options()->castDouble("nc_run_threads", 0); 
  const size_t nDlThreads = options()->castDouble("nc_download_threads", 0); 
  processAllInState(sh, JobState::INIT, 0); 
  processAllInState(sh, JobState::RUN, nRunThreads);
  processAllInState(sh, JobState::DOWNLOAD, nDlThreads); 
  processAllInState(sh, JobState::MERGE, 0); 

  sh.save("input");

  std::cout << std::endl;

  bool allDone = true;
  for (SH::SampleHandler::iterator s = sh.begin(); s != sh.end(); ++s) {    
    JobState::Enum state = sampleState(*s);
    std::string details = (*s)->meta()->castString("nc_ELG_state_details", "", SH::MetaObject::CAST_NOCAST_DEFAULT);
    if (not details.empty()) { details = '(' + details + ')'; }

    std::cout << (*s)->name() << "\t"; 
    switch (state) {
    case JobState::INIT:
    case JobState::RUN:
    case JobState::DOWNLOAD:
    case JobState::MERGE:
      std::cout << JobState::name[state] << "\t";
      break;
    case JobState::FINISHED:
      std::cout << "\033[1;32m" << JobState::name[state] << "\033[0m\t";
      break;
    case JobState::FAILED:
      std::cout << "\033[1;31m" << JobState::name[state] << "\033[0m\t";
      break;
    }
    std::cout << details << std::endl;
    
    allDone &= (state == JobState::FINISHED || state == JobState::FAILED);
  }

  std::cout << std::endl;
  
  data.retrieved = true;
  data.completed = allDone;
  return ::StatusCode::SUCCESS;
}
 
void EL::PrunDriver::status(const std::string& location)
{
  RCU_REQUIRE(not location.empty());  
  TmpCd tmpDir(location);
  SH::SampleHandler sh;
  sh.load("input");
  RCU_ASSERT(sh.size());
  processAllInState(sh, JobState::RUN, 0); 
  sh.save("input");
  for (SH::SampleHandler::iterator s = sh.begin(); s != sh.end(); ++s) {    
    JobState::Enum state = sampleState(*s);
    std::string details = (*s)->meta()->castString("nc_ELG_state_details", "", SH::MetaObject::CAST_NOCAST_DEFAULT);
    if (not details.empty()) { details = '(' + details + ')'; }
    std::cout << (*s)->name() << "\t" << JobState::name[state] 
	      << "\t" << details << std::endl;
  }
}

void EL::PrunDriver::setState(const std::string& location,
			      const std::string& task,
			      const std::string& state)
{
  RCU_REQUIRE(not location.empty());  
  RCU_REQUIRE(not task.empty());  
  RCU_REQUIRE(not state.empty());  
  TmpCd tmpDir(location);
  SH::SampleHandler sh;
  sh.load("input");
  RCU_ASSERT(sh.size());
  if (not sh.get(task)) {
    std::cout << "Unknown task: " << task << std::endl;
    std::cout << "Choose one of: " << std::endl;
    sh.print();
    return;
  }
  JobState::parse(state);
  sh.get(task)->meta()->setString("nc_ELG_state", state);
  sh.save("input");
}
