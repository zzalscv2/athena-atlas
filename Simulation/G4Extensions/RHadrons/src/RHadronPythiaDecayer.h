/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RHadronPythiaDecayer_H
#define RHadronPythiaDecayer_H

#include "G4ExternalDecay/Pythia8ForDecays.h"

#include "G4VExtDecayer.hh"
#include "G4Track.hh"
#include <string>

class G4DecayProducts;

class RHadronPythiaDecayer: public G4VExtDecayer
{
  public:
   RHadronPythiaDecayer( const std::string& s );
   virtual G4DecayProducts* ImportDecayProducts(const G4Track&);
  private:
   Pythia8ForDecays m_pythia;
};

#endif
