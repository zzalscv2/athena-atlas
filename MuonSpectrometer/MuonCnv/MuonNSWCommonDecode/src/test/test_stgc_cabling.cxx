/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <sstream>
#include "MuonNSWCommonDecode/MapperSTG.h"

int main (int argc, char **argv)
{
  (void)argc;
  (void)argv;
  Muon::nsw::MapperSTG m;
  printf("QUADRUPLET  CHAN_TYPE  VMM  VMM_CHAN  AB_CHAN  ATHENA_ID  NSWID_A  NSWID_C\n");
              
  for (uint8_t sector_type : {0, 1}) { // small, large
    for (uint8_t feb_radius : {0, 1, 2}) { // quadruplated radius
      for (uint8_t layer07{0}; layer07 < 8; ++layer07) { // layer in [0, 7]

        bool is_pivot   = (layer07 < 4)^(sector_type == 0);
        uint8_t layer14 = layer07%4 + 1;

        // quadruplet name
        std::stringstream quadname_ss;
        quadname_ss << (sector_type ? "QL" : "QS");
        quadname_ss << feb_radius + 1;
        quadname_ss << (is_pivot ?   'P' :     'C');          
        quadname_ss << (is_pivot ? layer14 : 5 - layer14); // gas_gap
        std::string quadname = quadname_ss.str(); 

        uint16_t chanATH, chanAB, chanNSW_A, chanNSW_C;
        std::string chan_string[] = { "pad", "strip", "wire" };
        
        // scan the vmm channel range, 1. for strips, 2. for wires, 3. for pads
        for (unsigned int channel_type : {Muon::nsw::OFFLINE_CHANNEL_TYPE_STRIP, Muon::nsw::OFFLINE_CHANNEL_TYPE_WIRE, Muon::nsw::OFFLINE_CHANNEL_TYPE_PAD}) {
          for (uint16_t vmm{0}; vmm < 8; ++vmm) {
            for (uint16_t vmm_chan{0}; vmm_chan < 64; ++vmm_chan) {

              chanATH   = m.channel_number(channel_type, sector_type, feb_radius, layer07, vmm, vmm_chan);
              if (!chanATH) continue; // disconnected channel

              chanAB    = m.Athena_to_AB_channel_number(channel_type, sector_type, feb_radius, layer07, chanATH);
              chanNSW_A = m.Athena_to_NSWID_channel_number(channel_type, sector_type, feb_radius, layer07, chanATH, true);
              chanNSW_C = m.Athena_to_NSWID_channel_number(channel_type, sector_type, feb_radius, layer07, chanATH, false);
              
              printf("%s   L%d %10s %4d %9d %8d %10d %8d %8d\n", quadname.c_str(), layer07+1, chan_string[channel_type].c_str(), vmm, vmm_chan, chanAB, chanATH, chanNSW_A, chanNSW_C);              
            } // eol vmm channel
          } // eol vmm 
        } // eol chan type
      } // eol layer
    } // eol quad radius
  } // eol sector size

  return 0;
}


