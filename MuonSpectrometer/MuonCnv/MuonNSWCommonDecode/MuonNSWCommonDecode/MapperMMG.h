/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_MMG_MAPPER_H_
#define _MUON_NSW_MMG_MAPPER_H_

#include <cstdint>
#include "MuonNSWCommonDecode/NSWDecodeHelper.h"

namespace Muon
{
  namespace nsw
  {
    class MapperMMG
    {
     public:
      MapperMMG() {};
      virtual ~MapperMMG () {};

      uint16_t channel_number (uint8_t feb_radius, uint16_t vmm, uint16_t vmm_chan) const;
      bool vmm_info (uint8_t mod_radius, uint16_t channel_number, uint8_t& feb_radius, uint16_t& vmm, uint16_t& vmm_chan) const;
    };
  }
}


//=====================================================================
inline uint16_t Muon::nsw::MapperMMG::channel_number (uint8_t feb_radius, uint16_t vmm, uint16_t vmm_chan) const
{
  // Retrieve the strip number for a given vmm/vmm_channel.
  // Input feb_radius is expected in [0, 15]
  // Layers with ID (0, 2, 4, 6) have even MMFE8 on the left side, odd on the right
  // Layers with ID (1, 3, 5, 7) have even MMFE8 on the right side, odd on the left
  // VMMs and VMM channels are always counted inwards in even radii
  
  uint16_t outw_vmm  = vmm;
  uint16_t outw_chan = vmm_chan;

  if ((feb_radius % 2) == 0) {
    outw_vmm  = Muon::nsw::VMM_per_MMFE8 - vmm  - 1;
    outw_chan = Muon::nsw::VMM_channels  - vmm_chan - 1;
  }

  return ((feb_radius < 10 ? feb_radius : feb_radius - 10) * Muon::nsw::VMM_per_MMFE8 + outw_vmm) * Muon::nsw::VMM_channels + outw_chan + 1;
}



//=====================================================================
inline bool Muon::nsw::MapperMMG::vmm_info (uint8_t mod_radius, uint16_t channel_number, uint8_t& feb_radius, uint16_t& vmm, uint16_t& vmm_chan) const
{
  // Retrieve {feb radius [0, 15], vmm number [0, 7], vmm channel [0, 63]} for a given strip
  // Input mod_radius is expected in [0, 1].
  
  channel_number -= 1;
  feb_radius = channel_number / (Muon::nsw::VMM_per_MMFE8 * Muon::nsw::VMM_channels);
  vmm        = channel_number / Muon::nsw::VMM_channels - feb_radius * Muon::nsw::VMM_per_MMFE8;
  vmm_chan   = channel_number % Muon::nsw::VMM_channels; 
  if (mod_radius == 1) feb_radius += 10;

  if ((feb_radius % 2) == 0) {
    vmm      = Muon::nsw::VMM_per_MMFE8 - vmm      - 1;
    vmm_chan = Muon::nsw::VMM_channels  - vmm_chan - 1;
  }
  
  return true;
}

#endif

