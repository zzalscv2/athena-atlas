/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "UserHooksUtils.h"
#include "UserSetting.h"
#include "Pythia8_i/UserHooksFactory.h"
#if PYTHIA_VERSION_INTEGER >= 8310
// Disable the pythia 8.310 plugin mechanism.
// We use our own mechanism to create these classes, and we get link errors
// if these are used in more than one compilation unit.
# include "Pythia8/Plugins.h"
# undef PYTHIA8_PLUGIN_CLASS
# undef PYTHIA8_PLUGIN_VERSIONS
# define PYTHIA8_PLUGIN_CLASS(BASE, CLASS, PYTHIA, SETTINGS, LOGGER)
# define PYTHIA8_PLUGIN_VERSIONS(...)
#endif
#include "Pythia8Plugins/PowhegHooks.h"

namespace Pythia8{
  Pythia8_UserHooks::UserHooksFactory::Creator<Pythia8::PowhegHooks> PowhegHooksCreator("PowhegMain31");
}


