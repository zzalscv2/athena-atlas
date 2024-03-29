/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4GENSHOWERLIB_TestActionShowerLib_H
#define LARG4GENSHOWERLIB_TestActionShowerLib_H

#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include "LArG4Code/ILArCalculatorSvc.h"

// Geant4 includes
#include "G4UserEventAction.hh"
#include "G4UserRunAction.hh"
#include "G4UserSteppingAction.hh"


// forward declarations in namespaces
namespace ShowerLib {
  class StepInfoCollection;
}
#include "AtlasHepMC/GenParticle_fwd.h"
// forward declarations in global namespace
class EnergyCalculator;
class G4VSolid;
class G4AffineTransform;


namespace G4UA
{

  /**
   *
   *   @short Class for collecting G4 hit information
   *
   *          Collect and store Geant4 hit information, i.e.
   *          position, deposited energy and time, from hits
   *          for the creation of a shower library
   *
   *  @author Wolfgang Ehrenfeld, University of Hamburg, Germany
   *  @author Sasha Glazov, DESY Hamburg, Germany
   *
   *
   */

  class TestActionShowerLib:
  public G4UserEventAction, public G4UserRunAction, public G4UserSteppingAction
  {

  public:

    TestActionShowerLib();
    virtual void BeginOfEventAction(const G4Event*) override;
    virtual void EndOfEventAction(const G4Event*) override;
    virtual void BeginOfRunAction(const G4Run*) override;
    virtual void EndOfRunAction(const G4Run*) override;
    virtual void UserSteppingAction(const G4Step*) override;

  private:

    /// Pointer to StoreGate (event store by default)
    ServiceHandle<StoreGateSvc> m_evtStore;

    /* data members */

    ServiceHandle<ILArCalculatorSvc> m_current_calculator;
    G4VSolid* m_current_solid;
    G4AffineTransform* m_current_transform;

    /// @name LAr calculators
    /// @{

    /// Pointer to EMEC inner wheel calculator
    ServiceHandle<ILArCalculatorSvc> m_calculator_EMECIW;
    /// Pointer to EMEC outer wheel calculator
    ServiceHandle<ILArCalculatorSvc> m_calculator_EMECOW;
    ServiceHandle<ILArCalculatorSvc> m_calculator_FCAL1;
    ServiceHandle<ILArCalculatorSvc> m_calculator_FCAL2;
    ServiceHandle<ILArCalculatorSvc> m_calculator_FCAL3;
    ServiceHandle<ILArCalculatorSvc> m_calculator_EMB;

    /// @}

    /// Collection of StepInfo
    ShowerLib::StepInfoCollection* m_eventSteps;

  }; // class TestActionShowerLib

} // namespace G4UA

#endif // LARG4GENSHOWERLIB_TestActionShowerLib_H
