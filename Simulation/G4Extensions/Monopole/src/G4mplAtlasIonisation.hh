//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: G4mplIonisation.hh,v 1.1.1.1 2007-06-06 15:54:14 dellacqu Exp $
// GEANT4 tag $Name: not supported by cvs2svn $
//
// -------------------------------------------------------------------
//
// GEANT4 Class header file
//
//
// File name:     G4mplAtlasIonisation
//
// Author:        Vladimir Ivanchenko
//
// Creation date: 25.08.2005
//
// Modifications:
//
//
// Class Description:
//
// This class manages the ionisation process for a magnetic monopole
// it inherites from G4VContinuousDiscreteProcess via G4VEnergyLossProcess.
// Magnetic charge of the monopole should be defined in the constructor of
// the process, unless it is assumed that it is classic Dirac monopole with
// the charge 67.5*eplus. The name of the particle should be "monopole".
//

// -------------------------------------------------------------------
//

#ifndef MONOPOLE_G4mplAtlasIonisation_h
#define MONOPOLE_G4mplAtlasIonisation_h 1

#include "G4Version.hh"
#include "G4VEnergyLossProcess.hh"
#include "globals.hh"
#include "G4VEmModel.hh"

class G4Material;
class G4VEmFluctuationModel;

class G4mplAtlasIonisation : public G4VEnergyLossProcess
{

public:

  G4mplAtlasIonisation(G4double mCharge = 0.0, const G4String& name = "mplAtlasIonisation");

  virtual ~G4mplAtlasIonisation();

  G4bool IsApplicable(const G4ParticleDefinition& p);

  // Print out of the class parameters
  virtual void PrintInfo();

protected:

  std::vector<G4DynamicParticle*>*  SecondariesPostStep(
                                                        G4VEmModel*,
                                                        const G4MaterialCutsCouple*,
                                                        const G4DynamicParticle*,
                                                        G4double&);

  virtual void InitialiseEnergyLossProcess(const G4ParticleDefinition*,
                                           const G4ParticleDefinition*);

private:

  // hide assignment operator
  G4mplAtlasIonisation & operator=(const G4mplAtlasIonisation &right);
  G4mplAtlasIonisation(const G4mplAtlasIonisation&);

  G4double    magneticCharge;
  G4bool      isInitialised;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

inline G4bool G4mplAtlasIonisation::IsApplicable(const G4ParticleDefinition& p)
{
  G4String lowPartName = p.GetParticleName();
#if G4VERSION_NUMBER < 1100
  lowPartName.toLower();
  return  ( (lowPartName.contains("monopole")) ||
            (lowPartName.contains("dyon")) );
#else
  G4StrUtil::to_lower(lowPartName);
  return ( (G4StrUtil::contains(lowPartName, "monopole")) ||
           (G4StrUtil::contains(lowPartName, "dyon")) );
#endif
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

inline std::vector<G4DynamicParticle*>* G4mplAtlasIonisation::SecondariesPostStep(
                                                                                  G4VEmModel*,
                                                                                  const G4MaterialCutsCouple*,
                                                                                  const G4DynamicParticle*,
                                                                                  G4double&)
{
  return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#endif
