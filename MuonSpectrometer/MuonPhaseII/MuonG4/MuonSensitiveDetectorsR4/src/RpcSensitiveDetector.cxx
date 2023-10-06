/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcSensitiveDetector.h"

#include "G4ThreeVector.hh"
#include "G4Trd.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

#include "MCTruth/TrackHelper.h"
#include <sstream>

#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "xAODMuonSimHit/MuonSimHitAuxContainer.h"
#include "MuonReadoutGeometryR4/StringUtils.h"
#include "GaudiKernel/SystemOfUnits.h"

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


// construction/destruction
namespace MuonG4R4 {

RpcSensitiveDetector::RpcSensitiveDetector(const std::string& name, const std::string& output_key,
                         const MuonGMR4::MuonDetectorManager* detMgr):
    G4VSensitiveDetector{name},
    AthMessaging{name},
    m_writeHandle{output_key},
    m_detMgr{detMgr} {}

void RpcSensitiveDetector::Initialize(G4HCofThisEvent*) {
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

G4bool RpcSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory*) {

  G4Track* track = aStep->GetTrack();

  /** RPCs sensitive to charged particle only */
  if (track->GetDefinition()->GetPDGCharge() == 0.0) {
    if (track->GetDefinition()!=G4Geantino::GeantinoDefinition()) {
      return true;
    }
  }

  G4Track* currentTrack = aStep->GetTrack();

  // MDTs sensitive to charged particle only
  if (currentTrack->GetDefinition()->GetPDGCharge() == 0.0) {
    if (currentTrack->GetDefinition()!= G4Geantino::GeantinoDefinition()) return true;
    else if (currentTrack->GetDefinition()==G4ChargedGeantino::ChargedGeantinoDefinition()) return true;
  }

  const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(aStep->GetPreStepPoint()->GetTouchable());
  const MuonGMR4::RpcReadoutElement* readOutEle = getReadoutElement(touchHist);
  if (!readOutEle) return true;


  const G4StepPoint* preStep = aStep->GetPreStepPoint();
  const G4StepPoint* postStep = aStep->GetPostStepPoint();
  G4VPhysicalVolume*  physVolPostStep = postStep->GetPhysicalVolume();
  if (!physVolPostStep)  {
    ATH_MSG_VERBOSE("No physical volume stored "<<(*postStep));
    return true;
  }
  
  const Amg::Transform3D globalToLocal = getTransform(currentTrack->GetTouchable(), 0).inverse();
  ATH_MSG_VERBOSE(" Track is inside volume "
                 <<currentTrack->GetTouchable()->GetHistory()->GetTopVolume()->GetName()
                 <<" transformation: "<<to_string(globalToLocal));
  // transform pre and post step positions to local positions
  const Amg::Vector3D globalVertex1{Amg::Hep3VectorToEigen(preStep->GetPosition())};
  const Amg::Vector3D globalVertex2{Amg::Hep3VectorToEigen(postStep->GetPosition())};

  const Amg::Vector3D localVertex1{globalToLocal * globalVertex1};
  const Amg::Vector3D localVertex2{globalToLocal * globalVertex2};

  const Amg::Vector3D localDir = (localVertex2 - localVertex1).unit();
  ATH_MSG_VERBOSE("Entry / exit point in "<<m_detMgr->idHelperSvc()->toStringDetEl(readOutEle->identify())
               <<" "<<Amg::toString(localVertex1, 2)<<" / "<<Amg::toString(localVertex2, 2));
  
  
  // The middle of the gas gap is at X= 0
  std::optional<double> travelDist = MuonGM::intersect<3>(localVertex1, localDir, Amg::Vector3D::UnitX(), 0.);
  if (!travelDist) return true;
  const Amg::Vector3D locGapCross = localVertex1 + (*travelDist) * localDir;
  ATH_MSG_VERBOSE("Propagation to the gas gap center: "<<Amg::toString(locGapCross, 2));
  const Amg::Vector3D gapCenterCross = globalToLocal.inverse() * locGapCross;

  const Identifier etaHitID = getIdentifier(readOutEle, 
                                            gapCenterCross, false);
  if (!etaHitID.is_valid()) {
      ATH_MSG_VERBOSE("No valid hit found");
      return true;
  }
  const double globalTime = preStep->GetGlobalTime() + (*travelDist) / preStep->GetVelocity();
  const Amg::Transform3D& gapTrans{readOutEle->globalToLocalTrans(m_gctx, etaHitID)};
  const Amg::Vector3D dir = gapTrans*(globalVertex2 - globalVertex1).unit();
  const Amg::Vector3D locDir = gapTrans * gapCenterCross;
  
  xAOD::MuonSimHit* hit = new xAOD::MuonSimHit();
  m_writeHandle->push_back(hit);  
  
  TrackHelper trHelp(aStep->GetTrack());
  hit->setIdentifier(etaHitID); 
  hit->setLocalPosition(xAOD::toStorage(locDir));  
  hit->setLocalDirection(xAOD::toStorage(dir));
  hit->setStepLength(aStep->GetStepLength());
  hit->setGlobalTime(globalTime);
  hit->setPdgId(currentTrack->GetDefinition()->GetPDGEncoding());
  hit->setEnergyDeposit(aStep->GetTotalEnergyDeposit());
  hit->setKineticEnergy(preStep->GetKineticEnergy());
  hit->setGenParticleLink(trHelp.GetParticleLink());
  return true;
}

Identifier RpcSensitiveDetector::getIdentifier(const MuonGMR4::RpcReadoutElement* readOutEle, 
                                               const Amg::Vector3D& hitAtGapPlane, bool phiGap) const {
  const RpcIdHelper& idHelper{m_detMgr->idHelperSvc()->rpcIdHelper()};

  const Identifier firstChan = idHelper.channelID(readOutEle->identify(),
                                                  readOutEle->doubletZ(),
                                                  readOutEle->doubletPhi(), 1, phiGap, 1);
   
  /// The 10 mm are introduced to match the old & new readout geometry. Revert the offset
  /// to ease the determination of the gasgap
  const Amg::Vector3D locHitPos{readOutEle->globalToLocalTrans(m_gctx, firstChan) * 
                                hitAtGapPlane};   
  const double gapHalfWidth = readOutEle->stripEtaLength() / 2;
  const double gapHalfLength = readOutEle->stripPhiLength()/ 2;
  ATH_MSG_VERBOSE("Detector element: "<<m_detMgr->idHelperSvc()->toStringDetEl(firstChan)
                <<" locPos: "<<Amg::toString(locHitPos, 2)
                <<" gap thickness "<<readOutEle->gasGapThickness()
                <<" gap width: "<<gapHalfWidth
                <<" gap length: "<<gapHalfLength);
  const int doubletPhi = locHitPos.x() < - gapHalfWidth ? readOutEle->doubletPhiMax() :
                                                          readOutEle->doubletPhi();
  const int gasGap = (std::abs(locHitPos.z()) /  readOutEle->gasGapThickness()) + 1;

  return idHelper.channelID(readOutEle->identify(),
                            readOutEle->doubletZ(),
                            doubletPhi, gasGap, phiGap, 1);
}
const MuonGMR4::RpcReadoutElement* RpcSensitiveDetector::getReadoutElement(const G4TouchableHistory* touchHist) const {
    /// The fourth volume is the envelope volume of the rpc gas gap
   const std::string stationVolume = touchHist->GetVolume(3)->GetName();
   
   const std::vector<std::string> volumeTokens = tokenize(stationVolume, "_");
   ATH_MSG_VERBOSE("Name of the station volume is "<<stationVolume);
   if (volumeTokens.size() != 7) {
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Cannot deduce the station name from "<<stationVolume);
      throw std::runtime_error("Invalid station Identifier");
   }
   /// Find the Detector element from the Identifier
    ///       <STATIONETA>_(<STATIONPHI>-1)_<DOUBLETR>_<DOUBLETPHI>_<DOUBLETZ>
   const std::string stName = volumeTokens[0].substr(0,3);
   const int stationEta = atoi(volumeTokens[2]);
   const int stationPhi = atoi(volumeTokens[3]) + 1;
   const int doubletR = atoi(volumeTokens[4]);
   const int doubletPhi = atoi(volumeTokens[5]);
   const int doubletZ = atoi(volumeTokens[6]);
   const RpcIdHelper& idHelper{m_detMgr->idHelperSvc()->rpcIdHelper()};

   const Identifier detElId = idHelper.padID(idHelper.stationNameIndex(stName), 
                                             stationEta, stationPhi, doubletR, doubletZ, doubletPhi);
   const RpcReadoutElement* readOutElem = m_detMgr->getRpcReadoutElement(detElId);
   if (!readOutElem) {
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to retrieve a valid detector element from "
                    <<m_detMgr->idHelperSvc()->toStringDetEl(detElId)<<" "<<stationVolume);
      /// Keep the failure for the moment commented because there're few ID issues
      /// throw std::runtime_error("Invalid detector Element");
   }
   return readOutElem;
}
}