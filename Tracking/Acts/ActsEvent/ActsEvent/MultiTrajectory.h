/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_MultiTrajectory_h
#define ActsEvent_MultiTrajectory_h
#include <memory>
#include <type_traits>
#include <variant>

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/TrackStatePropMask.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/Utilities/HashedString.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
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
#include "xAODTracking/TrackSurfaceAuxContainer.h"
#include "xAODTracking/TrackSurfaceContainer.h"

// #define DEBUG_MTJ
#ifdef DEBUG_MTJ
inline std::string_view name_only(const char* s) {
  return std::string_view(s+std::string_view(s).rfind('/'));
}
#define INSPECTCALL(_INFO) {std::cout << name_only(__FILE__) <<":"<<__LINE__<<" "<<__PRETTY_FUNCTION__<<" "<<_INFO<<std::endl; }
#else
#define INSPECTCALL(_INFO)
#endif
#include "ActsEvent/Decoration.h"

namespace ActsTrk {
class MutableMultiTrajectory;
class MultiTrajectory;
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
struct IsReadOnlyMultiTrajectory<ActsTrk::MultiTrajectory>
    : std::true_type {};

template <>
struct IsReadOnlyMultiTrajectory<ActsTrk::MutableMultiTrajectory>
    : std::false_type {};
}  // namespace Acts

namespace ActsTrk {

using IndexType = std::uint32_t;

using StoredSurface = std::variant<const Acts::Surface*, std::shared_ptr<const Acts::Surface>>;


/**
 * @brief Athena implementation of ACTS::MultiTrajectory (ReadWrite version)
 * The data is stored in 4 external backends. 
 * Backends lifetime are not maintained by this class.
 * except when objects are default constructed (this functionality will be removed). 
 * This class is meant to be used in track finding algorithms (e.g. CKF) and then converted
 * MultiTrajectory variant. These conversion is meant to be costless. 
 */
class MutableMultiTrajectory final
    : public Acts::MultiTrajectory<ActsTrk::MutableMultiTrajectory> {
 public:
  friend ActsTrk::MultiTrajectory;
  using TrackStateProxy = typename Acts::MultiTrajectory<
      ActsTrk::MutableMultiTrajectory>::TrackStateProxy;
  using ConstTrackStateProxy = typename Acts::MultiTrajectory<
      ActsTrk::MutableMultiTrajectory>::ConstTrackStateProxy;

  /**
   * @brief Construct a new Multi Trajectory object owning backends
   */
  MutableMultiTrajectory();
  MutableMultiTrajectory(const ActsTrk::MutableMultiTrajectory& other);
  MutableMultiTrajectory(ActsTrk::MutableMultiTrajectory&& other) = default;
  MutableMultiTrajectory& operator=(const ActsTrk::MutableMultiTrajectory& other) = delete;

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
  bool has_impl(Acts::HashedString key, ActsTrk::IndexType istate) const;

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
    return typename ConstTrackStateProxy::Parameters{m_trackParametersAux->params[index].data()};
  }

  typename TrackStateProxy::Parameters parameters_impl(ActsTrk::IndexType index) {
    return typename TrackStateProxy::Parameters{m_trackParametersAux->params[index].data()};
  }

  /**
   * @brief obtain covariances for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Covariance
   */
  typename ConstTrackStateProxy::Covariance covariance_impl(
      ActsTrk::IndexType index) const {
    return typename ConstTrackStateProxy::Covariance{m_trackParametersAux->covMatrix[index].data()};
  }
  typename TrackStateProxy::Covariance covariance_impl(ActsTrk::IndexType index) {
    return typename TrackStateProxy::Covariance{m_trackParametersAux->covMatrix[index].data()};    
  }

  /**
   * @brief obtain measurement covariances for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Covariance
   */
  typename ConstTrackStateProxy::Covariance trackMeasurementsCov(
      ActsTrk::IndexType index) const {
    return typename ConstTrackStateProxy::Covariance{m_trackMeasurementsAux->covMatrix[index].data()};

  }

  typename TrackStateProxy::Covariance trackMeasurementsCov(ActsTrk::IndexType index) {
    return typename TrackStateProxy::Covariance{m_trackMeasurementsAux->covMatrix[index].data()};

  }

  /**
   * @brief obtain jacobian for a state at given index
   *
   * @param index
   * @return TrackStateProxy::Covariance
   */

