// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SmartIF.h"

#include "TInterpreter.h"
#include "CxxUtils/ubsan_suppress.h"

#include <random>
#include <chrono>
#include <cassert>

constexpr size_t numSources = 1000;
constexpr size_t numMessages = 50000;

int main() {
  using namespace std;

  CxxUtils::ubsan_suppress ( []() { TInterpreter::Instance(); } );

  IAppMgrUI* appMgr = Gaudi::createApplicationMgr();
  SmartIF<IProperty> propMgr(appMgr);
  SmartIF<ISvcLocator> svcLoc(appMgr);

  propMgr->setProperty( "JobOptionsType", "NONE" ).
    orThrow("Cannot set property JobOptionsType", "test");

  propMgr->setProperty("MessageSvcType", "TrigMessageSvc").
    orThrow("Cannot set property MessageSvcType", "test");

  if ((appMgr->configure()).isFailure() ||
      (appMgr->initialize()).isFailure()) {
    return 1;
  }

  //--------------------------------------------------
  // Prepare the benchmark
  //--------------------------------------------------
  ServiceHandle<IMessageSvc> msgsvc("TrigMessageSvc/MessageSvc", "test");
  assert( msgsvc.retrieve().isSuccess() );

  std::vector<std::string> names;
  names.reserve(numSources);
  for (size_t i=1; i<=numSources; ++i) {
    names.push_back(std::string("TestMsgSource").append(std::to_string(i)));
  }

  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<size_t> distribution(0, numSources-1);

  //--------------------------------------------------
  // Run the benchmark
  //--------------------------------------------------
  auto t0 = std::chrono::high_resolution_clock::now();

  for (size_t i=0; i<numMessages; ++i) {
    size_t j = distribution(generator);
    msgsvc->reportMessage(names[j], 3, "This is a test message");
  }

  auto t1 = std::chrono::high_resolution_clock::now();
  auto td = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
  msgsvc->reportMessage("Benchmark", 3, std::string("Time: ").append(std::to_string(td.count()).append(" ms")));

  return 0;
}
