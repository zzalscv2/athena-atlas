/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file InDetPhysValMonitoring/test/IDPVM_GaudiFixtureBase.h
 * @author Shaun Roe
 * @date May 2020
 * @brief Base class for initialising Gaudi in fixtures
 */

#ifndef InDetPhysValMonitoring_IDPVM_GaudiFixtureBase_h
#define InDetPhysValMonitoring_IDPVM_GaudiFixtureBase_h


#include "TestTools/initGaudi.h"
#include "TInterpreter.h"
#include "CxxUtils/ubsan_suppress.h"
#include <string>
#include <mutex>


struct IDPVM_GaudiFixtureBase{
  ISvcLocator* svcLoc{};
  const std::string jobOpts{};
  IDPVM_GaudiFixtureBase(const std::string & jobOptionFile = "IDPVM_Test.txt"):jobOpts(jobOptionFile){
    CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); } );
    static std::once_flag flag;
    auto init = [&]() {
      std::string fullJobOptsName="InDetPhysValMonitoring/" + jobOpts;
      Athena_test::initGaudi(fullJobOptsName, svcLoc);
    };
    std::call_once (flag, init);
  }
};


#endif
