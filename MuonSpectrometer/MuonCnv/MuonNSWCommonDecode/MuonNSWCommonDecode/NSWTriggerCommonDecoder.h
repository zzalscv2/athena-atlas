/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_TRIGGER_COMMON_DECODER_H_
#define _MUON_NSW_TRIGGER_COMMON_DECODER_H_

#include <stdint.h>
#include <vector>
#include <string>

#include "eformat/eformat.h"

namespace Muon
{
  namespace nsw
  {
    class NSWTriggerElink;

    class NSWTriggerCommonDecoder
    {
     public:
      explicit NSWTriggerCommonDecoder (const eformat::read::ROBFragment &rob, const std::string triggerType);
      //will use trigger type for stgc/mm/pad specialization?
      virtual ~NSWTriggerCommonDecoder ();

      //it's up to the user dynamic casting to needed elink type
      //or we can make this a template but then loosing the ability to change trigger type at run time
      const std::vector <Muon::nsw::NSWTriggerElink *> &get_elinks () const {return m_elinks;};

      bool has_error () {return m_has_error;};

     private:
      bool m_has_error;
      std::string m_triggerType;
      std::vector <Muon::nsw::NSWTriggerElink *> m_elinks;
    };

  }
}

#endif // _MUON_NSW_TRIGGER_COMMON_DECODER_H_
