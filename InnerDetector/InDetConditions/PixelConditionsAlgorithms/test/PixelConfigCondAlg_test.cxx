/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file PixelConditionsAlgorithms/test/PixelConfigCondAlg_test.cxx
 * @author Shaun Roe
 * @date Aug 2023
 * @brief Some tests for PixelConfigCondAlg in the Boost framework
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

#include "src/PixelConfigCondAlg.h"
#include "StoreGate/ReadHandleKey.h"
#include <string>
namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

struct GaudiKernelFixture{
  static ISvcLocator* svcLoc;
  const std::string jobOpts{};
  GaudiKernelFixture(const std::string & jobOptionFile = "PixelConfigCondAlg_test.txt"):jobOpts(jobOptionFile){
    CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); } );
    if (svcLoc==nullptr){
      std::string fullJobOptsName="PixelConditionsAlgorithms/" + jobOpts;
      Athena_test::initGaudi(fullJobOptsName, svcLoc);
    }
  }
};

ISvcLocator* GaudiKernelFixture::svcLoc = nullptr;

constexpr double operator"" _m ( long double b ){
    return b*0.001;
} 

//from EventIDBase
typedef unsigned int number_type;
typedef uint64_t     event_number_t;

std::pair <EventIDBase, EventContext>
getEvent(number_type runNumber, number_type timeStamp){
  event_number_t eventNumber(0);
  EventIDBase eid(runNumber, eventNumber, timeStamp);
  EventContext ctx;
  ctx.setEventID (eid);
  return {eid, ctx};
}

std::pair<const PixelModuleData *, CondCont<PixelModuleData> *>
getData(const EventIDBase & eid, ServiceHandle<StoreGateSvc> & conditionStore){
  //ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "PixelModuleData");
  CondCont<PixelModuleData> * cc{};
  const PixelModuleData* data = nullptr;
  const EventIDRange* range2p = nullptr;
  if (not conditionStore->retrieve (cc, "PixelModuleData").isSuccess()){
    return {nullptr, nullptr};
  }
  cc->find (eid, data, &range2p);
  return {data,cc};
}

bool
canRetrievePixelModuleData(ServiceHandle<StoreGateSvc> & conditionStore){
  CondCont<PixelModuleData> * cc{};
  if (not conditionStore->retrieve (cc, "PixelModuleData").isSuccess()){
    return false;
  }
  return true;
}

