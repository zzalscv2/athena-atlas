

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
   * ATH_CHECK(m_handle.moveToConst(tc));
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
  std::unique_ptr<ActsTrk::future::TrackContainer> moveToConst(
      ActsTrk::future::MutableTrackContainer&& tc, const EventContext& context) const;

 private:
  ActsTrk::MutableMultiTrajectoryHandle<C> m_mtjBackendsHandle;
  SG::WriteHandleKey<ActsTrk::MultiTrajectory> m_mtjKey;

  // track and its backend
  SG::WriteHandleKey<xAOD::TrackStorageContainer> m_xAODTrackStorageKey;
  SG::WriteHandleKey<ActsTrk::TrackStorageContainer> m_trackBackendKey;
};

template <class C>
MutableTrackContainerHandle<C>::MutableTrackContainerHandle(
    C* algorithm, const std::string& propertyNamePrefix,
    const std::string& namePrefix)
    : m_mtjBackendsHandle(algorithm, propertyNamePrefix, namePrefix),
      m_mtjKey(algorithm, propertyNamePrefix + "MTJKey",
               namePrefix + "MultiTrajectory"),
      m_xAODTrackStorageKey(algorithm, propertyNamePrefix + "xAODTrackStorage",
                            namePrefix + "xAODTrackStorage"),
      m_trackBackendKey(algorithm, propertyNamePrefix + "TrackStorage",
                        namePrefix + "TrackStorage") {}

template <class C>
StatusCode MutableTrackContainerHandle<C>::initialize() {
  ATH_CHECK(m_mtjBackendsHandle.initialize());
  ATH_CHECK(m_mtjKey.initialize());
  ATH_CHECK(m_xAODTrackStorageKey.initialize());
  ATH_CHECK(m_trackBackendKey.initialize());
  return StatusCode::SUCCESS;
}

template <class C>
std::unique_ptr<ActsTrk::future::TrackContainer>
MutableTrackContainerHandle<C>::moveToConst(
    ActsTrk::future::MutableTrackContainer&& tc, const EventContext& context) const {

  std::unique_ptr<ActsTrk::MultiTrajectory> constMtj =
      m_mtjBackendsHandle.moveToConst(std::move(tc.trackStateContainer()), context);

  auto constMtjHandle = SG::makeHandle(m_mtjKey, context);
  if (constMtjHandle.record(std::move(constMtj)).isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::moveToConst, can't record "
        "ConstMultiTrajectory");
  }

  auto xAODTrackStorageHandle = SG::makeHandle(m_xAODTrackStorageKey, context);
  if (xAODTrackStorageHandle
          .record(std::move(tc.container().m_backend),
                  std::move(tc.container().m_backendAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::moveToConst, can't record "
        "xAODTrackStorage");
  }

  auto constTrackStorage =
      std::make_unique<ActsTrk::TrackStorageContainer>(
          DataLink<xAOD::TrackStorageContainer>(m_xAODTrackStorageKey.key(),
                                                context));
  constTrackStorage->fillSurfaces(tc.container());
  constTrackStorage->restoreDecorations();

  auto constTrackStorageHandle = SG::makeHandle(m_trackBackendKey, context);
  if (constTrackStorageHandle.record(std::move(constTrackStorage))
          .isFailure()) {
    throw std::runtime_error(
        "MutableTrackContainerHandle::moveToConst, can't record "
        "xAODTrackStorage");
  }
  auto constTrack = std::make_unique<ActsTrk::future::TrackContainer>(
      DataLink<ActsTrk::TrackStorageContainer>(m_trackBackendKey.key(),
                                                    context),
      DataLink<ActsTrk::MultiTrajectory>(m_mtjKey.key(), context));
  return constTrack;
}
}  // namespace ActsTrk

#endif