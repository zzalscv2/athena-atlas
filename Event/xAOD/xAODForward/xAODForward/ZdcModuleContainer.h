// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODFORWARD_ZDCMODULECONTAINER_H
#define XAODFORWARD_ZDCMODULECONTAINER_H

// EDM include(s):

// Local include(s):

#include "xAODForward/ZdcModule.h"
#include "xAODForward/versions/ZdcModuleContainer_v1.h"

namespace xAOD {
   typedef ZdcModuleContainer_v1 ZdcModuleContainer;
}

// Set up a CLID for the container:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::ZdcModuleContainer, 1110760440, 1 )

#endif // XAODFORWARD_ZDCMODULECONTAINER_H
