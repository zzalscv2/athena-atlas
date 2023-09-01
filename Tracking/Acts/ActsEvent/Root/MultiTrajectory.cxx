/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/MultiTrajectory.h"

ActsTrk::MutableMultiTrajectory::MutableMultiTrajectory(
    xAOD::TrackStateContainer* states,
    xAOD::TrackParametersContainer* parameters,
    xAOD::TrackJacobianContainer* jacobians,
    xAOD::TrackMeasurementContainer* measurements)
    : m_trackStates(states),
      m_trackParameters(parameters),
      m_trackJacobians(jacobians),
      m_trackMeasurements(measurements) {
  addColumn_impl<IndexType>("calibratedSourceLink");
}

ActsTrk::MutableMultiTrajectory::MutableMultiTrajectory(
    const ActsTrk::MutableMultiTrajectory& other)
    : m_trackStates(other.m_trackStates),
      m_trackParameters(other.m_trackParameters),
      m_trackJacobians(other.m_trackJacobians),
      m_trackMeasurements(other.m_trackMeasurements) {
  if (other.m_trackStatesAux or other.m_trackParametersAux or
      other.m_trackJacobiansAux or other.m_trackMeasurementsAux) {
    throw std::runtime_error(
        "MutableMultiTrajectory that was default constructed can not be "
        "copied");
  }
  for (auto& decoration : other.m_decorations) {
    m_decorations.push_back(decoration);
  }
}

ActsTrk::MutableMultiTrajectory::MutableMultiTrajectory() {
  m_trackStates = new xAOD::TrackStateContainer;
  m_trackStatesAux = new xAOD::TrackStateAuxContainer;
  m_trackStates->setStore(m_trackStatesAux);

  m_trackParameters = new xAOD::TrackParametersContainer;
  m_trackParametersAux = new xAOD::TrackParametersAuxContainer;
  m_trackParameters->setStore(m_trackParametersAux);

  m_trackJacobians = new xAOD::TrackJacobianContainer;
  m_trackJacobiansAux = new xAOD::TrackJacobianAuxContainer;
  m_trackJacobians->setStore(m_trackJacobiansAux);

  m_trackMeasurements = new xAOD::TrackMeasurementContainer;
  m_trackMeasurementsAux = new xAOD::TrackMeasurementAuxContainer;
  m_trackMeasurements->setStore(m_trackMeasurementsAux);
}

ActsTrk::MutableMultiTrajectory::~MutableMultiTrajectory() {
  // this is MTJ owning backends
  if (m_trackStatesAux or m_trackParametersAux or m_trackJacobiansAux or
      m_trackMeasurementsAux) {
    delete m_trackStatesAux;
    delete m_trackParametersAux;
    delete m_trackJacobiansAux;
    delete m_trackMeasurementsAux;
    delete m_trackStates;
    delete m_trackParameters;
    delete m_trackJacobians;
    delete m_trackMeasurements;
  }
}

bool ActsTrk::MutableMultiTrajectory::has_backends() const {
  return m_trackStates != nullptr and m_trackParameters != nullptr and
         m_trackJacobians != nullptr and m_trackMeasurements != nullptr;
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
  m_surfaces[istate] = surface.get();
  m_managedSurfaces.push_back(std::move(surface));
}

const Acts::Surface* ActsTrk::MutableMultiTrajectory::referenceSurface_impl(IndexType istate) const {
  if ( istate >= m_surfaces.size() ) throw std::out_of_range("MutableMultiTrajectory index out of range when accessing reference surface");
  return m_surfaces[istate];
}

void ActsTrk::ConstMultiTrajectory::fillSurfaces(const ActsTrk::MutableMultiTrajectory* mtj) {
  m_surfaces.resize(mtj->surfaces().size());
  m_surfaces.insert(m_surfaces.end(), mtj->surfaces().cbegin(), mtj->surfaces().cend());
}



/////////////////////////////////////////////////////////
// ReadOnly MTJ
ActsTrk::ConstMultiTrajectory::ConstMultiTrajectory(
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
        if ( hasColumn_impl(Acts::hashString(name)) ) { // already known coulmns
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
            std::bind(&ActsTrk::ConstMultiTrajectory::decorationGetter<float>, this,
                      _1, _2));          
        } else if ( *typeInfo == typeid(double) ) {
          m_decorations.emplace_back( name,
            static_cast<ActsTrk::detail::Decoration::SetterType>(nullptr),
            std::bind(&ActsTrk::ConstMultiTrajectory::decorationGetter<double>, this,
                      _1, _2));          

        } else if ( *typeInfo == typeid(short) ) {
          m_decorations.emplace_back( name,
            static_cast<ActsTrk::detail::Decoration::SetterType>(nullptr),
            std::bind(&ActsTrk::ConstMultiTrajectory::decorationGetter<short>, this,
                      _1, _2));  
        } else if ( *typeInfo == typeid(uint32_t) ) {
          m_decorations.emplace_back( name,
            static_cast<ActsTrk::detail::Decoration::SetterType>(nullptr),
            std::bind(&ActsTrk::ConstMultiTrajectory::decorationGetter<uint32_t>, this,
                      _1, _2));          
        }

    }
}

bool ActsTrk::ConstMultiTrajectory::has_impl(Acts::HashedString key,
                                             ActsTrk::IndexType istate) const {
  using namespace Acts::HashedStringLiteral;
  const auto& trackStates = *m_trackStates;
  std::optional<bool> inTrackState =
      ActsTrk::details::has_impl(trackStates, key, istate);
  if (inTrackState.has_value())
    return inTrackState.value();

  return false;
}

const std::any ActsTrk::ConstMultiTrajectory::component_impl(
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
      throw std::runtime_error("ConstMultiTrajectory no such component " + std::to_string(key));
    }
  }
}

bool ActsTrk::ConstMultiTrajectory::hasColumn_impl(
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
  ActsTrk::ConstMultiTrajectory::calibratedSize_impl(ActsTrk::IndexType istate) const {
    const ActsTrk::IndexType i = (*m_trackStates)[istate]->calibrated();
    return (*m_trackMeasurements)[i]->size();
  }


typename Acts::SourceLink
ActsTrk::ConstMultiTrajectory::getUncalibratedSourceLink_impl(ActsTrk::IndexType istate) const {
  auto el = (*m_trackStates)[istate]->uncalibratedMeasurementLink();
  return Acts::SourceLink(el);
}


void ActsTrk::ConstMultiTrajectory::fillSurfaces(const Acts::TrackingGeometry* geo ) {
  if ( not m_surfaces.empty() )
    return;
  m_surfaces.resize(m_trackStates->size(), nullptr);
  for ( IndexType i = 0; i < m_trackStates->size(); i++ ) {
      auto geoID = (*m_trackStates)[i]->geometryId();  
      if ( geoID != 0 ) {
        m_surfaces[i] = geo->findSurface(geoID);
      }
      // TODO here we will have a other part when the surfaces are read from surfaces collection
  }
}

const Acts::Surface* ActsTrk::ConstMultiTrajectory::referenceSurface_impl(IndexType istate) const {
  if ( istate >= m_surfaces.size() ) throw std::out_of_range("ConstMultiTrajectory index out of range when accessing reference surface");
  return m_surfaces[istate];
}
