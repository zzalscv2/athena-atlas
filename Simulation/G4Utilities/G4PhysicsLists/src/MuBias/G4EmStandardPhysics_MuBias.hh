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
//
// $Id: G4EmStandardPhysics.hh,v 1.5 2010-06-02 17:21:29 vnivanch Exp $
// GEANT4 tag $Name: geant4-09-04-patch-01 $
//
//---------------------------------------------------------------------------
//
// ClassName:   G4EmStandardPhysics
//
// Author:      V.Ivanchenko 09.11.2005
//
// Modified:
// 05.12.2005 V.Ivanchenko add controlled verbosity
// 23.11.2006 V.Ivanchenko remove mscStepLimit option and improve cout
//
//----------------------------------------------------------------------------
//
// This class provides construction of default EM standard physics
//

#ifndef G4EmStandardPhysics_MuBias_h
#define G4EmStandardPhysics_MuBias_h 1

#include "G4VPhysicsConstructor.hh"
#include "globals.hh"

struct biasValues {
	double bremsBias;
	double pairBias;
	biasValues(): bremsBias(1.),pairBias(1.) {}
	biasValues(int a,int b): bremsBias(a),pairBias(b) {}
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class G4EmStandardPhysics_MuBias : public G4VPhysicsConstructor
{
public:
  G4EmStandardPhysics_MuBias(G4int ver = 0);

  // obsolete
  G4EmStandardPhysics_MuBias(G4int ver, const G4String& name);

  virtual ~G4EmStandardPhysics_MuBias();

  virtual void ConstructParticle();
  virtual void ConstructProcess();

private:
  G4int  m_verbose;
  biasValues m_biases;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif






