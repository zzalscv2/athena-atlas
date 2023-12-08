/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_EscapedEnergyProcessing_H
#define ZDC_EscapedEnergyProcessing_H

// The SimulationEnergies class provides a common procedure for
// categorizing the energy deposited in a given G4Step.  However,
// special processing is required for escaped energy.

// The issue is that, if a particle's energy is lost by escaping the
// simulation, you don't want to record that energy in the volume it
// escapes; you want to record that energy in the volume in which the
// particle was created.  Neutrinos are a good example of this.

// In effect, the SimulationEnergies class has to issue an "interrupt"
// to some other volume than the current G4Step, telling that other
// volume to accumulate the energy of the escaped particle.

// This class contains the processing to handle the escaped energy
// processing for volumes in the ZdcG4 portion of the simulation.

#include "CaloG4Sim/VEscapedEnergyProcessing.h"

#include "G4TouchableHandle.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class ZDC_G4CalibSD;

class ZDC_EscapedEnergyProcessing : public CaloG4::VEscapedEnergyProcessing
{
public:
  ZDC_EscapedEnergyProcessing(ZDC_G4CalibSD* SD);
  ~ZDC_EscapedEnergyProcessing();

  // Method: The G4TouchableHandle to the volume in which "point" is
  // located; the value of "point" itself in case additional
  // processing is necessary, and the amount of escaped energy.

  G4bool Process( G4Step* fakeStep );

private:

  // Local pointer to the default SD that this should put hits into
  ZDC_G4CalibSD* m_defaultSD;
};


#endif // ZDC_G4_EscapedEnergyProcessing_H
