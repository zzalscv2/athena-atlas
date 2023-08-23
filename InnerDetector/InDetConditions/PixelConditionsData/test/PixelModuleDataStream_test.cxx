/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file PixelConditionsAlgorithms/test/PixelModuleDataStream_test.cxx
 * @author Shaun Roe
 * @date Aug 2023
 * @brief Some tests for PixelModuleDataStream in the Boost framework
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXELCONDITIONSDATA

#include <boost/test/unit_test.hpp>
//
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/EventContext.h"
//
#include "CxxUtils/checker_macros.h"

#include "CxxUtils/checker_macros.h"

#include "PixelConditionsData/PixelModuleData.h"
#include "PixelConditionsData/PixelModuleDataStream.h"
#include <string>
#include <sstream>
#include <fstream>
#include "PathResolver/PathResolver.h"

namespace utf = boost::unit_test;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

BOOST_AUTO_TEST_SUITE(PixelModuleDataStreamTest )

  BOOST_AUTO_TEST_CASE( PixelModuleDataStreamInsertionTest ){
    PixelModuleData pmd; //all default values
    //the following is a raw string literal, will (necessarily) preserve formatting "as-is"
    const std::string expected=R"(#bunchSpace
25
#barrelNumberOfBCID
1 1 1 1 
#endcapNumberOfBCID
1 1 1 
#DBMNumberOfBCID
1 1 1 
#barrelTimeOffset
5 5 5 5 
#endcapTimeOffset
5 5 5 
#DBMTimeOffset
5 5 5 
#barrelTimeJitter
0 0 0 0 
#endcapTimeJitter
0 0 0 
#DBMTimeJitter
0 0 0 
#defaultBarrelAnalogThreshold
-1 -1 -1 -1 
#defaultEndcapAnalogThreshold
-1 -1 -1 
#defaultDBMAnalogThreshold
-1 -1 -1 
#defaultBarrelAnalogThresholdSigma
45 35 30 30 
#defaultEndcapAnalogThresholdSigma
30 30 30 
#defaultDBMAnalogThresholdSigma
70 70 70 
#defaultBarrelAnalogThresholdNoise
130 150 160 160 
#defaultEndcapAnalogThresholdNoise
150 150 150 
#defaultDBMAnalogThresholdNoise
190 190 190 
#defaultBarrelInTimeThreshold
2000 5000 5000 5000 
#defaultEndcapInTimeThreshold
5000 5000 5000 
#defaultDBMInTimeThreshold
1200 1200 1200 
#barrelToTThreshold
-1 5 5 5 
#endcapToTThreshold
5 5 5 
#DBMToTThreshold
-1 -1 -1 
#barrelCrossTalk
0.3 0.12 0.12 0.12 
#endcapCrossTalk
0.06 0.06 0.06 
#DBMCrossTalk
0.06 0.06 0.06 
#barrelThermalNoise
160 160 160 160 
#endcapThermalNoise
160 160 160 
#DBMThermalNoise
160 160 160 
#barrelNoiseOccupancy
5e-08 5e-08 5e-08 5e-08 
#endcapNoiseOccupancy
5e-08 5e-08 5e-08 
#DBMNoiseOccupancy
5e-08 5e-08 5e-08 
#barrelDisableProbability
0.009 0.009 0.009 0.009 
#endcapDisableProbability
0.009 0.009 0.009 
#DBMDisableProbability
0.009 0.009 0.009 
#barrelNoiseShape
0 0 0 0 0.2204 0.5311 0.7493 0.8954 0.998 1 
0 0 0 0 0 0 0.2418 0.4397 0.5858 0.6949 0.7737 0.8414 0.8959 0.9414 0.9828 1 
0 0 0 0 0 0 0.2418 0.4397 0.5858 0.6949 0.7737 0.8414 0.8959 0.9414 0.9828 1 
#endcapNoiseShape
0 0 0 0 0 0 0.2418 0.4397 0.5858 0.6949 0.7737 0.8414 0.8959 0.9414 0.9828 1 
0 0 0 0 0 0 0.2418 0.4397 0.5858 0.6949 0.7737 0.8414 0.8959 0.9414 0.9828 1 
0 0 0 0 0 0 0.2418 0.4397 0.5858 0.6949 0.7737 0.8414 0.8959 0.9414 0.9828 1 
#DBMNoiseShape
0 0.033 0 0.3026 0.5019 0.676 0.8412 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 1 
0 0.033 0 0.3026 0.5019 0.676 0.8412 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 1 
0 0.033 0 0.3026 0.5019 0.676 0.8412 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 0.9918 1 
#FEI3BarrelLatency
0 151 256 256 
#FEI3EndcapLatency
256 256 256 
#FEI3BarrelHitDuplication
0 0 0 0 
#FEI3EndcapHitDuplication
0 0 0 
#FEI3BarrelSmallHitToT
-1 -1 -1 -1 
#FEI3EndcapSmallHitToT
-1 -1 -1 
#FEI3BarrelTimingSimTune
-1 2015 2015 2015 
#FEI3EndcapTimingSimTune
2015 2015 2015 
#FEI4BarrelHitDiscConfig
2 2 2 
#FEI4EndcapHitDiscConfig
2 2 2 
#scaleFEI4
1
#useFEI4SpecialScalingFunction
1
#FEI4ToTSigma
0 0.5 0.5 0.5 0.5 0.5 0.6 0.6 0.6 0.6 0.65 0.7 0.75 0.8 0.8 0.8 0.8 
#paramA
70.2
#paramE
-3561.25
#paramC
26000
#doLinearExtrapolation
0
#barrelLorentzAngleCorr
1 1 1 1 
#endcapLorentzAngleCorr
1 1 1 
#biasVoltage
150
#temperature
-7
#barrelBiasVoltage
80 350 200 150 
#endcapBiasVoltage
150 150 150 
#DBMBiasVoltage
500 500 500 
#fluenceLayer
8e+13 1.61e+14 7.1e+13 4.8e+13 
#radSimFluenceMapList
PixelDigitization/maps_IBL_PL_80V_fl0_8e14.root PixelDigitization/maps_PIX_350V_fl1_61e14.root PixelDigitization/maps_PIX_200V_fl0_71e14.root PixelDigitization/maps_PIX_150V_fl0_48e14.root 
#fluenceLayer3D
5e+15 
#radSimFluenceMapList3D
PixelDigitization/TCAD_IBL_3Dsensors_efields/phi_5e15_160V.root 
#cablingMapToFile
0
#cablingMapFileName
PixelCabling/Pixels_Atlas_IdMapping_2016.dat#distortionInputSource
4
#distortionVersion
-1
#distortionR1
0.0001
#distortionR2
0.0001
#distortionTwist
0.0005
#distortionMeanR
0.00012
#distortionRMSR
8e-05
#distortionMeanTwist
-0.0005
#distortionRMSTwist
0.0008
#distortionWriteToFile
0
#distortionFileName
/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/TrackingCP/PixelDistortions/PixelDistortionsData_v2_BB.txt)";
    std::stringstream s;
    s << pmd;
    BOOST_TEST (s.str() == expected);
  }
  BOOST_AUTO_TEST_CASE( PixelModuleDataStreamExtractionTest, *utf::tolerance( 1e-7 ) ){
    PixelModuleData pmd; 
    const std::string fname="testInput.txt";
    std::string fullName = PathResolver::find_file (fname, "DATAPATH");
    std::ifstream iss(fullName);
    BOOST_TEST_REQUIRE(iss.good());
    iss>>pmd;
    const int bec = 0;
    const int layer = 1;
    BOOST_TEST(pmd.getBunchSpace() == 26.0);
    BOOST_TEST(pmd.getNumberOfBCID(bec,layer) == 2);
    BOOST_TEST(pmd.getTimeOffset(bec,layer) == 5.);
    BOOST_TEST(pmd.getTimeJitter(bec,layer) == 8);
    BOOST_TEST(pmd.getDefaultAnalogThreshold(bec,layer) == 1);
    BOOST_TEST(pmd.getDefaultAnalogThresholdSigma(bec, layer) == 100);
    BOOST_TEST(pmd.getDefaultAnalogThresholdNoise(bec, layer) == 1000);
    BOOST_TEST(pmd.getDefaultInTimeThreshold(bec, layer) == 4);
    BOOST_TEST(pmd.getToTThreshold(bec, layer) == 5);
    BOOST_TEST(pmd.getCrossTalk(bec, layer) == 0.10);
    BOOST_TEST(pmd.getThermalNoise(bec, layer) == 200);
    BOOST_TEST(pmd.getNoiseOccupancy(bec, layer) ==  1000);
    BOOST_TEST(pmd.getDisableProbability(bec, layer)  == 0.9);
    const std::vector<float> expected{1, 0, 0, 0, 0, 0, 0.2418, 0.4397, 0.5858, 0.6949, 0.7737, 0.8414, 0.8959, 0.9414, 0.9828, 1};
    BOOST_TEST(pmd.getNoiseShape(bec, layer) == expected, boost::test_tools::per_element());
    BOOST_TEST(pmd.getFEI3Latency(bec, layer) == 200);
    BOOST_TEST(pmd.getFEI3HitDuplication(bec, layer) == 1);
    BOOST_TEST(pmd.getFEI3SmallHitToT(bec, layer) == 1);
    BOOST_TEST(pmd.getFEI3TimingSimTune(bec, layer) == 2023);
    BOOST_TEST(pmd.getFEI4HitDiscConfig(bec, layer) == 9);
    BOOST_TEST(pmd.getFEI4ChargScaling() == 0.9);
    BOOST_TEST(pmd.getUseFEI4SpecialScalingFunction() == false);
    // untested:   double getFEI4ToTSigma(int tot) const;
    BOOST_TEST(pmd.getDefaultQ2TotA() == 100.);
    BOOST_TEST(pmd.getDefaultQ2TotE() == 200.);
    BOOST_TEST(pmd.getDefaultQ2TotC() == 300.);
    BOOST_TEST(pmd.getPIXLinearExtrapolation() == false);
    BOOST_TEST(pmd.getPIXLinearExtrapolation() == false);
    //
    BOOST_TEST(pmd.getFluenceLayer()[0] == 7e+13);
    //
    BOOST_TEST(pmd.getDistortionFileName() == "sroe.txt");
     
  }
BOOST_AUTO_TEST_SUITE_END();



