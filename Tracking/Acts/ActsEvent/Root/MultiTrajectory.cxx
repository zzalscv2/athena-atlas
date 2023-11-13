/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/SurfaceEncoding.h"
#include "xAODTracking/TrackState.h"
#include "xAODTracking/TrackParameters.h"
#include "xAODTracking/TrackJacobian.h"
#include "xAODTracking/TrackMeasurement.h"

constexpr uint64_t InvalidGeoID = std::numeric_limits<uint64_t>::max();


ActsTrk::MutableMultiTrajectory::MutableMultiTrajectory() {
  INSPECTCALL("c-tor " << this)

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

  addColumn_impl<IndexType>("next");

}

ActsTrk::MutableMultiTrajectory::MutableMultiTrajectory(const ActsTrk::MutableMultiTrajectory& other) : MutableMultiTrajectory() {
  INSPECTCALL("copy c-tor " << this <<  " src " << &other << " " << other.size())
  for ( auto t: *other.m_trackStates.get()) {
    m_trackStates->push_back(new xAOD::TrackState());
    *m_trackStates->back() = *t;
  }

  for ( auto t: *other.m_trackParameters.get()){
    m_trackParameters->push_back(new xAOD::TrackParameters(*t));
    *m_trackParameters->back() = *t;

  }
  for ( auto t: *other.m_trackJacobians.get()){
    m_trackJacobians->push_back(new xAOD::TrackJacobian(*t));
    *m_trackJacobians->back() = *t;

  }
  for ( auto t: *other.m_trackMeasurements.get()){
    m_trackMeasurements->push_back(new xAOD::TrackMeasurement(*t));
    *m_trackMeasurements->back() = *t;
  }
  m_decorations = other.m_decorations;
  m_calibratedSourceLinks = other.m_calibratedSourceLinks;
  m_uncalibratedSourceLinks = other.m_uncalibratedSourceLinks;

  m_surfaces = other.m_surfaces;
  m_geoContext = other.m_geoContext;
  INSPECTCALL("copy c-tor  done")
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
  INSPECTCALL( this << " " <<  mask << " " << m_trackStates->size() << " " << previous);
  assert(m_trackStates && "Missing Track States backend");

  auto state = new xAOD::TrackState_v1();
  m_trackStates->push_back(state);
  m_surfaces.push_back(nullptr);

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

  state->setJacobian(kInvalid);
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Jacobian)) {
    state->setJacobian(addJacobian());
  }

  m_uncalibratedSourceLinks.emplace_back(std::nullopt);

  state->setCalibrated(kInvalid);
  state->setMeasDim(0);
  
  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Calibrated)) {
    state->setCalibrated(addMeasurement());
    m_calibratedSourceLinks.emplace_back(std::nullopt);
    state->setMeasDim(trackMeasurements().back()->size()); // default measurement size
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
    case "typeFlags"_hash:
      return (*m_trackStates)[istate]->typeFlagsPtr();
    default: {
      for (auto& d : m_decorations) {
        if (d.hash == key) {
          return d.setter(m_trackStates.get(), istate, d.name);
        }
      }
      throw std::runtime_error("MutableMultiTrajectory::component_impl no such component " + std::to_string(key));
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
    case "typeFlags"_hash:
      return trackStates[istate]->typeFlagsPtr();
    default: {
      for (auto& d : m_decorations) {
        if (d.hash == key) {
          INSPECTCALL("getting dymaic variable " << d.name << " " << istate);
          return d.getter(m_trackStates.get(), istate, d.name);
        }
      }
      throw std::runtime_error("MutableMultiTrajectory::component_impl const no such component " + std::to_string(key));
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

  // TODO remove once EL based source links are in use only
  using namespace Acts::HashedStringLiteral;
  if (key == "uncalibratedSourceLink"_hash){
    INSPECTCALL(key << " " << istate << " uncalibratedSourceLink")
    return m_uncalibratedSourceLinks[istate].has_value();
  }

  for (auto& d : m_decorations) {
    if (d.hash == key) {
      INSPECTCALL(key << " " << istate << " d.name")
      return true;
    }
  }

  return false;
}

void ActsTrk::MutableMultiTrajectory::clear_impl() {
  INSPECTCALL(this);
  m_trackStates->clear();
  m_trackParameters->clear();
  m_trackJacobians->clear();
  m_trackMeasurements->clear();
  m_surfacesBackend->clear();
  m_surfaces.clear();
  m_calibratedSourceLinks.clear();
  m_uncalibratedSourceLinks.clear();
}


void ActsTrk::MutableMultiTrajectory::allocateCalibrated_impl(ActsTrk::IndexType istate, std::size_t measdim) {
  // resize the calibrated measurement to the size measdim
  auto& trackStates = *m_trackStates;

  INSPECTCALL(trackStates[istate]->calibrated() << " " << trackMeasurements().size());
  trackStates[istate]->setMeasDim(measdim);
  trackMeasurements().at(trackStates[istate]->calibrated())->resize(measdim);
}


ActsTrk::IndexType ActsTrk::MutableMultiTrajectory::calibratedSize_impl(ActsTrk::IndexType istate) const {
  // Retrieve the calibrated measurement size
  INSPECTCALL(istate << " " << trackMeasurements().size());
  return trackStates().at(istate)->measDim();
}


void ActsTrk::MutableMultiTrajectory::setUncalibratedSourceLink_impl(ActsTrk::IndexType istate,
                                    const Acts::SourceLink& sourceLink) {
  INSPECTCALL( istate );

  // TODO restore this once tracking code uses source links with EL                                        
  // auto el =
  //     sourceLink.get<ElementLink<xAOD::UncalibratedMeasurementContainer>>();
  // trackStates()[istate]->setUncalibratedMeasurementLink(el);
  m_uncalibratedSourceLinks[istate] = std::move(sourceLink);
}


typename Acts::SourceLink
ActsTrk::MutableMultiTrajectory::getUncalibratedSourceLink_impl(
    ActsTrk::IndexType istate) const {
  // TODO see setUncalibratedSourceLink_impl
  // auto el = trackStates()[istate]->uncalibratedMeasurementLink();
  // return Acts::SourceLink(el);
  return m_uncalibratedSourceLinks[istate].value();
}
Acts::SourceLink
ActsTrk::MutableMultiTrajectory::getUncalibratedSourceLink_impl(
    ActsTrk::IndexType istate) {
  // TODO see setUncalibratedSourceLink_impl
  // auto el = trackStates()[istate]->uncalibratedMeasurementLink();
  // return Acts::SourceLink(el);
  return m_uncalibratedSourceLinks[istate].value();

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
  if ( istate >= m_surfaces.size() ) throw std::out_of_range("MutableMultiTrajectory index " + std::to_string(istate) + " out of range " + std::to_string(m_surfaces.size()) + " when accessing reference surface");
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
      INSPECTCALL("ctor " << this << " " << m_trackStates->size());

      for ( auto id : m_trackStates->getConstStore()->getAuxIDs() ) {

        const std::string name = SG::AuxTypeRegistry::instance().getName(id);
        if ( hasColumn_impl(Acts::hashString(name)) ) { // already known columns
          continue;
        }
        const std::type_info* typeInfo = SG::AuxTypeRegistry::instance().getType(id);

        // try making decoration accessor of matching type
        // there is a fixed set of supported types (as there is a fixed set available in MutableMTJ)
        // setters are not needed so replaced by a "nullptr"
        if ( *typeInfo == typeid(float) ) {
          m_decorations.emplace_back( name,
            static_cast<DecorationAccess::SetterType>(nullptr),
            ActsTrk::detail::decorationGetter<xAOD::TrackStateContainer, float>);
        } else if ( *typeInfo == typeid(double) ) {
          m_decorations.emplace_back( name,
            static_cast<DecorationAccess::SetterType>(nullptr),
            ActsTrk::detail::decorationGetter<xAOD::TrackStateContainer, double>);

        } else if ( *typeInfo == typeid(short) ) {
          m_decorations.emplace_back( name,
            static_cast<DecorationAccess::SetterType>(nullptr),
            ActsTrk::detail::decorationGetter<xAOD::TrackStateContainer, short>);

        } else if ( *typeInfo == typeid(uint32_t) ) {
          m_decorations.emplace_back( name,
            static_cast<DecorationAccess::SetterType>(nullptr),
            ActsTrk::detail::decorationGetter<xAOD::TrackStateContainer, short>);

        }

    }
}

bool ActsTrk::MultiTrajectory::has_impl(Acts::HashedString key,
                                             ActsTrk::IndexType istate) const {
  const auto& trackStates = *m_trackStates;
  std::optional<bool> inTrackState =
      ActsTrk::details::has_impl(trackStates, key, istate);
  if (inTrackState.has_value())
    return inTrackState.value();
  // TODO remove once EL based source links are in use only
  using namespace Acts::HashedStringLiteral;
  if (key == "uncalibratedSourceLink"_hash)
      return m_uncalibratedSourceLinks[istate].has_value();


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
    case "typeFlags"_hash:
      return trackStates[istate]->typeFlagsPtr();
    default: {
      for (auto& d : m_decorations) {
        if (d.hash == key) {
          return  d.getter(m_trackStates.cptr(), istate, d.name);
        }
      }
      throw std::runtime_error("MultiTrajectory::component_impl no such component " + std::to_string(key));
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
    case "typeFlags"_hash:
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
    return m_trackStates->at(istate)->measDim();
    }


typename Acts::SourceLink
ActsTrk::MultiTrajectory::getUncalibratedSourceLink_impl(ActsTrk::IndexType istate) const {
  // TODO restore this implementation, see other methods like this
  // auto el = (*m_trackStates)[istate]->uncalibratedMeasurementLink();
  // return Acts::SourceLink(el);
  return m_uncalibratedSourceLinks[istate].value();
}

void ActsTrk::MultiTrajectory::moveSurfaces(const ActsTrk::MutableMultiTrajectory* mtj) {
  m_surfaces = std::move(mtj->m_surfaces);
  INSPECTCALL(this << " " << m_surfaces.size());
}

// TODO remove this implementation once tracking uses only sourceLinks with EL
void ActsTrk::MultiTrajectory::moveLinks(const ActsTrk::MutableMultiTrajectory* mtj) {
  m_calibratedSourceLinks = std::move(mtj->m_calibratedSourceLinks);
  m_uncalibratedSourceLinks = std::move(mtj->m_uncalibratedSourceLinks);
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
  INSPECTCALL( this <<  " " << istate << " " << m_trackStates->size() << " " << m_surfaces.size());
  if ( istate >= m_surfaces.size() ) throw std::out_of_range("MultiTrajectory index " + std::to_string(istate) + " out of range " + std::to_string(m_surfaces.size()) + " when accessing reference surface");
  return toSurfacePtr(m_surfaces[istate]);
}
