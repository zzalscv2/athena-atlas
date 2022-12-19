/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MuonInertMaterialBuilder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef MUONTRACKINGGEOMETRY_MUONINERTMATERIALBUILDER_H
#define MUONTRACKINGGEOMETRY_MUONINERTMATERIALBUILDER_H

#include "MuonTrackingGeometry/MuonInertMaterialBuilderImpl.h"
#include "TrkDetDescrInterfaces/IDetachedTrackingVolumeBuilder.h"

namespace Muon {

/** @class MuonInertMaterialBuilder

    The Muon::MuonInertMaterialBuilder retrieves muon stations from Muon Geometry Tree

    by Sarka.Todorova@cern.ch, Marcin.Wolter@cern.ch
  */

class MuonInertMaterialBuilder : public Muon::MuonInertMaterialBuilderImpl,
                                 virtual public Trk::IDetachedTrackingVolumeBuilder {
 public:
  /** Constructor */
  MuonInertMaterialBuilder(const std::string&, const std::string&, const IInterface*);
  /** Destructor */
  virtual ~MuonInertMaterialBuilder() = default;
  /** AlgTool initailize method.*/
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  /** Method returning cloned and positioned material objects */
  virtual std::vector<Trk::DetachedTrackingVolume*>* buildDetachedTrackingVolumes(bool blend = false) override;

 private:
  const MuonGM::MuonDetectorManager* m_muonMgr = nullptr;  //!< the MuonDetectorManager
};

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONINERTMATERIALBUILDER_H
