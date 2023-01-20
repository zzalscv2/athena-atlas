//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
//  This is the (right-hand side for) equation of motion in a combined
//    magnetic and electric field for ordinary charged partilces
//    and magnetic monopoles (with magnetic charge.)
//
//  Was based on G4EqMagElectricField.hh (by V.Grichine)
//
//  30.04.10   S.Burdin (modified to use for the monopole trajectories).
//
// -------------------------------------------------------------------

// class header
#include "G4mplEqMagElectricField.hh"

// STL headers
#include <iomanip>

// CLHEP headers
#include "CLHEP/Units/PhysicalConstants.h"

// Geant4 headers
#include "globals.hh"
#include "G4Version.hh"

G4mplEqMagElectricField::G4mplEqMagElectricField(G4MagneticField *emField )
  : G4Mag_EqRhs( emField )
  , fMassCof(100000. )
  , fElCharge( 0 )
  , fMagCharge( 0 )
  , init( false ) {

}

void
G4mplEqMagElectricField::SetChargeMomentumMass(G4ChargeState particleCharge,
                                               G4double MomentumXc,
                                               G4double particleMass)
{
  init = true;

  fElCharge =CLHEP::eplus* particleCharge.GetCharge()*CLHEP::c_light;
  fMagCharge = (particleMass < 0.0) ?  CLHEP::eplus*particleCharge.MagneticCharge()*CLHEP::c_light  : 0.0;   // protection against ordinary particles

  fMassCof = particleMass*particleMass ;

  G4Mag_EqRhs::SetChargeMomentumMass(particleCharge, MomentumXc, particleMass); // allows electrically charged particles to use the AtlasRK4 and NystromRK4 propagators (still don't work with magnetic charges)

}

void
G4mplEqMagElectricField::EvaluateRhsGivenB(const G4double y[],
                                           const G4double Field[],
                                           G4double dydx[] ) const
{
  // Components of y:
  //    0-2 dr/ds,
  //    3-5 dp/ds - momentum derivatives

  if (!init)G4Exception("G4mplEqMagElectricField::EvaluateRhsGivenB","NotInitialized",FatalException,"SetChargeMomentumMass has not been called");

  const G4double pSquared = y[3]*y[3] + y[4]*y[4] + y[5]*y[5] ;

  const G4double Energy   = std::sqrt( pSquared + fMassCof );

  // const G4double pModuleInverse = (pSquared <= 0.0) ? 0.0 : 1.0/std::sqrt(pSquared);
  const G4double pModuleInverse = 1.0/std::sqrt(pSquared);

  const G4double inverse_velocity = Energy * pModuleInverse / CLHEP::c_light;

  const G4double cofEl  = fElCharge * pModuleInverse ;
  const G4double cofMag = fMagCharge * Energy * pModuleInverse;


  dydx[0] = y[3]*pModuleInverse ;
  dydx[1] = y[4]*pModuleInverse ;
  dydx[2] = y[5]*pModuleInverse ;

  // dp/ds = dp/dt * dt/ds = dp/dt / v = Force / velocity

  dydx[3] = cofMag * Field[0] + cofEl * (y[4]*Field[2] - y[5]*Field[1]);
  dydx[4] = cofMag * Field[1] + cofEl * (y[5]*Field[0] - y[3]*Field[2]);
  dydx[5] = cofMag * Field[2] + cofEl * (y[3]*Field[1] - y[4]*Field[0]);

  dydx[6] = 0.;//not used

  ////////

  // Lab Time of flight
  dydx[7] = inverse_velocity;

  return ;
}
