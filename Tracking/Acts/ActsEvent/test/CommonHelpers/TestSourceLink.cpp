/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// File based on Acts:
// https://github.com/acts-project/acts/blob/v21.0.0/Tests/CommonHelpers/Acts/Tests/CommonHelpers/TestSourceLink.cpp
// TODO: centralize the helper files


#include "Acts/Tests/CommonHelpers/TestSourceLink.hpp"

#include <ostream>

bool Acts::Test::operator==(const TestSourceLink& lhs,
                            const TestSourceLink& rhs) {
  return (lhs.geometryId() == rhs.geometryId()) and
         (lhs.sourceId == rhs.sourceId) and (lhs.indices == rhs.indices) and
         (lhs.parameters == rhs.parameters) and
         (lhs.covariance == rhs.covariance);
}

bool Acts::Test::operator!=(const TestSourceLink& lhs,
                            const TestSourceLink& rhs) {
  return not(lhs == rhs);
}

std::ostream& Acts::Test::operator<<(
    std::ostream& os, const Acts::Test::TestSourceLink& sourceLink) {
  os << "TestsSourceLink(geometryId=" << sourceLink.geometryId()
     << ",sourceId=" << sourceLink.sourceId;
  if (sourceLink.indices[0] != eBoundSize) {
    os << ",index0=" << sourceLink.indices[0];
  }
  if (sourceLink.indices[1] != eBoundSize) {
    os << ",index1=" << sourceLink.indices[1];
  }
  os << ")";
  return os;
}
