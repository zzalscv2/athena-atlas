/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_TrackBackendContainer_h
#define ActsEvent_TrackBackendContainer_h
#include <type_traits>

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/HashedString.hpp"
#include "ActsEvent/MultiTrajectory.h"
#include "xAODTracking/TrackBackendContainer.h"

namespace ActsTrk {

using IndexType = std::uint32_t;
using ConstParameters = Acts::TrackStateTraits<3>::Parameters;
using ConstCovariance = Acts::TrackStateTraits<3>::Covariance;
using Parameters = Acts::TrackStateTraits<3, false>::Parameters;
using Covariance = Acts::TrackStateTraits<3, false>::Covariance;

class MutableTrackBackendContainer;

class ConstTrackBackendContainer {
 public:
  ConstTrackBackendContainer(const xAOD::TrackBackendContainer* backend);
  constexpr bool hasColumn_impl(Acts::HashedString key) const;
  const Acts::Surface* referenceSurface_impl(ActsTrk::IndexType itrack) const;
  std::size_t size_impl() const;

  std::any component_impl(Acts::HashedString key,
                          ActsTrk::IndexType itrack) const;
  ActsTrk::ConstParameters parameters(ActsTrk::IndexType itrack) const;
  ActsTrk::ConstCovariance covariance(ActsTrk::IndexType itrack) const;
  friend class MutableTrackBackendContainer;

 protected:
  template <typename T>
  const std::any decorationGetter(ActsTrk::IndexType, const std::string&) const;
  std::vector<ActsTrk::detail::Decoration> m_decorations;

 private:
  const xAOD::TrackBackendContainer* m_backend = nullptr;
};

class MutableTrackBackendContainer : public ConstTrackBackendContainer {
 public:
  MutableTrackBackendContainer(xAOD::TrackBackendContainer*);
  ActsTrk::IndexType addTrack_impl();
  template <typename T>
  constexpr void addColumn_impl(const std::string& key);

  std::any component_impl(Acts::HashedString key,
                          ActsTrk::IndexType itrack);
  ActsTrk::Parameters parameters(ActsTrk::IndexType itrack);
  ActsTrk::Covariance covariance(ActsTrk::IndexType itrack);
  void ensureDynamicColumns_impl(const MutableTrackBackendContainer& other);
  void ensureDynamicColumns_impl(const ConstTrackBackendContainer& other);

  void reserve(ActsTrk::IndexType size);
  void clear();

  void setReferenceSurface_impl(ActsTrk::IndexType itrack,
                                std::shared_ptr<const Acts::Surface> surface);

 private:
  xAOD::TrackBackendContainer* m_backend = nullptr;
  template <typename T>
  std::any decorationSetter(ActsTrk::IndexType, const std::string&);
};


constexpr bool ActsTrk::ConstTrackBackendContainer::hasColumn_impl(
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
const std::any ConstTrackBackendContainer::decorationGetter(
    ActsTrk::IndexType idx, const std::string& name) const {
  const SG::AuxElement* el = (*m_backend)[idx];
  return const_cast<const T*>(&(el->auxdecor<T>(name)));
}

template <typename T>
std::any MutableTrackBackendContainer::decorationSetter(
    ActsTrk::IndexType idx, const std::string& name) {
  // TODO optimize like the MTJ  
  const SG::AuxElement* el = (*m_backend)[idx];
  return &(el->auxdecor<T>(name));
}

template <typename T>
constexpr void MutableTrackBackendContainer::addColumn_impl(
    const std::string& name) {
  if (not ActsTrk::detail::accepted_decoration_types<T>::value) {
    throw std::runtime_error(
        "TrackBackendContainer::addColumn_impl: "
        "unsupported decoration type");
  }
  using std::placeholders::_1;
  using std::placeholders::_2;
  m_decorations.push_back(ActsTrk::detail::Decoration(
      name,
      std::bind(&ActsTrk::MutableTrackBackendContainer::decorationSetter<T>,
                this, _1, _2),
      std::bind(&ActsTrk::ConstTrackBackendContainer::decorationGetter<T>, this,
                _1, _2)));
}

}  // namespace ActsTrk

#endif