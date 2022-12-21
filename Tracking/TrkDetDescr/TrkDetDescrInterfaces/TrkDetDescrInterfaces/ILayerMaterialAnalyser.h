/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ILayerMaterialAnalysier.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_ILAYERMATERIALANALYSER_H
#define TRKDETDESCRINTERFACES_ILAYERMATERIALANALYSER_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
// STL
#include <vector>
#include <cstring>

namespace Trk {

  class Layer;
  class LayerMaterialProperties;
  class LayerMaterialRecord;
  class MaterialProperties;
  
  typedef std::vector< std::vector < const MaterialProperties* > > MaterialPropertiesMatrix;
  
  /** @class ILayerMaterialAnalyser
    
    Interface class for LayerMaterial analysis 
  
    @author Andreas.Salzburger@cern.ch
    */
  class ILayerMaterialAnalyser : virtual public IAlgTool {
    
    public:
    /// Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(ILayerMaterialAnalyser, 1, 0);

      /**Virtual destructor*/
      virtual ~ILayerMaterialAnalyser(){}

      /** process the layer  - after material creation and loading */
      virtual StatusCode analyseLayerMaterial(const Layer& lay) const = 0;
            
      /** process the layer material - after material creation and beofre loading */
      virtual StatusCode analyseLayerMaterial(const Layer& lay, const LayerMaterialProperties& lmp) const = 0;
            
      /** process the layer material record  - before material creation */
      virtual StatusCode analyseLayerMaterial(const Layer& lay, const MaterialPropertiesMatrix& lmr) const = 0;
  
      /** process the layer material record  - before material creation */
      virtual StatusCode analyseLayerMaterial(const Layer& lay, const LayerMaterialRecord& lmr) const = 0;
  
  };

} // end of namespace

#endif // TRKDETDESCRINTERFACES_ILAYERMATERIALANALYSER_H


