/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkToActsConvertorAlg.h"

#include "Acts/EventData/VectorTrackContainer.hpp"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "xAODTracking/TrackStateAuxContainer.h"
#include "xAODTracking/TrackStateContainer.h"

StatusCode ActsTrk::TrkToActsConvertorAlg::initialize() {
  ATH_CHECK(m_trackCollectionKeys.initialize());
  ATH_CHECK(m_convertorTool.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode ActsTrk::TrkToActsConvertorAlg::execute(
    const EventContext& ctx) const {
  if (!m_convertorTool->trackingGeometryTool()) {
    ATH_MSG_WARNING(
        "Convertor Tool is not returning tracking geometry. Cannot proceed.");
    return StatusCode::SUCCESS;
  }
  Acts::GeometryContext tgContext = m_convertorTool->trackingGeometryTool()
                                        ->getGeometryContext(ctx)
                                        .context();
  std::vector<ATLASSourceLink::ElementsType> elementCollection;

  // Not used yet, but will be used once I have proper track conversions
  //   Acts::TrackContainer tc{Acts::VectorTrackContainer{},
  //                           Acts::VectorMultiTrajectory{}};

  for (auto handle : m_trackCollectionKeys.makeHandles(ctx)) {
    // bool conversionSuccessful{false}; Commented out for the moment. Will be
    // used soon.
    ATH_MSG_VERBOSE("Trying to load " << handle.key());
    ATH_CHECK(handle.isValid());
    ATH_MSG_VERBOSE("Got back " << handle->size());
    for (auto trk : *handle) {
      // Cannot yet convert tracks, so let's just call this to do *something*
      std::vector<ATLASSourceLink> trackSourceLinks =
          m_convertorTool->trkTrackToSourceLinks(tgContext, *trk,
                                                elementCollection);
    }

    // Will be adding this back in once I have proper track conversions
    // implemented
    //
    // if (conversionSuccessful) {
    //   SG::WriteHandle<xAOD::TrackStateContainer> wh_xaodout(
    //       handle.key() + "ActsStates", ctx);
    //   // Write out the TrackStateContainer
    //   ATH_CHECK(
    //       wh_xaodout.record(std::make_unique<xAOD::TrackStateContainer>(),
    //                         std::make_unique<xAOD::TrackStateAuxContainer>()));
    // }
  }

  return StatusCode::SUCCESS;
}