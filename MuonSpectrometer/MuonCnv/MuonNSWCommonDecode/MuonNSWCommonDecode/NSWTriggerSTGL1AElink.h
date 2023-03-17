/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_TRIGGER_STGL1A_ELINK_H_
#define _MUON_NSW_TRIGGER_STGL1A_ELINK_H_

#include <stdint.h>
#include <vector>
#include <exception>

#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWSTGTPDecodeBitmaps.h"
#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"
namespace Muon
{
  namespace nsw
  {
    class NSWResourceId;

    class NSWTriggerElinkException;

    class STGTPPadPacket;
    class STGTPSegmentPacket;

    class NSWTriggerSTGL1AElink : public NSWTriggerElink
    {
     public:

      NSWTriggerSTGL1AElink (const uint32_t *bs, uint32_t remaining);
      virtual ~NSWTriggerSTGL1AElink () = default;


      uint32_t head_fragID () const {return m_head_fragID;};
      uint32_t head_sectID () const {return m_head_sectID;};
      uint32_t head_EC () const {return m_head_EC;};
      uint32_t head_flags () const {return m_head_flags;};
      uint32_t head_BCID () const {return m_head_BCID;};
      uint32_t head_orbit () const {return m_head_orbit;};
      uint32_t head_spare () const {return m_head_spare;};
      uint32_t L1ID () const {return m_L1ID;};
      uint32_t head_wdw_open () const {return m_head_wdw_open;};
      uint32_t head_l1a_req () const {return m_head_l1a_req;};
      uint32_t head_wdw_close () const {return m_head_wdw_close;};
      uint32_t head_overflowCount () const {return m_head_overflowCount;};
      uint32_t head_wdw_matching_engines_usage () const {return m_head_wdw_matching_engines_usage;};
      uint32_t head_cfg_wdw_open_offset () const {return m_head_cfg_wdw_open_offset;};
      uint32_t head_cfg_l1a_req_offset () const {return m_head_cfg_l1a_req_offset;};
      uint32_t head_cfg_wdw_close_offset () const {return m_head_cfg_wdw_close_offset;};
      uint32_t head_cfg_timeout () const {return m_head_cfg_timeout;};
      uint32_t head_link_const () const {return m_head_link_const;};
      const std::vector<uint32_t>& stream_head_nbits () const {return  m_stream_head_nbits;};
      const std::vector<uint32_t>& stream_head_nwords () const {return  m_stream_head_nwords;};
      const std::vector<uint32_t>& stream_head_fifo_size () const {return  m_stream_head_fifo_size;};
      const std::vector<uint32_t>& stream_head_streamID () const {return m_stream_head_streamID;};
      const std::vector<std::vector<std::vector<uint32_t>>> stream_data () const {return  m_stream_data;};
      uint32_t trailer_CRC () const {return  m_trailer_CRC;};

      const std::vector<std::shared_ptr<STGTPPadPacket>>& pad_packets () const {return m_pad_packets;};
      const std::vector<std::shared_ptr<STGTPSegmentPacket>>& segment_packet () const {return m_segment_packets;};

     private:

      uint32_t m_head_fragID;
      uint32_t m_head_sectID;
      uint32_t m_head_EC;
      uint32_t m_head_flags;
      uint32_t m_head_BCID;
      uint32_t m_head_orbit;
      uint32_t m_head_spare;
      uint32_t m_L1ID;
      uint32_t m_head_wdw_open;
      uint32_t m_head_l1a_req;
      uint32_t m_head_wdw_close;
      uint32_t m_head_overflowCount;
      uint32_t m_head_wdw_matching_engines_usage;
      uint32_t m_head_cfg_wdw_open_offset;
      uint32_t m_head_cfg_l1a_req_offset;
      uint32_t m_head_cfg_wdw_close_offset;
      uint32_t m_head_cfg_timeout;
      uint32_t m_head_link_const;
      std::vector<uint32_t> m_stream_head_nbits;
      std::vector<uint32_t> m_stream_head_nwords;
      std::vector<uint32_t> m_stream_head_fifo_size;
      std::vector<uint32_t> m_stream_head_streamID;
      std::vector<std::vector<std::vector<uint32_t>>> m_stream_data; //size is potentially not known a priori...
      //first vector had stream index
      //second vector contains stream data words - length defined by m_stream_head_nwords
      //third vector used because stream data size (m_stream_head_nwords) can exceed maximum compiler size (uint64_t)
      uint32_t m_trailer_CRC;

      std::vector<std::shared_ptr<STGTPPadPacket>> m_pad_packets;
      std::vector<std::shared_ptr<STGTPSegmentPacket>> m_segment_packets; 
    };
  }
}


#endif // _MUON_NSW_TRIGGER_STGL1A_ELINK_H_
