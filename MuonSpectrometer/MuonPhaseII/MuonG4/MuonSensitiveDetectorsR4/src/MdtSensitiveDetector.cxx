/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtSensitiveDetector.h"


#include <MCTruth/TrackHelper.h>
#include <G4Geantino.hh>
#include <G4ChargedGeantino.hh>
#include <limits>
#include <iostream>
#include <GaudiKernel/MsgStream.h>
#include <GeoPrimitives/CLHEPtoEigenConverter.h>
#include <xAODMuonSimHit/MuonSimHitAuxContainer.h>
#include <MuonReadoutGeometryR4/StringUtils.h>
#include <GaudiKernel/SystemOfUnits.h>

inline std::ostream& operator<<(std::ostream& ostr, const G4StepPoint& step) {
      ostr<<"position: "<<Amg::toString(Amg::Hep3VectorToEigen(step.GetPosition()),2)<<", ";
      ostr<<"momentum: "<<Amg::toString(Amg::Hep3VectorToEigen(step.GetMomentum()),2)<<", ";
      ostr<<"time: "<<step.GetGlobalTime()<<", ";
      ostr<<"mass: "<<step.GetMass()<<", ";
      ostr<<"kinetic energy: "<<step.GetKineticEnergy()<<", ";
      ostr<<"charge: "<<step.GetCharge();
      return ostr;
}
inline Amg::Transform3D getTransform(const G4VTouchable* history, unsigned int level) {
   return Amg::Translation3D{Amg::Hep3VectorToEigen(history->GetTranslation(level))}*
            Amg::CLHEPRotationToEigen(*history->GetRotation(level)).inverse();
}
using namespace MuonGMR4;
namespace MuonG4R4{

MdtSensitiveDetector::MdtSensitiveDetector(const std::string& name, const std::string& output_key,
                         const MuonGMR4::MuonDetectorManager* detMgr):
    G4VSensitiveDetector{name},
    AthMessaging{name},
    m_writeHandle{output_key},
    m_detMgr{detMgr} {

}
// Implemenation of memebr functions
void MdtSensitiveDetector::Initialize(G4HCofThisEvent*) {
  if (m_writeHandle.isValid()) {
      ATH_MSG_VERBOSE("Simulation hit container "<<m_writeHandle.fullKey()<<" is already written");
      return;
  }
  if (!m_writeHandle.recordNonConst(std::make_unique<xAOD::MuonSimHitContainer>(),
                                    std::make_unique<xAOD::MuonSimHitAuxContainer>()).isSuccess()) {
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to record "<<m_writeHandle.fullKey());
      throw std::runtime_error("Container saving is impossible");
  }
  ATH_MSG_DEBUG("Output container "<<m_writeHandle.fullKey()<<" has been successfully created");
}
G4bool MdtSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory* /*ROHist*/) {
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
  
  const Amg::Transform3D localToGlobal = getTransform(currentTrack->GetTouchable(), 0);
  const Amg::Transform3D trans = localToGlobal.inverse();
  ATH_MSG_VERBOSE(" Track is inside volume "
                 <<currentTrack->GetTouchable()->GetHistory()->GetTopVolume()->GetName()
                 <<" transformation: "<<to_string(localToGlobal));
  // transform pre and post step positions to local positions
  const Amg::Vector3D globalVertex1{Amg::Hep3VectorToEigen(preStep->GetPosition())};
  const Amg::Vector3D globalVertex2{Amg::Hep3VectorToEigen(postStep->GetPosition())};

  const Amg::Vector3D localVertex1{trans * globalVertex1};
  const Amg::Vector3D localVertex2{trans * globalVertex2};

  const Amg::Vector3D localDir = (localVertex2 - localVertex1).unit();
  /// Calculate the direction in the x-y plane. Remember that the wire is parallel to the z-axis
  const Amg::Vector3D transDir = Amg::Vector3D(localDir.x(),localDir.y(), 0.);

  Amg::Vector3D localPosition{Amg::Vector3D::Zero()};
  double globalTime{0.}; 

  /// Check whether the tube passed the wire. Let's assume that the particle travels along the x-axis
  /// The wire is crosses the origin --> We crossed the wire if the sign changes
  if (transDir.dot(localVertex1) * transDir.dot(localVertex2) < 0.) {
    
    /// Calculate the closest approach to the wire
    const double lambda = MuonGM::intersect<3>(Amg::Vector3D::Zero(), Amg::Vector3D::UnitZ(),
                                               localVertex1, localDir).value_or(0.);
    localPosition = localVertex1 + lambda * localDir;
    /// Take the time 
    globalTime = preStep->GetGlobalTime() + 
                 std::abs(lambda) / preStep->GetVelocity();

    ATH_MSG_VERBOSE("The particle passed the wire."<<std::endl<<
                    "Pre step:  "<<(*preStep)<<" localPosition: "<<Amg::toString(localVertex1,2)<<" distance: "<<localVertex1.perp()<<std::endl<<
                    "Post step: "<<(*postStep)<<" localPosition: "<<Amg::toString(localVertex2,2)<<" distance: "<<localVertex2.perp()<<std::endl<<
                    "Set globalTime to "<<globalTime<<" Closest approach "<<Amg::toString(localPosition,2)<<" drift radius: "<<localPosition.perp());
    
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
     m_trans = trans;
  }
  ATH_MSG_VERBOSE("Check whether the particle should be dumped "<<static_cast<void*>(preStep->GetMaterial()) 
                 <<" "<< static_cast<void*>(postStep->GetMaterial()));
  if (currentTrack->GetTrackStatus() !=  fStopAndKill && preStep->GetMaterial() == postStep->GetMaterial()){
     ATH_MSG_VERBOSE("The particle is still in material volume "<<preStep->GetMaterial());
     return true;
  }
  ATH_MSG_VERBOSE("Particle left volume. Dumping to SimHit container ahead");

  const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(preStep->GetTouchable());

  TrackHelper trHelp(aStep->GetTrack());
  /// Consruct the mdtHit
  m_driftR = std::numeric_limits<double>::max();
  const Identifier HitID = getIdentifier(touchHist);
  if (!HitID.is_valid()) {
      ATH_MSG_VERBOSE("No valid hit found");
      return true;
  }
  xAOD::MuonSimHit* hit = new xAOD::MuonSimHit();
  m_writeHandle->push_back(hit);  
  hit->setIdentifier(HitID); 
  hit->setLocalPosition(xAOD::toStorage(m_locPos));  
  hit->setLocalDirection(xAOD::toStorage(m_locDir));
  hit->setStepLength(aStep->GetStepLength());
  hit->setGlobalTime(m_globalTime);
  hit->setPdgId(currentTrack->GetDefinition()->GetPDGEncoding());
  hit->setEnergyDeposit(aStep->GetTotalEnergyDeposit());
  hit->setKineticEnergy(preStep->GetKineticEnergy());
  /// Need to fix the gen particle link
  hit->setGenParticleLink(ElementLink<McEventCollection>{});

  return true;
}
Identifier MdtSensitiveDetector::getIdentifier(const G4TouchableHistory* touchHist) {
   /// The third volume in the history is the volume corresponding to the Muon multilayer
   const std::string stationVolume = touchHist->GetVolume(2)->GetName();
   using namespace MuonGMR4;
   const std::vector<std::string> volumeTokens = tokenize(stationVolume, "_");
   ATH_MSG_VERBOSE("Name of the station volume is "<<stationVolume);
   if (volumeTokens.size() != 5){
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Cannot deduce the station name from "<<stationVolume);
      throw std::runtime_error("Invalid station Identifier");
   }
   /// Find the Detector element from the Identifier
   const std::string stName = volumeTokens[0].substr(0,3);
   const int stationEta = atoi(volumeTokens[2]);
   const int stationPhi = atoi(volumeTokens[3]) + 1;
   const int multiLayer = atoi(volumeTokens[4]);
   const MdtIdHelper& idHelper{m_detMgr->idHelperSvc()->mdtIdHelper()};
   /// Build first the Identifier to find the detector element
   const Identifier stationId = idHelper.elementID(stName,stationEta, stationPhi);
   const Identifier detElId = idHelper.multilayerID(stationId, multiLayer);
   /// Then retrieve the Detector element
   const MdtReadoutElement* readOutEle = m_detMgr->getMdtReadoutElement(detElId);
   if (!readOutEle) {
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to retrieve a valid detector element from "
                    <<m_detMgr->idHelperSvc()->toStringDetEl(detElId)<<" "<<stationVolume);
      throw std::runtime_error("Invalid detector Element");
   }
   
   /// The Geant transform takes a hit global -> local --> inverse goes back to the global system
   /// Compose this one with the global to local transformation of the first tube in the layer -->
   const Amg::Vector3D refTubePos = (readOutEle->globalToLocalTrans(m_gctx, readOutEle->measurementHash(1,1)) * m_trans.inverse()).translation();
   ATH_MSG_VERBOSE("Position of the tube wire w.r.t. the first tube in the multi layer "<<Amg::toString(refTubePos, 2));
   /// equilateral triangle
   static const double layerPitch = 1./ std::sin(60*Gaudi::Units::deg);
   const int layer = std::round(refTubePos.x() * layerPitch / readOutEle->tubePitch()) +1;
   int tube = std::round(refTubePos.y() / readOutEle->tubePitch()) + 1;
   /// It can happen that the tube is assigned to zero by numerical precision
   /// Catch these cases if the layer is fine
   if (tube == 0 && layer >= 1) {
      const Amg::Vector3D tubeInLay = (readOutEle->globalToLocalTrans(m_gctx, readOutEle->measurementHash(layer,1)) * m_trans.inverse()).translation();
      if (tubeInLay.perp() <= readOutEle->tubeRadius()) tube = 1;
   }
   if (layer <= 0 || tube <= 0) {
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" It seems that the tube position "
                             <<Amg::toString(refTubePos, 2)<<", perp: "<<refTubePos.perp()
                             <<" is outside of the volume envelope "
                             <<m_detMgr->idHelperSvc()->toStringDetEl(detElId)<<". "
                             <<"Layer: "<<layer<<", tube: "<<tube<<","
                             <<touchHist->GetVolume(2)->GetName()<<" "<<touchHist->GetVolume(1)->GetCopyNo());
      throw std::runtime_error("Tube hit in Nirvana");
   }
   const IdentifierHash tubeHash = readOutEle->measurementHash(layer, tube);
   const Identifier tubeId = readOutEle->measurementId(tubeHash);
   ATH_MSG_VERBOSE("Tube & layer number candidate "<<tube<<", "<<layer<<" back and forth transformation "
                     <<to_string(readOutEle->globalToLocalTrans(m_gctx, tubeHash)*m_trans.inverse()));
   return tubeId;  
}

}