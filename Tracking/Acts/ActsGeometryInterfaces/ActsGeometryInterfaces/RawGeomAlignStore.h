/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_RawGeomAlignStore_H
#define ACTSGEOMETRYINTERFACES_RawGeomAlignStore_H

/// Includes the GeoPrimitives
#include "ActsGeometryInterfaces/GeometryDefs.h"
/// To be put first
#include "ActsGeometryInterfaces/AlignmentStore.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include "GeoModelUtilities/GeoAlignmentStore.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"

/// The ActsFromGeoAlignStore is an adaptor to go from the GeoModel world caching the
/// rigid transformations of the detector elements to the Acts world where transformations

namespace ActsTrk {

    class RawGeomAlignStore {
    public:
        /// Default constructor
        RawGeomAlignStore() = default;
        /// Default virtual destructor
        virtual ~RawGeomAlignStore() = default;
        /// Store written by the conditions algorithm aligning each subsystem
        GeoModel::TransientConstSharedPtr<GeoAlignmentStore> geoModelAlignment{std::make_unique<GeoAlignmentStore>()};
        /// Store holding the alignment of each Acts Detector Element transformation
        GeoModel::TransientConstSharedPtr<AlignmentStore> trackingAlignment{std::make_unique<AlignmentStore>()};
        /// The aligned detector element type
        DetectorType detType{DetectorType::UnDefined};
    };
}  // namespace ActsTrk
CLASS_DEF(ActsTrk::RawGeomAlignStore, 267137212, 1);
CONDCONT_DEF(ActsTrk::RawGeomAlignStore, 27391946);

#endif
