/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
// Muon
#include "MuonTrackingGeometry/MuonInertMaterialBuilderCond.h"

#include "StoreGate/ReadCondHandle.h"

// constructor
Muon::MuonInertMaterialBuilderCond::MuonInertMaterialBuilderCond(const std::string& t, const std::string& n,
                                                                 const IInterface* p)
    : MuonInertMaterialBuilderImpl(t, n, p) {}

StatusCode Muon::MuonInertMaterialBuilderCond::initialize() {
  if (m_muonMgrReadKey.initialize().isFailure()) {
    ATH_MSG_ERROR("Could not get MuonDetectorManager, no layers for muons will be built. ");
  }
  return Muon::MuonInertMaterialBuilderImpl::initialize();
}

StatusCode Muon::MuonInertMaterialBuilderCond::finalize() { return Muon::MuonInertMaterialBuilderImpl::finalize(); }

std::pair<std::unique_ptr<std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>>,
          std::unique_ptr<std::vector<std::vector<std::pair<std::unique_ptr<const Trk::Volume>, float>>>>>
Muon::MuonInertMaterialBuilderCond::buildDetachedTrackingVolumes(const EventContext& ctx,
                                                                 SG::WriteCondHandle<Trk::TrackingGeometry>& whandle,
                                                                 bool blend) const {
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> readHandle{m_muonMgrReadKey, ctx};
  if (!readHandle.isValid() || !(*readHandle)) {
    ATH_MSG_ERROR(m_muonMgrReadKey.fullKey()
                  << " is not available. Could not get MuonDetectorManager, no layers for muons will be built.");
    return {};
  }
  whandle.addDependency(readHandle);
  const MuonGM::MuonDetectorManager* muonMgr = *readHandle;
  /// We need to retrieve the detector manager from the detector store as this
  /// is the only which has passive material assigned
  detStore()->retrieve(muonMgr).ignore();
  if (!muonMgr) {
    ATH_MSG_FATAL("Somehow the Muon detector manager is missing ");
    return {};
  }
  // split output into objects to be kept and objects which may be released from memory (blended)
  std::pair<std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>,
            std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>>
      mInert;

  // retrieve muon station prototypes from GeoModel
  auto [msTypes, constituentsVector] = buildDetachedTrackingVolumeTypes(muonMgr, blend);
  ATH_MSG_INFO(name() << " obtained " << msTypes->size() << " prototypes");

  std::vector<std::pair<Trk::DetachedTrackingVolume*, std::vector<Amg::Transform3D>>>::const_iterator msTypeIter =
      msTypes->begin();

  for (; msTypeIter != msTypes->end(); ++msTypeIter) {
    std::string msTypeName = (*msTypeIter).first->name();
    bool perm = true;
    if (blend) {
      // decide if object suitable for blending; does not concern shields
      double protMass = 0.;
      for (const auto& ic : *(*msTypeIter).first->constituents()) {
        protMass += calculateVolume(ic.first.get()) * ic.second;
      }
      perm = msTypeName.compare(0, 1, "J") != 0 && m_blendLimit > 0 && protMass > m_blendLimit;
    }
    if (perm) msTypeName += "PERM";
    //
    const Trk::DetachedTrackingVolume* msTV = (*msTypeIter).first;
    for (auto combTr : (*msTypeIter).second) {
      std::unique_ptr<Trk::DetachedTrackingVolume> newStat{msTV->clone(msTypeName, combTr)};
      if (perm) {
        mInert.first.push_back(std::move(newStat));
      } else {
        mInert.second.push_back(std::move(newStat));
      }
    }
  }

  // clean up prototypes
  for (auto& it : *msTypes) delete it.first;
  delete msTypes;

  // merge
  std::unique_ptr<std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>> muonObjects = nullptr;
  if (mInert.first.empty()){
    muonObjects = std::make_unique<std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>>(std::move(mInert.second));
  }
  else {
    for (unsigned int i = 0; i < mInert.second.size(); i++) {
      mInert.first.push_back(std::move(mInert.second[i]));
    }
    muonObjects = std::make_unique<std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>>(std::move(mInert.first));
  }

  ATH_MSG_INFO(name() << " returns  " << (*muonObjects).size() << " objects (detached volumes)");

  return {std::move(muonObjects), std::move(constituentsVector)};
}

