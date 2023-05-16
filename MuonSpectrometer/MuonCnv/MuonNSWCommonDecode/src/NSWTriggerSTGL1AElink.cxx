/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonNSWCommonDecode/NSWTriggerSTGL1AElink.h"

#include <cmath>
#include <cstddef>
#include <exception>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "MuonNSWCommonDecode/NSWResourceId.h"
#include "MuonNSWCommonDecode/NSWSTGTPDecodeBitmaps.h"
#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/STGTPPackets.h"
#include "ers/ers.h"

Muon::nsw::NSWTriggerSTGL1AElink::NSWTriggerSTGL1AElink(const uint32_t* bs, const uint32_t remaining)
    : NSWTriggerElink(bs, remaining), m_data{bs, remaining} {
  // TODO: once format finalized, checking a minimum size

  // 2 felix header 32b words already decoded;
  constexpr static auto START_DATA = std::size_t{2 * 32};
  auto readPointer = std::size_t{START_DATA};
  decode_header(readPointer);
  decode_data(readPointer);
  decode_trailer(readPointer);
}

void Muon::nsw::NSWTriggerSTGL1AElink::decode_header(std::size_t& readPointer) {
  m_head_fragID = decode(readPointer, STGTPL1A::size_head_sectID);
  m_head_sectID = decode(readPointer, STGTPL1A::size_head_sectID);
  m_head_EC = decode(readPointer, STGTPL1A::size_head_EC);
  m_head_flags = decode(readPointer, STGTPL1A::size_head_flags);
  m_head_BCID = decode(readPointer, STGTPL1A::size_head_BCID);
  m_head_orbit = decode(readPointer, STGTPL1A::size_head_orbit);
  m_head_spare = decode(readPointer, STGTPL1A::size_head_spare);
  m_L1ID = decode(readPointer, STGTPL1A::size_L1ID);
  m_head_wdw_open = decode(readPointer, STGTPL1A::size_head_wdw_open);
  m_head_l1a_req = decode(readPointer, STGTPL1A::size_head_l1a_req);
  m_head_wdw_close = decode(readPointer, STGTPL1A::size_head_wdw_close);
  m_head_overflowCount = decode(readPointer, STGTPL1A::size_head_overflowCount);
  m_head_wdw_matching_engines_usage = decode(readPointer, STGTPL1A::size_head_wdw_matching_engines_usage);
  m_head_cfg_wdw_open_offset = decode(readPointer, STGTPL1A::size_head_cfg_wdw_open_offset);
  m_head_cfg_l1a_req_offset = decode(readPointer, STGTPL1A::size_head_cfg_l1a_req_offset);
  m_head_cfg_wdw_close_offset = decode(readPointer, STGTPL1A::size_head_cfg_wdw_close_offset);
  m_head_cfg_timeout = decode(readPointer, STGTPL1A::size_head_cfg_timeout);

  // this is important! It identifies the elink! It should be ABCD1230/ABCD1231/ABCD1232
  // not checked during decoding but must be checked at some point
  m_head_link_const = decode(readPointer, STGTPL1A::size_head_link_const);
}

void Muon::nsw::NSWTriggerSTGL1AElink::decode_data(std::size_t& readPointer) {
  static constexpr auto PADDING_BITS_END = std::size_t{16};
  const auto endOfData = m_wordCountFlx * WORD_SIZE - Muon::nsw::STGTPL1A::size_trailer_CRC - PADDING_BITS_END;
  while (readPointer < endOfData) {
    static constexpr auto SIZE_DATA_HEADER = STGTPL1A::size_stream_head_nbits + STGTPL1A::size_stream_head_nwords +
                                             STGTPL1A::size_stream_head_fifo_size + STGTPL1A::size_stream_head_streamID;
    if (readPointer + SIZE_DATA_HEADER >= endOfData) {
      throw std::length_error(
          Muon::format("Read pointer ({}) would excede memory dedicated to data chunks ({}) while parsing the header (size: {})",
                      readPointer, endOfData, SIZE_DATA_HEADER));
    }
    const auto header_data = decode_data_header(readPointer);

    if (header_data.total_expected_size > m_wordCountFlx - ceil(readPointer / WORD_SIZE) + 1) {
      throw std::length_error(Muon::format("STG TP stream size {} larger than expected packet size {}",
                                          header_data.total_expected_size,
                                          m_wordCountFlx - ceil(readPointer / WORD_SIZE) + 1));
    }
    if (header_data.nwords * header_data.data_size + readPointer > endOfData) {
      throw std::length_error(Muon::format("Requested to decode {} bits but only {} bits are remaining",
                                          header_data.nwords * header_data.data_size, endOfData - readPointer));
    }

    m_stream_data.push_back(decode_data_payload(readPointer, header_data));
  }
  analyze_data();
}

