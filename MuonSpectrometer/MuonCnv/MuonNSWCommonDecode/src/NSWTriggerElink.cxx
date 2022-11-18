/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include <vector>
#include <exception>
#include <sstream>

#include "ers/ers.h"

#include "MuonNSWCommonDecode/NSWTriggerElink.h"

Muon::nsw::NSWTriggerElink::NSWTriggerElink (const uint32_t *bs, const uint32_t remaining)
  : m_wordCount (0)
{
  // Felix header (2 words)
  uint32_t word = bs[m_wordCount++];
  unsigned int packet_nbytes = Muon::nsw::helper::get_bits (word, Muon::nsw::bitMaskFlxLENGTH, Muon::nsw::bitPosFlxLENGTH);
  m_wordCountFlx = packet_nbytes / sizeof (uint32_t);
  m_packet_status  = Muon::nsw::helper::get_bits (word, Muon::nsw::bitMaskFlxSTATUS, Muon::nsw::bitPosFlxSTATUS);

  if (m_packet_status != 0)
  {
    std::ostringstream s;
    s << "Packet status in FELIX header 0x" << std::hex << m_packet_status << std::dec;
    Muon::nsw::NSWTriggerElinkException e (s.str ().c_str ());
    throw e;
  }

  if (remaining * sizeof (uint32_t) < packet_nbytes)
  {
    std::ostringstream s;
    s << "Packet length in FELIX header " << packet_nbytes << " is larger than available data";
    Muon::nsw::NSWTriggerElinkException e (s.str ().c_str ());
    throw e;
  }

  m_elinkWord = bs[m_wordCount++];
  m_elinkId.reset(new Muon::nsw::NSWResourceId (m_elinkWord));

  m_wordCount=m_wordCountFlx; //just trusting felix... is felix m_wordCountFlx incliding felix header size? Should I use +=?

}
