/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/TrackBackendContainer.h"

#include "xAODTracking/TrackBackend.h"

ActsTrk::ConstTrackBackendContainer::ConstTrackBackendContainer(const DataLink<xAOD::TrackBackendContainer>& link) 
  : m_backend(link) {
}


const Acts::Surface* ActsTrk::ConstTrackBackendContainer::referenceSurface_impl(
    ActsTrk::IndexType itrack) const {
  if ( itrack >= m_surfaces.size() ) throw std::out_of_range("ConstTrackBackendContainer index out of range when accessing reference surface");
  return m_surfaces[itrack].get();
}

std::size_t ActsTrk::ConstTrackBackendContainer::size_impl() const {
  return m_backend->size();
}

namespace {
template <typename C>
std::any component_impl(C& container, Acts::HashedString key,
                        ActsTrk::IndexType itrack) {
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
}  // namespace

std::any ActsTrk::ConstTrackBackendContainer::component_impl(
    Acts::HashedString key, ActsTrk::IndexType itrack) const {
  std::any result = ::component_impl(*m_backend, key, itrack);
  if (result.has_value()) {
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

void ActsTrk::ConstTrackBackendContainer::fillSurfaces(ActsTrk::MutableTrackBackendContainer& mtb) {
  m_surfaces = std::move(mtb.m_surfaces);
}

void ActsTrk::ConstTrackBackendContainer::restoreDecorations() {

  for ( auto id : m_backend->getConstStore()->getAuxIDs() ) {
    if ( m_backend->getConstStore()->isDecoration(id) ) { continue; }
    const std::string name = SG::AuxTypeRegistry::instance().getName(id);
    const std::type_info* typeInfo = SG::AuxTypeRegistry::instance().getType(id);

    using std::placeholders::_1;
    using std::placeholders::_2;
    // try making decoration accessor of matching type
    // there is a fixed set of supported types (as there is a fixed set available in MutableMTJ)
    // setters are not needed so replaced by a "nullptr"
    if ( *typeInfo == typeid(float) ) {
      m_decorations.emplace_back( name,
        static_cast<ActsTrk::detail::Decoration::SetterType>(nullptr),
        std::bind(&ActsTrk::ConstTrackBackendContainer::decorationGetter<float>, this,
                  _1, _2));
    } else if ( *typeInfo == typeid(double) ) {
      m_decorations.emplace_back( name,
        static_cast<ActsTrk::detail::Decoration::SetterType>(nullptr),
        std::bind(&ActsTrk::ConstTrackBackendContainer::decorationGetter<double>, this,
                  _1, _2));

    } else if ( *typeInfo == typeid(short) ) {
      m_decorations.emplace_back( name,
        static_cast<ActsTrk::detail::Decoration::SetterType>(nullptr),
        std::bind(&ActsTrk::ConstTrackBackendContainer::decorationGetter<short>, this,
                  _1, _2));
    } else if ( *typeInfo == typeid(uint32_t) ) {
      m_decorations.emplace_back( name,
        static_cast<ActsTrk::detail::Decoration::SetterType>(nullptr),
        std::bind(&ActsTrk::ConstTrackBackendContainer::decorationGetter<uint32_t>, this,
                  _1, _2));
    }
  }
}



////////////////////////////////////////////////////////////////////
// write api
////////////////////////////////////////////////////////////////////
ActsTrk::MutableTrackBackendContainer::MutableTrackBackendContainer()
    : m_backend(std::make_unique<xAOD::TrackBackendContainer>()),
      m_backendAux(std::make_unique<xAOD::TrackBackendAuxContainer>()) {
  m_backend->setStore(m_backendAux.get());
  ConstTrackBackendContainer::m_backend = m_backend.get();

  addColumn_impl<IndexType>("tipIndex");
}

ActsTrk::IndexType ActsTrk::MutableTrackBackendContainer::addTrack_impl() {
  m_backend->push_back(std::make_unique<xAOD::TrackBackend>());
  m_backend->back()->resize();
  return m_backend->size() - 1;
}

void ActsTrk::MutableTrackBackendContainer::removeTrack_impl(
    ActsTrk::IndexType itrack) {
  if (itrack >= m_backend->size()) {
    throw std::out_of_range("removeTrack_impl");
  }
  m_backend->erase(m_backend->begin() + itrack);
}

void ActsTrk::MutableTrackBackendContainer::copyDynamicFrom_impl(
    ActsTrk::IndexType itrack, const MutableTrackBackendContainer& other,
    ActsTrk::IndexType other_itrack) {
  m_backend->at(itrack) = other.m_backend->at(other_itrack);
}

std::any ActsTrk::MutableTrackBackendContainer::component_impl(
    Acts::HashedString key, ActsTrk::IndexType itrack) {
  std::any result = ::component_impl(*m_backend, key, itrack);
  if (result.has_value()) {
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
  m_surfaces.clear();
}

void ActsTrk::MutableTrackBackendContainer::setReferenceSurface_impl(
    ActsTrk::IndexType itrack, std::shared_ptr<const Acts::Surface> surface) {
  m_surfaces.resize(itrack+1, nullptr);
  m_surfaces[itrack] = std::move(surface);
  
}
