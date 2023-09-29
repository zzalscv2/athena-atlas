/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_SurfaceEncoding_h
#define ActsEvent_SurfaceEncoding_h

#include <xAODTracking/SurfaceBackend.h>
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/ConeSurface.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"


namespace ActsTrk {

/**
 * Prepares persistifiable representation of surface into xAOD::SurfaceBackend
 * object
 * @warning supports only few types, unhandled surface type results in a
 * exception
 */

void encodeSurface(xAOD::SurfaceBackend* backend, const Acts::Surface* surface,
                   const ActsGeometryContext& geoContext);

/**
 * Creates transient Acts Surface objects given a surface backend
 * implementation should be exact mirror of encodeSurface
 */

std::shared_ptr<const Acts::Surface> decodeSurface(
    const xAOD::SurfaceBackend* backend, const ActsGeometryContext& geoContext);

}  // namespace ActsTrk

#endif