Muon::nsw::NSWTriggerSTGL1AElink::DataHeader Muon::nsw::NSWTriggerSTGL1AElink::decode_data_header(
    std::size_t& readPointer) {
  const auto current_stream_head_nbits = decode(readPointer, STGTPL1A::size_stream_head_nbits);
  const auto current_stream_head_nwords = decode(readPointer, STGTPL1A::size_stream_head_nwords);
  const auto current_stream_head_fifo_size = decode(readPointer, STGTPL1A::size_stream_head_fifo_size);
  const auto current_stream_head_streamID = decode(readPointer, STGTPL1A::size_stream_head_streamID);

  // zero padding to multiples of 16bits - TP logic - this is the real number
  // of bits to read
  const auto corrected_current_stream_head_nbits = correct_size_for_padding(current_stream_head_nbits);

  m_stream_head_nbits.push_back(corrected_current_stream_head_nbits);
  m_stream_head_nwords.push_back(current_stream_head_nwords);
  m_stream_head_fifo_size.push_back(current_stream_head_fifo_size);
  m_stream_head_streamID.push_back(current_stream_head_streamID);

  const auto data_size = static_cast<std::size_t>(std::ceil(corrected_current_stream_head_nbits / WORD_SIZE_DOUBLE));
  const auto total_expected_size = data_size * current_stream_head_nwords;

  ERS_DEBUG(2, Muon::format("stream_head_nbits: {}", corrected_current_stream_head_nbits));
  ERS_DEBUG(2, Muon::format("stream_head_nwords: {}", current_stream_head_nwords));
  ERS_DEBUG(2, Muon::format("stream_head_fifo_size: {}", current_stream_head_fifo_size));
  ERS_DEBUG(2, Muon::format("stream_head_streamID: {}", current_stream_head_streamID));
  ERS_DEBUG(2, Muon::format("total_expected_size: {}", data_size * current_stream_head_nwords));
  ERS_DEBUG(2, Muon::format("m_wordCountFlx: {}, ceil(readPointer/{}): {}", m_wordCountFlx, WORD_SIZE_DOUBLE,
                           ceil(readPointer / WORD_SIZE_DOUBLE)));

  return {corrected_current_stream_head_nbits, current_stream_head_nwords, current_stream_head_fifo_size,
          current_stream_head_streamID,        total_expected_size,        data_size};
}

std::vector<std::vector<std::uint32_t>> Muon::nsw::NSWTriggerSTGL1AElink::decode_data_payload(
    std::size_t& readPointer, const DataHeader& header) const {
  std::vector<std::vector<std::uint32_t>> current_stream_data{};

  for (std::size_t i = 0; i < header.nwords; ++i) {
    std::vector<std::uint32_t> data{};
    for (std::size_t j = 0; j < header.data_size; ++j) {
      data.push_back(decode(readPointer, WORD_SIZE));
    }
    current_stream_data.push_back(data);
  }
  return current_stream_data;
}

void Muon::nsw::NSWTriggerSTGL1AElink::decode_trailer(std::size_t& readPointer) {
  // TODO: warning: how the swROD is behaving if the last work is a uint16 only? Just 0-padding?
  m_trailer_CRC = decode(readPointer, Muon::nsw::STGTPL1A::size_trailer_CRC);
}

std::uint64_t Muon::nsw::NSWTriggerSTGL1AElink::decode(std::size_t& readPointer, const std::size_t size) const {
  return decode_and_advance<std::uint64_t, std::uint32_t>(m_data, readPointer, size);
}

void Muon::nsw::NSWTriggerSTGL1AElink::analyze_data() {
  auto counterChunk = std::size_t{0};
  for (const auto& dataChunk : m_stream_data) {
    for (const auto& dataWord : dataChunk) {
      const auto expectedSize =
          static_cast<std::size_t>(std::ceil(m_stream_head_nbits.at(counterChunk) / WORD_SIZE_DOUBLE));
      if (std::size(dataWord) != expectedSize) {
        throw std::length_error(Muon::format("Stream data size {} does not match expected number of messages {}",
                                            std::size(dataWord), expectedSize));
      }
      switch (m_stream_head_streamID.at(counterChunk)) {
        case STGTPPad::pad_stream_header:
          m_pad_packets.emplace_back(dataWord);
          break;
        case STGTPSegments::merge_stream_header:
          m_segment_packets.emplace_back(dataWord);
          break;
        default:
          throw std::runtime_error(Muon::format("Invalid stream type {}", m_stream_head_streamID.at(counterChunk)));
      }
    }
    ++counterChunk;
  }
}

std::uint64_t Muon::nsw::NSWTriggerSTGL1AElink::correct_size_for_padding(const std::uint64_t initial) {
  static constexpr auto PADDING = 16;
  if (initial % PADDING) {
    return ((initial + PADDING - 1) / PADDING) * PADDING;
  } else {
    return initial;
  }
}
