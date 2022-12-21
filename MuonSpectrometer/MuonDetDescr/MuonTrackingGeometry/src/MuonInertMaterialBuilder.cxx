/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackingGeometry/MuonInertMaterialBuilder.h"
// constructor
Muon::MuonInertMaterialBuilder::MuonInertMaterialBuilder(const std::string& t, const std::string& n,
                                                         const IInterface* p)
    : Muon::MuonInertMaterialBuilderImpl(t, n, p) {}

StatusCode Muon::MuonInertMaterialBuilder::initialize() {
  StatusCode sc = detStore()->retrieve(m_muonMgr);
  sc = Muon::MuonInertMaterialBuilderImpl::initialize();
  return sc;
}
StatusCode Muon::MuonInertMaterialBuilder::finalize() { return Muon::MuonInertMaterialBuilderImpl::finalize(); }

std::pair<std::unique_ptr<std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>>,
          std::unique_ptr<std::vector<std::vector<std::pair<std::unique_ptr<const Trk::Volume>, float>>>>>
Muon::MuonInertMaterialBuilder::buildDetachedTrackingVolumes(bool blend) const {
  // split output into objects to be kept and objects which may be released from memory (blended)
  //
  std::pair<std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>,
            std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>>
      mInert;

  // retrieve muon station prototypes from GeoModel
  auto [msTypes, constituentsVector] = buildDetachedTrackingVolumeTypes(m_muonMgr, blend);
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
    if (perm) {
      msTypeName += "PERM";
    }
    //
    Trk::DetachedTrackingVolume* msTV = (*msTypeIter).first;
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
  for (auto& msType : *msTypes) {
    delete msType.first;
  }
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

