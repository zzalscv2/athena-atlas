/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonNSWCommonDecode/MapperSTG.h"

#include <iostream>
//=====================================================================
uint16_t Muon::nsw::MapperSTG::channel_number (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer, uint16_t vmm, uint16_t vmm_chan) const
{
  // Returns the detector-channel index according to ATHENA conventions.

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
bool Muon::nsw::MapperSTG::vmm_info (uint8_t channel_type, uint8_t sector_type, uint8_t mod_radius, uint8_t layer, uint16_t channel_number, uint16_t& vmm, uint16_t& vmm_chan) const
{
  // Return vmm and vmm channel given the ATHENA channel index.

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


//=====================================================================
uint16_t Muon::nsw::MapperSTG::AB_to_Athena_channel_number (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer, uint16_t channel_number) const
{
  // Convert an Adapter-Board channel number into ATHENA channel index.

  if (channel_number == 0) return channel_number; // invalid (e.g. case of disconnected channel)

  if (channel_type == OFFLINE_CHANNEL_TYPE_WIRE) {
    
    // AB:     wire#1 is on the gas-inlet side; right (left) hand side for pivot (confirm) wedges when looking from the IP.
    // Athena: wire#1 is on the left-hand side both for pivot and confirm wedges.
    bool isPivot = (sector_type == 0)^(layer < 4);
    if (isPivot) channel_number = nchannels(channel_type, sector_type, feb_radius, layer) - channel_number + 1;

  } else if (channel_type == OFFLINE_CHANNEL_TYPE_PAD) {

    // AB:     pad#1 is on left (right) hand side for even (odd) layers when looking from the IP (counting layers from 0).
    // Athena: pad#1 is on the right-hand side.
    uint16_t pid = private_id(channel_type, sector_type, feb_radius, layer);
    std::pair<uint16_t, uint16_t> pad_grid = s_stgc_pad_grid.at(pid);

    uint16_t padRow_AB  = (channel_number - 1)/pad_grid.second + 1;
    uint16_t padCol_AB  = (channel_number - 1)%pad_grid.second + 1;
    uint16_t padRow_ATH = pad_grid.first - padRow_AB + 1;
    uint16_t padCol_ATH = (layer%2==0) ? pad_grid.second - padCol_AB + 1 : padCol_AB; // layer is in [0,7]
      
    // Athena pad numbering assumes 18 eta rows (even if a quadruplet has less)
    channel_number = (padCol_ATH - 1)*18 + padRow_ATH;
  }
  
  return channel_number; 
}


//=====================================================================
uint16_t Muon::nsw::MapperSTG::Athena_to_AB_channel_number (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer, uint16_t channel_number) const
{
  // Convert a channel index according to ATHENA numbering convention
  // into the Adapter-Board channel number.
  
  if (channel_number == 0) return channel_number; // invalid (e.g. case of disconnected channel)

  if (channel_type == OFFLINE_CHANNEL_TYPE_WIRE) {
    
    // AB:     wire#1 is on the gas-inlet side; right (left) hand side for pivot (confirm) wedges when looking from the IP.
    // Athena: wire#1 is on the left-hand side both for pivot and confirm wedges.
    bool isPivot = (sector_type == 0)^(layer < 4);
    if (isPivot) channel_number = nchannels(channel_type, sector_type, feb_radius, layer) - channel_number + 1;
  
  } else if (channel_type == OFFLINE_CHANNEL_TYPE_PAD) {

    // AB:     pad#1 is on left (right) hand side for even (odd) layers when looking from the IP (counting layers from 0).
    // Athena: pad#1 is on the right-hand side.
    uint16_t pid = private_id(channel_type, sector_type, feb_radius, layer);
    std::pair<uint16_t, uint16_t> pad_grid = s_stgc_pad_grid.at(pid);
    
    // Athena pad numbering assumes 18 eta rows (even if a quadruplet has less)
    uint16_t padRow_ATH = (channel_number - 1)%18 + 1;
    uint16_t padCol_ATH = (channel_number - 1)/18 + 1;
    uint16_t padRow_AB  = pad_grid.first - padRow_ATH + 1;
    uint16_t padCol_AB  = (layer%2==0) ? pad_grid.second - padCol_ATH + 1 : padCol_ATH;  // layer is in [0,7]

    channel_number = (padRow_AB - 1)*pad_grid.second + padCol_AB;
  }

  return channel_number;  
}


//=====================================================================
uint16_t Muon::nsw::MapperSTG::NSWID_to_Athena_channel_number (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer, uint16_t channel_number, bool sideA) const
{
  // Convert a channel index according to the NSWID numbering convention
  // into ATHENA channel index.

  if (channel_number == 0) return channel_number; // invalid (e.g. case of disconnected channel)

  // In NSWID numbering wires and pads follow ATLAS phi. 
  if (channel_type == OFFLINE_CHANNEL_TYPE_WIRE) {

    // NSWID:  wire#1 is on the left (right) hand side for A-side (C-side) when looking from the IP.
    // Athena: wire#1 is on the left-hand side both for A- and C-side.
    if (!sideA) channel_number = nchannels(channel_type, sector_type, feb_radius, layer) - channel_number + 1;

  } else if (channel_type == OFFLINE_CHANNEL_TYPE_PAD) {

    // NSWID:  pad#1 is on the left (right) hand side for A-side (C-side) when looking from the IP.
    // Athena: pad#1 is on the right-hand side both for A- and C-side.
    uint16_t pid = private_id(channel_type, sector_type, feb_radius, layer);
    std::pair<uint16_t, uint16_t> pad_grid = s_stgc_pad_grid.at(pid);

    uint16_t padRow     = (channel_number - 1)/pad_grid.second + 1;
    uint16_t padCol_NSW = (channel_number - 1)%pad_grid.second + 1;
    uint16_t padCol_ATH = (sideA) ? pad_grid.second - padCol_NSW + 1 : padCol_NSW;

    // Athena pad numbering assumes 18 eta rows (even if a quadruplet has less)
    channel_number = (padCol_ATH - 1)*18 + padRow;
  }
  
  return channel_number; 
}


//=====================================================================
uint16_t Muon::nsw::MapperSTG::Athena_to_NSWID_channel_number (uint8_t channel_type, uint8_t sector_type, uint8_t feb_radius, uint8_t layer, uint16_t channel_number, bool sideA) const
{
  // Convert a channel index according to ATHENA numbering convention
  // into NSWID channel index.

  if (channel_number == 0) return channel_number; // invalid (e.g. case of disconnected channel)

  // In NSWID numbering wires and pads follow ATLAS phi. 
  if (channel_type == OFFLINE_CHANNEL_TYPE_WIRE) {
    
    // NSWID:  wire#1 is on the left (right) hand side for A-side (C-side) when looking from the IP.
    // Athena: wire#1 is on the left-hand side both for A- and C-side.
    if (!sideA) channel_number = nchannels(channel_type, sector_type, feb_radius, layer) - channel_number + 1;

  } else if (channel_type == OFFLINE_CHANNEL_TYPE_PAD) {

    // NSWID:  pad#1 is on the left (right) hand side for A-side (C-side) when looking from the IP.
    // Athena: pad#1 is on the right-hand side both for A- and C-side.
    uint16_t pid = private_id(channel_type, sector_type, feb_radius, layer);
    std::pair<uint16_t, uint16_t> pad_grid = s_stgc_pad_grid.at(pid);
    
    // Athena pad numbering assumes 18 eta rows (even if a quadruplet has less)
    uint16_t padRow     = (channel_number - 1)%18 + 1;
    uint16_t padCol_ATH = (channel_number - 1)/18 + 1;
    uint16_t padCol_NSW = (sideA) ? pad_grid.second - padCol_ATH + 1 : padCol_ATH;
    channel_number = (padRow - 1)*pad_grid.second + padCol_NSW;
  }

  return channel_number;  
}




