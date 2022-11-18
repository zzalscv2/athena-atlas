/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "ers/ers.h"
#include "eformat/eformat.h"

#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMML1AElink.h"
#include "MuonNSWCommonDecode/NSWTriggerCommonDecoder.h"

#include <string>

Muon::nsw::NSWTriggerCommonDecoder::NSWTriggerCommonDecoder (const eformat::read::ROBFragment &robFrag, const std::string triggerType)
  : m_has_error (false),
    m_triggerType (triggerType)
{
  static const uint32_t s_min_packet_size = 4; //  2w felix header + 2w TP header (minimum scenario: 1 elink only)

  robFrag.check ();

  uint32_t nWords = robFrag.rod_ndata (); //total number of words (32-bit word)
  const uint32_t *bs = robFrag.rod_data (); //point directly to the first data element
  const uint32_t *pp = const_cast <uint32_t *> (bs); //pointer moving

  uint32_t wCount(0); // data-element (32-bit word) counter; it will increment according to each elink output

  ERS_DEBUG (1, "NWORDS FROM ROB HEADER: " << nWords);

  uint32_t remaining = nWords;

  while (remaining >= s_min_packet_size)
  {
    try
    {
      Muon::nsw::NSWTriggerElink* elink;

      if ( m_triggerType== "MML1A" ){
	       Muon::nsw::NSWTriggerMML1AElink* tmplink = new Muon::nsw::NSWTriggerMML1AElink(pp, remaining); elink = tmplink;
      } else {
	       Muon::nsw::NSWTriggerElink* tmplink = new Muon::nsw::NSWTriggerElink(pp, remaining); elink = tmplink;
      }

      m_elinks.push_back(elink);

      wCount += elink->nwords();
      pp += elink->nwords();



      ERS_DEBUG (1, "WORD COUNTER LAST ELINK: " << wCount);
      ERS_DEBUG (1, "CURRENT TOTAL NPACKETS: " << m_elinks.size ());

      remaining = nWords - wCount;
    }
    catch (Muon::nsw::NSWTriggerElinkException &e)
    {
      m_has_error = true;
      break;
    }
  }
}

Muon::nsw::NSWTriggerCommonDecoder::~NSWTriggerCommonDecoder ()
{
  for (auto i = m_elinks.begin (); i != m_elinks.end (); ++i)
    delete *i;
}
