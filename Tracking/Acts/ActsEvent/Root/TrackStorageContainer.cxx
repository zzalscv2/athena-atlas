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
    const DataLink<xAOD::TrackStorageContainer>& link,
    const DataLink<xAOD::TrackSurfaceContainer>& surfLink)
    : m_trackBackend(link), m_surfBackend(surfLink) {}

const Acts::Surface* ActsTrk::TrackStorageContainer::referenceSurface_impl(
    ActsTrk::IndexType itrack) const {
  if (itrack >= m_surfaces.size())
    throw std::out_of_range(
        "TrackStorageContainer index out of range when accessing reference "
        "surface");
  return m_surfaces[itrack].get();
}

std::size_t ActsTrk::TrackStorageContainer::size_impl() const {
  return m_trackBackend->size();
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
  std::any result = ::component_impl(*m_trackBackend, key, itrack);
  if (result.has_value()) {
    return result;
  }
  using namespace Acts::HashedStringLiteral;
  if (key == "particleHypothesis"_hash) {
    return &m_particleHypothesis[itrack];
  }
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      return d.getter(m_trackBackend.cptr(), itrack, d.name);
    }
  }
  throw std::runtime_error("TrackStorageContainer no such component " +
                           std::to_string(key));
}

ActsTrk::ConstParameters ActsTrk::TrackStorageContainer::parameters(
    ActsTrk::IndexType itrack) const {
  return m_trackBackend->at(itrack)->paramsEigen();
}

ActsTrk::ConstCovariance ActsTrk::TrackStorageContainer::covariance(
    ActsTrk::IndexType itrack) const {
  return m_trackBackend->at(itrack)->covParamsEigen();
}

std::shared_ptr<const Acts::Surface>  ActsTrk::TrackStorageContainer::surface(
    ActsTrk::IndexType itrack) const {
  const ActsGeometryContext& geoContext{};
  return   decodeSurface( m_surfBackend->at(itrack), geoContext );
}

void ActsTrk::TrackStorageContainer::fillFrom(
    ActsTrk::MutableTrackStorageContainer& mtb) {
  m_surfaces = std::move(mtb.m_surfaces);
  m_particleHypothesis = std::move(mtb.m_particleHypothesis);
}



