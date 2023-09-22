/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoopAlgs/DuplicateChecker.h>
#include <EventLoopAlgs/Global.h>
#include <AsgTesting/UnitTest.h>
#include <AthContainers/AuxStoreStandalone.h>
#include <AsgTools/StatusCode.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/ShellExec.h>
#include <RootCoreUtils/UnitTestDir.h>
#include <SampleHandler/SampleLocal.h>
#include <EventLoop/DirectDriver.h>
#include <EventLoop/Job.h>
#include <EventLoop/LocalDriver.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODEventInfo/EventAuxInfo.h>
#include <xAODRootAccess/Init.h>
#include <xAODRootAccess/TEvent.h>
#include <xAODRootAccess/TStore.h>
#include <cstdlib>

using namespace EL;

//
// unit test
//

/// \brief make an xAOD with duplicate events
::StatusCode makeXAOD (const std::string& file, unsigned runNumber,
	       unsigned firstEventNumber)
{
  xAOD::TEvent event;
  xAOD::TStore store;

  // so apparently I need an input file to make an output file...
  std::unique_ptr<TFile> inputFile (TFile::Open (getenv ("ROOTCORE_TEST_FILE"), "READ"));
  RCU_ASSERT_SOFT (inputFile != nullptr);
  TTree *tree = dynamic_cast<TTree*>(inputFile->Get ("CollectionTree"));
  RCU_ASSERT_SOFT (tree);
  if (event.readFrom (tree).isFailure())
    RCU_ASSERT0 ("failed to read from input tree");
  std::unique_ptr<TFile> myfile (TFile::Open (file.c_str(), "RECREATE"));
  if (event.writeTo (myfile.get()).isFailure())
  {
    std::cout << "failed to write to file" << std::endl;
    return ::StatusCode::FAILURE;
  }
   


  for (unsigned iter = 0, end = 9000; iter != end; ++ iter)
  {
    unsigned duplicates = 1;
    if (iter % 9 == 0)
      ++ duplicates;
    for (unsigned jter = 0; jter != duplicates; ++ jter)
    {
      store.clear ();
      event.getEntry (0);

      std::unique_ptr<xAOD::EventInfo> info (new xAOD::EventInfo);
      std::unique_ptr<xAOD::EventAuxInfo> aux (new xAOD::EventAuxInfo);
      info->setStore (aux.get());
      info->setRunNumber (runNumber);
      info->setEventNumber (firstEventNumber + iter);
      if (event.record (info.release(), "MyEventInfo").isFailure())
      {
	std::cout << "failed to record EventInfo" << std::endl;
	return ::StatusCode::FAILURE;
      }
      if (event.record (aux.release(), "MyEventInfoAux.").isFailure())
      {
      	std::cout << "failed to record EventInfoAux" << std::endl;
      	return ::StatusCode::FAILURE;
      }
      if (event.fill () < 0)
      {
	std::cout << "failed to write to file" << std::endl;
	return ::StatusCode::FAILURE;
      }
    }
  }
  if(event.finishWritingTo(myfile.get()).isFailure())
  {
    std::cout << "failed to finish writing to file" << std::endl;
    return ::StatusCode::FAILURE;
  }
  myfile->Write ();
  return ::StatusCode::SUCCESS;
}

/// \brief check the histogram output
/// \par Guarantee
///   basic
/// \par Failures
///   unit test failures\n
///   i/o errors
void checkHistograms (const std::string& submitdir,
		      unsigned raw, unsigned final,
		      bool expect_success)
{
  std::unique_ptr<TFile> file (TFile::Open ((submitdir + "/hist-sample.root").c_str(), "READ"));
  ASSERT_NE (file, nullptr);

  TH1 *hist = dynamic_cast<TH1*>(file->Get ("EventLoop_EventCount"));
  ASSERT_NE (hist, nullptr);
  ASSERT_EQ (hist->GetNbinsX(), 2);

  // hist->Print ("ALL");
  EXPECT_EQ (hist->GetBinContent(1), raw);
  EXPECT_EQ (hist->GetBinContent(2), final);

  TTree *summary = dynamic_cast<TTree*>(file->Get ("summary"));
  ASSERT_NE (summary, nullptr);

  // summary->Print ("ALL");
  EXPECT_EQ (summary->GetEntries(), raw);

  summary->SetBranchStatus ("*", 0);
  summary->SetBranchStatus ("processed", 1);
  Bool_t processed = false;
  summary->SetBranchAddress ("processed", &processed);
  unsigned count = 0;
  for (unsigned iter = 0; iter != raw; ++ iter)
  {
    summary->GetEntry (iter);
    if (processed)
      ++ count;
  }
  EXPECT_EQ (count, final);

  bool success = DuplicateChecker::processSummary (submitdir, "summary");
  EXPECT_EQ (success, expect_success);
}

TEST (DuplicateCheckerTest, all_tests)
{
  xAOD::TReturnCode::enableFailure();
  xAOD::Init ().ignore();

  RCU::UnitTestDir dir ("EventLoopAlgs", "DuplicateChecker");

  std::string prefix = dir.path() + "/";

  if (makeXAOD (prefix + "test1.root", 1, 0).isFailure())
  {
    FAIL() << "failed to make test file";
  }
  if (makeXAOD (prefix + "test2.root", 1, 8000).isFailure())
  {
    FAIL() << "failed to make test file";
  }
  if (makeXAOD (prefix + "test3.root", 2, 0).isFailure())
  {
    FAIL() << "failed to make test file";
  }
  std::unique_ptr<SH::SampleLocal> sample (new SH::SampleLocal ("sample"));
  sample->add (prefix + "test1.root");
  sample->add (prefix + "test2.root");
  sample->add (prefix + "test3.root");
  SH::SampleHandler sh;
  sh.add (sample.release());

  {
    Job job;
    std::unique_ptr<DuplicateChecker> alg (new DuplicateChecker);
    alg->setEventInfoName ("MyEventInfo");
    alg->setOutputTreeName ("summary");
    job.algsAdd (alg.release());
    job.sampleHandler (sh);

    {
      DirectDriver driver;
      driver.submit (job, prefix + "submit1");
      checkHistograms (prefix + "submit1", 30000, 26000, true);
    }
    {
      LocalDriver driver;
      driver.submit (job, prefix + "submit2");
      checkHistograms (prefix + "submit2", 30000, 27000, false);
    }
    RCU::Shell::exec ("cmp " + prefix + "submit1/duplicates " + prefix + "submit2/duplicates");
  }

  {
    Job job;
    std::unique_ptr<DuplicateChecker> alg (new DuplicateChecker);
    alg->setEventInfoName ("MyEventInfo");
    alg->setOutputTreeName ("summary");
    alg->addKnownDuplicatesFile (prefix + "submit1/duplicates");
    job.algsAdd (alg.release());
    job.sampleHandler (sh);

    {
      LocalDriver driver;
      driver.submit (job, prefix + "submit3");
      checkHistograms (prefix + "submit3", 30000, 26000, true);
    }
  }
}

ATLAS_GOOGLE_TEST_MAIN
