// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODFORWARD_ZDCMODULE_H
#define XAODFORWARD_ZDCMODULE_H


// Local include(s):
#include "xAODForward/versions/ZdcModule_v1.h"

namespace xAOD {
  typedef ZdcModule_v1 ZdcModule;
}

// Declare a CLID for the type:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::ZdcModule, 144816136, 1 )

#endif // XAODFORWARD_ZDCMODULE_H
