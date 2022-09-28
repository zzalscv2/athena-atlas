/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonNSWCommonDecode/MapperSTG.h"

#include <iostream>
//=====================================================================
uint16_t Muon::nsw::MapperSTG::channel_number (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer, uint16_t vmm, uint16_t vmm_chan) const
{
  int counter {1};
  int chan = vmm * Muon::nsw::VMM_channels + vmm_chan;
  const auto& ranges = Muon::nsw::s_stgc_channel_map.at( private_id(channel_type, sector_type, feb_radius, layer) );

  for (const auto& range : ranges) {
    int chanFirst = range[0]*Muon::nsw::VMM_channels + range[1];
    int chanLast  = range[2]*Muon::nsw::VMM_channels + range[3];
    int increment = chanLast >= chanFirst ? 1 : -1;
      
    if ( (chan - chanFirst)*(chan - chanLast) <= 0 ) {
      uint16_t offline_channel = counter + increment*(chan - chanFirst);
      return AB_to_Athena_channel_number(channel_type, sector_type, feb_radius, layer, offline_channel);
    }

    counter += increment*(chanLast - chanFirst) + 1;
  }
  
  return 0; // disconnected vmm channel
}


//=====================================================================
uint16_t Muon::nsw::MapperSTG::nchannels(uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer) const
{
  uint16_t pid = private_id(channel_type, sector_type, feb_radius, layer);
  const auto& ranges = s_stgc_channel_map.at(pid);

  int counter{0};
  for (const auto& range : ranges) {
    int chanFirst = range[0]*Muon::nsw::VMM_channels + range[1]; // custom id for the first channel in the range
    int chanLast  = range[2]*Muon::nsw::VMM_channels + range[3]; // custom id for the last channel in the range
    int increment = chanLast >= chanFirst ? 1 : -1;
    counter += increment*(chanLast - chanFirst) + 1; // number of channels in the range
  }

  return counter;
}


//=====================================================================
uint16_t Muon::nsw::MapperSTG::AB_to_Athena_channel_number (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer, uint16_t channel_number) const
{
  if (channel_number == 0) return channel_number; // invalid (e.g. case of disconnected channel)

  // WIRES: convert wire numbers from AB-level to ATHENA numbering.
  if (channel_type == OFFLINE_CHANNEL_TYPE_WIRE) {
    
    // In AB numbering, wire#1 is on the gas-inlet side (right-hand for pivot, left-hand for confirm wedges when looking from the IP) 
    // In Athena, wire#1 is on the left-hand side when looking from the IP
    bool isPivot = (sector_type == 0)^(layer < 4);
    if (isPivot) channel_number = nchannels(channel_type, sector_type, feb_radius, layer) - channel_number + 1;

  // PADS: convert pad numbers from AB-level to ATHENA numbering
  } else if (channel_type == OFFLINE_CHANNEL_TYPE_PAD) {

    uint16_t pid = private_id(channel_type, sector_type, feb_radius, layer);
    std::pair<uint16_t, uint16_t> pad_grid = s_stgc_pad_grid.at(pid);

    uint16_t padRow_AB  = (channel_number - 1)/pad_grid.second + 1;
    uint16_t padCol_AB  = (channel_number - 1)%pad_grid.second + 1;
    uint16_t padRow_ATH = pad_grid.first - padRow_AB + 1;
    uint16_t padCol_ATH = (layer%2!=0) ? padCol_AB : pad_grid.second - padCol_AB + 1; // layer is in [0,7]
      
    // Athena pad numbering assumes 18 eta rows (even if a quadruplet has less)
    channel_number = (padCol_ATH - 1)*18 + padRow_ATH;
  }
  
  return channel_number; 
}


//=====================================================================
uint16_t Muon::nsw::MapperSTG::Athena_to_AB_channel_number (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer, uint16_t channel_number) const
{
  if (channel_number == 0) return channel_number; // invalid (e.g. case of disconnected channel)

  // WIRES: convert wire numbers from AB-level to ATHENA numbering.
  if (channel_type == OFFLINE_CHANNEL_TYPE_WIRE) {
    
    // In AB numbering, wire#1 is on the gas-inlet side (right-hand for pivot, left-hand for confirm wedges when looking from the IP) 
    // In Athena, wire#1 is on the left-hand side when looking from the IP
    bool isPivot = (sector_type == 0)^(layer < 4);
    if (isPivot) channel_number = nchannels(channel_type, sector_type, feb_radius, layer) - channel_number + 1;
  
  // PADS: convert pad numbers from Athena to AB-level numbering 
  } else if (channel_type == OFFLINE_CHANNEL_TYPE_PAD) {
    uint16_t pid = private_id(channel_type, sector_type, feb_radius, layer);
    std::pair<uint16_t, uint16_t> pad_grid = s_stgc_pad_grid.at(pid);
    
    // Athena pad numbering assumes 18 eta rows (even if a quadruplet has less)
    uint16_t padRow_ATH = (channel_number - 1)%18 + 1;
    uint16_t padCol_ATH = (channel_number - 1)/18 + 1;
    uint16_t padRow_AB  = pad_grid.first - padRow_ATH + 1;
    uint16_t padCol_AB  = (layer%2!=0) ? padCol_ATH : pad_grid.second - padCol_ATH + 1;  // layer is in [0,7]
    channel_number = (padRow_AB - 1)*pad_grid.second + padCol_AB;
  }

  return channel_number;  
}
  

//=====================================================================
bool Muon::nsw::MapperSTG::vmm_info (uint8_t channel_type, uint8_t sector_type, uint8_t mod_radius, uint8_t layer, uint16_t channel_number, uint16_t& vmm, uint16_t& vmm_chan) const
{
  // return vmm and vmm channel given the ATHENA channel number

  uint16_t AB_channel = Athena_to_AB_channel_number(channel_type, sector_type, mod_radius, layer, channel_number);

  uint16_t pid = private_id(channel_type, sector_type, mod_radius, layer);
  const auto& ranges = s_stgc_channel_map.at(pid);

  int counter{1};
  for (const auto& range : ranges) {
    int chanFirst = range[0]*Muon::nsw::VMM_channels + range[1]; // custom id for the first channel in the range
    int chanLast  = range[2]*Muon::nsw::VMM_channels + range[3]; // custom id for the last channel in the range
    int increment = chanLast >= chanFirst ? 1 : -1;
    int nchan     = increment*(chanLast - chanFirst) + 1;        // number of channels in the range
      
    if (AB_channel < counter + nchan) {
      int chan = chanFirst + increment*(AB_channel - counter);
      vmm      = chan/64;
      vmm_chan = chan%64;
      return true;
    }

    counter += nchan;
  }
    
  return false;  
}
