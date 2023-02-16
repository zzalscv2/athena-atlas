/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONNSWCMOMONDECODE_NSWTRIGGERCOMMONDECODER_H
#define MUONNSWCMOMONDECODE_NSWTRIGGERCOMMONDECODER_H

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
      explicit NSWTriggerCommonDecoder (const eformat::read::ROBFragment &rob, const std::string& triggerType);
      //will use trigger type for stgc/mm/pad specialization?
      virtual ~NSWTriggerCommonDecoder () = default;

      //it's up to the user dynamic casting to needed elink type
      //or we can make this a template but then loosing the ability to change trigger type at run time
      const std::vector<std::shared_ptr<Muon::nsw::NSWTriggerElink>>& get_elinks () const {return m_elinks;};

      bool has_error () {return m_has_error;};

     private:
      bool m_has_error;
      std::string m_triggerType;
      std::vector<std::shared_ptr<Muon::nsw::NSWTriggerElink>> m_elinks;
    };

  }
}

#endif // not MUONNSWCMOMONDECODE_NSWTRIGGERCOMMONDECODER_H
