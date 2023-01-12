/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKDETDESCRINTERFACES_ILAYERBUILDER_H
#define TRKDETDESCRINTERFACES_ILAYERBUILDER_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "TrkSurfaces/Surface.h"
// STL
#include <string>
#include <vector>

namespace Trk {

class CylinderLayer;
class DiscLayer;
class PlaneLayer;
class Layer;

/** @class ILayerBuilder
  Interface class ILayerBuilders
  It inherits from IAlgTool. The actual implementation of the AlgTool depends on
  the SubDetector, more detailed information should be found there.

  @author Andreas.Salzburger@cern.ch
  */
class ILayerBuilder : virtual public IAlgTool
{

public:
  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID(ILayerBuilder, 1, 0);

  /**Virtual destructor*/
  virtual ~ILayerBuilder() {}

  /** LayerBuilder interface method - returning Barrel-like layers */
  virtual std::unique_ptr<const std::vector<CylinderLayer*> >
  cylindricalLayers() const = 0;

  /** LayerBuilder interface method - returning Endcap-like layers */
  virtual std::unique_ptr<const std::vector<DiscLayer*> >
  discLayers() const = 0;

  /** LayerBuilder interface method - returning Planar-like layers */
  virtual std::unique_ptr<const std::vector<PlaneLayer*> >
  planarLayers() const = 0;

  /** Name identification */
  virtual const std::string& identification() const = 0;

  /** Validation Action:
      Can be implemented optionally, outside access to internal validation steps
   */
  virtual void validationAction() const {}

protected:
  /** Protected method to register the Layer to the Surface */
  void associateLayer(const Layer& lay, Surface& sf) const
  {
    sf.associateLayer(lay);
  }
};

} // end of namespace

#endif // TRKDETDESCRINTERFACES_ILAYERBUILDER_H

