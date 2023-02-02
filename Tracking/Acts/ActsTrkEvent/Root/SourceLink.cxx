/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration 
*/

#include "ActsTrkEvent/SourceLink.h"

namespace ActsTrk {

  SourceLink::SourceLink( xAOD::UncalibratedMeasurement* uncalibrated )
    : m_uncalibrated(uncalibrated)
  {}

  const xAOD::UncalibratedMeasurement* 
  SourceLink::uncalibrated() const
  { return m_uncalibrated; }

  Acts::GeometryIdentifier 
  SourceLink::geometryId() const 
  { return m_geometryId; }

}