  inline typename ConstTrackStateProxy::Covariance jacobian_impl(
      ActsTrk::IndexType istate) const {
    xAOD::TrackStateIndexType jacIdx = m_trackStatesAux->jacobian[istate];
    return typename ConstTrackStateProxy::Covariance{m_trackJacobiansAux->jac[jacIdx].data()};
  }

  typename TrackStateProxy::Covariance jacobian_impl(ActsTrk::IndexType istate) {
    xAOD::TrackStateIndexType jacIdx = m_trackStatesAux->jacobian[istate];
    return typename TrackStateProxy::Covariance{m_trackJacobiansAux->jac[jacIdx].data()};
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
    xAOD::TrackStateIndexType measIdx = m_trackStatesAux->calibrated[index];
    return typename ConstTrackStateProxy::template Measurement<measdim>{m_trackMeasurementsAux->meas[measIdx].data()};
  }

  template <std::size_t measdim, bool Enable = true>
  std::enable_if_t<Enable,
                   typename TrackStateProxy::template Measurement<measdim>>
  measurement_impl(ActsTrk::IndexType index) {
    xAOD::TrackStateIndexType measIdx = m_trackStatesAux->calibrated[index];
    return typename TrackStateProxy::template Measurement<measdim>{m_trackMeasurementsAux->meas[measIdx].data()};
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
    xAOD::TrackStateIndexType measIdx = m_trackStatesAux->calibrated[index];
    return ConstTrackStateProxy::template MeasurementCovariance<measdim>{m_trackMeasurementsAux->covMatrix[measIdx].data()};
  }
  template <std::size_t measdim, bool Enable = true>
  std::enable_if_t<
      Enable, typename TrackStateProxy::template MeasurementCovariance<measdim>>
  measurementCovariance_impl(ActsTrk::IndexType index) {
    xAOD::TrackStateIndexType measIdx = m_trackStatesAux->calibrated[index];
    return TrackStateProxy::template MeasurementCovariance<measdim>{m_trackMeasurementsAux->covMatrix[measIdx].data()};
  }

  /**
   * @brief size of the MTJ
   *
   * @return size_t
   */

  inline size_t size_impl() const { 
    //return m_trackStatesAux->size(); 
    return m_trackStatesSize;
  }

  /**
   * @brief clears backends
   * decoration columns are still declared
   */
  void clear_impl();

  /**
   * @brief checks if the backends are connected (i.e. is safe to use, else any
   * other call will cause segfaults)
   */
  bool has_backends() const;

  /**
   * Implementation of allocation of calibrated measurements
   */
  void allocateCalibrated_impl(ActsTrk::IndexType istate, std::size_t measdim);

  /**
   * Implementation of calibrated size
   */
  ActsTrk::IndexType calibratedSize_impl(ActsTrk::IndexType istate) const;

  /**
   * Implementation of uncalibrated link insertion
   */
  void setUncalibratedSourceLink_impl(ActsTrk::IndexType istate,
                                      const Acts::SourceLink& sourceLink);

  /**
   * Implementation of uncalibrated link fetch
   */
  typename Acts::SourceLink getUncalibratedSourceLink_impl(
      ActsTrk::IndexType istate) const;
  typename Acts::SourceLink getUncalibratedSourceLink_impl(IndexType istate);


  void setReferenceSurface_impl(IndexType,
                                std::shared_ptr<const Acts::Surface>);
  const Acts::Surface* referenceSurface_impl(IndexType ) const;




  // access to some backends (for debugging purposes)
  // the reciever should not assume ownership or similar
  inline xAOD::TrackStateAuxContainer* trackStatesAux() { return m_trackStatesAux.get(); }

  inline const xAOD::TrackParametersAuxContainer* trackParametersAux() const {
    return m_trackParametersAux.get();
  }
  inline xAOD::TrackParametersAuxContainer* trackParametersAux() {
    return m_trackParametersAux.get();
  }

  inline xAOD::TrackJacobianAuxContainer* trackJacobiansAux() {
    return m_trackJacobiansAux.get();
  }

  inline xAOD::TrackMeasurementAuxContainer* trackMeasurementsAux() {
    return m_trackMeasurementsAux.get();
  }


  template<typename X>
  friend class MutableMultiTrajectoryHandle;


 private:

  // std::unique_ptr<xAOD::TrackStateContainer> m_trackStates;
  std::unique_ptr<xAOD::TrackStateAuxContainer> m_trackStatesAux;
  size_t m_trackStatesSize = 0;

