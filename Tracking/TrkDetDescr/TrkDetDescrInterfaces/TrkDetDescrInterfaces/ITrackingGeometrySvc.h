/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ITrackingGeometrySvc.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_ITRACKINGGEOMETRYSERVICE_H
#define TRKDETDESCRINTERFACES_ITRACKINGGEOMETRYSERVICE_H

// Include Files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StatusCode.h"


/** @class ITrackingGeometrySvc 

    The interface implemented by the TrackingGeometrySvc.

    @author Andreas.Salzburger@cern.ch
*/

namespace Trk {
  class TrackingGeometry;

  class ITrackingGeometrySvc : virtual public IInterface {
   
   public:
  /// Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(ITrackingGeometrySvc, 1, 0);

    /** Provide the TrackingGeometry */
    virtual const Trk::TrackingGeometry* trackingGeometry() const = 0;

    //!< Returns the name of the TrackingGeometry built with this Svc
    virtual const std::string& trackingGeometryName() const = 0;
  };
}

#endif // TRKDETDESCRINTERFACES_ITRACKINGGEOMETRYSERVICE_H

