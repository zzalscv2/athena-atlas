/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


#include "Sleptons/G4STauMinus.hh"
// ######################################################################
// ###                         STauMinus                              ###
// ######################################################################

G4STauMinus* G4STauMinus::theInstance = Definition();

G4STauMinus* G4STauMinus::Definition()
{
  if (theInstance !=0) return theInstance;

//    Arguments for constructor are as follows
//               name             mass          width         charge
//             2*spin           parity  C-conjugation
//          2*Isospin       2*Isospin3       G-parity
//               type    lepton number  baryon number   PDG encoding
//             stable         lifetime    decay table
//             shortlived      subType    anti_encoding

  G4ParticleDefinition* anInstance = new G4ParticleDefinition(
      "s_tau_minus_1",       100.*CLHEP::GeV,       0.0*CLHEP::MeV,    -1.*CLHEP::eplus, 
		    0,               0,             0,          
		    0,               0,             0,             
	    "slepton",               1,             0,       1000015,
                 true,            -1.0,          NULL,
                false,     "STauMinus");

  theInstance = reinterpret_cast<G4STauMinus*>(anInstance);
  return theInstance;
}
