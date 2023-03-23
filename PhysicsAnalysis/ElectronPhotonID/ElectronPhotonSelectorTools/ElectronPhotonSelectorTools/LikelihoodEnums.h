/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _LIKELIHOODENUMS_H
#define _LIKELIHOODENUMS_H


namespace LikeEnum {
  enum Menu {
    VeryLoose,
    Loose,
    LooseBL,
    Medium,
    Tight,
    VeryTight,
    LooseRelaxed,
    CustomOperatingPoint,
    VeryLooseLLP,
    LooseLLP,
    MediumLLP,
    TightLLP
  };

   struct ROOT6_NamespaceAutoloadHook{};
}

#endif
