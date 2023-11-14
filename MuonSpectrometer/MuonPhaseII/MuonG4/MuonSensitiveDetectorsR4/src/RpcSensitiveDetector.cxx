/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcSensitiveDetector.h"
#include "MuonSensitiveDetectorsR4/Utils.h"
#include "G4ThreeVector.hh"
#include "G4Trd.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

#include "MCTruth/TrackHelper.h"
#include <sstream>

#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "xAODMuonSimHit/MuonSimHitAuxContainer.h"
#include "GaudiKernel/SystemOfUnits.h"

using namespace MuonGMR4;
using namespace CxxUtils;
using namespace ActsTrk;


// construction/destruction
namespace MuonG4R4 {

RpcSensitiveDetector::RpcSensitiveDetector(const std::string& name, 
                                           const std::string& output_key,
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


  G4Track* currentTrack = aStep->GetTrack();

  // MDTs sensitive to charged particle only
  if (currentTrack->GetDefinition()->GetPDGCharge() == 0.0) {
    if (currentTrack->GetDefinition()!= G4Geantino::GeantinoDefinition()) return true;
    else if (currentTrack->GetDefinition()==G4ChargedGeantino::ChargedGeantinoDefinition()) return true;
  }

  /// Reject secondary particles
  constexpr double velCutOff = 10.*Gaudi::Units::micrometer / Gaudi::Units::second;
  if (currentTrack->GetVelocity() < velCutOff) return true;

  const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(currentTrack->GetTouchable());
  const MuonGMR4::RpcReadoutElement* readOutEle = getReadoutElement(touchHist);
  if (!readOutEle) {
      return false;
  }
  const Amg::Transform3D globalToLocal = getTransform(currentTrack->GetTouchable(), 0).inverse();
  ATH_MSG_VERBOSE(" Track is inside volume "
                 <<currentTrack->GetTouchable()->GetHistory()->GetTopVolume()->GetName()
                 <<" transformation: "<<Amg::toString(globalToLocal));
  // transform pre and post step positions to local positions
  
  const Amg::Vector3D localPos{globalToLocal*Amg::Hep3VectorToEigen(currentTrack->GetPosition())};
  const Amg::Vector3D localDir{globalToLocal.linear()*Amg::Hep3VectorToEigen(currentTrack->GetMomentumDirection())};
  ATH_MSG_VERBOSE("Entry / exit point in "<<m_detMgr->idHelperSvc()->toStringDetEl(readOutEle->identify())
               <<" "<<Amg::toString(localPos, 2)<<" / "<<Amg::toString(localDir, 2));
  
  
  // The middle of the gas gap is at X= 0
  std::optional<double> travelDist = Amg::intersect<3>(localPos, localDir, Amg::Vector3D::UnitX(), 0.);
  if (!travelDist) return true;
  const Amg::Vector3D locGapCross = localPos + (*travelDist) * localDir;
  ATH_MSG_VERBOSE("Propagation to the gas gap center: "<<Amg::toString(locGapCross, 2));
  const Amg::Vector3D gapCenterCross = globalToLocal.inverse() * locGapCross;

  const Identifier etaHitID = getIdentifier(readOutEle, 
                                            gapCenterCross, false);
  if (!etaHitID.is_valid()) {
      ATH_MSG_VERBOSE("No valid hit found");
      return true;
  }
  const double globalTime = currentTrack->GetGlobalTime() + (*travelDist) / currentTrack->GetVelocity();
  const Amg::Transform3D& gapTrans{readOutEle->globalToLocalTrans(m_gctx, etaHitID)};
  const Amg::Vector3D locHitDir = gapTrans.linear() * Amg::Hep3VectorToEigen(currentTrack->GetMomentumDirection());
  const Amg::Vector3D locHitPos = gapTrans * gapCenterCross;
  
  xAOD::MuonSimHit* hit = new xAOD::MuonSimHit();
  m_writeHandle->push_back(hit);  
  
  TrackHelper trHelp(aStep->GetTrack());
  hit->setIdentifier(etaHitID); 
  hit->setLocalPosition(xAOD::toStorage(locHitPos));  
  hit->setLocalDirection(xAOD::toStorage(locHitDir));
  hit->setStepLength(aStep->GetStepLength());
  hit->setGlobalTime(globalTime);
  hit->setPdgId(currentTrack->GetDefinition()->GetPDGEncoding());
  hit->setEnergyDeposit(aStep->GetTotalEnergyDeposit());
  hit->setKineticEnergy(currentTrack->GetKineticEnergy());
  hit->setGenParticleLink(trHelp.GetParticleLink());
  return true;
}

Identifier RpcSensitiveDetector::getIdentifier(const MuonGMR4::RpcReadoutElement* readOutEle, 
                                               const Amg::Vector3D& hitAtGapPlane, bool phiGap) const {
  const RpcIdHelper& idHelper{m_detMgr->idHelperSvc()->rpcIdHelper()};

  const Identifier firstChan = idHelper.channelID(readOutEle->identify(),
                                                  readOutEle->doubletZ(),
                                                  readOutEle->doubletPhi(), 1, phiGap, 1);
  
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