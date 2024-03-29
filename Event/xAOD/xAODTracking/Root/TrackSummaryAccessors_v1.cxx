/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// $Id: TrackSummaryAccessors_v1.cxx 576255 2013-12-19 12:54:41Z emoyse $

// System include(s):
#include <cstdint>
#include <iostream>

// Local include(s):
#include "xAODTracking/TrackSummaryAccessors_v1.h"

/// Helper macro for Accessor objects
#define DEFINE_ACCESSOR(TYPE, NAME)                                            \
  case xAOD::NAME: {                                                           \
    static const SG::AuxElement::Accessor<TYPE> a(#NAME);                      \
    return &a;                                                                 \
  } break;

namespace xAOD {

  template<>
   const SG::AuxElement::Accessor< uint8_t >*
   trackSummaryAccessorV1<uint8_t>( xAOD::SummaryType type ) {

    switch (type) {
      DEFINE_ACCESSOR(uint8_t, numberOfContribPixelLayers);
      case xAOD::numberOfBLayerHits: {
        static const SG::AuxElement::Accessor<uint8_t> a(
          "numberOfInnermostPixelLayerHits");
        return &a;
      } break;
      case xAOD::numberOfBLayerOutliers: {
        static const SG::AuxElement::Accessor<uint8_t> a(
          "numberOfInnermostPixelLayerOutliers");
        return &a;
      } break;
      case xAOD::numberOfBLayerSharedHits: {
        static const SG::AuxElement::Accessor<uint8_t> a(
          "numberOfInnermostPixelLayerSharedHits");
        return &a;
      } break;
      case xAOD::numberOfBLayerSplitHits: {
        static const SG::AuxElement::Accessor<uint8_t> a(
          "numberOfInnermostPixelLayerSplitHits");
        return &a;
      } break;
      case xAOD::expectBLayerHit: {
        static const SG::AuxElement::Accessor<uint8_t> a(
          "expectInnermostPixelLayerHit");
        return &a;
      } break;
        DEFINE_ACCESSOR( uint8_t, numberOfPixelHits);
        DEFINE_ACCESSOR( uint8_t, numberOfPixelOutliers             );
        DEFINE_ACCESSOR( uint8_t, numberOfPixelHoles                );
        DEFINE_ACCESSOR( uint8_t, numberOfPixelSharedHits           );	
        DEFINE_ACCESSOR( uint8_t, numberOfPixelSplitHits            );
        DEFINE_ACCESSOR( uint8_t, numberOfInnermostPixelLayerHits                );
        DEFINE_ACCESSOR( uint8_t, numberOfInnermostPixelLayerOutliers            );
        DEFINE_ACCESSOR( uint8_t, numberOfInnermostPixelLayerSharedHits          );
        DEFINE_ACCESSOR( uint8_t, numberOfInnermostPixelLayerSplitHits           );
        DEFINE_ACCESSOR( uint8_t, expectInnermostPixelLayerHit                   );
        DEFINE_ACCESSOR( uint8_t, numberOfInnermostPixelLayerEndcapHits          );
        DEFINE_ACCESSOR( uint8_t, numberOfInnermostPixelLayerEndcapOutliers      );
        DEFINE_ACCESSOR( uint8_t, numberOfInnermostPixelLayerSharedEndcapHits    );
        DEFINE_ACCESSOR( uint8_t, numberOfInnermostPixelLayerSplitEndcapHits     );
        DEFINE_ACCESSOR( uint8_t, numberOfNextToInnermostPixelLayerHits                );
        DEFINE_ACCESSOR( uint8_t, numberOfNextToInnermostPixelLayerOutliers            );
        DEFINE_ACCESSOR( uint8_t, numberOfNextToInnermostPixelLayerSharedHits          );
        DEFINE_ACCESSOR( uint8_t, numberOfNextToInnermostPixelLayerSplitHits           );
        DEFINE_ACCESSOR( uint8_t, expectNextToInnermostPixelLayerHit                   );
        DEFINE_ACCESSOR( uint8_t, numberOfNextToInnermostPixelLayerEndcapHits          );
        DEFINE_ACCESSOR( uint8_t, numberOfNextToInnermostPixelLayerEndcapOutliers      );
        DEFINE_ACCESSOR( uint8_t, numberOfNextToInnermostPixelLayerSharedEndcapHits    );
        DEFINE_ACCESSOR( uint8_t, numberOfNextToInnermostPixelLayerSplitEndcapHits     );
        DEFINE_ACCESSOR( uint8_t, numberOfGangedPixels              );
        DEFINE_ACCESSOR( uint8_t, numberOfGangedFlaggedFakes        );
        DEFINE_ACCESSOR( uint8_t, numberOfPixelDeadSensors          );
        DEFINE_ACCESSOR( uint8_t, numberOfPixelSpoiltHits           );
        DEFINE_ACCESSOR( uint8_t, numberOfDBMHits                   );
        DEFINE_ACCESSOR( uint8_t, numberOfSCTHits                   );
        DEFINE_ACCESSOR( uint8_t, numberOfSCTOutliers               );
        DEFINE_ACCESSOR( uint8_t, numberOfSCTHoles                  );
        DEFINE_ACCESSOR( uint8_t, numberOfSCTDoubleHoles            );
        DEFINE_ACCESSOR( uint8_t, numberOfSCTSharedHits             );
        DEFINE_ACCESSOR( uint8_t, numberOfSCTDeadSensors            );
        DEFINE_ACCESSOR( uint8_t, numberOfSCTSpoiltHits             );
        DEFINE_ACCESSOR( uint8_t, numberOfTRTHits                   );
        DEFINE_ACCESSOR( uint8_t, numberOfTRTOutliers               );
        DEFINE_ACCESSOR( uint8_t, numberOfTRTHoles                  );
        DEFINE_ACCESSOR( uint8_t, numberOfTRTHighThresholdHits      );
        DEFINE_ACCESSOR( uint8_t, numberOfTRTHighThresholdHitsTotal );
        DEFINE_ACCESSOR( uint8_t, numberOfTRTHighThresholdOutliers  );
        DEFINE_ACCESSOR( uint8_t, numberOfTRTDeadStraws             );
        DEFINE_ACCESSOR( uint8_t, numberOfTRTTubeHits               );
        DEFINE_ACCESSOR( uint8_t, numberOfTRTXenonHits              );
        DEFINE_ACCESSOR( uint8_t, numberOfTRTSharedHits              );
        DEFINE_ACCESSOR( uint8_t, numberOfPrecisionLayers );
        DEFINE_ACCESSOR( uint8_t, numberOfPrecisionHoleLayers );
        DEFINE_ACCESSOR( uint8_t, numberOfPhiLayers );
        DEFINE_ACCESSOR( uint8_t, numberOfPhiHoleLayers );
        DEFINE_ACCESSOR( uint8_t, numberOfTriggerEtaLayers );
        DEFINE_ACCESSOR( uint8_t, numberOfTriggerEtaHoleLayers );
        DEFINE_ACCESSOR( uint8_t, numberOfOutliersOnTrack           );
        DEFINE_ACCESSOR( uint8_t, standardDeviationOfChi2OS         );
        DEFINE_ACCESSOR( uint8_t, numberOfGoodPrecisionLayers       );
        DEFINE_ACCESSOR( uint8_t, numberOfContribPixelBarrelFlatLayers    );
        DEFINE_ACCESSOR( uint8_t, numberOfContribPixelBarrelInclinedLayers);
        DEFINE_ACCESSOR( uint8_t, numberOfContribPixelEndcap              );
        DEFINE_ACCESSOR( uint8_t, numberOfPixelBarrelFlatHits             );
        DEFINE_ACCESSOR( uint8_t, numberOfPixelBarrelInclinedHits         );
        DEFINE_ACCESSOR( uint8_t, numberOfPixelEndcapHits                 );
        DEFINE_ACCESSOR( uint8_t, numberOfPixelBarrelFlatHoles            );
        DEFINE_ACCESSOR( uint8_t, numberOfPixelBarrelInclinedHoles        );
        DEFINE_ACCESSOR( uint8_t, numberOfPixelEndcapHoles                );
        DEFINE_ACCESSOR( uint8_t, hasValidTime);

      default:                  
         std::cerr << "xAOD::TrackParticle_v1 ERROR Unknown SummaryType ("
                   << type << ") requested" << std::endl;
         return nullptr;
    }
   }
   
   template<>
    const SG::AuxElement::Accessor< float >*
    trackSummaryAccessorV1<float>( xAOD::SummaryType type ) {
      switch( type ) {
        DEFINE_ACCESSOR( float, eProbabilityComb       ); 
        DEFINE_ACCESSOR( float, eProbabilityHT       );   
        DEFINE_ACCESSOR( float, pixeldEdx       ); 
        DEFINE_ACCESSOR( float,  TRTTrackOccupancy ); 
      default:                  
         std::cerr << "xAOD::TrackParticle_v1 ERROR Unknown SummaryType ("
                   << type << ") requested" << std::endl;
         return nullptr;
      }
   }   
} // namespace xAOD
