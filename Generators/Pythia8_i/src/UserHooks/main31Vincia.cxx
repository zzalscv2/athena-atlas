/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "UserHooksUtils.h"
#include "UserSetting.h"
#include "Pythia8_i/UserHooksFactory.h"
#ifdef PYTHIA_VERSION_INTEGER
  #if PYTHIA_VERSION_INTEGER > 8308
    #include "Pythia8Plugins/PowhegHooks.h"
  #else
    #include "Pythia8Plugins/PowhegHooksVincia.h"
  #endif
#endif
namespace Pythia8{
#ifdef PYTHIA_VERSION_INTEGER
  #if PYTHIA_VERSION_INTEGER > 8308
     Pythia8_UserHooks::UserHooksFactory::Creator<Pythia8::PowhegHooks> PowhegHooksVinciaCreator("PowhegMain31Vincia");
  #else
     Pythia8_UserHooks::UserHooksFactory::Creator<Pythia8::PowhegHooksVincia> PowhegHooksVinciaCreator("PowhegMain31Vincia");
  #endif
#endif
}
