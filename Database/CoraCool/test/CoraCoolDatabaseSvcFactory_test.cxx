/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file /Database/CoraCool/test/CoraCoolDatabaseSvcFactory_test.cxx
 * @author Shaun Roe
 * @date June, 2019
 * @brief A test for CoraCoolDatabaseSvcFactory in the Boost framework
 */

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_CORACOOL

#include <boost/test/unit_test.hpp>

#include "../CoraCool/CoraCoolDatabaseSvcFactory.h"
#include "../CoraCool/CoraCoolDatabaseSvc.h"

BOOST_AUTO_TEST_SUITE(CoraCoolDatabaseSvcFactoryTest)
  BOOST_AUTO_TEST_CASE(ProduceDatabaseSvc){
    BOOST_CHECK_NO_THROW(CoraCoolDatabaseSvcFactory());
    CoraCoolDatabaseSvcFactory svcFactory;
    __attribute__((__unused__))
    CoraCoolDatabaseSvc & svc=svcFactory.databaseService();
    BOOST_TEST(true, "fake test following instantiation");
  }
BOOST_AUTO_TEST_SUITE_END()
