//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//

#include <AsgTesting/UnitTest.h>
#include <EventLoop/DirectDriver.h>
#include <EventLoop/Job.h>
#include <EventLoop/OutputStream.h>
#include <SampleHandler/SampleHandler.h>
#include <SampleHandler/SampleLocal.h>
#include <TFile.h>

using namespace EL;

TEST (WorkerConfigTest, DISABLED_baseTest)
{
  Job job;
  job.useXAOD();
  job.outputAdd (OutputStream ("out"));
  job.options()->setString (Job::optWorkerConfigFile, "EventLoopTest/worker_config.py");
  job.options()->setString (Job::optSubmitDirMode, "unique");

  SH::SampleHandler sh;
  sh.setMetaString ("nc_tree", "CollectionTree");
  auto sample = std::make_unique<SH::SampleLocal> ("mc");
  sample->add( "file://${ASG_TEST_FILE_MC}" );
  sh.add (std::move (sample));
  job.sampleHandler (sh);

  DirectDriver driver;
  std::string submitDir = driver.submit (job, "submitDir");
  std::unique_ptr<TFile> file (TFile::Open ((submitDir + "/hist-mc.root").c_str(), "READ"));
  ASSERT_NE (file.get(), nullptr);
  ASSERT_NE (file->Get ("dummy_hist"), nullptr);
}

// Declare the main() function.
ATLAS_GOOGLE_TEST_MAIN
