/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "eformat/eformat.h"

#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMML1AElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMMMonElink.h"
#include "MuonNSWCommonDecode/NSWTriggerSTGL1AElink.h"
#include "MuonNSWCommonDecode/NSWPadTriggerL1a.h"
#include "MuonNSWCommonDecode/NSWTriggerCommonDecoder.h"

#include <string>

Muon::nsw::NSWTriggerCommonDecoder::NSWTriggerCommonDecoder (const eformat::read::ROBFragment &robFrag, const std::string& triggerType)
  : m_has_error (false),
    m_triggerType (triggerType)
{
  static const uint32_t s_min_packet_size = 2; //  2w felix header

  robFrag.check ();

  uint32_t nWords = robFrag.rod_ndata (); //total number of words (32-bit word)
  const uint32_t *bs = robFrag.rod_data (); //point directly to the first data element
  const uint32_t *pp = const_cast <uint32_t *> (bs); //pointer moving

  uint32_t wCount(0); // data-element (32-bit word) counter; it will increment according to each elink output

  uint32_t remaining = nWords;

  while (remaining >= s_min_packet_size)
  {
    try
    {
      std::shared_ptr<Muon::nsw::NSWTriggerElink> elink;

      if ( m_triggerType== "MML1A" ){
	std::shared_ptr<Muon::nsw::NSWTriggerMML1AElink> tmplink = std::make_shared<Muon::nsw::NSWTriggerMML1AElink>(pp, remaining); 
	elink = tmplink;
      } else if ( m_triggerType== "MMMon" ) {
	std::shared_ptr<Muon::nsw::NSWTriggerMMMonElink> tmplink = std::make_shared<Muon::nsw::NSWTriggerMMMonElink>(pp, remaining);
	elink = tmplink;
      } else if ( m_triggerType== "PadL1A" ) {
	std::shared_ptr<Muon::nsw::NSWPadTriggerL1a> tmplink = std::make_shared<Muon::nsw::NSWPadTriggerL1a>(pp, remaining);
	elink = tmplink;
      } else if ( m_triggerType== "STGL1A" ) {
        std::shared_ptr<Muon::nsw::NSWTriggerSTGL1AElink> tmplink = std::make_shared<Muon::nsw::NSWTriggerSTGL1AElink>(pp, remaining);
        elink = tmplink;
      } else {
	std::shared_ptr<Muon::nsw::NSWTriggerElink> tmplink = std::make_shared<Muon::nsw::NSWTriggerElink>(pp, remaining);
	elink = tmplink;
      }

      m_elinks.push_back(elink);

      wCount += elink->nwords();
      pp += elink->nwords();
      remaining = nWords - wCount;

    }
    catch (Muon::nsw::NSWTriggerElinkException &e)
    {
      //could think of an error msg print in case needed
      m_has_error = true;
      break;
    }    
  }
}

