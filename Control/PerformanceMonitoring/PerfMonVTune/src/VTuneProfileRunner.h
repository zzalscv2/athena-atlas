/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VTUNE_PROFILERUNNER_H
#define VTUNE_PROFILERUNNER_H

// VTune include(s)
#include <ittnotify.h>

// Athena include(s)
#include "CxxUtils/checker_macros.h"

class ATLAS_NOT_THREAD_SAFE VTuneProfileRunner {

  public:

      /// Standard constructor uses the API to resume collection
      VTuneProfileRunner() { __itt_resume(); }
      /// Standard destructor uses the API to pause collection
      ~VTuneProfileRunner() { __itt_pause(); }

};

#endif // VTUNE_PROFILERUNNER_H 
