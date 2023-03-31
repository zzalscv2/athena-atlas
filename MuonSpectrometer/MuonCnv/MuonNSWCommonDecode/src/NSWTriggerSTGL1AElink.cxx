/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <vector>
#include <exception>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cmath>
#include "ers/ers.h"

#include "MuonNSWCommonDecode/NSWSTGTPDecodeBitmaps.h"

#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWTriggerSTGL1AElink.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"
#include "MuonNSWCommonDecode/STGTPPackets.h"

#define DBG_L std::cout << "E123: " << __LINE__ << " " << __FILE__ << std::endl;
Muon::nsw::NSWTriggerSTGL1AElink::NSWTriggerSTGL1AElink (const uint32_t *bs, const uint32_t remaining):
  NSWTriggerElink (bs, remaining)
{
  // 2 felix header 32b words already decoded;
  uint pp = 2 * 32;
  //once format finalized, checking a minimum size

  //NB bit_slice(start, end) includes edges
  m_head_fragID =                       bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_fragID-1);                     pp+= Muon::nsw::STGTPL1A::size_head_fragID;
  m_head_sectID =                       bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_sectID-1);                     pp+= Muon::nsw::STGTPL1A::size_head_sectID;
  m_head_EC =                           bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_EC-1);                         pp+= Muon::nsw::STGTPL1A::size_head_EC;
  m_head_flags =                        bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_flags-1);                      pp+= Muon::nsw::STGTPL1A::size_head_flags;
  m_head_BCID =                         bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_BCID-1);                       pp+= Muon::nsw::STGTPL1A::size_head_BCID;
  m_head_orbit =                        bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_orbit-1);                      pp+= Muon::nsw::STGTPL1A::size_head_orbit;
  m_head_spare =                        bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_spare-1);                      pp+= Muon::nsw::STGTPL1A::size_head_spare;
  m_L1ID =                              bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_L1ID-1);                            pp+= Muon::nsw::STGTPL1A::size_L1ID;
  m_head_wdw_open =                     bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_wdw_open-1);                   pp+= Muon::nsw::STGTPL1A::size_head_wdw_open;
  m_head_l1a_req =                      bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_l1a_req-1);                    pp+= Muon::nsw::STGTPL1A::size_head_l1a_req;
  m_head_wdw_close =                    bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_wdw_close-1);                  pp+= Muon::nsw::STGTPL1A::size_head_wdw_close;
  m_head_overflowCount =                bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_overflowCount-1);              pp+= Muon::nsw::STGTPL1A::size_head_overflowCount;
  m_head_wdw_matching_engines_usage =   bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_wdw_matching_engines_usage-1); pp+= Muon::nsw::STGTPL1A::size_head_wdw_matching_engines_usage;
  m_head_cfg_wdw_open_offset =          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_cfg_wdw_open_offset-1);        pp+= Muon::nsw::STGTPL1A::size_head_cfg_wdw_open_offset;
  m_head_cfg_l1a_req_offset =           bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_cfg_l1a_req_offset-1);         pp+= Muon::nsw::STGTPL1A::size_head_cfg_l1a_req_offset;
  m_head_cfg_wdw_close_offset =         bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_cfg_wdw_close_offset-1);       pp+= Muon::nsw::STGTPL1A::size_head_cfg_wdw_close_offset;
  m_head_cfg_timeout =                  bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_cfg_timeout-1);                pp+= Muon::nsw::STGTPL1A::size_head_cfg_timeout;
  m_head_link_const =                   bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_head_link_const-1);                 pp+= Muon::nsw::STGTPL1A::size_head_link_const;
  //this is important! It identifies the elink! It should be ABCD1230/ABCD1231/ABCD1232
  //not checked during decoding but must be checked at some point
  unsigned int max_pp = (m_wordCountFlx-1) * sizeof(uint32_t) * 8; // we use this to make sure we are consistent with the size
  while ( pp < remaining * sizeof(uint32_t) * 8 && pp < (m_wordCountFlx-1) * sizeof(uint32_t) * 8 ){
    // DEBUG
    ERS_DEBUG (2, "pp: " << pp << " rem: " << remaining << " wc: " << m_wordCountFlx );
    //a -2 needed cause remaining includes felix header words
    uint32_t current_stream_head_nbits =     bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_stream_head_nbits-1);     pp+= Muon::nsw::STGTPL1A::size_stream_head_nbits;
    uint32_t current_stream_head_nwords =    bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_stream_head_nwords-1);    pp+= Muon::nsw::STGTPL1A::size_stream_head_nwords;
    uint32_t current_stream_head_fifo_size = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_stream_head_fifo_size-1); pp+= Muon::nsw::STGTPL1A::size_stream_head_fifo_size;
    uint32_t current_stream_head_streamID =  bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_stream_head_streamID-1);  pp+= Muon::nsw::STGTPL1A::size_stream_head_streamID;

    //zero padding to multiples of 16bits - TP logic - this is the real number of bits to read
    current_stream_head_nbits = current_stream_head_nbits%16? ((current_stream_head_nbits+15)/16)*16 : current_stream_head_nbits;

    m_stream_head_nbits.push_back ( current_stream_head_nbits );
    m_stream_head_nwords.push_back ( current_stream_head_nwords );
    m_stream_head_fifo_size.push_back ( current_stream_head_fifo_size );
    m_stream_head_streamID.push_back ( current_stream_head_streamID );

    int dataSize = ceil(current_stream_head_nbits/32.0);
    std::vector<std::vector<uint32_t>> current_stream_data;

    // DEBUG
    ERS_DEBUG (2," stream_head_nbits: " << current_stream_head_nbits );
    ERS_DEBUG (2," stream_head_nwords: "  << current_stream_head_nwords);
    ERS_DEBUG (2," stream_head_fifo_size: " << current_stream_head_fifo_size);
    ERS_DEBUG (2," stream_head_streamID: " << current_stream_head_streamID);

    unsigned int total_expected_size = dataSize * current_stream_head_nwords;
    // DEBUG
    ERS_DEBUG (2, "total_expected_size: " << dataSize * current_stream_head_nwords);
    ERS_DEBUG (2, "m_wordCountFlx: " << m_wordCountFlx << " ceil(pp/32.0): " << ceil(pp/32.0));

    if (total_expected_size > m_wordCountFlx-ceil(pp/32.0) + 1)
    {
       throw std::length_error("STG stream inconsistent size inconsistent with expected packet size");
    }

    //this block data agnostic
    for (uint i = 0; i<current_stream_head_nwords; i++){
      std::vector<uint32_t> data;
      for (int j = 0; j < dataSize && pp < max_pp; j++, pp += 32){
        data.push_back( bit_slice<uint64_t,uint32_t>(bs, pp, pp+31));
      }
      current_stream_data.push_back(data);
    }
    m_stream_data.push_back(current_stream_data);
  }

  for (uint i=0; i<m_stream_data.size(); i++){
     for (uint j=0; j<m_stream_data.at(i).size(); j++)
     {
        if (m_stream_head_streamID[i] == Muon::nsw::STGTPPad::pad_stream_header){
          int inconsistent_message_size = (m_stream_data.at(i).at(j).size()  != std::ceil(m_stream_head_nbits[i]/32.0));
          if (inconsistent_message_size){
            throw std::length_error("Pad stream inconsistent size inconsistent with integer number of pad-messages");
          }
          std::vector<uint32_t> pad_packet = m_stream_data.at(i).at(j);
	  m_pad_packets.push_back( std::make_shared<STGTPPadPacket>(pad_packet));
          continue;
        }
        if (m_stream_head_streamID[i] == Muon::nsw::STGTPSegments::merge_stream_header){
          int inconsistent_message_size = (m_stream_data.at(i).at(j).size() != std::ceil(m_stream_head_nbits[i]/32.0));
          if (inconsistent_message_size){
            throw std::length_error("Merge stream inconsistent size inconsistent with integer number of segment-messages");
          }
          m_segment_packets.push_back( std::make_shared<Muon::nsw::STGTPSegmentPacket>(m_stream_data.at(i).at(j)));
          continue;
        }
      }
   }
  //warning: how the swROD is behaving if the last work is a uint16 only? Just 0-padding?
  m_trailer_CRC = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::STGTPL1A::size_trailer_CRC-1); pp+= Muon::nsw::STGTPL1A::size_trailer_CRC;

}
