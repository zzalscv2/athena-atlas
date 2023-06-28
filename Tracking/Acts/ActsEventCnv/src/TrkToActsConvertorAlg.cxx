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
#include "ActsTrkEvent/MultiTrajectory.h"

StatusCode ActsTrk::TrkToActsConvertorAlg::initialize() {
  ATH_CHECK(m_trackCollectionKeys.initialize());
  ATH_CHECK(m_vectorTrackContainer.initialize());
  ATH_CHECK(m_trackStatesKey.initialize());
  ATH_CHECK(m_jacobiansKey.initialize());
  ATH_CHECK(m_measurementsKey.initialize());
  ATH_CHECK(m_parametersKey.initialize());
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
  ATH_MSG_VERBOSE("create containers");

  SG::WriteHandle<xAOD::TrackStateContainer> states(
      m_trackStatesKey, ctx);
  ATH_CHECK(states.record(std::make_unique<xAOD::TrackStateContainer>(),
                        std::make_unique<xAOD::TrackStateAuxContainer>()));

  SG::WriteHandle<xAOD::TrackJacobianContainer> jacobians(
      m_jacobiansKey, ctx);
  ATH_CHECK(jacobians.record(std::make_unique<xAOD::TrackJacobianContainer>(),
                        std::make_unique<xAOD::TrackJacobianAuxContainer>()));

  SG::WriteHandle<xAOD::TrackMeasurementContainer> measurements(
      m_measurementsKey, ctx);
  ATH_CHECK(measurements.record(std::make_unique<xAOD::TrackMeasurementContainer>(),
                        std::make_unique<xAOD::TrackMeasurementAuxContainer>()));
  
  SG::WriteHandle<xAOD::TrackParametersContainer> parameters(
      m_parametersKey, ctx);
  ATH_CHECK(parameters.record(std::make_unique<xAOD::TrackParametersContainer>(),
                        std::make_unique<xAOD::TrackParametersAuxContainer>()));
  
  ATH_MSG_VERBOSE("About to create multiTraj");
  auto multiTraj = std::make_unique<MutableMultiTrajectory>(&(*states), &(*parameters), &(*jacobians), &(*measurements));
  Acts::VectorTrackContainer vecTrk;
  Acts::TrackContainer<Acts::VectorTrackContainer,
                       ActsTrk::MultiTrajectory<ActsTrk::IsReadWrite>>
      tc{vecTrk, *multiTraj};
  // auto constMultiTraj = multiTraj->convertToReadOnly(); 
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

  // Let's dump some information for debugging (will be removed later)
  ATH_MSG_VERBOSE("TrackStateContainer has  " << states->size() << " states");
  ATH_MSG_VERBOSE("TrackParametersContainer has  " << parameters->size() << " parameters");

  // Store the VectorTrackContainer
  auto constVecTrackCont = std::make_unique<Acts::ConstVectorTrackContainer>(std::move(vecTrk)); 
  SG::WriteHandle<Acts::ConstVectorTrackContainer> wh_vtc(m_vectorTrackContainer, ctx);
  ATH_MSG_VERBOSE("Saving " << constVecTrackCont->size_impl() << " tracks to "<< wh_vtc.key());
  ATH_CHECK(wh_vtc.record(std::move(constVecTrackCont)));
  return StatusCode::SUCCESS;
}


