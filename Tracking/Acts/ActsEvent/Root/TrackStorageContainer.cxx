/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/TrackStorageContainer.h"

#include "xAODTracking/TrackStorage.h"
// this is list of xAOD container varaible names that are "hardcoded" in TrackStorage_v1
// their compatibility is maintained by the unit tests: AllStaticxAODVaraiblesAreKnown
const std::set<std::string> ActsTrk::TrackStorageContainer::staticVariables = {
    "params", "covParams", "nMeasurements", "nHoles",   "chi2f",
    "ndf",    "nOutliers", "nSharedHits",   "tipIndex", "stemIndex"};

ActsTrk::TrackStorageContainer::TrackStorageContainer(
    const DataLink<xAOD::TrackStorageContainer>& link)
    : m_backend(link) {}

const Acts::Surface* ActsTrk::TrackStorageContainer::referenceSurface_impl(
    ActsTrk::IndexType itrack) const {
  if (itrack >= m_surfaces.size())
    throw std::out_of_range(
        "TrackStorageContainer index out of range when accessing reference "
        "surface");
  return m_surfaces[itrack].get();
}

std::size_t ActsTrk::TrackStorageContainer::size_impl() const {
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
      return container.at(itrack)->chi2fPtr();
    case "ndf"_hash:
      return container.at(itrack)->ndfPtr();
    case "nOutliers"_hash:
      return container.at(itrack)->nOutliersPtr();
    case "nSharedHits"_hash:
      return container.at(itrack)->nSharedHitsPtr();
    case "tipIndex"_hash:
      return container.at(itrack)->tipIndexPtr();
    case "stemIndex"_hash:
      return container.at(itrack)->stemIndexPtr();

    default:
      return std::any();
  }
}
}  // namespace

std::any ActsTrk::TrackStorageContainer::component_impl(
    Acts::HashedString key, ActsTrk::IndexType itrack) const {
  std::any result = ::component_impl(*m_backend, key, itrack);
  if (result.has_value()) {
    return result;
  }
  using namespace Acts::HashedStringLiteral;
  if (key == "particleHypothesis"_hash) {
    return &m_particleHypothesis[itrack];
  }
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      return d.getter(m_backend.cptr(), itrack, d.name);
    }
  }
  throw std::runtime_error("TrackStorageContainer no such component " +
                           std::to_string(key));
}

ActsTrk::ConstParameters ActsTrk::TrackStorageContainer::parameters(
    ActsTrk::IndexType itrack) const {
  return m_backend->at(itrack)->paramsEigen();
}

ActsTrk::ConstCovariance ActsTrk::TrackStorageContainer::covariance(
    ActsTrk::IndexType itrack) const {
  return m_backend->at(itrack)->covParamsEigen();
}

void ActsTrk::TrackStorageContainer::fillFrom(
    ActsTrk::MutableTrackStorageContainer& mtb) {
  m_surfaces = std::move(mtb.m_surfaces);
  m_particleHypothesis = std::move(mtb.m_particleHypothesis);
}

void ActsTrk::TrackStorageContainer::restoreDecorations() {

  for (auto id : m_backend->getConstStore()->getAuxIDs()) {
    const std::string name = SG::AuxTypeRegistry::instance().getName(id);
    const std::type_info* typeInfo =
        SG::AuxTypeRegistry::instance().getType(id);
    if (staticVariables.count(name) == 1) {
      continue;
    }

    // try making decoration accessor of matching type
    // there is a fixed set of supported types (as there is a fixed set
    // available in MutableMTJ) setters are not needed so replaced by a
    // "nullptr"
    if (*typeInfo == typeid(float)) {
      m_decorations.emplace_back(
          name, static_cast<DecorationAccess::SetterType>(nullptr),
          ActsTrk::detail::constDecorationGetter<xAOD::TrackStorageContainer,
                                                 float>,
          ActsTrk::detail::decorationCopier<xAOD::TrackStorageContainer,
                                            float>);
    } else if (*typeInfo == typeid(double)) {
      m_decorations.emplace_back(
          name, static_cast<DecorationAccess::SetterType>(nullptr),
          ActsTrk::detail::constDecorationGetter<xAOD::TrackStorageContainer,
                                                 double>,
          ActsTrk::detail::decorationCopier<xAOD::TrackStorageContainer,
                                            double>);
    } else if (*typeInfo == typeid(short)) {
      m_decorations.emplace_back(
          name, static_cast<DecorationAccess::SetterType>(nullptr),
          ActsTrk::detail::constDecorationGetter<xAOD::TrackStorageContainer,
                                                 short>,
          ActsTrk::detail::decorationCopier<xAOD::TrackStorageContainer,
                                            short>);
    } else if (*typeInfo == typeid(uint32_t)) {
      m_decorations.emplace_back(
          name, static_cast<DecorationAccess::SetterType>(nullptr),
          ActsTrk::detail::constDecorationGetter<xAOD::TrackStorageContainer,
                                                 uint32_t>,
          ActsTrk::detail::decorationCopier<xAOD::TrackStorageContainer,
                                            uint32_t>);
    } else {
      throw std::runtime_error(
          "TrackStorageContainer Can't resore decoration of  " + name +
          " because it is of an unsupported type");
    }
  }
}

