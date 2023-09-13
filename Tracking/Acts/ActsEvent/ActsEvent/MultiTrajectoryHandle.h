/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_MultiTrajectoryHandle_h
#define ActsEvent_MultiTrajectoryHandle_h
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "ActsEvent/MultiTrajectory.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "GaudiKernel/StatusCode.h"

namespace ActsTrk {

/**
 * Helper to simplify MutableMTJ construction and creation of the ConstMTJ
 * sharing the same backends
 *
 * It is modelled to be like the the usual SG handles.
 * The declaration in the algorithm would look like this.
 * In the class declaration:
 * MutableMultiTrajectoryHandle m_mtjHandle{this, "PropNamePrefix",
 * "TrackNamePrefix"}; Then in initialize():
 * ATH_CHECK(m_mtjHandle.initialize());
 * again identically to how it is done with the handles.
 *
 * The only differences is that the handle has additional methods
 * to build the MutableMTJ and ConstMTJ.
 *
 */
template <class C>
class MutableMultiTrajectoryHandle {
 public:
  /**
   * Defines how the properties should be named:
   * propertyNamePrefix+TrackStateKey, propertyNamePrefix+TrackMeasurementsKey,
   * ... with the default values: name+TracState, name+TrackMeasurements finally
   * the const MTJ version property and name are:
   * propertyNamePrefix+MultiTrajectoryKey and name: namePrefix+MultiTrajectory
   */
  MutableMultiTrajectoryHandle(C* algorithm,
                               const std::string& propertyNamePrefix,
                               const std::string& namePrefix);

  /**
   * Sets up the handles
   */
  StatusCode initialize();

  /**
   * builds MutableMTJ and registers backends in SG
   */
  std::unique_ptr<ActsTrk::MutableMultiTrajectory> build(
      const EventContext& context) const;
  /**
   * builds ConstMTJ pointing to the same backends as MutableMTJ and saves it in
   * SG
   * @warning this call should happen last e.g. once MutableMTJ is not altered
   * anymore
   */
  StatusCode storeConst(const ActsTrk::MutableMultiTrajectory* mmtj,
                        const ActsGeometryContext& geoContext,
                        const EventContext& context) const;

 private:
  SG::WriteHandleKey<xAOD::TrackStateContainer> m_statesKey;
  SG::WriteHandleKey<xAOD::TrackParametersContainer> m_parametersKey;
  SG::WriteHandleKey<xAOD::TrackJacobianContainer> m_jacobiansKey;
  SG::WriteHandleKey<xAOD::TrackMeasurementContainer> m_measurementsKey;
  SG::WriteHandleKey<xAOD::SurfaceBackendContainer> m_surfacesKey;
  SG::WriteHandleKey<ActsTrk::ConstMultiTrajectory> m_constMTJKey;
};

/**
 * Helper to simplify the ConstMTJ construction when reading from a file
 * It is expected that there will be an algorithm which will use this
 * functionality to build the ConstMTJ and clients would just use the ConstMTJ
 * directly
 */
template <class C>
class ConstMultiTrajectoryHandle {
 public:
  /**
   * see constructor of MutableMultiTrajectory
   */
  ConstMultiTrajectoryHandle(C* algorithm, const std::string& prefix,
                             const std::string& name);
  /**
   * Sets up the handles
   */
  StatusCode initialize();

  /**
   * builds ConstMTJ, the geometry is used to obtain Acts::Surface pointers
   * if geo pointer is a nullptr and the surfaces are not in the original
   * collection an exception is thrown
   */
  const ActsTrk::ConstMultiTrajectory* build(const Acts::TrackingGeometry* geo,
                                             const EventContext& context) const;

