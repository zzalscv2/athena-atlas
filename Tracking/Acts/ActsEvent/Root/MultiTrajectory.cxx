/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/SurfaceEncoding.h"

constexpr uint64_t InvalidGeoID = std::numeric_limits<uint64_t>::max();


ActsTrk::MutableMultiTrajectory::MutableMultiTrajectory() {
  m_trackStates = std::make_unique<xAOD::TrackStateContainer>();
  m_trackStatesAux = std::make_unique<xAOD::TrackStateAuxContainer>();
  m_trackStates->setStore(m_trackStatesAux.get());

  m_trackParameters = std::make_unique<xAOD::TrackParametersContainer>();
  m_trackParametersAux = std::make_unique<xAOD::TrackParametersAuxContainer>();
  m_trackParameters->setStore(m_trackParametersAux.get());

  m_trackJacobians = std::make_unique<xAOD::TrackJacobianContainer>();
  m_trackJacobiansAux = std::make_unique<xAOD::TrackJacobianAuxContainer>();
  m_trackJacobians->setStore(m_trackJacobiansAux.get());

  m_trackMeasurements = std::make_unique<xAOD::TrackMeasurementContainer>();
  m_trackMeasurementsAux = std::make_unique<xAOD::TrackMeasurementAuxContainer>();
  m_trackMeasurements->setStore(m_trackMeasurementsAux.get());

  m_surfacesBackend = std::make_unique<xAOD::TrackSurfaceContainer>();
  m_surfacesBackendAux = std::make_unique<xAOD::TrackSurfaceAuxContainer>();
  m_surfacesBackend->setStore(m_surfacesBackendAux.get());
  addColumn_impl<IndexType>("calibratedSourceLink");
}

bool ActsTrk::MutableMultiTrajectory::has_backends() const {
  return m_trackStates != nullptr and m_trackParameters != nullptr and
         m_trackJacobians != nullptr and m_trackMeasurements != nullptr
         and m_surfacesBackend != nullptr;
}


ActsTrk::IndexType ActsTrk::MutableMultiTrajectory::addTrackState_impl(
    Acts::TrackStatePropMask mask,
    ActsTrk::IndexType previous) {
  using namespace Acts::HashedStringLiteral;

  assert(m_trackStates && "Missing Track States backend");

  auto state = new xAOD::TrackState_v1();
  m_trackStates->push_back(state);

  // set kInvalid
  using Acts::MultiTrajectoryTraits::kInvalid;

  if (previous >= kInvalid - 1)
    previous = kInvalid;  // fix needed in Acts::MTJ
  m_trackStates->back()->setPrevious(previous);
  using namespace Acts;

  auto addParam = [this]() -> ActsTrk::IndexType {
    trackParameters().push_back(new xAOD::TrackParameters_v1());
    trackParameters().back()->resize();
    return trackParameters().size() - 1;
  };

  auto addJacobian = [this]() -> ActsTrk::IndexType {
    trackJacobians().push_back(new xAOD::TrackJacobian_v1());
    trackJacobians().back()->resize();
    return trackJacobians().size() - 1;
  };

  auto addMeasurement = [this]() -> ActsTrk::IndexType {
    trackMeasurements().push_back(new xAOD::TrackMeasurement_v1());
    // trackMeasurements are resized by allocateCalibrated()
    return trackMeasurements().size() - 1;
  };

  state->setPredicted(kInvalid);
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Predicted)) {
    state->setPredicted(addParam());
  }

  state->setFiltered(kInvalid);
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Filtered)) {
    state->setFiltered(addParam());
  }

  state->setSmoothed(kInvalid);
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Smoothed)) {
    state->setSmoothed(addParam());
  }

  state->setCalibrated(kInvalid);
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Calibrated)) {
    xAOD::TrackMeasurement* meas = new xAOD::TrackMeasurement();
    m_trackMeasurements->push_back(meas);
    state->setCalibrated(m_trackMeasurements->size() - 1);
  }
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Calibrated)) {
    state->setCalibrated(addMeasurement());
    // TODO: in Acts there is m_projections collection
    // https://github.com/acts-project/acts/blob/main/Core/src/EventData/VectorMultiTrajectory.cpp#L83
  }

  state->setJacobian(kInvalid);
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Jacobian)) {
    state->setJacobian(addJacobian());
  }

  state->setGeometryId(InvalidGeoID); // surface is invalid until set

  return m_trackStates->size() - 1;
}