////////////////////////////////////////////////////////////////////
// write api
////////////////////////////////////////////////////////////////////
ActsTrk::MutableTrackStorageContainer::MutableTrackStorageContainer() {

  m_mutableBackend = std::make_unique<xAOD::TrackStorageContainer>();
  m_mutableBackendAux = std::make_unique<xAOD::TrackStorageAuxContainer>();
  m_mutableBackend->setStore(m_mutableBackendAux.get());

  TrackStorageContainer::m_backend = m_mutableBackend.get();
}

ActsTrk::MutableTrackStorageContainer::MutableTrackStorageContainer(
    MutableTrackStorageContainer&& other) noexcept {
  m_mutableBackend = std::move(other.m_mutableBackend);
  m_mutableBackendAux = std::move(other.m_mutableBackendAux);
  m_mutableBackend->setStore(m_mutableBackendAux.get());
  TrackStorageContainer::m_backend = m_mutableBackend.get();
  m_surfaces = std::move(other.m_surfaces);
  m_particleHypothesis = std::move(other.m_particleHypothesis);
  m_decorations = std::move(other.m_decorations);
}

ActsTrk::IndexType ActsTrk::MutableTrackStorageContainer::addTrack_impl() {
  m_mutableBackend->push_back(std::make_unique<xAOD::TrackStorage>());
  m_mutableBackend->back()->resize();
  m_particleHypothesis.resize(m_mutableBackend->size(),
                              Acts::ParticleHypothesis::pion());
  return m_mutableBackend->size() - 1;
}

void ActsTrk::MutableTrackStorageContainer::removeTrack_impl(
    ActsTrk::IndexType itrack) {
  if (itrack >= m_mutableBackend->size()) {
    throw std::out_of_range("removeTrack_impl");
  }
  m_mutableBackend->erase(m_mutableBackend->begin() + itrack);
}

// this in fact may be a copy from other MutableTrackStorageContainer
void ActsTrk::MutableTrackStorageContainer::copyDynamicFrom_impl(
    ActsTrk::IndexType itrack, const ActsTrk::TrackStorageContainer& other,
    ActsTrk::IndexType other_itrack) {
  std::set<std::string> usedDecorations;
  for ( const auto& other_decor: other.m_decorations) {
    if ( staticVariables.count(other_decor.name) == 1)  { continue; }

    other_decor.copier(backend(), itrack, other_decor.name, other.backend(),
                      other_itrack);
    }
}

std::any ActsTrk::MutableTrackStorageContainer::component_impl(
    Acts::HashedString key, ActsTrk::IndexType itrack) {
  std::any result = ::component_impl(*m_mutableBackend, key, itrack);
  if (result.has_value()) {
    return result;
  }
  using namespace Acts::HashedStringLiteral;
  if (key == "particleHypothesis"_hash) {
    return &m_particleHypothesis[itrack];
  }
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      return d.setter(m_mutableBackend.get(), itrack, d.name);
    }
  }
  throw std::runtime_error("TrackStorageContainer no such component " +
                           std::to_string(key));
}

ActsTrk::Parameters ActsTrk::MutableTrackStorageContainer::parameters(
    ActsTrk::IndexType itrack) {
  return m_mutableBackend->at(itrack)->paramsEigen();
}

ActsTrk::Covariance ActsTrk::MutableTrackStorageContainer::covariance(
    ActsTrk::IndexType itrack) {
  return m_mutableBackend->at(itrack)->covParamsEigen();
}

void ActsTrk::MutableTrackStorageContainer::ensureDynamicColumns_impl(
    const MutableTrackStorageContainer& other) {
  for (auto& d : other.m_decorations) {
    m_decorations.push_back(d);
  }
}

void ActsTrk::MutableTrackStorageContainer::ensureDynamicColumns_impl(
    const TrackStorageContainer& other) {
  for (auto& d : other.m_decorations) {
    m_decorations.push_back(d);
  }
}

void ActsTrk::MutableTrackStorageContainer::reserve(ActsTrk::IndexType size) {
  m_mutableBackend->reserve(size);
}

void ActsTrk::MutableTrackStorageContainer::clear() {
  m_mutableBackend->clear();
  m_surfaces.clear();
}

void ActsTrk::MutableTrackStorageContainer::setReferenceSurface_impl(
    ActsTrk::IndexType itrack, std::shared_ptr<const Acts::Surface> surface) {
  m_surfaces.resize(itrack + 1, nullptr);
  m_surfaces[itrack] = std::move(surface);
}
