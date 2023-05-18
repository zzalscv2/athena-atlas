/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @author Jonas Roemer
 * @date April 2023
 * @brief Tests for STG TP packet decoder
 */

#include <memory>
#include <stdexcept>

#include <gtest/gtest.h>

#include <AsgTesting/UnitTest.h>

#include "MuonNSWCommonDecode/NSWTriggerSTGL1AElink.h"

class NSWTriggerSTGL1AElinkTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const auto data = std::vector(std::cbegin(s_DATA), std::cend(s_DATA));
    m_decoder = std::make_unique<Muon::nsw::NSWTriggerSTGL1AElink>(data.data(), s_REMAINING);
  }

  std::unique_ptr<Muon::nsw::NSWTriggerSTGL1AElink> m_decoder{nullptr};
  static constexpr std::array<std::uint32_t, 69> s_DATA{
      69,         1828757504, 22194816,   1508,       352408085,  1073745920,
      544,        536994048,  2881530817, 5767173,    8432336,    4294967295,
      4294967295, 4279566336, 4294967295, 4294967295, 4279570432, 4294967295,
      4294967295, 4279574528, 4294967295, 4294967295, 4279578624, 4294967295,
      4294967295, 4279582720, 16777221,   8433376,    4294967040, 1024,
      262144,     67108868,   1024,       262144,     67108868,   5384,
      4294967040, 1024,       262144,     67108868,   1024,       262144,
      67108868,   5400,       4294967040, 1024,       262144,     67108868,
      1024,       262144,     67108868,   5416,       4294967040, 1024,
      262144,     67108868,   1024,       262144,     67108868,   5432,
      4294967040, 1024,       262144,     67108868,   1024,       262144,
      67108868,   5448,       796590080};
  static constexpr uint32_t s_REMAINING{69};
};

TEST_F(NSWTriggerSTGL1AElinkTest, head_fragID) {
  EXPECT_EQ(m_decoder->head_fragID(), 0);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_sectID) {
  EXPECT_EQ(m_decoder->head_sectID(), 1);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_EC) {
  EXPECT_EQ(m_decoder->head_EC(), 0);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_flags) {
  EXPECT_EQ(m_decoder->head_flags(), 82);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_BCID) {
  EXPECT_EQ(m_decoder->head_BCID(), 2728);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_orbit) {
  EXPECT_EQ(m_decoder->head_orbit(), 0);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_spare) {
  EXPECT_EQ(m_decoder->head_spare(), 0);
}

TEST_F(NSWTriggerSTGL1AElinkTest, L1ID) {
  EXPECT_EQ(m_decoder->L1ID(), 1508);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_wdw_open) {
  EXPECT_EQ(m_decoder->head_wdw_open(), 336);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_l1a_req) {
  EXPECT_EQ(m_decoder->head_l1a_req(), 338);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_wdw_close) {
  EXPECT_EQ(m_decoder->head_wdw_close(), 340);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_overflowCount) {
  EXPECT_EQ(m_decoder->head_overflowCount(), 0);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_wdw_matching_engines_usage) {
  EXPECT_EQ(m_decoder->head_wdw_matching_engines_usage(), 268435456);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_cfg_wdw_open_offset) {
  EXPECT_EQ(m_decoder->head_cfg_wdw_open_offset(), 34);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_cfg_l1a_req_offset) {
  EXPECT_EQ(m_decoder->head_cfg_l1a_req_offset(), 32);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_cfg_wdw_close_offset) {
  EXPECT_EQ(m_decoder->head_cfg_wdw_close_offset(), 30);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_cfg_timeout) {
  EXPECT_EQ(m_decoder->head_cfg_timeout(), 256);
}

TEST_F(NSWTriggerSTGL1AElinkTest, head_link_const) {
  EXPECT_EQ(m_decoder->head_link_const(), 2881530817);
}

TEST_F(NSWTriggerSTGL1AElinkTest, stream_head_nbits) {
  constexpr static auto SIZE = std::size_t{2};
  constexpr static auto EXPECTATION = std::array<uint32_t, SIZE>{96, 256};

  EXPECT_EQ(std::size(m_decoder->stream_head_nbits()), SIZE);
  for (std::size_t i = 0; i< SIZE; ++i) {
    EXPECT_EQ(m_decoder->stream_head_nbits().at(i), EXPECTATION.at(i));
  }
}

TEST_F(NSWTriggerSTGL1AElinkTest, stream_head_nwords) {
  constexpr static auto SIZE = std::size_t{2};
  constexpr static auto EXPECTATION = std::array<uint32_t, SIZE>{5, 5};

  EXPECT_EQ(std::size(m_decoder->stream_head_nwords()), SIZE);
  for (std::size_t i = 0; i< SIZE; ++i) {
    EXPECT_EQ(m_decoder->stream_head_nwords().at(i), EXPECTATION.at(i));
  }
}

TEST_F(NSWTriggerSTGL1AElinkTest, stream_head_fifo_size) {
  constexpr static auto SIZE = std::size_t{2};
  constexpr static auto EXPECTATION = std::array<uint32_t, SIZE>{128, 128};

  EXPECT_EQ(std::size(m_decoder->stream_head_fifo_size()), SIZE);
  for (std::size_t i = 0; i< SIZE; ++i) {
    EXPECT_EQ(m_decoder->stream_head_fifo_size().at(i), EXPECTATION.at(i));
  }
}

TEST_F(NSWTriggerSTGL1AElinkTest, stream_head_streamID) {
  constexpr static auto SIZE = std::size_t{2};
  constexpr static auto EXPECTATION = std::array<uint32_t, SIZE>{43728, 44768};

  EXPECT_EQ(std::size(m_decoder->stream_head_streamID()), SIZE);
  for (std::size_t i = 0; i< SIZE; ++i) {
    EXPECT_EQ(m_decoder->stream_head_streamID().at(i), EXPECTATION.at(i));
  }
}

TEST_F(NSWTriggerSTGL1AElinkTest, trailer_CRC) {
  EXPECT_EQ(m_decoder->trailer_CRC(), 12155);
}

TEST_F(NSWTriggerSTGL1AElinkTest, pad_packets) {
  constexpr static auto SIZE = std::size_t{5};
  EXPECT_EQ(std::size(m_decoder->pad_packets()), SIZE);
}

TEST_F(NSWTriggerSTGL1AElinkTest, segment_packet) {
  constexpr static auto SIZE = std::size_t{5};
  EXPECT_EQ(std::size(m_decoder->segment_packet()), SIZE);
}

ATLAS_GOOGLE_TEST_MAIN
