/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkToActsConvertorAlg.h"

#include "Acts/EventData/VectorTrackContainer.hpp"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "xAODTracking/TrackJacobianAuxContainer.h"
#include "xAODTracking/TrackMeasurementAuxContainer.h"
#include "xAODTracking/TrackParametersAuxContainer.h"
#include "xAODTracking/TrackStateAuxContainer.h"
#include "ActsEvent/MultiTrajectory.h"

StatusCode ActsTrk::TrkToActsConvertorAlg::initialize() {
  ATH_CHECK(m_trackCollectionKeys.initialize());
  ATH_CHECK(m_vectorTrackContainer.initialize());
  ATH_CHECK(m_mtjHandle.initialize());
  ATH_CHECK(m_convertorTool.retrieve());
  ATH_CHECK(m_constMTJKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode ActsTrk::TrkToActsConvertorAlg::execute(
    const EventContext& ctx) const {
  if (!m_convertorTool->trackingGeometryTool()) {
    ATH_MSG_WARNING(
        "Convertor Tool is not returning tracking geometry. Cannot proceed.");
    return StatusCode::SUCCESS;
  }
  ATH_MSG_VERBOSE("create containers");

  ATH_MSG_VERBOSE("About to create multiTraj");
  auto multiTraj = std::make_unique<MutableMultiTrajectory>();
  Acts::VectorTrackContainer vecTrk;
  Acts::TrackContainer<Acts::VectorTrackContainer,
                       ActsTrk::MutableMultiTrajectory>
      tc{vecTrk, *multiTraj};
  Acts::GeometryContext tgContext = m_convertorTool->trackingGeometryTool()
                                            ->getGeometryContext(ctx)
                                            .context();

    
  ATH_MSG_VERBOSE("Loop over track collections");
  for (auto handle : m_trackCollectionKeys.makeHandles(ctx)) {
    ATH_CHECK(handle.isValid());
    ATH_MSG_VERBOSE("Got back " << handle->size() << " tracks from "<< handle.key());

    m_convertorTool->trkTrackCollectionToActsTrackContainer(
          tc, *handle, tgContext);
    ATH_MSG_VERBOSE("multiTraj has  " << multiTraj->size() << " states");
  }

  // // Let's dump some information for debugging (will be removed later)
  ATH_MSG_VERBOSE("TrackStateContainer has  " << multiTraj->trackStates().size() << " states");
  ATH_MSG_VERBOSE("TrackParametersContainer has  " << multiTraj->trackParameters().size() << " parameters");
  // TODO the TrackContainer should be constructed with ConstMTJ
  std::unique_ptr<ActsTrk::ConstMultiTrajectory> constMultiTraj = m_mtjHandle.convertToConst(multiTraj.get(), ctx);
  SG::WriteHandle<ActsTrk::ConstMultiTrajectory> mtjHandle(m_constMTJKey, ctx);
  ATH_CHECK(mtjHandle.record(std::move(constMultiTraj)));  



  // Store the VectorTrackContainer
  auto constVecTrackCont = std::make_unique<Acts::ConstVectorTrackContainer>(std::move(vecTrk)); 
  SG::WriteHandle<Acts::ConstVectorTrackContainer> wh_vtc(m_vectorTrackContainer, ctx);
  ATH_MSG_VERBOSE("Saving " << constVecTrackCont->size_impl() << " tracks to "<< wh_vtc.key());
  ATH_CHECK(wh_vtc.record(std::move(constVecTrackCont)));
  return StatusCode::SUCCESS;
}


