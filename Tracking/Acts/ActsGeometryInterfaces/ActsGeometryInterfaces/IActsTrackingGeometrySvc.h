/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IACTSTRACKINGGEOMETRYSVC_H
#define ACTSGEOMETRYINTERFACES_IACTSTRACKINGGEOMETRYSVC_H

#include "GaudiKernel/IService.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometryInterfaces/RawGeomAlignStore.h"

#include <memory>


namespace Acts {
    class TrackingGeometry;
}

class IActsTrackingGeometrySvc : virtual public IService {
public:
    DeclareInterfaceID(IActsTrackingGeometrySvc, 1, 0);

    virtual ~IActsTrackingGeometrySvc() = default;
    /// Returns a pointer to the internal ACTS tracking geometry
    virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() = 0;

    /// Caches the final transformations in the alignment store for a given sub detector type
    /// (defined by an internal flag in the Store). Returns the number of added elements
    virtual unsigned int populateAlignmentStore(ActsTrk::RawGeomAlignStore& store) const = 0;
    /// Checks whether the GeometryContext has alignment stores foreach active subdetector
    /// excluding the TRTs. Returns a StatusCode::FAILURE if an AlignmentStore is missing
    virtual StatusCode checkAlignComplete(const ActsGeometryContext& ctx) const = 0;
    /// Returns an empty nominal context without any alignment caches
    virtual const ActsGeometryContext& getNominalContext() const = 0;
};

#endif