void ActsTrk::MutableMultiTrajectory::shareFrom_impl(
    ActsTrk::IndexType iself,
    ActsTrk::IndexType iother,
    Acts::TrackStatePropMask shareSource,
    Acts::TrackStatePropMask shareTarget) {
  auto self = (*m_trackStates)[iself];
  auto other = (*m_trackStates)[iother];

  assert(ACTS_CHECK_BIT(this->getTrackState(iother).getMask(), shareSource) &&
         "Source has incompatible allocation");

  using PM = Acts::TrackStatePropMask;
  // set kInvalid
  using Acts::MultiTrajectoryTraits::kInvalid;

  ActsTrk::IndexType sourceIndex{kInvalid};
  switch (shareSource) {
    case PM::Predicted:
      sourceIndex = other->predicted();
      break;
    case PM::Filtered:
      sourceIndex = other->filtered();
      break;
    case PM::Smoothed:
      sourceIndex = other->smoothed();
      break;
    case PM::Jacobian:
      sourceIndex = other->jacobian();
      break;
    default:
      throw std::domain_error{"MutableMultiTrajectory Unable to share this component"};
  }

  assert(sourceIndex != kInvalid);

  switch (shareTarget) {
    case PM::Predicted:
      assert(shareSource != PM::Jacobian);
      self->setPredicted(sourceIndex);
      break;
    case PM::Filtered:
      assert(shareSource != PM::Jacobian);
      self->setFiltered(sourceIndex);
      break;
    case PM::Smoothed:
      assert(shareSource != PM::Jacobian);
      self->setSmoothed(sourceIndex);
      break;
    case PM::Jacobian:
      assert(shareSource == PM::Jacobian);
      self->setJacobian(sourceIndex);
      break;
    default:
      throw std::domain_error{"MutableMultiTrajectory Unable to share this component"};
  }
}


void ActsTrk::MutableMultiTrajectory::unset_impl(
    Acts::TrackStatePropMask target,
    ActsTrk::IndexType istate) {

  using PM = Acts::TrackStatePropMask;
  // set kInvalid
  using Acts::MultiTrajectoryTraits::kInvalid;

  switch (target) {
    case PM::Predicted:
      (*m_trackStates)[istate]->setPredicted(kInvalid);
      break;
    case PM::Filtered:
      (*m_trackStates)[istate]->setFiltered(kInvalid);
      break;
    case PM::Smoothed:
      (*m_trackStates)[istate]->setSmoothed(kInvalid);
      break;
    case PM::Jacobian:
      (*m_trackStates)[istate]->setJacobian(kInvalid);
      break;
    case PM::Calibrated:
      (*m_trackStates)[istate]->setCalibrated(kInvalid);
      // TODO here m_measOffset[istate] and m_measCovOffset[istate] should be
      // set to kInvalid

      break;
    default:
      throw std::domain_error{"MutableMultiTrajectory Unable to unset this component"};
  }
}

std::any ActsTrk::MutableMultiTrajectory::component_impl(
    Acts::HashedString key, ActsTrk::IndexType istate) {
  using namespace Acts::HashedStringLiteral;
  assert(istate < m_trackStates->size() &&
         "Attempt to reach beyond the Track States container size");

  switch (key) {
    case "previous"_hash:
      return (*m_trackStates)[istate]->previousPtr();
    case "chi2"_hash:
      return (*m_trackStates)[istate]->chi2Ptr();
    case "pathLength"_hash:
      return (*m_trackStates)[istate]->pathLengthPtr();
    case "predicted"_hash:
      return (*m_trackStates)[istate]->predictedPtr();
    case "filtered"_hash:
      return (*m_trackStates)[istate]->filteredPtr();
    case "smoothed"_hash:
      return (*m_trackStates)[istate]->smoothedPtr();
    case "projector"_hash:
      return trackMeasurements()
          .at((*m_trackStates)[istate]->calibrated())
          ->projectorPtr();
    case "measdim"_hash:
      return (*m_trackStates)[istate]->measDimPtr();
    default: {
      for (auto& d : m_decorations) {
        if (d.hash == key) {
          return d.getter(istate, d.name);
        }
      }
      throw std::runtime_error("MutableMultiTrajectory no such component " + std::to_string(key));
    }
  }
}


