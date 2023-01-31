/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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

    class NSWTriggerElinkException: public std::exception
    {
     public:
      explicit NSWTriggerElinkException (const char *s)
	: m_description (s) {};

      virtual const char *what () const throw () {return m_description.c_str ();};

     private:
      std::string m_description;
    };


    class NSWTriggerElink
    {
     public:

      NSWTriggerElink (const uint32_t *bs, uint32_t remaining);
      virtual ~NSWTriggerElink () = default;

      //general checks
      unsigned int nwords () const {return m_wordCount;}; //self counted or expected in case it will be possible 
      unsigned int nwordsFlx  () const {return m_wordCountFlx;}; //reading from packet header
      unsigned int status () const {return m_packet_status;};

      // Detector Logical ID and components
      uint32_t elinkWord  () const {return m_elinkWord;};
      const std::shared_ptr<Muon::nsw::NSWResourceId>& elinkId () const {return m_elinkId;};

     protected:
      unsigned int m_wordCount;
      unsigned int m_wordCountFlx;
      unsigned int m_packet_status;

      //decoding felix header
      uint32_t m_elinkWord;
      std::shared_ptr<Muon::nsw::NSWResourceId> m_elinkId;

    };
  }
}


#endif // _MUON_NSW_TRIGGER_ELINK_H_
