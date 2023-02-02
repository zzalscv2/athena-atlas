/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsTrkEvent_SourceLink_h
#define ActsTrkEvent_SourceLink_h

#include "Acts/EventData/SourceLink.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"

#include "xAODMeasurementBase/UncalibratedMeasurement.h"

namespace ActsTrk {
    /**
     * @brief lightweight source link based on EL to UncalibratedMeasurementBase container
     */
  class SourceLink final {
        public:
            SourceLink(xAOD::UncalibratedMeasurement* uncalibrated);
            ~SourceLink() = default;
            const xAOD::UncalibratedMeasurement* uncalibrated() const;
	    Acts::GeometryIdentifier geometryId() const;
        private:
	    Acts::GeometryIdentifier m_geometryId{};
            // TODO, we may reconsider using ElementLink
            xAOD::UncalibratedMeasurement* m_uncalibrated = nullptr;
    };
}

#endif
