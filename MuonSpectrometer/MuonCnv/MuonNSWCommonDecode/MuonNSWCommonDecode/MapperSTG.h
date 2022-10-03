/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_STG_MAPPER_H_
#define _MUON_NSW_STG_MAPPER_H_

#include <cstdint>
#include <map>
#include <vector>

#include "MuonNSWCommonDecode/NSWDecodeHelper.h"

namespace Muon
{
  namespace nsw
  {
    class MapperSTG
    {
     public:
      MapperSTG() {};
      virtual ~MapperSTG () {};

      uint16_t nchannels (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer) const;
      uint16_t channel_number (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer, uint16_t vmm, uint16_t vmm_chan) const;
      bool vmm_info (uint8_t channel_type, uint8_t sector_type, uint8_t mod_radius, uint8_t layer, uint16_t channel_number, uint16_t& vmm, uint16_t& vmm_chan) const;

      static uint16_t private_id (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer);
      uint16_t AB_to_Athena_channel_number (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer, uint16_t channel_number) const;
      uint16_t Athena_to_AB_channel_number (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer, uint16_t channel_number) const;
    };
  
    static const std::map <uint16_t, std::vector<std::vector<uint8_t>>> s_stgc_channel_map =
    {
      // channel range: {vmm0, vmm0_channel, vmm1, vmm1_channel}
      // as a function of channel_type, sector_type [1=large 0=small], feb_radius [0->2], layer [0->7]

      // This mapping reflects the spreadsheet Adapter-Board-level numbering in sTGC_AB_Mapping_WithPadTDSChannels.xlsx,
      // where vmm ids are the nominal ones (as on the FEB).

      //**** PADS
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 0),  { {2, 39, 1, 36} }}, // QS1C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 1),  { {1, 24, 2, 27} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 2),  { {1, 24, 2, 31} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 3),  { {2, 39, 1, 32} }}, 
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 4),  { {2, 39, 1, 36} }}, // QS1P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 5),  { {1, 24, 2, 27} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 6),  { {1, 24, 2, 10} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 7),  { {2, 39, 1, 53} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 0),  { {2, 23, 1, 40} }}, // QS2C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 1),  { {1, 40, 2, 23} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 2),  { {1, 42, 2, 22} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 3),  { {2, 21, 1, 41} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 4),  { {2, 14, 1, 49} }}, // QS2P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 5),  { {1, 49, 2, 14} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 6),  { {1, 42, 2, 22} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 7),  { {2, 21, 1, 41} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 0),  { {2, 40, 2,  2} }}, // QS3C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 1),  { {1,  1, 1, 39} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 2),  { {1,  0, 1, 41} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 3),  { {2, 41, 2,  0} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 4),  { {2, 32, 2,  9} }}, // QS3P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 5),  { {2,  9, 2, 32} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 6),  { {2,  2, 2, 40} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 7),  { {2, 39, 2,  1} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 0),  { {2, 39, 1,  2} }}, // QL1P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 1),  { {1,  8, 2, 45} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 2),  { {1,  8, 2, 55} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 3),  { {2, 39, 1,  0}, {2, 40, 2, 47} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 4),  { {2, 39, 1,  8} }}, // QL1C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 5),  { {1,  8, 2, 39} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 6),  { {1, 13, 2, 44} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 7),  { {2, 34, 1,  3} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 0),  { {2, 29, 1, 38} }}, // QL2P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 1),  { {1, 34, 2, 25} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 2),  { {1, 24, 2, 34} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 3),  { {2, 39, 1, 29} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 4),  { {2, 29, 1, 38} }}, // QL2C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 5),  { {1, 34, 2, 25} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 6),  { {1, 34, 2, 25} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 7),  { {2, 29, 1, 38} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 0),  { {2, 21, 1, 26} }}, // QL3P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 1),  { {1, 44, 2, 39} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 2),  { {1, 34, 2, 39} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 3),  { {2, 31, 1, 26} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 4),  { {2, 13, 1, 26} }}, // QL3C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 5),  { {1, 52, 2, 39} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 6),  { {1, 48, 2, 39} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 7),  { {2, 17, 1, 26} }},

      //**** STRIPS
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 0, 0),  { {7, 21, 6, 48}, {6, 15, 5, 48}, {5, 15, 0,  0} }}, 
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 0, 1),  { {0, 42, 1, 15}, {1, 48, 2, 15}, {2, 48, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 0, 2),  { {0, 42, 1, 15}, {1, 48, 2, 15}, {2, 48, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 0, 3),  { {7, 21, 6, 48}, {6, 15, 5, 48}, {5, 15, 0,  0} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 0, 4),  { {7, 21, 6, 48}, {6, 15, 5, 48}, {5, 15, 0,  0} }}, 
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 0, 5),  { {0, 42, 1, 15}, {1, 48, 2, 15}, {2, 48, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 0, 6),  { {0, 42, 1, 15}, {1, 48, 2, 15}, {2, 48, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 0, 7),  { {7, 21, 6, 48}, {6, 15, 5, 48}, {5, 15, 0,  0} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 1, 0),  { {7, 63, 2, 19} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 1, 1),  { {2, 19, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 1, 2),  { {2, 19, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 1, 3),  { {7, 63, 2, 19} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 1, 4),  { {7, 63, 2, 19} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 1, 5),  { {2, 19, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 1, 6),  { {2, 19, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 1, 7),  { {7, 63, 2, 19} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 2, 0),  { {7, 63, 3, 13} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 2, 1),  { {3, 13, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 2, 2),  { {3, 13, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 2, 3),  { {7, 63, 3, 13} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 2, 4),  { {7, 63, 3, 13} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 2, 5),  { {3, 13, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 2, 6),  { {3, 13, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 0, 2, 7),  { {7, 63, 3, 13} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 0, 0),  { {7, 23, 6, 48}, {6, 15, 5, 48}, {5, 15, 0,  0} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 0, 1),  { {0, 40, 1, 15}, {1, 48, 2, 15}, {2, 48, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 0, 2),  { {0, 40, 1, 15}, {1, 48, 2, 15}, {2, 48, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 0, 3),  { {7, 23, 6, 48}, {6, 15, 5, 48}, {5, 15, 0,  0} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 0, 4),  { {7, 23, 6, 48}, {6, 15, 5, 48}, {5, 15, 0,  0} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 0, 5),  { {0, 40, 1, 15}, {1, 48, 2, 15}, {2, 48, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 0, 6),  { {0, 40, 1, 15}, {1, 48, 2, 15}, {2, 48, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 0, 7),  { {7, 23, 6, 48}, {6, 15, 5, 48}, {5, 15, 0,  0} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 1, 0),  { {7, 63, 2, 18} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 1, 1),  { {2, 18, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 1, 2),  { {2, 18, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 1, 3),  { {7, 63, 2, 18} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 1, 4),  { {7, 63, 2, 18} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 1, 5),  { {2, 18, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 1, 6),  { {2, 18, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 1, 7),  { {7, 63, 2, 18} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 2, 0),  { {7, 63, 2, 31} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 2, 1),  { {2, 31, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 2, 2),  { {2, 31, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 2, 3),  { {7, 63, 2, 31} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 2, 4),  { {7, 63, 2, 31} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 2, 5),  { {2, 31, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 2, 6),  { {2, 31, 7, 63} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_STRIP, 1, 2, 7),  { {7, 63, 2, 31} }},

      //**** WIRES
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 0, 0),  { {0, 57, 0, 39} }}, // QS1C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 0, 1),  { {0, 57, 0, 39} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 0, 2),  { {0, 57, 0, 38} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 0, 3),  { {0, 57, 0, 39} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 0, 4),  { {0, 57, 0, 39} }}, // QS1P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 0, 5),  { {0, 57, 0, 38} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 0, 6),  { {0, 57, 0, 39} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 0, 7),  { {0, 57, 0, 39} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 1, 0),  { {0, 57, 0, 29} }}, // QS2C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 1, 1),  { {0, 57, 0, 29} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 1, 2),  { {0, 57, 0, 28} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 1, 3),  { {0, 57, 0, 29} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 1, 4),  { {0, 57, 0, 29} }}, // QS2P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 1, 5),  { {0, 57, 0, 28} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 1, 6),  { {0, 57, 0, 29} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 1, 7),  { {0, 57, 0, 29} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 2, 0),  { {0, 57, 0, 20} }}, // QS3C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 2, 1),  { {0, 57, 0, 20} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 2, 2),  { {0, 57, 0, 20} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 2, 3),  { {0, 57, 0, 21} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 2, 4),  { {0, 57, 0, 21} }}, // QS3P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 2, 5),  { {0, 57, 0, 20} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 2, 6),  { {0, 57, 0, 20} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 0, 2, 7),  { {0, 57, 0, 20} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 0, 0),  { {0, 57, 0, 26} }}, // QL1P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 0, 1),  { {0, 57, 0, 26} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 0, 2),  { {0, 57, 0, 26} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 0, 3),  { {0, 57, 0, 26} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 0, 4),  { {0, 57, 0, 26} }}, // QL1C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 0, 5),  { {0, 57, 0, 26} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 0, 6),  { {0, 57, 0, 26} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 0, 7),  { {0, 57, 0, 26} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 1, 0),  { {0, 57, 0, 10} }}, // QL2P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 1, 1),  { {0, 57, 0,  9} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 1, 2),  { {0, 57, 0,  9} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 1, 3),  { {0, 57, 0, 10} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 1, 4),  { {0, 57, 0, 10} }}, // QL2C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 1, 5),  { {0, 57, 0,  9} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 1, 6),  { {0, 57, 0,  9} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 1, 7),  { {0, 57, 0, 10} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 2, 0),  { {0, 57, 0,  1} }}, // QL3P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 2, 1),  { {0, 57, 0,  0} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 2, 2),  { {0, 57, 0,  0} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 2, 3),  { {0, 57, 0,  1} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 2, 4),  { {0, 57, 0,  1} }}, // QL3C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 2, 5),  { {0, 57, 0,  0} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 2, 6),  { {0, 57, 0,  0} }},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_WIRE, 1, 2, 7),  { {0, 57, 0,  1} }}
    };
    
    static const std::map <uint16_t, std::pair<uint16_t, uint16_t>> s_stgc_pad_grid =
    {
      // Number of pad eta-rows and phi-columns 
      // as a function of channel_type, sector_type [1=large 0=small], feb_radius [0->2], layer [0->7].

      // This is used to convert the AB channel number to the pad index that ATHENA understands
    
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 0), {17, 4}}, // QS1C, Layer 0 {nEtaRows, nPhiColumns}
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 1), {17, 4}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 2), {18, 4}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 3), {18, 4}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 4), {17, 4}}, // QS1P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 5), {17, 4}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 6), {17, 3}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 0, 7), {17, 3}},

      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 0), {16, 3}}, // QS2C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 1), {16, 3}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 2), {15, 3}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 3), {15, 3}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 4), {15, 2}}, // QS2P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 5), {15, 2}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 6), {15, 3}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 1, 7), {15, 3}},

      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 0), {13, 3}}, // QS3C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 1), {13, 3}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 2), {14, 3}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 3), {14, 3}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 4), {12, 2}}, // QS3P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 5), {12, 2}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 6), {13, 3}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 0, 2, 7), {13, 3}},

      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 0), {17, 6}}, // QL1P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 1), {17, 6}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 2), {16, 7}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 3), {16, 7}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 4), {16, 6}}, // QL1C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 5), {16, 6}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 6), {16, 6}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 0, 7), {16, 6}},

      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 0), {14, 4}}, // QL2P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 1), {14, 4}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 2), {15, 5}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 3), {15, 5}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 4), {14, 4}}, // QL2C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 5), {14, 4}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 6), {14, 4}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 1, 7), {14, 4}},

      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 0), {15, 4}}, // QL3P
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 1), {15, 4}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 2), {14, 5}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 3), {14, 5}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 4), {13, 4}}, // QL3C
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 5), {13, 4}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 6), {14, 4}},
      {MapperSTG::private_id(OFFLINE_CHANNEL_TYPE_PAD, 1, 2, 7), {14, 4}}
    };
  }
}


//=====================================================================
inline uint16_t Muon::nsw::MapperSTG::private_id (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer)
{
  // an internal unique ID for every VMM channel
  return (channel_type & 0xf) << 12 | (sector_type & 0xf) << 8 | (feb_radius & 0xf) << 4 | (layer & 0xf);
}

#endif // _MUON_NSW_STGC_MAPPER_H_


