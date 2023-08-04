/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <AsgMessaging/MessageCheck.h>
#include <EventLoop/Job.h>
#include <EventLoop/LSFDriver.h>
#include <EventLoopTest/UnitTestConfig.h>
#include <EventLoopTest/UnitTestFixture.h>
#include <EventLoop/VomsProxySvc.h>
#include <RootCoreUtils/ShellExec.h>
#include <SampleHandler/DiskWriterXRD.h>
#include <gtest/gtest.h>

#ifdef ROOTCORE
#include <xAODRootAccess/Init.h>
#endif

//
// method implementations
//

using namespace asg::msgUserCode;

struct MyUnitTestConfig : public EL::UnitTestConfig
{
  MyUnitTestConfig ()
  {
    static const std::shared_ptr<EL::LSFDriver>
      driver (new EL::LSFDriver);
    driver->options()->setString (EL::Job::optSubmitFlags, "-L /bin/zsh -q test -W 3:00");
    m_driver = driver;
  }



  virtual std::unique_ptr<SH::DiskWriter>
  make_file_writer (const std::string& name) const override
  {
    static std::string date = RCU::Shell::exec_read ("date +%Y%m%d-%H%M");
    RCU_ASSERT (!date.empty());
    std::string path = "root://eosatlas.cern.ch//eos/atlas/user/k/krumnack/el_ut-" + date + "/" + name;
    return std::unique_ptr<SH::DiskWriterXRD>
      (new SH::DiskWriterXRD (path));
   }



  virtual void setupJob (EL::Job& job) const override
  {
    job.algsAdd (new EL::VomsProxySvc);
  }
};

namespace EL
{
  // this has to be SLOW, since it requires the entire release to have
  // been build for it to work
  INSTANTIATE_TEST_SUITE_P(MANUAL_LSFDriverTest, UnitTestFixture,
                           ::testing::Values(MyUnitTestConfig()));
}

int main (int argc, char **argv)
{
#ifdef ROOTCORE
  StatusCode::enableFailure();
  ANA_CHECK (xAOD::Init ());
#endif
  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS();
}
