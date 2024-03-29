/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ILayerProviderCond.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_ILAYERPROVIDERCOND_H
#define TRKDETDESCRINTERFACES_ILAYERPROVIDERCOND_H

// Gaudi
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/EventIDRange.h"
#include "GaudiKernel/IAlgTool.h"
#include "StoreGate/WriteCondHandle.h"
// STL
#include <string>
#include <vector>

namespace Trk {

class Layer;
class TrackingGeometry;

/** @class ILayerProviderCond

  Interface class ILayerProviderConds
  it feeds into the StagedGeometryBuilder

  @author Andreas.Salzburger@cern.ch
  */
class ILayerProviderCond : virtual public IAlgTool
{

public:
  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID(ILayerProviderCond, 1, 0);

  /**Virtual destructor*/
  virtual ~ILayerProviderCond() {}

  /** LayerBuilder interface method - returning the endcap layer */
  virtual std::pair<const std::vector<Layer*>, const std::vector<Layer*> >
  endcapLayer(const EventContext& ctx,
              SG::WriteCondHandle<TrackingGeometry>& whandle) const = 0;

  /** LayerBuilder interface method - returning the central layers */
  virtual const std::vector<Layer*>
  centralLayers(const EventContext& ctx,
                SG::WriteCondHandle<TrackingGeometry>& whandle) const = 0;

  /** Name identification */
  virtual const std::string& identification() const = 0;
};

} // end of namespace

#endif // TRKDETDESCRINTERFACES_ILAYERPROVIDER_H

