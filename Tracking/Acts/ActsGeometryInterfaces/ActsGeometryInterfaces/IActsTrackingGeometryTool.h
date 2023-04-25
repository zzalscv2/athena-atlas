/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IACTSTRACKINGGEOMETRYTOOL_H
#define ACTSGEOMETRYINTERFACES_IACTSTRACKINGGEOMETRYTOOL_H

#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IInterface.h"

namespace Acts {
    class TrackingGeometry;
}

class IActsTrackingGeometryTool : virtual public IAlgTool {
public:
    DeclareInterfaceID(IActsTrackingGeometryTool, 1, 0);

    virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const = 0;

    /// Return the ActsGeometryContext for the current event with the cached alignment constants
    virtual const ActsGeometryContext& getGeometryContext(const EventContext& ctx) const = 0;
    /// Method without explicitly piping the event context
    virtual const ActsGeometryContext& getGeometryContext() const = 0;

    /// Returns the refrence to the nominal ActsGeometryContext. The context is hold
    /// by the tracking geometry service and does not contain any alignable transforms
    virtual const ActsGeometryContext& getNominalGeometryContext() const = 0;
};

#endif
