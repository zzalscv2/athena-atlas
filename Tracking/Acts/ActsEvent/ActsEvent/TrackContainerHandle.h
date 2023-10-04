

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_TRACKCONTAINERHANDLE_H
#define ACTSTRKEVENT_TRACKCONTAINERHANDLE_H 1

#include "Acts/Geometry/TrackingGeometry.hpp"
#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/MultiTrajectoryHandle.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "GaudiKernel/StatusCode.h"

namespace ActsTrk {

template <class C>
class MutableTrackContainerHandle {
 public:
  /**
   * Defines how the properties should be named:
   * propertyNamePrefix+TrackStateKey, propertyNamePrefix+TrackMeasurementsKey,
   * ... with the default values: name+TrackState, name+TrackMeasurements
   * finally the Acts compatible frontends and the track container itself
   *
   * usage in execute:
   * ActsTrk::TrackContainer tc;
   * //fill the tc
   * ATH_CHECK(m_handle.convertToConst(tc));
   * @arg algorithm/tool pointer
   */
  MutableTrackContainerHandle(C* algorithm,
                              const std::string& propertyNamePrefix,
                              const std::string& namePrefix);

  /**
   * Sets up the handles
   */
  StatusCode initialize();

  /**
   * produces ActsTrk::ConstTrackContainer with all backends stored in SG
   * @arg tc - MutableTrackContainer the source convert (will  be disassembled
   * after the operation)
   * @arg context - event context (needed for SG)
   */
  std::unique_ptr<ActsTrk::future::ConstTrackContainer> convertToConst(
      ActsTrk::future::TrackContainer& tc, const EventContext& context) const;

 private:
  ActsTrk::MutableMultiTrajectoryHandle<C> m_mtjBackendsHandle;
  SG::WriteHandleKey<ActsTrk::ConstMultiTrajectory> m_mtjKey;

  // track and its backend
  SG::WriteHandleKey<xAOD::TrackBackendContainer> m_xAODTrackBackendKey;
  SG::WriteHandleKey<ActsTrk::ConstTrackBackendContainer> m_trackBackendKey;
};

template <class C>
MutableTrackContainerHandle<C>::MutableTrackContainerHandle(
    C* algorithm, const std::string& propertyNamePrefix,
    const std::string& namePrefix)
    : m_mtjBackendsHandle(algorithm, propertyNamePrefix, namePrefix),
      m_mtjKey(algorithm, propertyNamePrefix + "MTJKey",
               namePrefix + "MultiTrajectory"),
      m_xAODTrackBackendKey(algorithm, propertyNamePrefix + "xAODTrackBackend",
                            namePrefix + "xAODTrackBackend"),
      m_trackBackendKey(algorithm, propertyNamePrefix + "TrackBackend",
                        namePrefix + "TrackBackend") {}

template <class C>
StatusCode MutableTrackContainerHandle<C>::initialize() {
  ATH_CHECK(m_mtjBackendsHandle.initialize());
  ATH_CHECK(m_mtjKey.initialize());
  ATH_CHECK(m_xAODTrackBackendKey.initialize());
  ATH_CHECK(m_trackBackendKey.initialize());
  return StatusCode::SUCCESS;
}

template <class C>
std::unique_ptr<ActsTrk::future::ConstTrackContainer>
MutableTrackContainerHandle<C>::convertToConst(
    ActsTrk::future::TrackContainer& tc, const EventContext& context) const {

  std::unique_ptr<ActsTrk::ConstMultiTrajectory> constMtj =
      m_mtjBackendsHandle.convertToConst(tc.trackStateContainer(), context);

  auto constMtjHandle = SG::makeHandle(m_mtjKey, context);
  if (constMtjHandle.record(std::move(constMtj)).isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::build, can't record "
        "ConstMultiTrajectory");
  }

  auto xAODTrackBackendHandle = SG::makeHandle(m_xAODTrackBackendKey, context);
  if (xAODTrackBackendHandle
          .record(std::move(tc.container().m_backend),
                  std::move(tc.container().m_backendAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::convertToConst, can't record "
        "xAODTrackBackend");
  }

  auto constTrackBackend =
      std::make_unique<ActsTrk::ConstTrackBackendContainer>(
          DataLink<xAOD::TrackBackendContainer>(m_xAODTrackBackendKey.key(),
                                                context));
  constTrackBackend->fillSurfaces(tc.container());
  constTrackBackend->restoreDecorations();

  auto constTrackBackendHandle = SG::makeHandle(m_trackBackendKey, context);
  if (constTrackBackendHandle.record(std::move(constTrackBackend))
          .isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::convertToConst, can't record "
        "xAODTrackBackend");
  }
  auto constTrack = std::make_unique<ActsTrk::future::ConstTrackContainer>(
      DataLink<ActsTrk::ConstTrackBackendContainer>(m_trackBackendKey.key(),
                                                    context),
      DataLink<ActsTrk::ConstMultiTrajectory>(m_mtjKey.key(), context));
  return constTrack;
}
}  // namespace ActsTrk

#endif