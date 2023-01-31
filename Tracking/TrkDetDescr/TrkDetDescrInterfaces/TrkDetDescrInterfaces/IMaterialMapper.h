/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IMaterialMapper.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_IMATERIALMAPPER_H
#define TRKDETDESCRINTERFACES_IMATERIALMAPPER_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

#include "GeoPrimitives/GeoPrimitives.h"

namespace Trk {
    class TrackingVolume;
    class Layer;
    class AssociatedMaterial;
}

namespace Trk {


  /** @class IMaterialMapper
     
     Interface class IMaterialMapper
  
     it allows to log the material for single track events for validation purpose
     - it runs during the MaterialMapping & MaterialValidation algorithms
     - can also be plugged into the MaterialEffectsUpdator
  
     @author Andreas.Salzburger@cern.ch
    */
  
  class IMaterialMapper : virtual public IAlgTool {
      
     public:
    /// Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(IMaterialMapper, 1, 0);

       /** Virtual destructor */
       virtual ~IMaterialMapper(){}
       
       /** Record material hit - while stepping outwards */
       virtual void recordMaterialHit(const AssociatedMaterial& amhit, const Amg::Vector3D& projectedPosition) const = 0;

       /** Record material hit - if various hits per layer are recorded, full associates to the  */
       virtual void recordLayerHit(const AssociatedMaterial& amhit, bool full = false) const = 0;
                                            
       /** Record material hit */
       virtual void recordSurfaceHit(const Amg::Vector2D& locpos, const AssociatedMaterial& amhit) const = 0;

   };

} // end of namespace

#endif // TRKDETDESCRINTERFACES_IMATERIALMAPPER_H
