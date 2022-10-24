/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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
    class SourceLink : public Acts::SourceLink {
        public:
            SourceLink( const xAOD::UncalibratedMeasurement* uncalibrated);
            ~SourceLink() = default;
            const xAOD::UncalibratedMeasurement* uncalibrated() const;
        private:
            // TODO, we may reconsider using ElementLink
            xAOD::UncalibratedMeasurement* m_uncalibrated = nullptr;
    };
}

#endif