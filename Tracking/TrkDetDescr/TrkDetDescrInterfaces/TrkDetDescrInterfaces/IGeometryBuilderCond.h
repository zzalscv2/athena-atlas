/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IGeometryBuilderCond.hm (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_IGEOMETRYBUILDERCOND_H
#define TRKDETDESCRINTERFACES_IGEOMETRYBUILDERCOND_H

// Gaudi
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
// Trk - enum
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkSurfaces/Surface.h"
#include "StoreGate/WriteCondHandle.h"

// STL
#include <vector>

#include "CxxUtils/checker_macros.h"

class EventIDRange;
namespace Trk {

class TrackingGeometry;
class TrackingVolume;
class Layer;

/** @class IGeometryBuilderCond

  Interface class IGeometryBuilderCond,
  the GeometryBuilder inherits from this one.

  VolumeBounds can be given optionally to force a specific size/shape/boundary

  This interface class implements protected glue and surface
  exchange methods, that require friend rights to the classes

  @author Andreas.Salzburger@cern.ch
  */
class IGeometryBuilderCond : virtual public IAlgTool
{

public:
  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID(IGeometryBuilderCond, 1, 0);

  /**Virtual destructor*/
  virtual ~IGeometryBuilderCond() {}

  /** TrackingGeometry Interface methode -
   *   - Event context
   *   - corresponding TrackingVolume, from which
   *     we retrieve the volume to wrap the TrackingGeometry around.
   */
  virtual std::unique_ptr<TrackingGeometry> trackingGeometry(
    const EventContext& ctx,
    Trk::TrackingVolume* tVol,
    SG::WriteCondHandle<TrackingGeometry>& whandle) const = 0;

  /** The unique signature */
  virtual GeometrySignature geometrySignature() const = 0;
};

} // end of namespace

#endif // TRKDETDESCRINTERFACES_IGEOMETRYBUILDERCOND_H

