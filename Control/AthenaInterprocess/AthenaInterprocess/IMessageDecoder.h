/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAINTERPROCESS_IMESSAGEDECODER_H
#define ATHENAINTERPROCESS_IMESSAGEDECODER_H

#include "CxxUtils/checker_macros.h"
#include <memory>

namespace AthenaInterprocess {
  struct ScheduledWork {
    void* data;
    int size;
  };

  class IMessageDecoder
  {
  public:
    virtual ~IMessageDecoder() {}

    virtual std::unique_ptr<ScheduledWork> operator() ATLAS_NOT_THREAD_SAFE (const ScheduledWork&) = 0;
  };
}  

#endif
