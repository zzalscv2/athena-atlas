/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelChargeCondData_test.cxx
 * @author Shaun Roe
 * @date June, 2023
 * @brief basic tests in the Boost framework for PixelChargeCondData
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXEL_CHARGE_CALIB_COND_DATA

#include "CxxUtils/checker_macros.h"
//
#include <boost/test/unit_test.hpp>
//
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;
//
BOOST_AUTO_TEST_SUITE(PixelChargeCalibCondDataTest)
  BOOST_AUTO_TEST_CASE( Construction ){
    BOOST_CHECK_NO_THROW(PixelChargeCalibCondData());
    BOOST_CHECK_NO_THROW(PixelChargeCalibCondData(100));
  }
  
  BOOST_AUTO_TEST_CASE( SetAndGet, * utf::expected_failures(3)){
    std::vector<int> dummy(6,10);
    std::vector<int> thresholds{0,20,40, 100, 4, 30};//normally 16 values, 1 for each chip
    size_t maxModuleHash(100); //normally given by the maximum hash
    InDetDD::PixelDiodeType type = InDetDD::PixelDiodeType::NORMAL;
    unsigned int moduleHash=10;
    PixelChargeCalibCondData calib(maxModuleHash);
    BOOST_CHECK_NO_THROW(calib.setAnalogThreshold(type,moduleHash, std::move(thresholds)));
    unsigned int hashTooBig=200;
    BOOST_CHECK_THROW(calib.setAnalogThreshold(type,hashTooBig, std::move(dummy)), std::out_of_range);
    //
    unsigned int frontEndIdx=1;
    unsigned int frontEndIdxTooBig=15;
    BOOST_CHECK(calib.getAnalogThreshold(type, moduleHash, frontEndIdx) == 20);
    BOOST_CHECK_THROW(calib.getAnalogThreshold(type, hashTooBig, frontEndIdx), std::out_of_range);
    BOOST_CHECK_THROW(calib.getAnalogThreshold(type, moduleHash, frontEndIdxTooBig), std::range_error);
    //set and get for sigma, noise are the same pattern, and all use the same underlying template
    std::vector<int> sigma(6);
    std::iota(sigma.begin(), sigma.end(), 1);
    BOOST_CHECK_NO_THROW(calib.setAnalogThresholdSigma(type,moduleHash, std::move(sigma)));
    BOOST_CHECK(calib.getAnalogThresholdSigma(type, moduleHash, frontEndIdx) == 2);
    //
    std::vector<int> noise(6);
    std::iota(noise.begin(), noise.end(), 2);
    BOOST_CHECK_NO_THROW(calib.setAnalogThresholdNoise(type,moduleHash, std::move(noise)));
    BOOST_CHECK(calib.getAnalogThresholdNoise(type, moduleHash, frontEndIdx) == 3);
    //
    std::vector<int> inTime(6);
    std::iota(inTime.begin(), inTime.end(), 3);
    BOOST_CHECK_NO_THROW(calib.setInTimeThreshold(type,moduleHash, std::move(inTime)));
    BOOST_CHECK(calib.getInTimeThreshold(type, moduleHash, frontEndIdx) == 4);
    //
    std::vector<float> A(6);
    std::iota(A.begin(), A.end(), 0.);
    BOOST_CHECK_NO_THROW(calib.setQ2TotA(type, moduleHash, std::move(A)));
    BOOST_CHECK(calib.getQ2TotA(type, moduleHash, frontEndIdx) == 1.);
    //
    std::vector<float> E(6);
    std::iota(E.begin(), E.end(), 1.);
    BOOST_CHECK_NO_THROW(calib.setQ2TotE(type, moduleHash, std::move(E)));
    BOOST_CHECK(calib.getQ2TotE(type, moduleHash, frontEndIdx) == 2.);
    //
    std::vector<float> C(6);
    std::iota(C.begin(), C.end(), 2.);
    BOOST_CHECK_NO_THROW(calib.setQ2TotC(type, moduleHash, std::move(C)));
    BOOST_CHECK(calib.getQ2TotC(type, moduleHash, frontEndIdx) == 3.);
     //
    std::vector<float> F(6);
    std::iota(F.begin(), F.end(), 3.);
    BOOST_CHECK_NO_THROW(calib.setQ2TotF(type, moduleHash, std::move(F)));
    BOOST_CHECK(calib.getQ2TotF(type, moduleHash, frontEndIdx) == 4.);
    //
    std::vector<float> G(6);
    std::iota(G.begin(), G.end(), 4.);
    BOOST_CHECK_NO_THROW(calib.setQ2TotG(type, moduleHash, std::move(G)));
    BOOST_CHECK(calib.getQ2TotG(type, moduleHash, frontEndIdx) == 5.);
    //
    std::vector<float> res1(6);
    std::iota(res1.begin(), res1.end(), 5.);
    BOOST_CHECK_NO_THROW(calib.setTotRes1(moduleHash, std::move(res1)));
    //
    std::vector<float> res2(6);
    std::iota(res2.begin(), res2.end(), 5.);
    BOOST_CHECK_NO_THROW(calib.setTotRes2(moduleHash, std::move(res2)));
    //
    float Q=5.f;
    float expectedResult = 6.f*Q + 6.f;
    BOOST_TEST(calib.getTotRes(moduleHash, frontEndIdx, Q) == expectedResult);
    //round trip calculation
    float expectedTot = 0.875f;
    BOOST_TEST(calib.getToT(type, moduleHash, frontEndIdx, Q) == expectedTot);
    BOOST_TEST(calib.getCharge(type, moduleHash, frontEndIdx, expectedTot) == Q);
    //
    PixelChargeCalibCondData::CalibrationStrategy strategy = PixelChargeCalibCondData::CalibrationStrategy::RUN3PIX;
    BOOST_CHECK_NO_THROW(calib.setCalibrationStrategy(moduleHash, strategy));
    //this should throw, but does not
    BOOST_CHECK_THROW(calib.setCalibrationStrategy(hashTooBig, strategy), std::out_of_range);
    //IBLModule is a vector of array<float,16>
    std::array<float,16> x;
    std::iota(x.begin(), x.end(), 5.);
    PixelChargeCalibCondData::IBLModule charges{x,x};
    BOOST_CHECK_NO_THROW(calib.setTot2Charges(moduleHash, charges));
    //this should throw, but does not:
    BOOST_CHECK_THROW(calib.setTot2Charges(hashTooBig, charges), std::out_of_range);
    //
    BOOST_TEST((calib.getCalibrationStrategy(moduleHash) == PixelChargeCalibCondData::CalibrationStrategy::RUN3PIX));
    BOOST_CHECK_THROW(calib.getCalibrationStrategy(hashTooBig), std::out_of_range); //should throw; does not
   
    
  }
  BOOST_AUTO_TEST_CASE( CalculatedQuantities, * utf::tolerance(0.00001) ){
    //preamble
    size_t maxModuleHash(100);
    unsigned int hashTooBig=200;
    unsigned int moduleHash=10;
     InDetDD::PixelDiodeType type = InDetDD::PixelDiodeType::NORMAL;
    PixelChargeCalibCondData calib(maxModuleHash);
    unsigned int frontEndIdx=1;//so only need two elements in the vectors
    calib.setAnalogThreshold(type,moduleHash, std::vector<int>{0,1});
    calib.setAnalogThresholdSigma(type,moduleHash, std::vector<int>{1,2});
    calib.setAnalogThresholdNoise(type,moduleHash, std::vector<int>{2,3});
    calib.setInTimeThreshold(type,moduleHash, std::vector<int>{3,4});
    calib.setQ2TotA(type, moduleHash, std::vector<float>{0.f, 1.f});
    calib.setQ2TotE(type, moduleHash, std::vector<float>{1.f, 2.f});
    calib.setQ2TotC(type, moduleHash, std::vector<float>{2.f, 3.f});
    calib.setQ2TotF(type, moduleHash, std::vector<float>{3.f, 4.f});
    calib.setQ2TotG(type, moduleHash, std::vector<float>{4.f, 5.f});
    calib.setTotRes1(moduleHash, std::vector<float>{5.f, 6.f});
    calib.setTotRes2(moduleHash, std::vector<float>{5.f, 6.f});
    //
    float Q=5.f;
    float expectedResult = 6.f*Q + 6.f;
    BOOST_TEST(calib.getTotRes(moduleHash, frontEndIdx, Q) == expectedResult);
    //round trip calculation
    float expectedTot = 0.875f;
    BOOST_TEST(calib.getToT(type, moduleHash, frontEndIdx, Q) == expectedTot);
    BOOST_TEST(calib.getCharge(type, moduleHash, frontEndIdx, expectedTot) == Q);
    //
    std::array<float,16> x;
    std::iota(x.begin(), x.end(), 5.);
    PixelChargeCalibCondData::IBLModule charges{x,x};
    calib.setTot2Charges(moduleHash, charges);
    unsigned int ToT =1;
    //
    //ToT indices start from 1, not zero, and have 1 subtracted when indexing into the array
    BOOST_TEST(calib.getChargeLUTFEI4(moduleHash, frontEndIdx, ToT) == 5.f);
    BOOST_CHECK_THROW(calib.getChargeLUTFEI4(hashTooBig, frontEndIdx, ToT), std::range_error);
    BOOST_CHECK_THROW(calib.getChargeLUTFEI4(moduleHash, frontEndIdx, 0), std::runtime_error);//too small
    BOOST_TEST(calib.getChargeLUTFEI4(moduleHash, frontEndIdx, 16) == 20.f);//16 is OK
    BOOST_CHECK_THROW(calib.getChargeLUTFEI4(moduleHash, frontEndIdx, 17), std::runtime_error);//too big
    //
    BOOST_TEST(calib.getToTLUTFEI4(moduleHash, frontEndIdx, 20.f) == 16.f);
    BOOST_CHECK_THROW(calib.getToTLUTFEI4(hashTooBig, frontEndIdx, 20.f), std::range_error);
    //method 'clear()' is declared, never defined
  }
BOOST_AUTO_TEST_SUITE_END();