 private:
  SG::ReadHandleKey<xAOD::TrackStateContainer> m_statesKey;
  SG::ReadHandleKey<xAOD::TrackParametersContainer> m_parametersKey;
  SG::ReadHandleKey<xAOD::TrackJacobianContainer> m_jacobiansKey;
  SG::ReadHandleKey<xAOD::TrackMeasurementContainer> m_measurementsKey;
  SG::ReadHandleKey<xAOD::SurfaceBackendContainer> m_surfacesKey;
  SG::WriteHandleKey<ActsTrk::ConstMultiTrajectory> m_constMTJKey;
};

/// implementations

template <class C>
MutableMultiTrajectoryHandle<C>::MutableMultiTrajectoryHandle(
    C* algorithm, const std::string& propertyNamePrefix,
    const std::string& namePrefix)
    : m_statesKey(algorithm, propertyNamePrefix + "TrackStatesKey",
                  namePrefix + "TrackStates"),
      m_parametersKey(algorithm, propertyNamePrefix + "TrackParametersKey",
                      namePrefix + "TrackParameters"),
      m_jacobiansKey(algorithm, propertyNamePrefix + "TrackJacobiansKey",
                     namePrefix + "TrackJacobians"),
      m_measurementsKey(algorithm, propertyNamePrefix + "TrackMeasurementsKey",
                        namePrefix + "TrackMeasurements"),
      m_surfacessKey(algorithm, propertyNamePrefix + "SurfacesBaackendKey",
                     namePrefix + "SurfacesBackend"),
      m_constMTJKey(algorithm, propertyNamePrefix + "MultiTrajectoryKey",
                    namePrefix + "MultiTrajectory") {}

template <class C>
StatusCode MutableMultiTrajectoryHandle<C>::initialize() {
  ATH_CHECK(m_statesKey.initialize());
  ATH_CHECK(m_parametersKey.initialize());
  ATH_CHECK(m_jacobiansKey.initialize());
  ATH_CHECK(m_measurementsKey.initialize());
  ATH_CHECK(m_surfacesKey.initialize());
  ATH_CHECK(m_constMTJKey.initialize());

  return StatusCode::SUCCESS;
}

template <class C>
std::unique_ptr<ActsTrk::MutableMultiTrajectory>
MutableMultiTrajectoryHandle<C>::build(const ActsGeometryContext& geoContext,
                                       const EventContext& context) const {

  auto statesBackend = std::make_unique<xAOD::TrackStateContainer>();
  auto statesBackendAux = std::make_unique<xAOD::TrackStateAuxContainer>();
  statesBackend->setStore(statesBackendAux.get());

  auto parametersBackend = std::make_unique<xAOD::TrackParametersContainer>();
  auto parametersBackendAux =
      std::make_unique<xAOD::TrackParametersAuxContainer>();
  parametersBackend->setStore(parametersBackendAux.get());

  auto jacobiansBackend = std::make_unique<xAOD::TrackJacobianContainer>();
  auto jacobiansBackendAux =
      std::make_unique<xAOD::TrackJacobianAuxContainer>();
  jacobiansBackend->setStore(jacobiansBackendAux.get());

  auto measurementsBackend =
      std::make_unique<xAOD::TrackMeasurementContainer>();
  auto measurementsBackendAux =
      std::make_unique<xAOD::TrackMeasurementAuxContainer>();
  measurementsBackend->setStore(measurementsBackendAux.get());

  auto surfacesBackend = std::make_unique<xAOD::SurfaceBackendContainer>();
  auto surfacesBackendAux =
      std::make_unique<xAOD::SurfaceBackendAuxContainer>();
  surfacesBackend->setStore(surfacesBackendAux.get());

  auto mtj = std::make_unique<ActsTrk::MutableMultiTrajectory>(
      statesBackend.get(), parametersBackend.get(), jacobiansBackend.get(),
      measurementsBackend.get(), surfaceBackend.get());

  auto statesBackendHandle = SG::makeHandle(m_statesKey, context);
  if (statesBackendHandle
          .record(std::move(statesBackend), std::move(statesBackendAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableMultiTrajectoryHandle::build, can't record TrackStates "
        "backend");
  }

  auto parametersBackendHandle = SG::makeHandle(m_parametersKey, context);
  if (parametersBackendHandle
          .record(std::move(parametersBackend), std::move(parametersBackendAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableMultiTrajectoryHandle::build, can't record TrackParameters "
        "backend");
  }

  auto jacobiansBackendHandle = SG::makeHandle(m_jacobiansKey, context);
  if (jacobiansBackendHandle
          .record(std::move(jacobiansBackend), std::move(jacobiansBackendAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableMultiTrajectoryHandle::build, can't record TrackJacobians "
        "backend");
  }

  auto measurementsBackendHandle = SG::makeHandle(m_measurementsKey, context);
  if (measurementsBackendHandle
          .record(std::move(measurementsBackend),
                  std::move(measurementsBackendAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableMultiTrajectoryHandle::build, can't record TrackMeasurements "
        "backend");
  }

  auto surfacesBackendHandle = SG::makeHandle(m_surfacesKey, context);
  if (surfacesBackendHandle
          .record(std::move(surfacesBackend), std::move(surfacesBackendAux))
          .isFailure()) {
    throw std::runtime_error(
        "MutableMultiTrajectoryHandle::build, can't record Surfaces "
        "backend");
  }

  return mtj;
}

template <class C>
StatusCode MutableMultiTrajectoryHandle<C>::storeConst(
    const ActsTrk::MutableMultiTrajectory* mmtj,
    const ActsGeometryContext& geoContext, const EventContext& context) const {
  auto cmtj = std::make_unique<ActsTrk::ConstMultiTrajectory>(
      DataLink<xAOD::TrackStateContainer>(m_statesKey.key(), context),
      DataLink<xAOD::TrackParametersContainer>(m_parametersKey.key(), context),
      DataLink<xAOD::TrackJacobianContainer>(m_jacobiansKey.key(), context),
      DataLink<xAOD::TrackMeasurementContainer>(m_measurementsKey.key(),
                                                context),
      DataLink<xAOD::SurfaceBackendContainer>(m_surfacesKey.key(), context));
  cmtj->fillSurfaces(mmtj);
  auto handle = SG::makeHandle(m_constMTJKey, context);
  ATH_CHECK(handle.record(std::move(cmtj)));
  return StatusCode::SUCCESS;
}

/**
 * Helper for ConstMTJ creation
 * It would be needed only in an algorithm scheduled bring the ConstMTJ
 * from persistent form.
 */

template <class C>
ConstMultiTrajectoryHandle<C>::ConstMultiTrajectoryHandle(
    C* algorithm, const std::string& propertyNamePrefix,
    const std::string& namePrefix)
    : m_statesKey(algorithm, propertyNamePrefix + "TrackStatesKey",
                  namePrefix + "TrackStates"),
      m_parametersKey(algorithm, propertyNamePrefix + "TrackParametersKey",
                      namePrefix + "TrackParameters"),
      m_jacobiansKey(algorithm, propertyNamePrefix + "TrackJacobiansKey",
                     namePrefix + "TrackJacobians"),
      m_measurementsKey(algorithm, propertyNamePrefix + "TrackMeasurementsKey",
                        namePrefix + "TrackMeasurements"),
      m_surfacesKey(algorithm, propertyNamePrefix + "SurfaceBackendKey",
                        namePrefix + "SurfaceBackend") {}

template <class C>
StatusCode ConstMultiTrajectoryHandle<C>::initialize() {
  ATH_CHECK(m_statesKey.initialize());
  ATH_CHECK(m_parametersKey.initialize());
  ATH_CHECK(m_jacobiansKey.initialize());
  ATH_CHECK(m_measurementsKey.initialize());
  ATH_CHECK(m_surfacesKey.initialize());
  ATH_CHECK(m_constMTJKey.initialize());

  return StatusCode::SUCCESS;
}

template <class C>
const ActsTrk::ConstMultiTrajectory* ConstMultiTrajectoryHandle<C>::build(
    const Acts::TrackingGeometry* geo, const EventContext& context) const {
  // we need to build it from backends
  auto statesBackendHandle = SG::makeHandle(m_statesKey, context);
  auto parametersBackendHandle = SG::makeHandle(m_parametersKey, context);
  auto jacobiansBackendHandle = SG::makeHandle(m_jacobiansKey, context);
  auto measurementsBackendHandle = SG::makeHandle(m_measurementsKey, context);
  auto surfacesBackendHandle = SG::makeHandle(m_surfacesKey, context);

  auto mtj = std::make_unique<ActsTrk::ConstMultiTrajectory>(
      *statesBackendHandle, *parametersBackendHandle, *jacobiansBackendHandle,
      *measurementsBackendHandle, *surfacesBackendHandle);
  if (geo != nullptr)
    mtj->fillSurfaces(geo);

  auto mtjHandle = SG::makeHandle(m_constMTJKey, context);
  if (mtjHandle.record(std::move(mtj)).isFailure()) {
    return nullptr;
  }
  return mtjHandle.ptr();
}

}  // namespace ActsTrk

#endif  // ActsEvent_MultiTrajectory_h