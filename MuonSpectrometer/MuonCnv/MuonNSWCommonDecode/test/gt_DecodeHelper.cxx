/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @author Jonas Roemer
 * @date April 2023
 * @brief Tests for helper functions
 */

#include <AsgTesting/UnitTest.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include "MuonNSWCommonDecode/NSWDecodeHelper.h"

TEST(DecodeHelper, safeBitSliceWorksShort) {
  const auto bytestream = std::array{0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU};
  const int start = 0;
  const int end = 2;
  const auto res = Muon::safe_bit_slice<std::uint64_t, std::uint32_t>(
      {bytestream.data(), bytestream.size()}, start, end);
  EXPECT_EQ(res, 0b111ULL);
}

TEST(DecodeHelper, safeBitSliceWorksLong) {
  const auto bytestream = std::array{
      0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
      0xFFU,       0x0U,        0x0U,        0x0U,        0x0U};
  const int start = 159;
  const int end = 161;
  const auto res = Muon::safe_bit_slice<std::uint64_t, std::uint32_t>(
      {bytestream.data(), bytestream.size()}, start, end);
  EXPECT_EQ(res, 0b100ULL);
}

TEST(DecodeHelper, safeBitSliceThrowsEndSmallerStart) {
  const auto bytestream = std::array{0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU};
  const int start = 2;
  const int end = 0;
  constexpr auto func = Muon::safe_bit_slice<std::uint64_t, std::uint32_t>;
  EXPECT_THROW(func({bytestream.data(), bytestream.size()}, start, end),
               std::runtime_error);
}

TEST(DecodeHelper, safeBitSliceThrowsEndOutOfRange) {
  const auto bytestream = std::array{0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU};
  const int start = 0;
  const int end = 100;
  constexpr auto func = Muon::safe_bit_slice<std::uint64_t, std::uint32_t>;
  EXPECT_THROW(func({bytestream.data(), bytestream.size()}, start, end),
               std::runtime_error);
}

TEST(DecodeHelper, safeBitSliceThrowsTypeTooSmall) {
  const auto bytestream = std::array{0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU};
  const int start = 31;
  const int end = 64;
  constexpr auto func = Muon::safe_bit_slice<std::uint64_t, std::uint32_t>;
  EXPECT_THROW(func({bytestream.data(), bytestream.size()}, start, end),
               std::runtime_error);
}

TEST(DecodeHelper, decodeAndAdvanceWorks) {
  const auto bytestream = std::array{0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU};
  constexpr auto START = std::size_t{10};
  auto start{START};
  const auto size = std::size_t{5};
  const auto res = Muon::decode_and_advance<std::uint64_t, std::uint32_t>(
      {bytestream.data(), bytestream.size()}, start, size);
  EXPECT_EQ(res, 0b11111);
  EXPECT_EQ(start, START + size);
}

ATLAS_GOOGLE_TEST_MAIN
