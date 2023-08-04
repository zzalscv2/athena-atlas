/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/TrackBackendContainer.h"
#include "xAODTracking/TrackBackend.h"

ActsTrk::ConstTrackBackendContainer::ConstTrackBackendContainer(
    const xAOD::TrackBackendContainer* backend)
    : m_backend(backend) {}

const Acts::Surface* ActsTrk::ConstTrackBackendContainer::referenceSurface_impl(
    ActsTrk::IndexType) const {
  return nullptr;
}

std::size_t ActsTrk::ConstTrackBackendContainer::size_impl() const {
  return m_backend->size();
}

namespace {
template<typename C>
std::any component_impl(C& container, Acts::HashedString key, ActsTrk::IndexType itrack) {
  using namespace Acts::HashedStringLiteral;
  switch (key) {
    case "nMeasurements"_hash:
      return container.at(itrack)->nMeasurementsPtr();
    case "nHoles"_hash:
      return container.at(itrack)->nHolesPtr();
    case "chi2"_hash:
      return container.at(itrack)->chi2Ptr();
    case "ndf"_hash:
      return container.at(itrack)->ndfPtr();
    case "nOutliers"_hash:
      return container.at(itrack)->nOutliersPtr();
    case "nSharedHits"_hash:
      return container.at(itrack)->nSharedHitsPtr();
    default:
      return std::any();
  }
}
}

std::any ActsTrk::ConstTrackBackendContainer::component_impl(
    Acts::HashedString key, ActsTrk::IndexType itrack) const {
  std::any result = ::component_impl(*m_backend, key, itrack);
  if ( result.has_value()) {
    return result;
  }
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      return d.getter(itrack, d.name);
    }
  }
  throw std::runtime_error("ConstTrackBackendContainer no such component " +
                           std::to_string(key));
}

ActsTrk::ConstParameters ActsTrk::ConstTrackBackendContainer::parameters(
    ActsTrk::IndexType itrack) const {
  return m_backend->at(itrack)->paramsEigen();
}

ActsTrk::ConstCovariance ActsTrk::ConstTrackBackendContainer::covariance(
    ActsTrk::IndexType itrack) const {
  return m_backend->at(itrack)->covParamsEigen();
}

ActsTrk::MutableTrackBackendContainer::MutableTrackBackendContainer(
    xAOD::TrackBackendContainer* backend)
    : ConstTrackBackendContainer(backend), m_backend(backend) {}

ActsTrk::IndexType ActsTrk::MutableTrackBackendContainer::addTrack_impl() {
  m_backend->push_back(std::make_unique<xAOD::TrackBackend>());
  return m_backend->size() - 1;
}

void ActsTrk::MutableTrackBackendContainer::removeTrack_impl (ActsTrk::IndexType itrack) {
  if (itrack >= m_backend->size()) {
    throw std::out_of_range ("removeTrack_impl");
  }
  m_backend->erase(m_backend->begin() + itrack);
}

void ActsTrk::MutableTrackBackendContainer::copyDynamicFrom_impl \
  (ActsTrk::IndexType itrack,
   const MutableTrackBackendContainer& other,
   ActsTrk::IndexType other_itrack)
{
  m_backend->at(itrack) = other.m_backend->at(other_itrack);
}


std::any ActsTrk::MutableTrackBackendContainer::component_impl(
    Acts::HashedString key, ActsTrk::IndexType itrack) {
  std::any result = ::component_impl(*m_backend, key, itrack);
  if ( result.has_value()) {
    return result;
  }
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      return d.setter(itrack, d.name);
    }
  }
  throw std::runtime_error("ConstTrackBackendContainer no such component " +
                           std::to_string(key));
}


ActsTrk::Parameters ActsTrk::MutableTrackBackendContainer::parameters(
    ActsTrk::IndexType itrack) {
  return m_backend->at(itrack)->paramsEigen();
}

ActsTrk::Covariance ActsTrk::MutableTrackBackendContainer::covariance(
    ActsTrk::IndexType itrack) {
  return m_backend->at(itrack)->covParamsEigen();
}

void ActsTrk::MutableTrackBackendContainer::ensureDynamicColumns_impl(
    const MutableTrackBackendContainer& other) {
  for (auto& d : other.m_decorations) {
    m_decorations.push_back(d);
  }
}

void ActsTrk::MutableTrackBackendContainer::ensureDynamicColumns_impl(
    const ConstTrackBackendContainer& other) {
  for (auto& d : other.m_decorations) {
    m_decorations.push_back(d);
  }
}

void ActsTrk::MutableTrackBackendContainer::reserve(ActsTrk::IndexType size) {
    m_backend->reserve(size);
}

void ActsTrk::MutableTrackBackendContainer::clear() {
    m_backend->clear();
}

void ActsTrk::MutableTrackBackendContainer::setReferenceSurface_impl(
    ActsTrk::IndexType , std::shared_ptr<const Acts::Surface> ){ 
      // TODO implement
    }
