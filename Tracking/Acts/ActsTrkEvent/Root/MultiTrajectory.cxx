/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsTrkEvent/MultiTrajectory.h"

namespace ActsTrk {
template<>
MultiTrajectory<IsReadWrite>::MultiTrajectory(MultiTrajectory<IsReadWrite>::TrackStateContainerBackendPtr states, 
                                            MultiTrajectory<IsReadWrite>::TrackParametersContainerBackendPtr parameters) 
    : m_trackStates(states),
      m_trackParameters(parameters)
{} 

template<>
MultiTrajectory<IsReadOnly>::MultiTrajectory(MultiTrajectory<IsReadWrite>&& rhs) 
    : m_trackStates(rhs.m_trackStates),
      m_trackParameters(rhs.m_trackParameters)
{
    rhs.m_trackStates = nullptr;
    rhs.m_trackParameters = nullptr;
} 


} // EOF namespace ActsTrk 
