/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IActsToTrkConverterTool_H
#define ACTSGEOMETRYINTERFACES_IActsToTrkConverterTool_H

// ATHENA
#include <memory>

#include "ActsGeometry/ATLASSourceLink.h"  // inner class, cannot fwd declare
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IInterface.h"
#include "StoreGate/WriteHandle.h"
#include "TrkParameters/TrackParameters.h"  //typedef, cannot fwd declare
#include "TrkTrack/TrackCollection.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"  //typedef, cannot fwd declare
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackMeasurementContainer.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackStateContainer.h"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "ActsEvent/MultiTrajectory.h"

namespace Trk {
class Surface;
class Track;
class MeasurementBase;
}  // namespace Trk

namespace InDetDD {
class SiDetectorElementCollection;
}

namespace Acts {
class Surface;
}

class IActsTrackingGeometryTool;

namespace ActsTrk {
class IActsToTrkConverterTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(IActsToTrkConverterTool, 1, 0);

  virtual const Trk::Surface& actsSurfaceToTrkSurface(
      const Acts::Surface& actsSurface) const = 0;

  virtual const Acts::Surface& trkSurfaceToActsSurface(
      const Trk::Surface& atlasSurface) const = 0;

  virtual const ATLASSourceLink trkMeasurementToSourceLink(
      const Acts::GeometryContext& gctx,
      const Trk::MeasurementBase& measurement,
      std::vector<ATLASSourceLink::ElementsType>& Collection) const = 0;

  virtual
  const ATLASUncalibSourceLink
  uncalibratedTrkMeasurementToSourceLink(const InDetDD::SiDetectorElementCollection &detectorElements, 
				      const xAOD::UncalibratedMeasurement &measurement,
				      std::vector<ATLASUncalibSourceLink::ElementsType>& Collection) const = 0;

  virtual const std::vector<ATLASSourceLink> trkTrackToSourceLinks(
      const Acts::GeometryContext& gctx, const Trk::Track& track,
      std::vector<ATLASSourceLink::ElementsType>& collection) const = 0;

  virtual const Acts::BoundTrackParameters trkTrackParametersToActsParameters(
      const Trk::TrackParameters& atlasParameter) const = 0;

  virtual std::unique_ptr<const Trk::TrackParameters>
  actsTrackParametersToTrkParameters(
      const Acts::BoundTrackParameters& actsParameter,
      const Acts::GeometryContext& gctx) const = 0;


  virtual void trkTrackCollectionToActsTrackContainer(
      Acts::TrackContainer<Acts::VectorTrackContainer,
                       ActsTrk::MultiTrajectory<ActsTrk::IsReadWrite>> &tc,
      const TrackCollection& trackColl,
      const Acts::GeometryContext& gctx) const = 0;

  virtual const IActsTrackingGeometryTool* trackingGeometryTool() const = 0;
};
}  // namespace ActsTrk

#endif
