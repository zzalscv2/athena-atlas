/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include <vector>
#include <exception>
#include <sstream>
#include <string>

#include "ers/ers.h"

#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"

#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMML1AElink.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"

#include <bitset>

Muon::nsw::NSWTriggerMML1AElink::NSWTriggerMML1AElink (const uint32_t *bs, const uint32_t remaining):
  NSWTriggerElink (bs, remaining)
{
  // 2 felix header 32b words already decoded;
  uint pp = 2 * 32;

  //once format finalized, checking a minimum size

  //NB bit_slice(start, end) includes edges
  m_head_fragID =                       bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_fragID-1);                     pp+= Muon::nsw::MMTPL1A::size_head_fragID;
  m_head_sectID =                       bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_sectID-1);                     pp+= Muon::nsw::MMTPL1A::size_head_sectID;
  m_head_EC =                           bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_EC-1);                         pp+= Muon::nsw::MMTPL1A::size_head_EC;
  m_head_flags =                        bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_flags-1);                      pp+= Muon::nsw::MMTPL1A::size_head_flags;
  m_head_BCID =                         bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_BCID-1);                       pp+= Muon::nsw::MMTPL1A::size_head_BCID;
  m_head_orbit =                        bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_orbit-1);                      pp+= Muon::nsw::MMTPL1A::size_head_orbit;
  m_head_spare =                        bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_spare-1);                      pp+= Muon::nsw::MMTPL1A::size_head_spare;
  m_L1ID =                              bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_L1ID-1);                            pp+= Muon::nsw::MMTPL1A::size_L1ID;
  m_head_wdw_open =                     bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_wdw_open-1);                   pp+= Muon::nsw::MMTPL1A::size_head_wdw_open;
  m_head_l1a_req =                      bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_l1a_req-1);                    pp+= Muon::nsw::MMTPL1A::size_head_l1a_req;
  m_head_wdw_close =                    bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_wdw_close-1);                  pp+= Muon::nsw::MMTPL1A::size_head_wdw_close;
  m_head_overflowCount =                bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_overflowCount-1);              pp+= Muon::nsw::MMTPL1A::size_head_overflowCount;
  m_head_wdw_matching_engines_usage =   bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_wdw_matching_engines_usage-1); pp+= Muon::nsw::MMTPL1A::size_head_wdw_matching_engines_usage;
  m_head_cfg_wdw_open_offset =          bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_cfg_wdw_open_offset-1);        pp+= Muon::nsw::MMTPL1A::size_head_cfg_wdw_open_offset;
  m_head_cfg_l1a_req_offset =           bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_cfg_l1a_req_offset-1);         pp+= Muon::nsw::MMTPL1A::size_head_cfg_l1a_req_offset;
  m_head_cfg_wdw_close_offset =         bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_cfg_wdw_close_offset-1);       pp+= Muon::nsw::MMTPL1A::size_head_cfg_wdw_close_offset;
  m_head_cfg_timeout =                  bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_cfg_timeout-1);                pp+= Muon::nsw::MMTPL1A::size_head_cfg_timeout;
  m_head_link_const =                   bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_head_link_const-1);                 pp+= Muon::nsw::MMTPL1A::size_head_link_const;
  //this is important! It identifies the elink! It should be ABCD1230/ABCD1231/ABCD1232
  //not checked during decoding but must be checked at some point

  while ( pp < (remaining-2) * sizeof(uint32_t) ){
    //a -2 needed cause remaining includes felix header words
    uint32_t current_stream_head_nbits =     bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_stream_head_nbits-1);     pp+= Muon::nsw::MMTPL1A::size_stream_head_nbits;
    uint32_t current_stream_head_nwords =    bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_stream_head_nwords-1);    pp+= Muon::nsw::MMTPL1A::size_stream_head_nwords;
    uint32_t current_stream_head_fifo_size = bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_stream_head_fifo_size-1); pp+= Muon::nsw::MMTPL1A::size_stream_head_fifo_size;
    uint32_t current_stream_head_streamID =  bit_slice<uint64_t,uint32_t>(bs, pp, pp+Muon::nsw::MMTPL1A::size_stream_head_streamID-1);  pp+= Muon::nsw::MMTPL1A::size_stream_head_streamID;

    m_stream_head_nbits.push_back ( current_stream_head_nbits );
    m_stream_head_nwords.push_back ( current_stream_head_nwords );
    m_stream_head_fifo_size.push_back ( current_stream_head_fifo_size );
    m_stream_head_streamID.push_back ( current_stream_head_streamID );

    int dataSize = (current_stream_head_nbits-1)/32 +1; //how many words needed
    std::vector<std::vector<uint32_t>> current_stream_data;

    //this block is very generic and agnostic of MMTP stream data format!
    for (uint i = 0; i<current_stream_head_nwords; i++){
      std::vector<uint32_t> data;
      for (int j = 0; j < dataSize-1; j++){
        data.push_back( bit_slice<uint64_t,uint32_t>(bs, pp+32*j, pp+32*(j+1)-1) );
      }
      data.push_back( bit_slice<uint64_t,uint32_t>(bs, pp+32*(dataSize-1),pp+32*(dataSize-1)+(current_stream_head_nbits-1)%32) );
      current_stream_data.push_back(data);
      pp+=current_stream_head_nbits;
    }
    m_stream_data.push_back(current_stream_data);

  }


  //will uncomment this once a dedicated ART Packet class has been created
  /*
  for (int i=0; i<m_stream_data.size(); i++){
    if (m_stream_head_streamID[i]!=0xAAA4){
      //the it's an ART packet
      for (int j = 0; j < m_stream_data[i].size(); j++){
          ARTPacket packet = ARTPacket(m_stream_data[i][j]); //arg will be a vector<uint32_t> of size 3
          //will see if it's will be preferred to add a list of arts as class members or directly art attributes unpacked
      }
    }
  }
  */

  //warning: how the swROD is behaving if the last work is a uint16 only? Just 0-padding?
  m_trailer_CRC = bit_slice<uint64_t,uint32_t>(bs, pp, Muon::nsw::MMTPL1A::size_trailer_CRC-1); pp+= Muon::nsw::MMTPL1A::size_trailer_CRC;

}

