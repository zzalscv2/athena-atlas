/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IMaterialEffectsOnTrackProvider.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_IMATERIALEFFECTSONTRACKPROVIDER_H
#define TRKDETDESCRINTERFACES_IMATERIALEFFECTSONTRACKPROVIDER_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
// TrkEventPrimitives
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkMaterialOnTrack/MaterialEffectsOnTrack.h"
#include "TrkParameters/TrackParameters.h"

// STL
#include <vector>

namespace Trk {

  //class TrackParameters;
  class Surface;
  class IPropagator;
  class TrackingVolume;

  /** @class IMaterialEffectsOnTrackProvider
    Interface class IMaterialEffectsOnTrackProvider
    It inherits from IAlgTool. The actual implementation of the AlgTool depends on the SubDetector,
    more detailed information should be found there.

    Purpose of this Tool is the creation of material layers according to a given tracking procedure;
    
    @author David.Lopez@cern.ch
    */
  class IMaterialEffectsOnTrackProvider : virtual public IAlgTool {
    
    public:
    /// Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(IMaterialEffectsOnTrackProvider, 1, 0);

      /**Virtual destructor*/
      virtual ~IMaterialEffectsOnTrackProvider(){}

      /** Interface method for MaterialEffectsOnTrack updates.
	  The provider creates surfaces inside the volume given
	  and calculates the MaterialEffectsOnTrack that should be applied
	  in each of this surfaces. The navigation is done inside. 
      */

      virtual std::vector< Trk::MaterialEffectsOnTrack > extrapolationSurfacesAndEffects(const Trk::TrackingVolume& ,
											 const Trk::IPropagator& ,
											 const Trk::TrackParameters& ,
											 const Trk::Surface& ,
											 Trk::PropDirection ,
											 Trk::ParticleHypothesis ) const = 0;

      virtual void validationAction() const {}      
  };

} // end of namespace


#endif // TRKDETDESCRINTERFACES_IMATERIALEFFECTSONTRACKPROVIDER_H


