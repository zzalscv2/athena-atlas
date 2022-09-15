/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsTrkEvent/MultiTrajectory.h"

namespace ActsTrk {
template<>
MultiTrajectory<IsReadWrite>::MultiTrajectory(MultiTrajectory<IsReadWrite>::TrackStateContainerBackendPtr states, 
                                            MultiTrajectory<IsReadWrite>::TrackParametersContainerBackendPtr parameters,
                                            MultiTrajectory<IsReadWrite>::TrackJacobianContainerBackendPtr jacobians, 
                                            MultiTrajectory<IsReadWrite>::TrackMeasurementsContainerBackendPtr measurements )
    : m_trackStates(states),
      m_trackParameters(parameters),
      m_jacobians(jacobians),
      m_measurements(measurements)
{} 

template<>
MultiTrajectory<IsReadOnly>::MultiTrajectory(MultiTrajectory<IsReadWrite>&& rhs) 
    : m_trackStates(rhs.m_trackStates),
      m_trackParameters(rhs.m_trackParameters), 
      m_jacobians(rhs.m_jacobians),
      m_measurements(rhs.m_measurements)
{
    rhs.m_trackStates = nullptr;
    rhs.m_trackParameters = nullptr;
    rhs.m_jacobians = nullptr;
    rhs.m_measurements = nullptr;
} 


} // EOF namespace ActsTrk 