const std::any ActsTrk::MutableMultiTrajectory::component_impl(
    Acts::HashedString key,
    ActsTrk::IndexType istate) const {
  using namespace Acts::HashedStringLiteral;
  assert(istate < m_trackStates->size() &&
         "Attempt to reach beyond the Track States container size");
  const auto& trackStates = *m_trackStates;
  switch (key) {
    case "previous"_hash:
      return trackStates[istate]->previousPtr();
    case "chi2"_hash:
      return trackStates[istate]->chi2Ptr();
    case "pathLength"_hash:
      return trackStates[istate]->pathLengthPtr();
    case "predicted"_hash:
      return trackStates[istate]->predictedPtr();
    case "filtered"_hash:
      return trackStates[istate]->filteredPtr();
    case "smoothed"_hash:
      return trackStates[istate]->smoothedPtr();
    case "jacobian"_hash:
      return trackStates[istate]->jacobianPtr();
    case "projector"_hash:
      return trackMeasurements()
          .at(trackStates[istate]->calibrated())
          ->projectorPtr();
    case "calibrated"_hash: {
      return trackStates[istate]->calibratedPtr();
    }
    case "calibratedCov"_hash: {
      return trackStates[istate]->calibratedPtr();
    }
    case "measdim"_hash:
      return trackStates[istate]->measDimPtr();
    default: {
      for (auto& d : m_decorations) {
        if (d.hash == key) {
          return d.getter(istate, d.name);
        }
      }
      throw std::runtime_error("MutableMultiTrajectory no such component " + std::to_string(key));
    }
  }
}

bool ActsTrk::MutableMultiTrajectory::has_impl(
    Acts::HashedString key, ActsTrk::IndexType istate) const {
  const auto& trackStates = *(m_trackStates.get());
  std::optional<bool> inTrackState =
      ActsTrk::details::has_impl(trackStates, key, istate);
  if (inTrackState.has_value())
    return inTrackState.value();
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      return true;
    }
  }

  return false;
}

typename Acts::SourceLink
ActsTrk::MutableMultiTrajectory::getUncalibratedSourceLink_impl(
    ActsTrk::IndexType istate) const {
  auto el = trackStates()[istate]->uncalibratedMeasurementLink();
  return Acts::SourceLink(el);
}
Acts::SourceLink
ActsTrk::MutableMultiTrajectory::getUncalibratedSourceLink_impl(
    ActsTrk::IndexType istate) {
  auto el = trackStates()[istate]->uncalibratedMeasurementLink();
  return Acts::SourceLink(el);
}

void ActsTrk::MutableMultiTrajectory::setReferenceSurface_impl(IndexType istate,
                                std::shared_ptr<const Acts::Surface> surface) {
  if ( istate >= m_surfaces.size() )
    m_surfaces.resize(istate+1, nullptr);

  trackStates()[istate]->setGeometryId(surface->geometryId().value()); // for free surface this will be 0

  if (surface->geometryId().value() == 0) { // free surface, needs recording of properties
    m_surfaces[istate] = std::move(surface); // and memory management
    auto surfaceBackend = new xAOD::TrackSurface();
    m_surfacesBackend->push_back(surfaceBackend);
    encodeSurface(surfaceBackend, surface.get(), m_geoContext); // TODO
    auto el = ElementLink<xAOD::TrackSurfaceContainer>(*m_surfacesBackend, m_surfacesBackend->size()-1);
    trackStates()[istate]->setSurfaceLink(el);

  } else {
    m_surfaces[istate] = surface.get(); // no memory management, bare pointer
  }
  // store surface representation in
}

