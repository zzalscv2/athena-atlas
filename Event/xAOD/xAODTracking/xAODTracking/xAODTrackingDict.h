// Dear emacs, this is -*- c++ -*-
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_XAODTRACKINGDICT_H
#define XAODTRACKING_XAODTRACKINGDICT_H

// Local include(s).
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/versions/TrackParticle_v1.h"
#include "xAODTracking/versions/TrackParticleContainer_v1.h"
#include "xAODTracking/versions/TrackParticleAuxContainer_v1.h"
#include "xAODTracking/versions/TrackParticleAuxContainer_v2.h"
#include "xAODTracking/versions/TrackParticleAuxContainer_v3.h"
#include "xAODTracking/versions/TrackParticleAuxContainer_v4.h"
#include "xAODTracking/versions/TrackParticleAuxContainer_v5.h"

#include "xAODTracking/NeutralParticle.h"
#include "xAODTracking/NeutralParticleContainer.h"
#include "xAODTracking/NeutralParticleAuxContainer.h"
#include "xAODTracking/versions/NeutralParticle_v1.h"
#include "xAODTracking/versions/NeutralParticleContainer_v1.h"
#include "xAODTracking/versions/NeutralParticleAuxContainer_v1.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/versions/Vertex_v1.h"
#include "xAODTracking/versions/VertexContainer_v1.h"
#include "xAODTracking/versions/VertexAuxContainer_v1.h"

#include "xAODTracking/TrackMeasurementValidation.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODTracking/TrackMeasurementValidationAuxContainer.h"
#include "xAODTracking/versions/TrackMeasurementValidation_v1.h"
#include "xAODTracking/versions/TrackMeasurementValidationContainer_v1.h"
#include "xAODTracking/versions/TrackMeasurementValidationAuxContainer_v1.h"

#include "xAODTracking/TrackStateValidation.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "xAODTracking/TrackStateValidationAuxContainer.h"
#include "xAODTracking/versions/TrackStateValidation_v1.h"
#include "xAODTracking/versions/TrackStateValidationContainer_v1.h"
#include "xAODTracking/versions/TrackStateValidationAuxContainer_v1.h"

#include "xAODTracking/SCTRawHitValidation.h"
#include "xAODTracking/SCTRawHitValidationContainer.h"
#include "xAODTracking/SCTRawHitValidationAuxContainer.h"
#include "xAODTracking/versions/SCTRawHitValidation_v1.h"
#include "xAODTracking/versions/SCTRawHitValidationContainer_v1.h"
#include "xAODTracking/versions/SCTRawHitValidationAuxContainer_v1.h"

#include "xAODTracking/TrackingPrimitives.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"

#include "xAODTracking/TrackState.h"
#include "xAODTracking/TrackStateContainer.h"
#include "xAODTracking/TrackStateAuxContainer.h"
#include "xAODTracking/versions/TrackState_v1.h"
#include "xAODTracking/versions/TrackStateContainer_v1.h"
#include "xAODTracking/versions/TrackStateAuxContainer_v1.h"

#include "xAODTracking/TrackParameters.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackParametersAuxContainer.h"
#include "xAODTracking/versions/TrackParameters_v1.h"
#include "xAODTracking/versions/TrackParametersContainer_v1.h"
#include "xAODTracking/versions/TrackParametersAuxContainer_v1.h"


#include "xAODTracking/TrackJacobian.h"
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackJacobianAuxContainer.h"
#include "xAODTracking/versions/TrackJacobian_v1.h"
#include "xAODTracking/versions/TrackJacobianContainer_v1.h"
#include "xAODTracking/versions/TrackJacobianAuxContainer_v1.h"

#include "xAODTracking/TrackMeasurement.h"
#include "xAODTracking/TrackMeasurementContainer.h"
#include "xAODTracking/TrackMeasurementAuxContainer.h"
#include "xAODTracking/versions/TrackMeasurement_v1.h"
#include "xAODTracking/versions/TrackMeasurementContainer_v1.h"
#include "xAODTracking/versions/TrackMeasurementAuxContainer_v1.h"

#include "xAODTracking/TrackSummary.h"
#include "xAODTracking/TrackSummaryContainer.h"
#include "xAODTracking/TrackSummaryAuxContainer.h"
#include "xAODTracking/versions/TrackSummary_v1.h"
#include "xAODTracking/versions/TrackSummaryContainer_v1.h"
#include "xAODTracking/versions/TrackSummaryAuxContainer_v1.h"

#include "xAODTracking/TrackSurface.h"
#include "xAODTracking/TrackSurfaceContainer.h"
#include "xAODTracking/TrackSurfaceAuxContainer.h"
#include "xAODTracking/versions/TrackSurface_v1.h"
#include "xAODTracking/versions/TrackSurfaceContainer_v1.h"
#include "xAODTracking/versions/TrackSurfaceAuxContainer_v1.h"

// EDM include(s).
#include "xAODCore/tools/DictHelpers.h"

// Instantiate all necessary types for the dictionary.
namespace {
   struct GCCXML_DUMMY_INSTANTIATION_XAODTRACKING {
      // Local type(s).
      XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, TrackParticleContainer_v1 );
      XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, NeutralParticleContainer_v1 );
      XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD, VertexContainer_v1 );

      XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD,
                                           TrackMeasurementValidationContainer_v1 );
      XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD,
                                           TrackStateValidationContainer_v1 );
      XAOD_INSTANTIATE_NS_CONTAINER_TYPES( xAOD,
                                           SCTRawHitValidationContainer_v1 );

      // Type(s) needed for the dictionary generation to succeed.
#ifndef XAOD_ANALYSIS
      XAOD_INSTANTIATE_CONTAINER_TYPES( TrackCollection );
#endif // not XAOD_ANALYSIS
      xAOD::CurvilinearParameters_t dummy;
   };
}

#endif // XAODTRACKPARTICLE_XAODTRACKPARTICLEDICT_H
