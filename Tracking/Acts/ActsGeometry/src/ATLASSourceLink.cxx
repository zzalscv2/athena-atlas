/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ATLASSourceLink.h"

ATLASSourceLink::ATLASSourceLink(const Acts::Surface& surface, const Trk::MeasurementBase& atlasHit,
                size_t dim, Acts::BoundVector values, Acts::BoundMatrix cov)
      : Acts::SourceLink{surface.geometryId()},
        m_values(values),
        m_cov(cov),
        m_dim(dim),
        m_atlasHit(&atlasHit) {}

