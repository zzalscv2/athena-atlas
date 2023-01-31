/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _MUON_NSW_MMTP_DECODE_BITMAPS_H_
#define _MUON_NSW_MMTP_DECODE_BITMAPS_H_

namespace Muon
{
  namespace nsw
  {
    namespace MMTPL1A {
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

    namespace MMTPMON {
      constexpr int size_head_fragID =                        4;
      constexpr int size_head_sectID =                        4;
      constexpr int size_head_EC =                            1;
      constexpr int size_head_flags =                         7;
      constexpr int size_head_BCID =                         12;
      constexpr int size_head_orbit =                         2;
      constexpr int size_head_spare =                         2;
      constexpr int size_L1ID =                              32;
      constexpr int size_head_coincBCID =                    12;
      constexpr int size_head_regionCount =                   4;
      constexpr int size_head_coincRegion =                   4;
      constexpr int size_head_reserved =                     44;
      constexpr int size_finder_streamID =                    8;
      constexpr int size_finder_regionCount =                 4;
      constexpr int size_finder_triggerID =                   4;
      constexpr int size_finder_V1 =                         16;
      constexpr int size_finder_V0 =                         16;
      constexpr int size_finder_U1 =                         16;
      constexpr int size_finder_U0 =                         16;
      constexpr int size_finder_X3 =                         16;
      constexpr int size_finder_X2 =                         16;
      constexpr int size_finder_X1 =                         16;
      constexpr int size_finder_X0 =                         16;
      constexpr int size_fitter_streamID =                    8;
      constexpr int size_fitter_regionCount =                 4;
      constexpr int size_fitter_triggerID =                   4;
      constexpr int size_fitter_filler =                     16;
      constexpr int size_fitter_mxG =                        16;
      constexpr int size_fitter_muG =                        16;
      constexpr int size_fitter_mvG =                        16;
      constexpr int size_fitter_mxL =                        16;
      constexpr int size_fitter_mx_ROI =                     16;
      constexpr int size_fitter_dTheta =                     16;
      constexpr int size_fitter_zero =                        2;
      constexpr int size_fitter_phiSign =                     1;
      constexpr int size_fitter_phiBin =                      5;
      constexpr int size_fitter_rBin =                        8;
      constexpr int size_trailer_CRC =                       16;
    };

    namespace MMART {
      constexpr int size_art_BCID =             11;
      constexpr int size_art_pipeID =            2;
      constexpr int size_art_fiberID =           3;
      constexpr int size_art_VMMmap =           32;
      constexpr int size_art_ARTs =              6;
    };

  }
}

template <typename T, typename X>
T bit_slice(const X words[], int start, int end){
  //start and end are the positions in the entire stream                                                                                                
  //start and end included;                                                                                                                             
  int wordSize = sizeof(X)*8;
  T s = 0;
  int n = end / wordSize;
  for(int i= 0; i <= n; ++i){
    s = (s << wordSize) + words[i]; //if T is too small, does not care, it's user fault                                                                 
    //when a fragment is splitted between N words, T should be at least of the size of N*words (in order to accomodate it)                              
  }
  s >>= (n+1) * wordSize - (end+1);
  T mask = (((T)1) << (end - start + 1))- 1; //len = end - start + 1                                                                                    
  s &= mask;
  return s;
}

#endif // _MUON_NSW_MMTP_DECODE_BITMAPS_H_
