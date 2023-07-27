/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_MultiTrajectory_h
#define ActsEvent_MultiTrajectory_h
#include <type_traits>

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/TrackStatePropMask.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/Utilities/HashedString.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "CxxUtils/concepts.h"
#include "xAODTracking/TrackJacobianAuxContainer.h"
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackMeasurement.h"
#include "xAODTracking/TrackMeasurementAuxContainer.h"
#include "xAODTracking/TrackMeasurementContainer.h"
#include "xAODTracking/TrackParametersAuxContainer.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackStateAuxContainer.h"
#include "xAODTracking/TrackStateContainer.h"

namespace ActsTrk {
class MutableMultiTrajectory;
class ConstMultiTrajectory;
}  // namespace ActsTrk

namespace Acts {
class Surface;
template <typename T>
struct IsReadOnlyMultiTrajectory {};

template <typename T>
struct IsReadOnlyMultiTrajectory<T&> : IsReadOnlyMultiTrajectory<T> {};

template <typename T>
struct IsReadOnlyMultiTrajectory<T&&> : IsReadOnlyMultiTrajectory<T> {};

template <>
struct IsReadOnlyMultiTrajectory<ActsTrk::ConstMultiTrajectory>
    : std::true_type {};

template <>
struct IsReadOnlyMultiTrajectory<ActsTrk::MutableMultiTrajectory>
    : std::false_type {};
}  // namespace Acts

