/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackingGeometry/MuonInertMaterialBuilder.h"
// constructor
Muon::MuonInertMaterialBuilder::MuonInertMaterialBuilder(const std::string& t, const std::string& n,
                                                         const IInterface* p)
    : Muon::MuonInertMaterialBuilderImpl(t, n, p) {
  declareInterface<Trk::IDetachedTrackingVolumeBuilder>(this);
}

StatusCode Muon::MuonInertMaterialBuilder::initialize() {
  StatusCode sc = detStore()->retrieve(m_muonMgr);
  sc = Muon::MuonInertMaterialBuilderImpl::initialize();
  return sc;
}
StatusCode Muon::MuonInertMaterialBuilder::finalize() {
  return Muon::MuonInertMaterialBuilderImpl::finalize();
}

std::vector<Trk::DetachedTrackingVolume*>* Muon::MuonInertMaterialBuilder::buildDetachedTrackingVolumes(bool blend) {
  // split output into objects to be kept and objects which may be released from memory (blended)
  std::pair<std::vector<Trk::DetachedTrackingVolume*>, std::vector<Trk::DetachedTrackingVolume*>> mInert;

  // retrieve muon station prototypes from GeoModel
  auto [msTypes, constituentsVector]  = buildDetachedTrackingVolumeTypes(m_muonMgr, blend);
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
      Trk::DetachedTrackingVolume* newStat = msTV->clone(msTypeName, combTr);
      if (perm) {
        mInert.first.push_back(newStat);
      } else {
        mInert.second.push_back(newStat);
      }
    }
  }

  // clean up prototypes
  for (auto& msType : *msTypes) {
    delete msType.first;
  }
  delete msTypes;

  // merge
  std::vector<Trk::DetachedTrackingVolume*>* muonObjects = nullptr;
  if (mInert.first.empty())
    muonObjects = new std::vector<Trk::DetachedTrackingVolume*>(mInert.second);
  else {
    for (unsigned int i = 0; i < mInert.second.size(); i++)
      mInert.first.push_back(mInert.second[i]);
    muonObjects = new std::vector<Trk::DetachedTrackingVolume*>(mInert.first);
  }

  ATH_MSG_INFO(name() << " returns  " << (*muonObjects).size() << " objects (detached volumes)");

  return muonObjects;
}

