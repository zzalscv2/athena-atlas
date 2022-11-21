/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IHitCreator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_FATRASINTERFACES_IHITCREATOR_H
#define ISF_FATRASINTERFACES_IHITCREATOR_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "CxxUtils/checker_macros.h"

// Trk
#include "TrkParameters/TrackParameters.h"
#include <utility>


namespace Trk {    
  class Layer;
  class RIO_OnTrack;
}

namespace ISF {
  class ISFParticle;
}  

typedef std::pair< const Trk::TrackParameters*, const Trk::RIO_OnTrack* > ParametersROT;
typedef std::pair< const Trk::TrackParameters*, const Trk::Layer* >       ParametersLayer;  
    
namespace iFatras {
     
  /** 
   @class IHitCreator

   Interface definition for Sim Hit creation in Fatras, 
   starting from intersection on an active surface

   @author Andreas.Salzburger@cern.ch
   @author Sarka.Todorova@cern.ch
   */
      
  class ATLAS_NOT_THREAD_SAFE IHitCreator : virtual public IAlgTool {  // deprecated: ATLASSIM-6020
     public:
     
       /** Virtual destructor */
       virtual ~IHitCreator(){}

      /// Creates the InterfaceID and interfaceID() method
      DeclareInterfaceID(IHitCreator, 1, 0);

       /** Return nothing - store the HIT in hit collection */
       virtual void createSimHit(const ISF::ISFParticle& isp, const Trk::TrackParameters&, double time  ) const = 0;
       
       /** Return the cluster on Track -- the PrepRawData is contained in this one */       
       virtual const ParametersROT* createHit(const ISF::ISFParticle& isp, const Trk::TrackParameters& tpars ) const = 0;
       
       /** Return the cluster on Track -- the PrepRawData is contained in this one */       
       virtual const std::vector< ParametersROT >* createHits(const ISF::ISFParticle& isp, const ParametersLayer& tparsLayer ) const = 0;
       
  };

} // end of namespace

#endif 

