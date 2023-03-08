/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ILayerProvider.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_ILAYERPROVIDER_H
#define TRKDETDESCRINTERFACES_ILAYERPROVIDER_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
// STL
#include <string>
#include <vector>

namespace Trk {

class Layer;

/** @class ILayerProvider

  Interface class ILayerProviders
  it feeds into the StagedGeometryBuilder

  @author Andreas.Salzburger@cern.ch
  */
class ILayerProvider : virtual public IAlgTool
{

public:
  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID(ILayerProvider, 1, 0);

  /**Virtual destructor*/
  virtual ~ILayerProvider() {}

  /** LayerBuilder interface method - returning the endcap layer */
  virtual std::pair<const std::vector<Layer*>, const std::vector<Layer*> >
  endcapLayer() const = 0;

  /** LayerBuilder interface method - returning the central layers */
  virtual const std::vector<Layer*> centralLayers() const = 0;

  /** Name identification */
  virtual const std::string& identification() const = 0;
};

} // end of namespace

#endif // TRKDETDESCRINTERFACES_ILAYERPROVIDER_H