namespace {
  const Acts::Surface* toSurfacePtr(const ActsTrk::StoredSurface& surfaceVariant) {
    if ( std::holds_alternative<const Acts::Surface*>(surfaceVariant)) 
      return std::get<const Acts::Surface*>(surfaceVariant);
    return std::get<std::shared_ptr<const Acts::Surface>>(surfaceVariant).get();
  }
}

const Acts::Surface* ActsTrk::MutableMultiTrajectory::referenceSurface_impl(IndexType istate) const {
  if ( istate >= m_surfaces.size() ) throw std::out_of_range("MutableMultiTrajectory index out of range when accessing reference surface");
  return toSurfacePtr(m_surfaces[istate]);
}



/////////////////////////////////////////////////////////
// ReadOnly MTJ
ActsTrk::MultiTrajectory::MultiTrajectory(
    DataLink<xAOD::TrackStateContainer> trackStates,
    DataLink<xAOD::TrackParametersContainer> trackParameters,
    DataLink<xAOD::TrackJacobianContainer> trackJacobians,
    DataLink<xAOD::TrackMeasurementContainer> trackMeasurements)
    : m_trackStates(trackStates),
      m_trackParameters(trackParameters),
      m_trackJacobians(trackJacobians),
      m_trackMeasurements(trackMeasurements) {
      for ( auto id : m_trackStates->getConstStore()->getAuxIDs() ) {

        const std::string name = SG::AuxTypeRegistry::instance().getName(id);
        if ( hasColumn_impl(Acts::hashString(name)) ) { // already known columns
          continue;
        }
        const std::type_info* typeInfo = SG::AuxTypeRegistry::instance().getType(id);

        using std::placeholders::_1;
        using std::placeholders::_2;
        // try making decoration accessor of matching type
        // there is a fixed set of supported types (as there is a fixed set available in MutableMTJ)
        // setters are not needed so replaced by a "nullptr"
        if ( *typeInfo == typeid(float) ) {
          m_decorations.emplace_back( name,
            static_cast<ActsTrk::detail::Decoration::SetterType>(nullptr),
            std::bind(&ActsTrk::MultiTrajectory::decorationGetter<float>, this,
                      _1, _2));
        } else if ( *typeInfo == typeid(double) ) {
          m_decorations.emplace_back( name,
            static_cast<ActsTrk::detail::Decoration::SetterType>(nullptr),
            std::bind(&ActsTrk::MultiTrajectory::decorationGetter<double>, this,
                      _1, _2));

        } else if ( *typeInfo == typeid(short) ) {
          m_decorations.emplace_back( name,
            static_cast<ActsTrk::detail::Decoration::SetterType>(nullptr),
            std::bind(&ActsTrk::MultiTrajectory::decorationGetter<short>, this,
                      _1, _2));
        } else if ( *typeInfo == typeid(uint32_t) ) {
          m_decorations.emplace_back( name,
            static_cast<ActsTrk::detail::Decoration::SetterType>(nullptr),
            std::bind(&ActsTrk::MultiTrajectory::decorationGetter<uint32_t>, this,
                      _1, _2));
        }

    }
}

bool ActsTrk::MultiTrajectory::has_impl(Acts::HashedString key,
                                             ActsTrk::IndexType istate) const {
  using namespace Acts::HashedStringLiteral;
  const auto& trackStates = *m_trackStates;
  std::optional<bool> inTrackState =
      ActsTrk::details::has_impl(trackStates, key, istate);
  if (inTrackState.has_value())
    return inTrackState.value();

  return false;
}

