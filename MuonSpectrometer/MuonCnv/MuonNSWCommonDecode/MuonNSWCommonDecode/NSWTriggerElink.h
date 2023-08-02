/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_TRIGGER_ELINK_H_
#define _MUON_NSW_TRIGGER_ELINK_H_

#include <stdint.h>
#include <vector>
#include <exception>
#include <memory>

#include "MuonNSWCommonDecode/NSWResourceId.h"

namespace Muon
{
  namespace nsw
  {

    class NSWTriggerException: public std::exception
    {
    public:
      explicit NSWTriggerException (const std::string s, const int id = -999)
	: m_description(s), m_id(id) {};
      
      virtual const char *what () const throw () {return m_description.c_str ();};
      int id () const {return m_id;};

    private:
      std::string m_description;
      int m_id;
    };

    //giving a name to the exceptions
    enum NSWTriggerExceptions { 
      not_assigned = -999,   //default constructor valued for NSWTriggerException
      unknown      =   -1,   //unknown exception
      incomplete   =    0,   //incomplete decoding for ROBFrag
      too_short    =    1,   //shorter than minimum (2)
      flx_status   =    2,   //felix status different from 0
      flx_short    =    3,   //flx length shorter than bystream available
      mmmon_inv_id =    4,   //MMTP Mon elink invalid streamID
      mml1a_inv_id =    5,   //MMTP L1A elink invalid streamID
      mml1a_art_sz =    6,   //MMTP L1A ART stream data size invalid
      mml1a_trg_sz =    7    //MMTP L1A Trigger stream data size invalid
    };

    class NSWTriggerElink
    {
    public:
      
      NSWTriggerElink (const uint32_t *bs, uint32_t remaining);
      virtual ~NSWTriggerElink () = default;
      
      //general checks
      unsigned int nwords () const {return m_wordCount;}; //self counted or expected in case it will be possible 
      unsigned int nwordsFlx  () const {return m_wordCountFlx;}; //reading from packet felix header
      unsigned int status () const {return m_packet_status;}; //felix status
      bool suspect () const {return m_packet_sus;}; //elink decoded but with triggering suspects
      

      // Detector Logical ID and components
      uint32_t elinkWord  () const {return m_elinkWord;};
      const std::shared_ptr<Muon::nsw::NSWResourceId>& elinkId () const {return m_elinkId;};

    protected:
      unsigned int m_wordCount;
      unsigned int m_wordCountFlx;
      unsigned int m_packet_status;
      bool         m_packet_sus;

      //decoding felix header
      uint32_t m_elinkWord;
      std::shared_ptr<Muon::nsw::NSWResourceId> m_elinkId;

    };
  }
}


#endif // _MUON_NSW_TRIGGER_ELINK_H_
