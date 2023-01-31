/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SLEPTONS_G4SElectronLMinus_H
#define SLEPTONS_G4SElectronLMinus_H

#include "globals.hh"
#include "G4ios.hh"
#include "G4ParticleDefinition.hh"

#include "CxxUtils/checker_macros.h"

class G4SElectronLMinus: public G4ParticleDefinition
{
  // singleton implementation

private:

  static G4SElectronLMinus* theInstance ATLAS_THREAD_SAFE; // used in single-threaded Geant4 initialization
  G4SElectronLMinus(){}
  ~G4SElectronLMinus(){}

public:

  static G4SElectronLMinus* Definition(G4double mass=-1, G4double width=-1, G4double charge=-1, G4double PDG=-1, G4bool stable=true, G4double lifetime=-1, G4bool shortlived=false);

};

#endif // SLEPTONS_G4SElectronLMinus_H