const std::any ActsTrk::MultiTrajectory::component_impl(
    Acts::HashedString key, ActsTrk::IndexType istate) const {
  using namespace Acts::HashedStringLiteral;
  const auto& trackStates = *m_trackStates;
  switch (key) {
    case "previous"_hash:
      return trackStates[istate]->previousPtr();
    case "chi2"_hash:
      return trackStates[istate]->chi2Ptr();
    case "pathLength"_hash:
      return trackStates[istate]->pathLengthPtr();
    case "predicted"_hash:
      return trackStates[istate]->predictedPtr();
    case "filtered"_hash:
      return trackStates[istate]->filteredPtr();
    case "smoothed"_hash:
      return trackStates[istate]->smoothedPtr();
    case "jacobian"_hash:
      return trackStates[istate]->jacobianPtr();
    case "projector"_hash: {
      const auto& trackMeasurements = *m_trackMeasurements;
      return trackMeasurements.at(trackStates[istate]->calibrated())
          ->projectorPtr();
    }
    case "calibrated"_hash: {
      return trackStates[istate]->calibratedPtr();
    }
    case "calibratedCov"_hash: {
      return trackStates[istate]->calibratedPtr();
    }
    case "measdim"_hash:
      return trackStates[istate]->measDimPtr();
    default: {
      for (auto& d : m_decorations) {
        if (d.hash == key) {
          return  d.getter(istate, d.name);
        }
      }
      throw std::runtime_error("MultiTrajectory no such component " + std::to_string(key));
    }
  }
}

bool ActsTrk::MultiTrajectory::hasColumn_impl(
    Acts::HashedString key) const {
  using namespace Acts::HashedStringLiteral;
  switch (key) {
    case "previous"_hash:
    case "chi2"_hash:
    case "pathLength"_hash:
    case "predicted"_hash:
    case "filtered"_hash:
    case "smoothed"_hash:
    case "jacobian"_hash:
    case "projector"_hash:
    case "uncalibratedSourceLink"_hash:
    case "calibrated"_hash:
    case "calibratedCov"_hash:
    case "measdim"_hash:
      return true;
  }
  for (auto& d : m_decorations) {
      if (d.hash == key) {
        return true;
      }
    }
  return false;
}

  ActsTrk::IndexType
  ActsTrk::MultiTrajectory::calibratedSize_impl(ActsTrk::IndexType istate) const {
    const ActsTrk::IndexType i = (*m_trackStates)[istate]->calibrated();
    return (*m_trackMeasurements)[i]->size();
  }


typename Acts::SourceLink
ActsTrk::MultiTrajectory::getUncalibratedSourceLink_impl(ActsTrk::IndexType istate) const {
  auto el = (*m_trackStates)[istate]->uncalibratedMeasurementLink();
  return Acts::SourceLink(el);
}

void ActsTrk::MultiTrajectory::fillSurfaces(const ActsTrk::MutableMultiTrajectory* mtj) {
  m_surfaces = std::move(mtj->m_surfaces);
}


void ActsTrk::MultiTrajectory::fillSurfaces(const Acts::TrackingGeometry* geo, const ActsGeometryContext& geoContext ) {
  if ( not m_surfaces.empty() )
    return;
  m_surfaces.resize(m_trackStates->size(), nullptr);
  for ( IndexType i = 0; i < m_trackStates->size(); i++ ) {
      auto geoID = (*m_trackStates)[i]->geometryId();
      if ( geoID ==  InvalidGeoID ) {
        m_surfaces[i] = nullptr;
        continue;
      }
      if ( geoID != 0 ) {
        m_surfaces[i] = geo->findSurface(geoID);
      } else {
        ElementLink<xAOD::TrackSurfaceContainer> backendLink = (*m_trackStates)[i]->surfaceLink();
        std::shared_ptr<const Acts::Surface> surface = decodeSurface( *backendLink, geoContext);
        m_surfaces[i] = surface; // TODO

      }
  }
}


const Acts::Surface* ActsTrk::MultiTrajectory::referenceSurface_impl(IndexType istate) const {
  if ( istate >= m_surfaces.size() ) throw std::out_of_range("MultiTrajectory index out of range when accessing reference surface");
  return toSurfacePtr(m_surfaces[istate]);
}
