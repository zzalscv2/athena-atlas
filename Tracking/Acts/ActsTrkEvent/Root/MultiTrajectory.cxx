/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsTrkEvent/MultiTrajectory.h"

namespace ActsTrk {

template <>
ActsTrk::MultiTrajectory<ActsTrk::IsReadOnly>::MultiTrajectory() =
    delete;  // read-only owning MTJ does not make sense

template <>
ActsTrk::MultiTrajectory<ActsTrk::IsReadWrite>::MultiTrajectory() {
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

}  // namespace ActsTrk