void ActsTrk::TrackStorageContainer::restoreDecorations() {

  for (auto id : m_trackBackend->getConstStore()->getAuxIDs()) {
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
          name,
          ActsTrk::detail::constDecorationGetter<xAOD::TrackStorageContainer,
                                                 float>,
          ActsTrk::detail::decorationCopier<xAOD::TrackStorageContainer,
                                            float>);
    } else if (*typeInfo == typeid(double)) {
      m_decorations.emplace_back(
          name,
          ActsTrk::detail::constDecorationGetter<xAOD::TrackStorageContainer,
                                                 double>,
          ActsTrk::detail::decorationCopier<xAOD::TrackStorageContainer,
                                            double>);
    } else if (*typeInfo == typeid(short)) {
      m_decorations.emplace_back(
          name,
          ActsTrk::detail::constDecorationGetter<xAOD::TrackStorageContainer,
                                                 short>,
          ActsTrk::detail::decorationCopier<xAOD::TrackStorageContainer,
                                            short>);
    } else if (*typeInfo == typeid(uint32_t)) {
      m_decorations.emplace_back(
          name,
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

  m_mutableTrackBackend = std::make_unique<xAOD::TrackStorageContainer>();
  m_mutableTrackBackendAux = std::make_unique<xAOD::TrackStorageAuxContainer>();
  m_mutableTrackBackend->setStore(m_mutableTrackBackendAux.get());

  TrackStorageContainer::m_trackBackend = m_mutableTrackBackend.get();
  m_mutableSurfBackend = std::make_unique<xAOD::TrackSurfaceContainer>();
  m_mutableSurfBackendAux = std::make_unique<xAOD::TrackSurfaceAuxContainer>();
  m_mutableSurfBackend->setStore(m_mutableSurfBackendAux.get());

  TrackStorageContainer::m_surfBackend = m_mutableSurfBackend.get();  
}

ActsTrk::MutableTrackStorageContainer::MutableTrackStorageContainer(
    MutableTrackStorageContainer&& other) noexcept {
  m_mutableTrackBackend = std::move(other.m_mutableTrackBackend);
  m_mutableTrackBackendAux = std::move(other.m_mutableTrackBackendAux);
  m_mutableTrackBackend->setStore(m_mutableTrackBackendAux.get());
  TrackStorageContainer::m_trackBackend = m_mutableTrackBackend.get();

  m_mutableSurfBackend = std::move(other.m_mutableSurfBackend);
  m_mutableSurfBackendAux = std::move(other.m_mutableSurfBackendAux);
  m_mutableSurfBackend->setStore(m_mutableSurfBackendAux.get());
  TrackStorageContainer::m_surfBackend = m_mutableSurfBackend.get();

  m_surfaces = std::move(other.m_surfaces);
  m_particleHypothesis = std::move(other.m_particleHypothesis);
  m_decorations = std::move(other.m_decorations);
}

ActsTrk::IndexType ActsTrk::MutableTrackStorageContainer::addTrack_impl() {
  m_mutableTrackBackend->push_back(std::make_unique<xAOD::TrackStorage>());
  m_mutableTrackBackend->back()->resize();
  m_particleHypothesis.resize(m_mutableTrackBackend->size(),
                              Acts::ParticleHypothesis::pion());
  return m_mutableTrackBackend->size() - 1;
}

void ActsTrk::MutableTrackStorageContainer::removeTrack_impl(
    ActsTrk::IndexType itrack) {
  if (itrack >= m_mutableTrackBackend->size()) {
    throw std::out_of_range("removeTrack_impl");
  }
  m_mutableTrackBackend->erase(m_mutableTrackBackend->begin() + itrack);
}

// Add and remove surface
ActsTrk::IndexType ActsTrk::MutableTrackStorageContainer::addSurface_impl() {
  m_mutableSurfBackend->push_back(std::make_unique<xAOD::TrackSurface>());
  return m_mutableSurfBackend->size() - 1;
}

void ActsTrk::MutableTrackStorageContainer::removeSurface_impl(
    ActsTrk::IndexType isurf) {
  if (isurf >= m_mutableSurfBackend->size()) {
    throw std::out_of_range("removeSurface_impl");
  }
  m_mutableSurfBackend->erase(m_mutableSurfBackend->begin() + isurf);
}



// this in fact may be a copy from other MutableTrackStorageContainer
void ActsTrk::MutableTrackStorageContainer::copyDynamicFrom_impl(
    ActsTrk::IndexType itrack, const ActsTrk::TrackStorageContainer& other,
    ActsTrk::IndexType other_itrack) {
  std::set<std::string> usedDecorations;
  for ( const auto& other_decor: other.m_decorations) {
    if ( staticVariables.count(other_decor.name) == 1)  { continue; }

    other_decor.copier(trackBackend(), itrack, other_decor.name, other.trackBackend(),
                      other_itrack);
    }
}

std::any ActsTrk::MutableTrackStorageContainer::component_impl(
    Acts::HashedString key, ActsTrk::IndexType itrack) {
  std::any result = ::component_impl(*m_mutableTrackBackend, key, itrack);
  if (result.has_value()) {
    return result;
  }
  using namespace Acts::HashedStringLiteral;
  if (key == "particleHypothesis"_hash) {
    return &m_particleHypothesis[itrack];
  }
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      return d.setter(m_mutableTrackBackend.get(), itrack, d.name);
    }
  }
  throw std::runtime_error("TrackStorageContainer no such component " +
                           std::to_string(key));
}

ActsTrk::Parameters ActsTrk::MutableTrackStorageContainer::parameters(
    ActsTrk::IndexType itrack) {
  return m_mutableTrackBackend->at(itrack)->paramsEigen();
}

ActsTrk::Covariance ActsTrk::MutableTrackStorageContainer::covariance(
    ActsTrk::IndexType itrack) {
  return m_mutableTrackBackend->at(itrack)->covParamsEigen();
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
  m_mutableTrackBackend->reserve(size);
}

void ActsTrk::MutableTrackStorageContainer::clear() {
  m_mutableTrackBackend->clear();
  m_surfaces.clear();
}

void ActsTrk::MutableTrackStorageContainer::setReferenceSurface_impl(
    ActsTrk::IndexType itrack, std::shared_ptr<const Acts::Surface> surface) {
  m_surfaces.resize(itrack + 1, nullptr);
  m_surfaces[itrack] = std::move(surface);
}
