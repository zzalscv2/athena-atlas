/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_TrackStorageContainer_h
#define ActsEvent_TrackStorageContainer_h
#include <type_traits>

#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/HashedString.hpp"
#include "ActsEvent/Decoration.h"
#include "xAODTracking/TrackStorageContainer.h"
#include "xAODTracking/TrackStorageAuxContainer.h"
#include "xAODTracking/TrackSurfaceAuxContainer.h"
#include "xAODTracking/TrackSurfaceContainer.h"
#include "ActsEvent/SurfaceEncoding.h"


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
  using IndexType = uint32_t; // TODO find common place for it
  static constexpr auto kInvalid = Acts::MultiTrajectoryTraits::kInvalid;
  TrackStorageContainer(const DataLink<xAOD::TrackStorageContainer>& lin = nullptr,
                        const DataLink<xAOD::TrackSurfaceContainer>& surfLink = nullptr);
  static const std::set<std::string> staticVariables;
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
  
  /**
  * surface
  */
  std::shared_ptr<const Acts::Surface> surface(ActsTrk::IndexType itrack) const;

  void fillFrom(ActsTrk::MutableTrackStorageContainer& mtb);

  template<typename T>
  friend class MutableTrackContainerHandle;
  friend class MutableTrackStorageContainer;

  void restoreDecorations();

  const xAOD::TrackStorageContainer* trackBackend() const{
    return m_trackBackend.cptr();
  }

  const xAOD::TrackSurfaceContainer* surfBackend() const{
    return m_surfBackend.cptr();
  }

 protected:
  using DecorationAccess = ActsTrk::detail::Decoration<xAOD::TrackStorageContainer>;

  std::vector<DecorationAccess> m_decorations;
  DataLink<xAOD::TrackStorageContainer> m_trackBackend = nullptr;
  DataLink<xAOD::TrackSurfaceContainer> m_surfBackend = nullptr;


  std::vector<std::shared_ptr<const Acts::Surface>> m_surfaces;
  std::vector<Acts::ParticleHypothesis> m_particleHypothesis; // TODO move the storage to the backend
};

class MutableTrackStorageContainer : public TrackStorageContainer {
 public:
  MutableTrackStorageContainer();
  MutableTrackStorageContainer(const MutableTrackStorageContainer&) = delete;
  MutableTrackStorageContainer operator=(const MutableTrackStorageContainer&) = delete;
  MutableTrackStorageContainer(MutableTrackStorageContainer&&) noexcept;

// Add and remove surface
  /**
  * adds new surface to the tail of the container
  */
  ActsTrk::IndexType addSurface_impl();

  /**
  * clears surface data under index
  */
  void removeSurface_impl(ActsTrk::IndexType isurf);

  //TODO: function(s) needed to fill the surface in the way analog to tracks


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
                             const ActsTrk::TrackStorageContainer& other,
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

  /** 
  * zeroes container
  */
  void clear();

  /**
  * point given track to surface
  * The surface ownership is shared
  */
  void setReferenceSurface_impl(ActsTrk::IndexType itrack,
                                std::shared_ptr<const Acts::Surface> surface);


 void setParticleHypothesis_impl(
    ActsTrk::IndexType itrack, const Acts::ParticleHypothesis& particleHypothesis) {
    m_particleHypothesis.resize(itrack+1, Acts::ParticleHypothesis::pion());
    m_particleHypothesis[itrack] = particleHypothesis;
  }

  template<typename T>
  friend class MutableTrackContainerHandle;


  xAOD::TrackStorageContainer* trackBackend(){
    return m_mutableTrackBackend.get();
  }

  xAOD::TrackSurfaceContainer* surfBackend(){
    return m_mutableSurfBackend.get();
  }

 private:
  std::unique_ptr<xAOD::TrackStorageContainer> m_mutableTrackBackend;
  std::unique_ptr<xAOD::TrackStorageAuxContainer> m_mutableTrackBackendAux;

  std::unique_ptr<xAOD::TrackSurfaceContainer> m_mutableSurfBackend;
  std::unique_ptr<xAOD::TrackSurfaceAuxContainer> m_mutableSurfBackendAux;
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
    case "tipIndex"_hash:
    case "stemIndex"_hash:

      return true;
  }
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      return true;
    }
  }
  return false;
}

namespace details{
} // EOF detail

template <typename T>
constexpr void MutableTrackStorageContainer::addColumn_impl(
    const std::string& name) {
  if (not ActsTrk::detail::accepted_decoration_types<T>::value) {
    throw std::runtime_error(
        "TrackStorageContainer::addColumn_impl: "
        "unsupported decoration type");
  }
  m_decorations.push_back(DecorationAccess(
      name,
      ActsTrk::detail::constDecorationGetter<xAOD::TrackStorageContainer, T>,
      ActsTrk::detail::decorationCopier<xAOD::TrackStorageContainer, T>,
      ActsTrk::detail::decorationSetter<xAOD::TrackStorageContainer, T>
      ));
}

}  // namespace ActsTrk

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::TrackStorageContainer , 1333051576 , 1 )

#endif
