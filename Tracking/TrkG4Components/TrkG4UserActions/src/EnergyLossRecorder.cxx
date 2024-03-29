/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EnergyLossRecorder.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkG4UserActions/EnergyLossRecorder.h"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4ThreeVector.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4TouchableHistory.hh"
#include "G4LogicalVolume.hh"
// TrkValInterfaces
#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkValInterfaces/IPositionMomentumWriter.h"

namespace G4UA
{

  EnergyLossRecorder::EnergyLossRecorder(const Config& config)
    : m_config(config)
    , m_entries(0)
  {

  }

  // FIXME: Why implement run action if empty?
  void EnergyLossRecorder::BeginOfRunAction(const G4Run*)
  {
     }

  void EnergyLossRecorder::EndOfRunAction(const G4Run*)
  {
     }

  void EnergyLossRecorder::BeginOfEventAction(const G4Event*)
  {
     }

  void EnergyLossRecorder::EndOfEventAction(const G4Event*)
  {
    if (m_config.pmWriter) {
      // FIXME: thread-unsafe usage of a component in a thread-local action?
      // See ATLASSIM-3562.
      m_config.pmWriter->finalizeTrack();
    }
    m_entries = 0;
 }

  void EnergyLossRecorder::UserSteppingAction(const G4Step* aStep)
  {
    // kill secondary particles
    if (aStep->GetTrack()->GetParentID()) {
      aStep->GetTrack()->SetTrackStatus(fStopAndKill);
      return;
    }
    if(!m_config.pmWriter) return;
    // we require a minimum amount of material for recording the step

    // the material information
    const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(aStep->GetPreStepPoint()->GetTouchable());
    // G4LogicalVolume
    const G4LogicalVolume *lv= touchHist ? touchHist->GetVolume()->GetLogicalVolume() : nullptr;
    const G4Material *mat    = lv ? lv->GetMaterial() : nullptr;

    // log the information // cut off air
    if (mat && mat->GetRadlen() < 200000.) {
      // keep primary particles - calculate the kinematics for them
      G4ThreeVector pos   = aStep->GetPreStepPoint()->GetPosition();
      double px = aStep->GetPreStepPoint()->GetMomentum().x();
      double py = aStep->GetPreStepPoint()->GetMomentum().y();
      double pz = aStep->GetPreStepPoint()->GetMomentum().z();
      Amg::Vector3D position(pos.x(),pos.y(),pos.z());
      Amg::Vector3D momentum(px ,py, pz);

      // record the starting parameters at the first step
      if (m_entries==0) {
        // increase the counter
        ++m_entries;
        double  m   = aStep->GetTrack()->GetDynamicParticle()->GetMass();
        int pdgCode = aStep->GetTrack()->GetDynamicParticle()->GetPDGcode();
        m_config.pmWriter->initializeTrack(position,momentum,m,pdgCode);
      }
      else {
        m_config.pmWriter->recordTrackState(position,momentum);
      }
    }
  }

} // namespace G4UA
