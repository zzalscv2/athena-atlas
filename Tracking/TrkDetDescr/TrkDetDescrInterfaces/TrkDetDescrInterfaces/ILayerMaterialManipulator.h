/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ILayerMaterialManipulator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_ILAYERMATERIALMANIPULATOR_H
#define TRKDETDESCRINTERFACES_ILAYERMATERIALMANIPULATOR_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
// STL
#include <vector>
#include <cstring>

namespace Trk {

  class LayerMaterialProperties;  
  class LayerIndex;
  
  /** @class ILayerMaterialManipulator
    
    Interface class for LayerMaterial manipulation, it creates new LayerMaterial
  
    @author Andreas.Salzburger@cern.ch
    */
  class ILayerMaterialManipulator : virtual public IAlgTool {
    
    public:
    /// Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(ILayerMaterialManipulator, 1, 0);

      /**Virtual destructor*/
      virtual ~ILayerMaterialManipulator(){}

      /** process the layer material - after material creation and before loading */
      virtual const LayerMaterialProperties* processLayerMaterial(const LayerIndex& layIndex, const LayerMaterialProperties& lmp) const = 0;
              
  };

} // end of namespace

#endif // TRKDETDESCRINTERFACES_ILAYERMATERIALMANIPULATOR_H