BOOST_AUTO_TEST_SUITE(PixelConfigCondAlgTest )
  GaudiKernelFixture g;

  BOOST_AUTO_TEST_CASE( SanityCheck ){
    const bool svcLocatorIsOk=(g.svcLoc != nullptr);
    BOOST_TEST(svcLocatorIsOk);
  }
  BOOST_AUTO_TEST_CASE(ExecuteOptions, *utf::tolerance( 1e-5 )){
    PixelConfigCondAlg a("MyAlg", g.svcLoc);
    a.addRef();
    //add property definitions for later (normally in job opts)
    BOOST_TEST(a.setProperty("BarrelThermalNoise","{110.0,120.0,130.0,140.0}").isSuccess());
    BOOST_TEST(a.setProperty("DefaultBarrelAnalogThresholdSigma","{10,20,30,40}").isSuccess());
    //
    BOOST_TEST(a.sysInitialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "PixelModuleData");
    CondCont<PixelModuleData> * cc{};
    BOOST_TEST( canRetrievePixelModuleData(conditionStore));
    //execute for the following event:
    EventContext ctx;
    //
    number_type runNumber(222222 - 100);
    event_number_t eventNumber(0);
    number_type timeStamp(0);
    EventIDBase eidRun1 (runNumber, eventNumber, timeStamp);
    ctx.setEventID (eidRun1);
    BOOST_TEST(a.execute(ctx).isSuccess());
     //now we have something in store to retrieve
    BOOST_TEST( conditionStore->retrieve (cc, "PixelModuleData").isSuccess() );
    const PixelModuleData* data = nullptr;
    const EventIDRange* range2p = nullptr;
    BOOST_TEST (cc->find (eidRun1, data, &range2p));
    //
    //look at the ones we changed from default, to check
    constexpr int barrel_ec=0;
    constexpr int layer=1;
    BOOST_TEST(data->getThermalNoise(barrel_ec, layer) == 120.0f);
    BOOST_TEST(data->getDefaultAnalogThresholdSigma(barrel_ec, layer) == 20.0f);
    
    BOOST_TEST(data->getFluenceLayer()[layer] == 44000000000000);

    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
  }
   BOOST_AUTO_TEST_CASE(ExecuteRun2Mc15, *utf::tolerance( 1e-5 )){
    PixelConfigCondAlg a("MyAlg", g.svcLoc);
    a.addRef();
    BOOST_TEST(a.initialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "PixelModuleData");
    BOOST_TEST(canRetrievePixelModuleData(conditionStore));
    //execute for the following event:
    const auto & [eid,ctx] = getEvent(240000 - 10, 1448928000u); //December 1, 2015, in seconds of unix epoch
    BOOST_TEST(a.execute(ctx).isSuccess());
    const auto & [data, cc] = getData(eid, conditionStore);
    constexpr int barrel_ec=0;
    constexpr int layer=1;
    BOOST_TEST(data->getThermalNoise(barrel_ec, layer) == 160.0f);
    BOOST_TEST(data->getDefaultAnalogThresholdSigma(barrel_ec, layer) == 35.0f);
    BOOST_TEST(data->getFluenceLayer()[layer] == 161000000000000);
    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
 }
  //
  BOOST_AUTO_TEST_CASE(ExecuteRun2, *utf::tolerance( 1e-5 )){
    BOOST_TEST((g.svcLoc != nullptr));
    PixelConfigCondAlg a("MyAlg", g.svcLoc);
    a.addRef();
    BOOST_TEST(a.initialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "PixelModuleData");
    BOOST_TEST( canRetrievePixelModuleData(conditionStore) );
    //execute for the following event:
    const auto & [eid,ctx] = getEvent(240000 + 10, 1438387200u); //August 1, 2015, in seconds of unix epoch
    BOOST_TEST(a.execute(ctx).isSuccess());
    const auto & [data, cc] = getData(eid, conditionStore);
    constexpr int barrel_ec=0;
    constexpr int layer=1;
    BOOST_TEST(data->getThermalNoise(barrel_ec, layer) == 160.0f);
    BOOST_TEST(data->getDefaultAnalogThresholdSigma(barrel_ec, layer) == 35.0f);
    BOOST_TEST(data->getFluenceLayer()[layer] == 161000000000000);
    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
 }
 //
  BOOST_AUTO_TEST_CASE(ExecuteRun2Mc16a, *utf::tolerance( 1e-5 )){
    BOOST_TEST((g.svcLoc != nullptr));
    PixelConfigCondAlg a("MyAlg", g.svcLoc);
    a.addRef();
    BOOST_TEST(a.initialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "PixelModuleData");
    BOOST_TEST( canRetrievePixelModuleData(conditionStore) );
    const auto & [eid,ctx] = getEvent(300000 - 10, 1470009600u); ////August 1, 2016, in seconds of unix epoch
    BOOST_TEST(a.execute(ctx).isSuccess());
    const auto & [data, cc] = getData(eid, conditionStore);
    constexpr int barrel_ec=0;
    constexpr int layer=1;
    BOOST_TEST(data->getThermalNoise(barrel_ec, layer) == 160.0f);
    BOOST_TEST(data->getDefaultAnalogThresholdSigma(barrel_ec, layer) == 35.0f);
    BOOST_TEST(data->getFluenceLayer()[layer] == 161000000000000);
    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
 }
 BOOST_AUTO_TEST_CASE(ExecuteRun2Mc16d, *utf::tolerance( 1e-5 )){
    BOOST_TEST((g.svcLoc != nullptr));
    PixelConfigCondAlg a("MyAlg", g.svcLoc);
    a.addRef();
    BOOST_TEST(a.initialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "PixelModuleData");
    BOOST_TEST( canRetrievePixelModuleData(conditionStore) );
    const auto & [eid,ctx] = getEvent(310000 - 10, 1501545600u); //August 1, 2017, in seconds of unix epoch
    BOOST_TEST(a.execute(ctx).isSuccess());
    const auto & [data, cc] = getData(eid, conditionStore);
    constexpr int barrel_ec=0;
    constexpr int layer=1;
    BOOST_TEST(data->getThermalNoise(barrel_ec, layer) == 160.0f);
    BOOST_TEST(data->getDefaultAnalogThresholdSigma(barrel_ec, layer) == 35.0f);
    BOOST_TEST(data->getFluenceLayer()[layer] == 342000000000000);
    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
 }
 BOOST_AUTO_TEST_CASE(ExecuteRun2Mc16e, *utf::tolerance( 1e-5 )){
    BOOST_TEST((g.svcLoc != nullptr));
    PixelConfigCondAlg a("MyAlg", g.svcLoc);
    a.addRef();
    BOOST_TEST(a.initialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "PixelModuleData");
    BOOST_TEST( canRetrievePixelModuleData(conditionStore) );
    const auto & [eid,ctx] = getEvent(330000 - 10, 1533081600u); //August 1, 2018, in seconds of unix epoch
    BOOST_TEST(a.execute(ctx).isSuccess());
    const auto & [data, cc] = getData(eid, conditionStore);
    constexpr int barrel_ec=0;
    constexpr int layer=1;
    BOOST_TEST(data->getThermalNoise(barrel_ec, layer) == 160.0f);
    BOOST_TEST(data->getDefaultAnalogThresholdSigma(barrel_ec, layer) == 35.0f);
    BOOST_TEST(data->getFluenceLayer()[layer] == 519000000000000);
    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
 }

 BOOST_AUTO_TEST_CASE(ExecuteRun3, *utf::tolerance( 1e-5 )){
    BOOST_TEST((g.svcLoc != nullptr));
    PixelConfigCondAlg a("MyAlg", g.svcLoc);
    a.addRef();
    BOOST_TEST(a.initialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "PixelModuleData");
    BOOST_TEST( canRetrievePixelModuleData(conditionStore) );
    const auto & [eid,ctx] = getEvent(450000 - 10, 1659312035u); //August 1, 2022, in seconds of unix epoch
    BOOST_TEST(a.execute(ctx).isSuccess());
    const auto & [data, cc] = getData(eid, conditionStore);
    constexpr int barrel_ec=0;
    constexpr int layer=1;
    BOOST_TEST(data->getThermalNoise(barrel_ec, layer) == 160.0f);
    BOOST_TEST(data->getDefaultAnalogThresholdSigma(barrel_ec, layer) == 35.0f);
    BOOST_TEST(data->getFluenceLayer()[layer] == 680000000000000);
    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
 }
  BOOST_AUTO_TEST_CASE(ExecuteRun3Mc23c, *utf::tolerance( 1e-5 )){
    BOOST_TEST((g.svcLoc != nullptr));
    PixelConfigCondAlg a("MyAlg", g.svcLoc);
    a.addRef();
    BOOST_TEST(a.initialize().isSuccess() );
    ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "PixelModuleData");
    BOOST_TEST( canRetrievePixelModuleData(conditionStore) );
    const auto & [eid,ctx] = getEvent(450000 + 10, 1690848000u); //August 1, 2023, in seconds of unix epoch
    BOOST_TEST(a.execute(ctx).isSuccess());
    const auto & [data, cc] = getData(eid, conditionStore);
    constexpr int barrel_ec=0;
    constexpr int layer=1;
    BOOST_TEST(data->getThermalNoise(barrel_ec, layer) == 160.0f);
    BOOST_TEST(data->getDefaultAnalogThresholdSigma(barrel_ec, layer) == 35.0f);
    BOOST_TEST(data->getFluenceLayer()[layer] == 920000000000000);
    BOOST_TEST(conditionStore->removeDataAndProxy(cc).isSuccess());
    BOOST_TEST(a.sysFinalize().isSuccess() );
 }
BOOST_AUTO_TEST_SUITE_END();