  // std::unique_ptr<xAOD::TrackParametersContainer> m_trackParameters;
  std::unique_ptr<xAOD::TrackParametersAuxContainer> m_trackParametersAux;
  size_t m_trackParametersSize = 0;

  // std::unique_ptr<xAOD::TrackJacobianContainer> m_trackJacobians;
  std::unique_ptr<xAOD::TrackJacobianAuxContainer> m_trackJacobiansAux;
  size_t m_trackJacobiansSize = 0;

  // std::unique_ptr<xAOD::TrackMeasurementContainer> m_trackMeasurements;
  std::unique_ptr<xAOD::TrackMeasurementAuxContainer> m_trackMeasurementsAux;
  size_t m_trackMeasurementsSize = 0;

  std::unique_ptr<xAOD::TrackSurfaceContainer> m_surfacesBackend;
  std::unique_ptr<xAOD::TrackSurfaceAuxContainer> m_surfacesBackendAux;
  using DecorationAccess = ActsTrk::detail::Decoration<xAOD::TrackStateContainer>;
  std::vector<DecorationAccess> m_decorations;

  std::vector<std::optional<Acts::SourceLink>> m_calibratedSourceLinks;
  std::vector<std::optional<Acts::SourceLink>> m_uncalibratedSourceLinks;

  std::vector<StoredSurface> m_surfaces;
  ActsGeometryContext m_geoContext;
};

/**
 * Read only version of MTJ
 * The implementation is separate as the details are significantly different 
 * and in addition only const methods are ever needed
 */
class MultiTrajectory
    : public Acts::MultiTrajectory<MultiTrajectory> {
 public:

  MultiTrajectory(
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
  measurementCovariance_impl(IndexType index) const {
    xAOD::TrackStateIndexType measIdx = (*m_trackStates)[index]->calibrated();
    return m_trackMeasurements->at(measIdx)->template covMatrixEigen<measdim>();
  }
  inline size_t size_impl() const { return m_trackStates->size(); }

  ActsTrk::IndexType calibratedSize_impl(ActsTrk::IndexType istate) const;
  typename Acts::SourceLink getUncalibratedSourceLink_impl(ActsTrk::IndexType istate) const;

  const Acts::Surface* referenceSurface_impl(IndexType) const;

  /**
   * Fill surfaces either from persistency or from geometry
   * If the surfaces are already there it means that the container is trainsient and this is void operation
   */
  void fillSurfaces(const Acts::TrackingGeometry* geo, const ActsGeometryContext& geoContext );
  /**
   * reuse surfaces from MutableMultiTrajectory
   */
  void moveSurfaces(const ActsTrk::MutableMultiTrajectory* mtj);

  void moveLinks(const ActsTrk::MutableMultiTrajectory* mtj);

 private:
  // TODO these 4 DATA links will be replaced by a reference to storable object that would contain those
  const DataLink<xAOD::TrackStateContainer> m_trackStates;
  const xAOD::TrackStateAuxContainer* m_trackStatesAux{};
  const DataLink<xAOD::TrackParametersContainer> m_trackParameters;
  const DataLink<xAOD::TrackJacobianContainer> m_trackJacobians;
  const DataLink<xAOD::TrackMeasurementContainer> m_trackMeasurements;
  using DecorationAccess = ActsTrk::detail::Decoration<xAOD::TrackStateContainer>;
  std::vector<DecorationAccess> m_decorations;
  // TODO remove once tracking code switches to sourceLinks with EL
  std::vector<std::optional<Acts::SourceLink>> m_calibratedSourceLinks;  
  std::vector<std::optional<Acts::SourceLink>> m_uncalibratedSourceLinks;  

  std::vector<StoredSurface> m_surfaces;
};


/**
* @brief helper to construct interface container for already filled Aux container
* TODO maybe should be moved to xAOD area
*/
template<typename IFACE, typename AUX>
std::unique_ptr<IFACE> makeInterfaceContainer(const AUX* aux) {
  auto interface = std::make_unique<IFACE>();
  for ( size_t i = 0, sz = aux->size(); i < sz; ++i) {
    interface->emplace_back( new std::remove_pointer_t<typename IFACE::value_type>() );
  }
  interface->setStore(aux);
  return interface;
}


}  // namespace ActsTrk

#include "MultiTrajectory.icc"


#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::MultiTrajectory , 51219308 , 1 )

// These two lines shouldn't be here, but necessary until we have a proper
// solution
#include "Acts/EventData/VectorTrackContainer.hpp"
CLASS_DEF(Acts::ConstVectorTrackContainer, 1074811884, 1)

#endif