namespace ActsTrk {

using IndexType = std::uint32_t;

namespace detail {
struct Decoration;
}

/**
 * @brief Athena implementation of ACTS::MultiTrajectory (ReadWrite version)
 * The data is stored in 4 external backends. 
 * Backends lifetime are not maintained by this class.
 * except when objects are default constructed (this functionality will be removed). 
 * This class is meant to be used in track finding algorithms (e.g. CKF) and then converted
 * ConstMultiTrajectory varaiant. Thes conversion is meant to be costless. 
 */
class MutableMultiTrajectory final
    : public Acts::MultiTrajectory<ActsTrk::MutableMultiTrajectory> {
 public:
  friend ConstMultiTrajectory;
  using TrackStateProxy = typename Acts::MultiTrajectory<
      ActsTrk::MutableMultiTrajectory>::TrackStateProxy;
  using ConstTrackStateProxy = typename Acts::MultiTrajectory<
      ActsTrk::MutableMultiTrajectory>::ConstTrackStateProxy;

  /**
   * @brief Construct a new Multi Trajectory object owning backends
   */
  MutableMultiTrajectory();

  /**
   * @brief Construct a new Multi Trajectory object given backends
   * @note the MTJ does claim ownership over the data in the backend
   * @param state, parametrs, jacobians, measuremnts - pointers to xAOD
   * interface backend containers
   */
  MutableMultiTrajectory(xAOD::TrackStateContainer* states,
                         xAOD::TrackParametersContainer* parameters,
                         xAOD::TrackJacobianContainer* jacobians,
                         xAOD::TrackMeasurementContainer* measurements);
  /**
   * @brief Copy-Construct a new Multi Trajectory object other one
   * Warning, default constructed MTJ can not be copied (runtime error)
   */
  MutableMultiTrajectory(const ActsTrk::MutableMultiTrajectory& other);

  /**
   * @brief Destructor needed for MTJ owing backends
   */
  ~MutableMultiTrajectory();

  /**
   * @brief Add state with stograge for data that depends on the mask
   *
   * @param mask - bitmask deciding which backends are extended
   * @param istate - previous state
   * @return index of just added state
   */
  ActsTrk::IndexType addTrackState_impl(Acts::TrackStatePropMask mask,
                                        ActsTrk::IndexType iprevious);

  /**
   * @brief Access component by key
   *
   * @param key
   * @param istate
   * @return std::any - that needs to be cast to a const ptr (non const for the
   * nonconst variant)
   */

  const std::any component_impl(Acts::HashedString key, ActsTrk::IndexType istate) const;
  std::any component_impl(Acts::HashedString key, ActsTrk::IndexType istate);

  /**
   * @brief checks if given state has requested component
   *
   * @param key - name (const char*)
   * @param istate - index in the
   * @return true
   * @return false
   */

  constexpr bool has_impl(Acts::HashedString key, ActsTrk::IndexType istate) const;

  /**
   * @brief checks if MTJ has requested column (irrespectively of the state)
   *
   * @param key - name (const char*)
   * @return true - if the column is present
   * @return false - if not
   */
  constexpr bool hasColumn_impl(Acts::HashedString key) const;

  /**
   * @brief enables particular decoration, type & name need to be specified
   *
   * @tparam T type of decoration (usually POD)
   * @param key name of the decoration
   */
  template <typename T>
  void addColumn_impl(const std::string& key);

  /**
   * @brief unsets a given state
   *
   * @param target - property
   * @param istate - index in the
   */

  void unset_impl(Acts::TrackStatePropMask target, ActsTrk::IndexType istate);

  /**
   * @brief shares from a given state
   *
   * @param shareSource, shareTarget - property
   * @param iself, iother  - indexes
   */

  void shareFrom_impl(ActsTrk::IndexType iself, ActsTrk::IndexType iother,
                      Acts::TrackStatePropMask shareSource,
                      Acts::TrackStatePropMask shareTarget);

  /**
   * @brief obtains proxy to the track state under given index
   *
   * @param index
   * @return TrackStateProxy::Parameters
   */

  typename ConstTrackStateProxy::Parameters parameters_impl(
      ActsTrk::IndexType index) const {
    return trackParameters().at(index)->paramsEigen();
  }

  typename TrackStateProxy::Parameters parameters_impl(ActsTrk::IndexType index) {
    return trackParameters().at(index)->paramsEigen();
  }

  /**
   * @brief obtain covariances for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Covariance
   */
  typename ConstTrackStateProxy::Covariance covariance_impl(
      ActsTrk::IndexType index) const {
    return trackParameters().at(index)->covMatrixEigen();
  }
  typename TrackStateProxy::Covariance covariance_impl(ActsTrk::IndexType index) {
    return trackParameters().at(index)->covMatrixEigen();
  }

  /**
   * @brief obtain measurement covariances for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Covariance
   */
  typename ConstTrackStateProxy::Covariance trackMeasurementsCov(
      ActsTrk::IndexType index) const {
    return trackMeasurements().at(index)->covMatrixEigen();
  }

  typename TrackStateProxy::Covariance trackMeasurementsCov(ActsTrk::IndexType index) {
    return trackMeasurements().at(index)->covMatrixEigen();
  }

  /**
   * @brief obtain jacobian for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Covariance
   */

  inline typename ConstTrackStateProxy::Covariance jacobian_impl(
      ActsTrk::IndexType istate) const {
    xAOD::TrackStateIndexType jacIdx = (*m_trackStates)[istate]->jacobian();
    return trackJacobians().at(jacIdx)->jacEigen();
  }
  typename TrackStateProxy::Covariance jacobian_impl(ActsTrk::IndexType istate) {
    xAOD::TrackStateIndexType jacIdx = (*m_trackStates)[istate]->jacobian();
    return trackJacobians().at(jacIdx)->jacEigen();
  }

  /**
   * @brief obtain measurements for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Measurement
   */

  template <std::size_t measdim>
  inline typename ConstTrackStateProxy::template Measurement<measdim>
  measurement_impl(ActsTrk::IndexType index) const {
    xAOD::TrackStateIndexType measIdx = (*m_trackStates)[index]->calibrated();
    return trackMeasurements().at(measIdx)->template measEigen<measdim>();
  }
  template <std::size_t measdim, bool Enable = true>
  std::enable_if_t<Enable,
                   typename TrackStateProxy::template Measurement<measdim>>
  measurement_impl(ActsTrk::IndexType index) {
    xAOD::TrackStateIndexType measIdx = (*m_trackStates)[index]->calibrated();
    return trackMeasurements().at(measIdx)->template measEigen<measdim>();
  }

  /**
   * @brief obtain measurements covariance for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Covariance
   */

  template <std::size_t measdim>
  inline typename ConstTrackStateProxy::template MeasurementCovariance<measdim>
  measurementCovariance_impl(ActsTrk::IndexType index) const {
    xAOD::TrackStateIndexType measIdx = (*m_trackStates)[index]->calibrated();
    return trackMeasurements().at(measIdx)->template covMatrixEigen<measdim>();
  }
  template <std::size_t measdim, bool Enable = true>
  std::enable_if_t<
      Enable, typename TrackStateProxy::template MeasurementCovariance<measdim>>
  measurementCovariance_impl(ActsTrk::IndexType index) {
    xAOD::TrackStateIndexType measIdx = (*m_trackStates)[index]->calibrated();
    return trackMeasurements().at(measIdx)->template covMatrixEigen<measdim>();
  }

  /**
   * @brief size of the MTJ
   *
   * @return size_t
   */

  inline size_t size_impl() const { return m_trackStates->size(); }

  /**
   * @brief clears backends
   * decoration columns are still declared
   */
  inline void clear_impl() {
    m_trackStates->clear();
    m_trackParameters->clear();
    m_trackJacobians->clear();
    m_trackMeasurements->clear();
  }

  /**
   * @brief checks if the backends are connected (i.e. is safe to use, else any
   * other call will cause segfaults)
   */
  bool has_backends() const;

  /**
   * Implementation of allocation of calibrated measurements
   */
  void allocateCalibrated_impl(ActsTrk::IndexType istate, std::size_t measdim) {
    // resize the calibrated measurement to the size measdim
    const auto& trackStates = *m_trackStates;
    trackMeasurements().at(trackStates[istate]->calibrated())->resize(measdim);
  }

  /**
   * Implementation of calibrated size
   */
  ActsTrk::IndexType calibratedSize_impl(ActsTrk::IndexType istate) const {
    // Retrieve the calibrated measurement size
    const auto& trackStates = *m_trackStates;
    return trackMeasurements().at(trackStates[istate]->calibrated())->size();
  }

  /**
   * Implementation of uncalibrated link insertion
   */
  void setUncalibratedSourceLink_impl(ActsTrk::IndexType istate,
                                      const Acts::SourceLink& sourceLink) {
    auto el =
        sourceLink.get<ElementLink<xAOD::UncalibratedMeasurementContainer>>();
    trackStates()[istate]->setUncalibratedMeasurementLink(el);
    trackStates()[istate]->setGeometryId(sourceLink.geometryId().value());
  }
  /**
   * Implementation of uncalibrated link fetch
   */
  typename Acts::SourceLink getUncalibratedSourceLink_impl(
      ActsTrk::IndexType istate) const;
  typename Acts::SourceLink getUncalibratedSourceLink_impl(IndexType istate);


  void setReferenceSurface_impl(IndexType,
                                std::shared_ptr<const Acts::Surface>);
  const Acts::Surface* referenceSurface_impl(IndexType ) const;


 private:
  // bare pointers to the backend (need to be fast and we do not claim ownership
  // anyways)
  xAOD::TrackStateContainer* m_trackStates = nullptr;
  xAOD::TrackStateAuxContainer* m_trackStatesAux = nullptr;

  inline const xAOD::TrackStateContainer& trackStates() const {
    return *m_trackStates;
  }
  inline xAOD::TrackStateContainer& trackStates() { return *m_trackStates; }

  xAOD::TrackParametersContainer* m_trackParameters = nullptr;
  xAOD::TrackParametersAuxContainer* m_trackParametersAux = nullptr;

  inline const xAOD::TrackParametersContainer& trackParameters() const {
    return *m_trackParameters;
  }
  inline xAOD::TrackParametersContainer& trackParameters() {
    return *m_trackParameters;
  }

  xAOD::TrackJacobianContainer* m_trackJacobians = nullptr;
  xAOD::TrackJacobianAuxContainer* m_trackJacobiansAux = nullptr;

  inline const xAOD::TrackJacobianContainer& trackJacobians() const {
    return *m_trackJacobians;
  }
  inline xAOD::TrackJacobianContainer& trackJacobians() {
    return *m_trackJacobians;
  }

  xAOD::TrackMeasurementContainer* m_trackMeasurements = nullptr;
  xAOD::TrackMeasurementAuxContainer* m_trackMeasurementsAux = nullptr;

  inline const xAOD::TrackMeasurementContainer& trackMeasurements() const {
    return *m_trackMeasurements;
  }
  inline xAOD::TrackMeasurementContainer& trackMeasurements() {
    return *m_trackMeasurements;
  }

  std::vector<detail::Decoration> m_decorations;
  //!< decoration accessors, one per type
  template <typename T>
  std::any decorationSetter(ActsTrk::IndexType, const std::string&);
  template <typename T>
  const std::any decorationGetter(ActsTrk::IndexType, const std::string&) const;

  std::vector<const Acts::Surface*> m_surfaces;
  std::vector<std::shared_ptr<const Acts::Surface>> m_managedSurfaces;
  const std::vector<const Acts::Surface*>& surfaces() const { return m_surfaces; }

};

namespace detail {
struct Decoration {
  using SetterType =
      std::function<std::any(ActsTrk::IndexType, const std::string&)>;
  using GetterType =
      std::function<const std::any(ActsTrk::IndexType, const std::string&)>;

