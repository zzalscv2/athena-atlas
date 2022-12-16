/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTRACKINGGEOMETRY_MUONSTATIONBUILDER_H
#define MUONTRACKINGGEOMETRY_MUONSTATIONBUILDER_H

#include "MuonTrackingGeometry/MuonStationBuilderImpl.h"
#include "TrkDetDescrInterfaces/IDetachedTrackingVolumeBuilder.h"

namespace Muon {
/** @class MuonStationBuilder

    The Muon::MuonStationBuilder retrieves muon stations from Muon Geometry Tree
    prototypes built with help of Muon::MuonStationTypeBuilder

    by Sarka.Todorova@cern.ch
  */

class MuonStationBuilder : public MuonStationBuilderImpl,
                           virtual public Trk::IDetachedTrackingVolumeBuilder {

 public:
  MuonStationBuilder(const std::string&, const std::string&, const IInterface*);
  virtual ~MuonStationBuilder() = default;
  StatusCode initialize();
  std::vector<Trk::DetachedTrackingVolume*>* buildDetachedTrackingVolumes(
      bool blend = false);

 private:
  const MuonGM::MuonDetectorManager* m_muonMgr = nullptr;
};

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONSTATIONBUILDER_H
