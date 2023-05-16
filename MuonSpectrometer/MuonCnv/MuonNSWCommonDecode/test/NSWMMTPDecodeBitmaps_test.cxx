/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file MuonNSWCommonDecode/test/NSWMMTPDecodeBitmaps_test.cxx
 * @author Shaun Roe
 * @date February 2023
 * @brief Some tests for bit_slice
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE MuonNSWCommonDecode



#include <boost/test/unit_test.hpp>

#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"
#include <cstdint>


BOOST_AUTO_TEST_SUITE(NSWMMTPDecodeBitmaps)
  BOOST_AUTO_TEST_CASE(NSWMMTPDecodeBitmaps_Functionality){
    //no need to template this
    constexpr auto bitSlice=Muon::bit_slice<uint64_t,uint32_t>;
    //fill a posiible bytestream fragment with all 1's
    uint32_t bytestream[3]={0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU};
    int start=0;
    int end=2; //inclusive interval, so three bits
    BOOST_TEST(bitSlice(bytestream, start, end) == 0b111ULL);
  }

  BOOST_AUTO_TEST_CASE(NSWMMTPDecodeBitmaps_Functionality_Long){
    //no need to template this
    constexpr auto bitSlice=Muon::bit_slice<uint64_t,uint32_t>;
    uint32_t bytestream[10]={0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFU, 0x0U, 0x0U, 0x0U, 0x0U};
    int start=159;
    int end=161; //inclusive interval, so three bits
    BOOST_TEST(bitSlice(bytestream, start, end) == 0b100ULL);
  }

BOOST_AUTO_TEST_SUITE_END()