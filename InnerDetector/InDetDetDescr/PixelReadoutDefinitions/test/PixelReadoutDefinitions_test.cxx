/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelReadoutDefinitions/test/PixelReadoutDefinitions_test.cxx
 * @author Shaun Roe
 * @date July, 2023
 * @brief Some tests for PixelReadoutDefinitions  enum2uint
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXELREADOUTDEFINITIONS


#include <boost/test/unit_test.hpp>
//

#include "PixelReadoutDefinitions/PixelReadoutDefinitions.h"
using namespace InDetDD;

BOOST_AUTO_TEST_SUITE(PixelReadoutDefinitionsTest)
  BOOST_AUTO_TEST_CASE(enum2uint_test){
    BOOST_TEST(enum2uint(PixelModuleType::IBL_PLANAR) == 1);
    BOOST_TEST(enum2uint(PixelDiodeType::LONG) == 1);
    BOOST_TEST(enum2uint(PixelReadoutTechnology::FEI4) == 1);
  }
BOOST_AUTO_TEST_SUITE_END()
