/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _MUON_NSW_STGTP_DECODE_BITMAPS_H_
#define _MUON_NSW_STGTP_DECODE_BITMAPS_H_

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
      constexpr int pad_stream_header =                      0xAAD0;
      constexpr int n_words =                                 3; // size in 32 bit words
      constexpr int size_coincidence_wedge =                 16;
      constexpr int size_phiID_3 =                            6;
      constexpr int size_phiID_2 =                            6;
      constexpr int size_phiID_1 =                            6;
      constexpr int size_phiID_0 =                            6;
      constexpr int size_bandID_3 =                           8;
      constexpr int size_bandID_2 =                           8;
      constexpr int size_bandID_1 =                           8;
      constexpr int size_bandID_0 =                           8;
      constexpr int size_BCID =                              12;
      constexpr int size_spare =                              3;
      constexpr int size_idleFlag =                           1;
      constexpr int size_padding =                            8;

    };


    namespace STGTPSegments {
       constexpr int merge_stream_header  =              0xAEE0;
       constexpr int n_words =                                32; 
       constexpr int size_lut_choice_selection =              24;
       constexpr int size_nsw_segment_selector =              12;
       constexpr int size_valid_segment_selector =            12;
       
       constexpr int size_output_segment_7_monitor =           1;
       constexpr int size_output_segment_7_spare =             2;
       constexpr int size_output_segment_7_lowRes =            1;
       constexpr int size_output_segment_7_phiRes =            1;
       constexpr int size_output_segment_7_dTheta =            5;
       constexpr int size_output_segment_7_phiID =             6;
       constexpr int size_output_segment_7_rIndex =            8;

       constexpr int size_output_segment_6_monitor =           1;
       constexpr int size_output_segment_6_spare =             2;
       constexpr int size_output_segment_6_lowRes =            1;
       constexpr int size_output_segment_6_phiRes =            1;
       constexpr int size_output_segment_6_dTheta =            5;
       constexpr int size_output_segment_6_phiID =             6;
       constexpr int size_output_segment_6_rIndex =            8;

       constexpr int size_output_segment_5_monitor =           1;
       constexpr int size_output_segment_5_spare =             2;
       constexpr int size_output_segment_5_lowRes =            1;
       constexpr int size_output_segment_5_phiRes =            1;
       constexpr int size_output_segment_5_dTheta =            5;
       constexpr int size_output_segment_5_phiID =             6;
       constexpr int size_output_segment_5_rIndex =            8;

       constexpr int size_output_segment_4_monitor =           1;
       constexpr int size_output_segment_4_spare =             2;
       constexpr int size_output_segment_4_lowRes =            1;
       constexpr int size_output_segment_4_phiRes =            1;
       constexpr int size_output_segment_4_dTheta =            5;
       constexpr int size_output_segment_4_phiID =             6;
       constexpr int size_output_segment_4_rIndex =            8;

       constexpr int size_output_segment_3_monitor =           1;
       constexpr int size_output_segment_3_spare =             2;
       constexpr int size_output_segment_3_lowRes =            1;
       constexpr int size_output_segment_3_phiRes =            1;
       constexpr int size_output_segment_3_dTheta =            5;
       constexpr int size_output_segment_3_phiID =             6;
       constexpr int size_output_segment_3_rIndex =            8;

       constexpr int size_output_segment_2_monitor =           1;
       constexpr int size_output_segment_2_spare =             2;
       constexpr int size_output_segment_2_lowRes =            1;
       constexpr int size_output_segment_2_phiRes =            1;
       constexpr int size_output_segment_2_dTheta =            5;
       constexpr int size_output_segment_2_phiID =             6;
       constexpr int size_output_segment_2_rIndex =            8;

       constexpr int size_output_segment_1_monitor =           1;
       constexpr int size_output_segment_1_spare =             2;
       constexpr int size_output_segment_1_lowRes =            1;
       constexpr int size_output_segment_1_phiRes =            1;
       constexpr int size_output_segment_1_dTheta =            5;
       constexpr int size_output_segment_1_phiID =             6;
       constexpr int size_output_segment_1_rIndex =            8;

       constexpr int size_output_segment_0_monitor =           1;
       constexpr int size_output_segment_0_spare =             2;
       constexpr int size_output_segment_0_lowRes =            1;
       constexpr int size_output_segment_0_phiRes =            1;
       constexpr int size_output_segment_0_dTheta =            5;
       constexpr int size_output_segment_0_phiID =             6;
       constexpr int size_output_segment_0_rIndex =            8;

       constexpr int size_bcid             =                   12;
       constexpr int size_sectorID         =                   4;
 

    };

  }
}

#endif // _MUON_NSW_STGTP_DECODE_BITMAPS_H_
