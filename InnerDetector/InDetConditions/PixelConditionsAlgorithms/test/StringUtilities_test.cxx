/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file PixelConditionsAlgorithms/test/StringUtilities_test.cxx
 * @author Shaun Roe
 * @date Aug, 2022
 * @brief Some tests for StringUtilities 
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PixelConditionsAlgorithms


#include <boost/test/unit_test.hpp>
//
#include <utility>
#include <vector>

#include "../src/StringUtilities.h"
#include "DeadMapTestString.h"
#include <stdexcept>

//also check: ctest -R Trigger_athenaHLT_v1Cosmic
//after checking out CITest

using ValueType = std::pair<int, int>;
std::ostream & 
operator<<(std::ostream & o, const ValueType& v){
  o<<"{"<<v.first<<", "<<v.second<<"}";
  return o;
}

namespace boost::test_tools::tt_detail {
  template<>           
  struct print_log_value<ValueType > {
    void operator()( std::ostream& os, ValueType const& v){
      ::operator<<(os,v);
    }
  };                                                          
}


BOOST_AUTO_TEST_SUITE(PixelConditionsAlgorithmsStringUtilitiesTest)
  BOOST_AUTO_TEST_CASE(parseDeadMapString){
    const std::string emptyString;
    const auto result=PixelConditionsAlgorithms::parseDeadMapString(emptyString);
    BOOST_TEST(result.empty(),"Parsing empty string should give empty result");
    const std::string stringTooSmall=R"({"":0})";
    const auto result2=PixelConditionsAlgorithms::parseDeadMapString(stringTooSmall);
    BOOST_TEST(result2.empty(),"Parsing string which is too small should give empty result");
    const std::string oneEntry=R"({"1":100})";
    const auto oneResult=PixelConditionsAlgorithms::parseDeadMapString(oneEntry);
    BOOST_TEST(oneResult[0] == ValueType(1,100));
    BOOST_TEST(oneResult.size() == 1);
    const std::string validTestString{PixelConditionsAlgorithms::testString};
    const auto run3Result=PixelConditionsAlgorithms::parseDeadMapString(validTestString);
    BOOST_TEST(run3Result.size() == 164, "Test string has 164 entries");
    //first entry is {12, 0}, the zero will cause the module status to be written as '1'
    //in the Algorithm
    BOOST_TEST(run3Result[0] == ValueType(12,0));
    //check entry three which should be {64, 256}, so should be a chip status
    BOOST_TEST(run3Result[3] == ValueType(64,256));
    BOOST_TEST_MESSAGE("Check results with the one-off anomalous db string format");
    const std::string anomaly =   R"({"12",0:"19",0:"53",0:"64",256:"65",0:"139",0})";
    const auto anomalousStringResult = PixelConditionsAlgorithms::parseDeadMapString(anomaly);
    BOOST_TEST(anomalousStringResult.size() == 6);
    BOOST_TEST(anomalousStringResult[3] == ValueType(64,256));
    const std::string brokenDbString = R"({"494":512,"501":1024,"503":0,517:128})";
    BOOST_CHECK_THROW(PixelConditionsAlgorithms::parseDeadMapString(brokenDbString), std::runtime_error);
  }


BOOST_AUTO_TEST_SUITE_END()