/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CHARGINOS_AMSBNeutralino_H
#define CHARGINOS_AMSBNeutralino_H

#include "globals.hh"
#include "G4ios.hh"
#include "G4ParticleDefinition.hh"

#include "CxxUtils/checker_macros.h"

class AMSBNeutralino: public G4ParticleDefinition
{
private:

  static AMSBNeutralino* s_theInstance ATLAS_THREAD_SAFE; // used in single-threaded Geant4 initialization
  AMSBNeutralino(){}
  ~AMSBNeutralino(){}

public:

  static AMSBNeutralino* Definition(G4double mass=-1, G4double width=-1, G4double charge=-1, G4double PDG=-1, G4bool stable=true, G4double lifetime=-1, G4bool shortlived=false);

};
#endif //CHARGINOS_AMSBNeutralino_H
