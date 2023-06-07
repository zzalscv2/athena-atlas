/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _MUON_NSW_STGTP_DECODE_BITMAPS_H_
#define _MUON_NSW_STGTP_DECODE_BITMAPS_H_

#include "MuonNSWCommonDecode/NSWDecodeHelper.h"
#include <stdexcept>
#include <cstddef>

namespace Muon
{
  namespace nsw
  {
    namespace STGTPL1A {
      constexpr int size_head_fragID =                        4;
      constexpr int size_head_sectID =                        4;
      constexpr int size_head_EC =                            1;
      constexpr int size_head_flags =                         7;
      constexpr int size_head_BCID =                         12;
      constexpr int size_head_orbit =                         2;
      constexpr int size_head_spare =                         2;
      constexpr int size_L1ID =                              32;
      constexpr int size_head_wdw_open =                     12;
      constexpr int size_head_l1a_req =                      12;
      constexpr int size_head_wdw_close =                    12;
      constexpr int size_head_overflowCount =                12;
      constexpr int size_head_wdw_matching_engines_usage =   32;
      constexpr int size_head_cfg_wdw_open_offset =          12;
      constexpr int size_head_cfg_l1a_req_offset =           12;
      constexpr int size_head_cfg_wdw_close_offset =         12;
      constexpr int size_head_cfg_timeout =                  12;
      constexpr int size_head_link_const =                   32;
      constexpr int size_stream_head_nbits =                 16;
      constexpr int size_stream_head_nwords =                16;
      constexpr int size_stream_head_fifo_size =             16;
      constexpr int size_stream_head_streamID =              16;
      constexpr int size_trailer_CRC =                       16;
    };

    namespace STGTPPad {
      constexpr std::size_t num_pads =                                4;
      constexpr std::size_t pad_stream_header =                      0xAAD0;
      constexpr std::size_t n_words =                                 3; // size in 32 bit words
      constexpr std::size_t size_coincidence_wedge =                 16;
      constexpr std::size_t size_phiID =                              6;
      constexpr std::size_t size_bandID =                             8;
      constexpr std::size_t size_BCID =                              12;
      constexpr std::size_t size_spare =                              3;
      constexpr std::size_t size_idleFlag =                           1;
      constexpr std::size_t size_padding =                            8;

    };

     
    namespace STGTPSegments {
       constexpr std::size_t num_segments =                    8;

       constexpr int merge_stream_header  =              0xAEE0;
       constexpr int n_words =                                32; 
       constexpr int size_lut_choice_selection =              24;
       constexpr int size_nsw_segment_selector =              12;
       constexpr int size_valid_segment_selector =            12;
       
       constexpr int size_output_segment_monitor =             1;
       constexpr int size_output_segment_spare =               2;
       constexpr int size_output_segment_lowRes =              1;
       constexpr int size_output_segment_phiRes =              1;
       constexpr int size_output_segment_dTheta =              5;
       constexpr int size_output_segment_phiID =               6;
       constexpr int size_output_segment_rIndex =              8;

       constexpr int size_bcid             =                   12;
       constexpr int size_sectorID         =                   4;
      
      enum class MergedSegmentProperty {
          Monitor = fill_bitmask<uint32_t>(0, size_output_segment_monitor),
          Spare = fill_bitmask<uint32_t>(max_bit(Monitor) + 1, size_output_segment_spare),
          lowRes = fill_bitmask<uint32_t>(max_bit(Spare) + 1, size_output_segment_lowRes),
          phiRes = fill_bitmask<uint32_t>(max_bit(lowRes) + 1 , size_output_segment_phiRes),
          dTheta = fill_bitmask<uint32_t>(max_bit(phiRes)+ 1, size_output_segment_dTheta),
          phiID = fill_bitmask<uint32_t>(max_bit(dTheta)+1 , size_output_segment_phiID),
          rIndex = fill_bitmask<uint32_t>(max_bit(phiID)+1, size_output_segment_rIndex)
      };
      
      constexpr uint32_t getSegmentProperty(const uint32_t mask, const MergedSegmentProperty prop) {
          const auto shift = min_bit(static_cast<uint32_t>(prop));
          if (shift < 0) {
            throw std::runtime_error("bitshift is negative in NSWSTGTPDecodeBitmaps getSegmentProperty");
          }
          return (mask & static_cast<uint32_t>(prop) ) >> shift;
      }
      
      constexpr void encodeSegmentProperty(const MergedSegmentProperty prop, const uint32_t word, uint32_t& buffer) {
          const auto shift = min_bit(static_cast<uint32_t>(prop));
          if (shift < 0) {
            throw std::runtime_error("bitshift is negative in NSWSTGTPDecodeBitmaps encodeSegmentProperty");
          }
          uint32_t shifted_word = (word << shift);
          buffer = (buffer) | (shifted_word & static_cast<uint32_t>(prop));
      }
      
      namespace moduleIDBits{
          /// Large or Small wedge
          constexpr uint8_t stationID = 1;
          /// Side 0 for A / 1 for C
          constexpr uint8_t detectorSite = 1;
          /// 1 to 3 
          constexpr uint8_t stationEta = 4;
          /// station Phi 1 to 8
          constexpr uint8_t stationPhi = 8;
      }
      enum class ModuleIDProperty{
          stationID = fill_bitmask<uint32_t>(0, moduleIDBits::stationID),
          detectorSite = fill_bitmask<uint32_t>(max_bit(stationID) +1, moduleIDBits::detectorSite),
          stationEta = fill_bitmask<uint32_t>(max_bit(detectorSite) + 1, moduleIDBits::stationEta),
          stationPhi = fill_bitmask<uint32_t>(max_bit(stationEta) + 1, moduleIDBits::stationPhi),
      };
      constexpr uint32_t getIdentifierProperty(const uint32_t mask, const ModuleIDProperty prop) {
        const auto shift = min_bit(static_cast<uint32_t>(prop));
        if (shift < 0) {
          throw std::runtime_error("bitshift is negative in NSWSTGTPDecodeBitmaps getIdentifierProperty");
        }
        return (mask &  static_cast<uint32_t>(prop)) >> shift;
        
      }
      constexpr void encodeIdentifierProperty(const ModuleIDProperty prop, const uint32_t word, uint32_t& buffer) {
          const auto shift = min_bit(static_cast<uint32_t>(prop));
          if (shift < 0) {
            throw std::runtime_error("bitshift is negative in NSWSTGTPDecodeBitmaps encodeIdentifierProperty");
          }
          uint32_t shifted_word = (word << shift);
          buffer = (buffer) | (shifted_word & static_cast<uint32_t>(prop));
      }
    }
  }
}

#endif // _MUON_NSW_STGTP_DECODE_BITMAPS_H_
