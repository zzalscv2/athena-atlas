/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ATLASOURCELINK_H
#define ACTSGEOMETRY_ATLASOURCELINK_H

#include "AthContainers/DataVector.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

#include "Acts/EventData/Measurement.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"

#include <cassert>

namespace ActsTrk {
   using ATLASSourceLink = const Trk::MeasurementBase *;
   using ATLASUncalibSourceLink = ElementLink<xAOD::UncalibratedMeasurementContainer>;

   inline float localXFromSourceLink(const ATLASUncalibSourceLink &source_link) {
      assert( source_link.isValid());
      return (*source_link)->type() == xAOD::UncalibMeasType::PixelClusterType
         ? (*source_link)->localPosition<2>()[Trk::locX]
         : (*source_link)->localPosition<1>()[Trk::locX];
   }

   inline float localYFromSourceLink(const ATLASUncalibSourceLink &source_link) {
      assert( source_link.isValid() && (*source_link)->type() == xAOD::UncalibMeasType::PixelClusterType );
      return (*source_link)->localPosition<2>()[Trk::locY];
   }
}

#endif
