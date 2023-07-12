/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MDTSensitiveDetector.h"


#include <MCTruth/TrackHelper.h>
#include <G4Geantino.hh>
#include <G4ChargedGeantino.hh>
#include <string>
#include <limits>

#include <GeoPrimitives/CLHEPtoEigenConverter.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <xAODMuonSimHit/MuonSimHitAuxContainer.h>

namespace {
    inline Amg::Vector3D applyTransformation(const G4AffineTransform& trans, const G4ThreeVector& vec ){
        return Amg::Hep3VectorToEigen(trans.TransformPoint(vec));
    }
    std::ostream& operator<<(std::ostream& ostr, const G4StepPoint& step) {
        ostr<<"position: "<<Amg::toString(Amg::Hep3VectorToEigen(step.GetPosition()),2)<<", ";
        ostr<<"momentum: "<<Amg::toString(Amg::Hep3VectorToEigen(step.GetMomentum()),2)<<", ";
        ostr<<"kinetic energy: "<<step.GetKineticEnergy()<<", ";
        ostr<<"charge: "<<step.GetCharge()<<", ";
        return ostr;
    }
}

namespace MuonG4R4 {
MDTSensitiveDetector::MDTSensitiveDetector(const std::string& name, const std::string& output_key,
                         const MuonGMR4::MuonDetectorManager* detMgr):
    G4VSensitiveDetector{name},
    AthMessaging{name},
    m_writeHandle{output_key},
    m_detMgr{detMgr} {

}
// Implemenation of memebr functions
void MDTSensitiveDetector::Initialize(G4HCofThisEvent*) {
  if (m_writeHandle.isValid()) {
      ATH_MSG_VERBOSE("Simulation hit container "<<m_writeHandle.fullKey()<<" is already written");
      return;
  }
  if (!m_writeHandle.recordNonConst(std::make_unique<xAOD::MuonSimHitContainer>(),
                                    std::make_unique<xAOD::MuonSimHitAuxContainer>()).isSuccess()) {
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to record "<<m_writeHandle.fullKey());
      throw std::runtime_error("Container saving is impossible");
  }
}
G4bool MDTSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory* /*ROHist*/) {
   G4Track* currentTrack = aStep->GetTrack();

  // MDTs sensitive to charged particle only
  if (currentTrack->GetDefinition()->GetPDGCharge() == 0.0) {
    if (currentTrack->GetDefinition()!= G4Geantino::GeantinoDefinition()) return true;
    else if (currentTrack->GetDefinition()==G4ChargedGeantino::ChargedGeantinoDefinition()) return true;
  }

  const G4StepPoint* preStep = aStep->GetPreStepPoint();
  const G4StepPoint* postStep = aStep->GetPostStepPoint();
  G4VPhysicalVolume*  physVolPostStep = postStep->GetPhysicalVolume();
  if (!physVolPostStep)  {
    ATH_MSG_VERBOSE("No physical volume stored "<<(*postStep));
    return true;
  }
  
   // get top transformation
  const G4AffineTransform& trans = currentTrack->GetTouchable()->GetHistory()->GetTopTransform();

  // transform pre and post step positions to local positions
  const Amg::Vector3D localVertex1{applyTransformation(trans,preStep->GetPosition())};
  const Amg::Vector3D localVertex2{applyTransformation(trans,postStep->GetPosition())};

  const Amg::Vector3D localDir = (localVertex2 - localVertex1).unit();
  /// Calculate the direction in the x-y plane. Remember that the wire is parallel to the z-axis
  const Amg::Vector3D transDir = Amg::Vector3D(localDir.x(),localDir.y(), 0.).unit();
  const double dirDotVtx1 = transDir.dot(localVertex1);
  const double dirDotVtx2 = transDir.dot(localVertex2);
  

  Amg::Vector3D localPosition{Amg::Vector3D::Zero()};
  double globalTime{0.}; 
  /// Check whether the tube passed the wire. Let's assume that the particle travels along the x-axis
  /// The wire is crosses the origin --> We crossed the wire if the sign changes
  if (dirDotVtx1 * dirDotVtx2 < 0.) {
    
    localPosition = localVertex1 - dirDotVtx1 * localDir;
    /// Take the time 
    globalTime = preStep->GetGlobalTime() + 
                 dirDotVtx1 / preStep->GetVelocity();
    ATH_MSG_VERBOSE("The particle "<<(*preStep)<<" passed the wire. Set globalTime to "<<globalTime);
  } else {
    /// The particle travels still miles before the wire or
    /// left it behind ages ago. Take the one which comes closest to the wire
    if (localVertex1.perp() < localVertex2.perp()) {
        localPosition = localVertex1;
        globalTime = preStep->GetGlobalTime();
        ATH_MSG_VERBOSE("Particle "<<(*preStep)<<" departs from the wire. Current distance: "<<Amg::toString(localPosition,3));
    } else {
        localPosition = localVertex2;
        globalTime = postStep->GetGlobalTime();
        ATH_MSG_VERBOSE("Particle "<<(*postStep)<<" approaches the wire. Current distance: "<<Amg::toString(localPosition,3));
    }
  }
  const double driftRadius = localPosition.perp();
  /// Update the drift radius if we came in fact closer to the wire
  if (driftRadius < m_driftR) {
     m_driftR = driftRadius;
     m_globalTime = globalTime;
     m_locPos = std::move(localPosition);
     m_locDir = localDir;
  }
  /// Check whether the material 

  if (currentTrack->GetTrackStatus() !=  fStopAndKill && preStep->GetMaterial() == postStep->GetMaterial()){
     ATH_MSG_VERBOSE("The particle is still in material volume "<<preStep->GetMaterial()->GetName());
     return true;
  }

  const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(preStep->GetTouchable());
  
  
  TrackHelper trHelp(aStep->GetTrack());

  /// Consruct the mdtHit
  xAOD::MuonSimHit* hit = new xAOD::MuonSimHit();
  m_writeHandle->push_back(hit);
  
  const Identifier HitID = getIdentifier(touchHist);
  hit->setIdentifier(HitID);
 
  hit->setLocalPosition(xAOD::toStorage(m_locPos));  
  hit->setLocalDirection(xAOD::toStorage(m_locDir));
  hit->setStepLength(aStep->GetStepLength());
  hit->setGlobalTime(m_globalTime);
  hit->setPdgId(currentTrack->GetDefinition()->GetPDGEncoding());
  hit->setEnergyDeposit(aStep->GetTotalEnergyDeposit());
  hit->setKineticEnergy(preStep->GetKineticEnergy());
  m_driftR = std::numeric_limits<double>::max();
  return true;
}
Identifier MDTSensitiveDetector::getIdentifier(const G4TouchableHistory* /*touchHist*/){
   return Identifier{};
}

}