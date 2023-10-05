/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_TrackStorageContainer_h
#define ActsEvent_TrackStorageContainer_h
#include <type_traits>

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/HashedString.hpp"
#include "ActsEvent/MultiTrajectory.h"
#include "xAODTracking/TrackBackendContainer.h"
#include "xAODTracking/TrackBackendAuxContainer.h"



namespace ActsTrk {
class MutableTrackStorageContainer;
class TrackStorageContainer;
}  // namespace ActsTrk

namespace Acts {
class Surface;
template <typename T>
struct IsReadOnlyTrackContainer {};

template <typename T>
struct IsReadOnlyTrackContainer<T&> : IsReadOnlyTrackContainer<T> {};

template <typename T>
struct IsReadOnlyTrackContainer<T&&> : IsReadOnlyTrackContainer<T> {};

template <>
struct IsReadOnlyTrackContainer<ActsTrk::TrackStorageContainer>
    : std::true_type {};

template <>
struct IsReadOnlyTrackContainer<ActsTrk::MutableTrackStorageContainer>
    : std::false_type {};
}  // namespace Acts

namespace ActsTrk {

using ConstParameters = Acts::TrackStateTraits<3>::Parameters;
using ConstCovariance = Acts::TrackStateTraits<3>::Covariance;
using Parameters = Acts::TrackStateTraits<3, false>::Parameters;
using Covariance = Acts::TrackStateTraits<3, false>::Covariance;

class MutableTrackStorageContainer;

class TrackStorageContainer {
 public:
  using IndexType = Acts::MultiTrajectoryTraits::IndexType;
  static constexpr auto kInvalid = Acts::MultiTrajectoryTraits::kInvalid;
  TrackStorageContainer(const DataLink<xAOD::TrackBackendContainer>& lin = nullptr);

  /**
  * return true if the container has specific decoration
  */
  constexpr bool hasColumn_impl(Acts::HashedString key) const;

  /**
  * return pointer to reference surface
  */
  const Acts::Surface* referenceSurface_impl(ActsTrk::IndexType itrack) const;

  /**
  * returns number of stored tracks
  */
  std::size_t size_impl() const;

  /**
  * access to components by pointer with type
  */
  std::any component_impl(Acts::HashedString key,
                          ActsTrk::IndexType itrack) const;

  /**
  * parameters of the track
  */
  ActsTrk::ConstParameters parameters(ActsTrk::IndexType itrack) const;

  /**
  * covariance of the track fit
  */
  ActsTrk::ConstCovariance covariance(ActsTrk::IndexType itrack) const;

  void fillSurfaces(ActsTrk::MutableTrackStorageContainer& mtb);

  template<typename T>
  friend class MutableTrackContainerHandle;
  friend class MutableTrackStorageContainer;

 protected:
  void restoreDecorations();
  template <typename T>
  const std::any decorationGetter(ActsTrk::IndexType, const std::string&) const;
  std::vector<ActsTrk::detail::Decoration> m_decorations;
  DataLink<xAOD::TrackBackendContainer> m_backend = nullptr;
  std::vector<std::shared_ptr<const Acts::Surface>> m_surfaces;
};

class MutableTrackStorageContainer : public TrackStorageContainer {
 public:
  MutableTrackStorageContainer();
  /**
  * adds new track to the tail of the container
  */
  ActsTrk::IndexType addTrack_impl();

  /**
  * clears track data under index
  */
  void removeTrack_impl(ActsTrk::IndexType itrack);

  /**
  * enables the container to support decoration of given name and type
  */
  template <typename T>
  constexpr void addColumn_impl(const std::string& key);

  /**
  * copies decorations from other container
  */
  void copyDynamicFrom_impl (ActsTrk::IndexType itrack,
                             const MutableTrackStorageContainer& other,
                             ActsTrk::IndexType other_itrack);


  /**
  * write access to decorations
  */
  std::any component_impl(Acts::HashedString key,
                          ActsTrk::IndexType itrack);
  using TrackStorageContainer::component_impl;

  /**
  * write access to parameters
  */
  ActsTrk::Parameters parameters(ActsTrk::IndexType itrack);
  using TrackStorageContainer::parameters;

  /**
  * write access to covariance
  */
  ActsTrk::Covariance covariance(ActsTrk::IndexType itrack);
  using TrackStorageContainer::covariance;

  /**
  * synchronizes decorations
  */
  void ensureDynamicColumns_impl(const MutableTrackStorageContainer& other);
  void ensureDynamicColumns_impl(const TrackStorageContainer& other);

  /**
  * preallocate number of track objects 
  */
  void reserve(ActsTrk::IndexType size);

  /**3660251998 
  * zeroes container
  */
  void clear();

  /**
  * point given track to surface
  * The surface ownership is shared
  */
  void setReferenceSurface_impl(ActsTrk::IndexType itrack,
                                std::shared_ptr<const Acts::Surface> surface);

  template<typename T>
  friend class MutableTrackContainerHandle;

 private:
  std::unique_ptr<xAOD::TrackBackendContainer> m_backend;
  std::unique_ptr<xAOD::TrackBackendAuxContainer> m_backendAux;

  template <typename T>
  std::any decorationSetter(ActsTrk::IndexType, const std::string&);
};


constexpr bool ActsTrk::TrackStorageContainer::hasColumn_impl(
    Acts::HashedString key) const {
  using namespace Acts::HashedStringLiteral;
  switch (key) {
    case "params"_hash:
    case "cov"_hash:
    case "nMeasurements"_hash:
    case "nHoles"_hash:
    case "d0"_hash:
    case "chi2"_hash:
    case "ndf"_hash:
    case "nOutliers"_hash:
    case "hSharedHits"_hash:
      return true;
  }
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      return true;
    }
  }
  return false;
}


template <typename T>
const std::any TrackStorageContainer::decorationGetter(
    ActsTrk::IndexType idx, const std::string& name) const {
  const SG::ConstAuxElement el(m_backend.cptr(), idx);  
  return &(el.auxdataConst<T>(name));

}

template <typename T>
std::any MutableTrackStorageContainer::decorationSetter(
    ActsTrk::IndexType idx, const std::string& name) {
  const SG::AuxElement* el = (*m_backend)[idx];
  return &(el->auxdecor<T>(name));
}

template <typename T>
constexpr void MutableTrackStorageContainer::addColumn_impl(
    const std::string& name) {
  if (not ActsTrk::detail::accepted_decoration_types<T>::value) {
    throw std::runtime_error(
        "TrackStorageContainer::addColumn_impl: "
        "unsupported decoration type");
  }
  using std::placeholders::_1;
  using std::placeholders::_2;
  m_decorations.push_back(ActsTrk::detail::Decoration(
      name,
      std::bind(&ActsTrk::MutableTrackStorageContainer::decorationSetter<T>,
                this, _1, _2),
      std::bind(&ActsTrk::TrackStorageContainer::decorationGetter<T>, this,
                _1, _2)));
}

}  // namespace ActsTrk

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::TrackStorageContainer , 1333051576 , 1 )

#endif