  Decoration(const std::string& n, SetterType s, GetterType g)
      : name(n), hash(Acts::hashString(name)), setter(s), getter(g) {}
  std::string name;   // xAOD API needs this
  uint32_t hash;      // Acts API comes with this
  SetterType setter;  // type aware accessors
  GetterType getter;
};
}  // namespace detail

/**
 * Read only version of MTJ
 * The implementation is separate as the details are significantly different 
 * and in addition only const methods are ever needed
 */
class ConstMultiTrajectory
    : public Acts::MultiTrajectory<ConstMultiTrajectory> {
 public:

  ConstMultiTrajectory(
      DataLink<xAOD::TrackStateContainer> trackStates,
      DataLink<xAOD::TrackParametersContainer> trackParameters,
      DataLink<xAOD::TrackJacobianContainer> trackJacobians,
      DataLink<xAOD::TrackMeasurementContainer> trackMeasurements);

  bool has_impl(Acts::HashedString key, ActsTrk::IndexType istate) const;

  const std::any component_impl(Acts::HashedString key, ActsTrk::IndexType istate) const;

  bool hasColumn_impl(Acts::HashedString key) const;

  typename ConstTrackStateProxy::Parameters parameters_impl(
      ActsTrk::IndexType index) const {
    return m_trackParameters->at(index)->paramsEigen();
  }

  typename ConstTrackStateProxy::Covariance covariance_impl(
      ActsTrk::IndexType index) const {
    return m_trackParameters->at(index)->covMatrixEigen();
  }

  inline typename ConstTrackStateProxy::Covariance jacobian_impl(
      ActsTrk::IndexType istate) const {
    xAOD::TrackStateIndexType jacIdx = (*m_trackStates)[istate]->jacobian();
    return m_trackJacobians->at(jacIdx)->jacEigen();
  }

  template <std::size_t measdim>
  inline typename ConstTrackStateProxy::template Measurement<measdim>
  measurement_impl(IndexType index) const {
    xAOD::TrackStateIndexType measIdx = (*m_trackStates)[index]->calibrated();
    return m_trackMeasurements->at(measIdx)->template measEigen<measdim>();
  }

  template <std::size_t measdim>
  inline typename ConstTrackStateProxy::template MeasurementCovariance<measdim>
  measurementCovariance_impl(IndexType index) const;
  inline size_t size_impl() const { return m_trackStates->size(); }

  ActsTrk::IndexType calibratedSize_impl(ActsTrk::IndexType istate) const;
  typename Acts::SourceLink getUncalibratedSourceLink_impl(ActsTrk::IndexType istate) const;

  const Acts::Surface* referenceSurface_impl(IndexType) const;

  /**
   * Cache Surface pointers for every state.
   * If the surfaces are already there it means that the container is trainsient and this is void operation
   */
  void fillSurfaces(const Acts::TrackingGeometry* geo );
  void fillSurfaces(const ActsTrk::MutableMultiTrajectory* mtj);

 private:
  // TODO these 4 DATA links will be replaced by a reference to storable object that would contain those
  DataLink<xAOD::TrackStateContainer> m_trackStates;
  DataLink<xAOD::TrackParametersContainer> m_trackParameters;
  DataLink<xAOD::TrackJacobianContainer> m_trackJacobians;
  DataLink<xAOD::TrackMeasurementContainer> m_trackMeasurements;
  std::vector<detail::Decoration> m_decorations;
  template <typename T>
  const std::any decorationGetter(ActsTrk::IndexType, const std::string&) const;

  std::vector<const Acts::Surface*> m_surfaces;

};

}  // namespace ActsTrk

#include "MultiTrajectory.icc"


#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::ConstMultiTrajectory, 237752966, 1)

// These two lines shouldn't be here, but necessary until we have a proper
// solution
#include "Acts/EventData/VectorTrackContainer.hpp"
CLASS_DEF(Acts::ConstVectorTrackContainer, 1074811884, 1)

#endif
