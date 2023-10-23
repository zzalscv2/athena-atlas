/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// PACKAGE
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ActsWriteTrackingGeometryTransforms.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"


// ATHENA
#include "AthenaKernel/RNGWrapper.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ISvcLocator.h"
#include "ActsInterop/Logger.h"

// ACTS
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"

// STL
#include <string>

using gid = Acts::GeometryIdentifier;

ActsWriteTrackingGeometryTransforms::ActsWriteTrackingGeometryTransforms(const std::string& name,
                                 ISvcLocator* pSvcLocator)
    : AthAlgorithm(name, pSvcLocator)
{
}

StatusCode ActsWriteTrackingGeometryTransforms::initialize() {
  ATH_MSG_INFO("initializing");

  ATH_CHECK(m_trackingGeometryTool.retrieve());

  std::ofstream os(m_outputName); // truncate

  return StatusCode::SUCCESS;
}

StatusCode ActsWriteTrackingGeometryTransforms::execute() {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  auto trackingGeometry = m_trackingGeometryTool->trackingGeometry();
  const ActsGeometryContext& gctx = m_trackingGeometryTool->getGeometryContext(ctx);

  std::stringstream ss;


  std::ofstream os(m_outputName, std::ios_base::app);

  trackingGeometry->visitSurfaces([&] (const Acts::Surface* srf) {
    const Acts::DetectorElementBase *detElem = srf->associatedDetectorElement();
    const auto *gmde = static_cast<const ActsDetectorElement *>(detElem);

    if(dynamic_cast<const InDetDD::TRT_BaseElement*>(gmde->upstreamDetectorElement()) != nullptr) { 
      return;
    }

    const auto side = dynamic_cast<const InDetDD::SiDetectorElement*>(gmde->upstreamDetectorElement());
    if(side == nullptr) {
      throw std::runtime_error{"Not TRT but not Si either"}; // this shouldn't happen
    }

    gid geoID = srf->geometryId();
    os << geoID.volume() << ";";
    os << geoID.boundary() << ";";
    os << geoID.layer() << ";";
    os << geoID.approach() << ";";
    os << geoID.sensitive() << ";";

    os << ctx.eventID().event_number() << ";";

    if(side->isPixel()) {
      os << 0;
    }
    else if(side->isSCT()) {
      os << 1;
    }

    os << ";";

    // iterate over components of transform
    const auto* p = srf->transform(gctx.context()).data();
    for(size_t i=0;i<16;i++) {
      if(i>0) {
        os << ",";
      }
      os << *(p+i);
    }
    os << "\n";
  });


  return StatusCode::SUCCESS;
}

StatusCode ActsWriteTrackingGeometryTransforms::finalize() {
  return StatusCode::SUCCESS;
}
