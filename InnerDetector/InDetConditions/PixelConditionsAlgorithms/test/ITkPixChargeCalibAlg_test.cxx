/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file PixelConditionsAlgorithms/test/ITkPixChargeCalibAlg_test.cxx
 * @author Shaun Roe
 * @date Sept 2023
 * @brief Some tests for ITkPixChargeCalibAlg in the Boost framework
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXELCONDITIONSALGORITHMS

#include <boost/test/unit_test.hpp>
//
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/EventContext.h"
//
#include "CxxUtils/checker_macros.h"

#include "TestTools/initGaudi.h"
#include "TInterpreter.h"
#include "CxxUtils/ubsan_suppress.h"
#include "CxxUtils/checker_macros.h"

#include "src/ITkPixChargeCalibAlg.h"
#include "StoreGate/ReadHandleKey.h"

// #include "PixelReadoutGeometry/PixelDetectorManager.h"
#include <string>

namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

struct GaudiKernelFixture{
  static ISvcLocator* svcLoc;
  const std::string jobOpts{};
  GaudiKernelFixture(const std::string & jobOptionFile = "ITkPixChargeCalibAlg_test.txt"):jobOpts(jobOptionFile){
    CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); } );
    if (svcLoc==nullptr){
      std::string fullJobOptsName="PixelConditionsAlgorithms/" + jobOpts;
      Athena_test::initGaudi(fullJobOptsName, svcLoc);
    }
  }
};

ISvcLocator* GaudiKernelFixture::svcLoc = nullptr;


//from EventIDBase
typedef unsigned int number_type;
typedef uint64_t     event_number_t;


std::pair<const PixelChargeCalibCondData *, CondCont<PixelChargeCalibCondData> *>
getData(const EventIDBase & eid, ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<PixelChargeCalibCondData> * cc{};
  const PixelChargeCalibCondData* data = nullptr;
  const EventIDRange* range2p = nullptr;
  if (not conditionStore->retrieve (cc, "PixelChargeCalibCondData").isSuccess()){
    return {nullptr, nullptr};
  }
  cc->find (eid, data, &range2p);
  return {data,cc};
}

bool
canRetrieveITkCalibData(ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<PixelChargeCalibCondData> * cc{};
  if (not conditionStore->retrieve (cc, "PixelChargeCalibCondData").isSuccess()){
    return false;
  }
  return true;
}

BOOST_AUTO_TEST_SUITE(ITkPixChargeCalibAlgTest )
  GaudiKernelFixture g;

  BOOST_AUTO_TEST_CASE( SanityCheck ){
    const bool svcLocatorIsOk=(g.svcLoc != nullptr);
    BOOST_TEST(svcLocatorIsOk);
  }
  BOOST_AUTO_TEST_CASE(Initialise){
    ITkPixChargeCalibAlg a("MyAlg", g.svcLoc);
    a.addRef();
    //add property definitions for later (normally in job opts)
    BOOST_TEST(a.setProperty("WriteKey","PixelChargeCalibCondData").isSuccess());
    BOOST_TEST(a.sysInitialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "PixelChargeCalibCondData");
    BOOST_TEST( canRetrieveITkCalibData(conditionStore));
    BOOST_TEST(a.sysFinalize().isSuccess() );
  }
  //need test for execute, but thats more work... to be done!
   
 
 
BOOST_AUTO_TEST_SUITE_END();
