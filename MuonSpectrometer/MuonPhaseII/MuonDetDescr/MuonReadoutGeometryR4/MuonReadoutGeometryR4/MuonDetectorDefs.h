/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELR4_MUONDETECTORDEFS_H
#define MUONGEOMODELR4_MUONDETECTORDEFS_H

#include <GeoPrimitives/GeoPrimitives.h>
///
#include <MuonReadoutGeometry/ArrayHelper.h>
#include <MuonReadoutGeometry/GlobalUtilities.h>

#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <ActsGeometryInterfaces/RawGeomAlignStore.h>

#include <GeoModelKernel/GeoVAlignmentStore.h>
#include <Identifier/Identifier.h>
#include <Identifier/IdentifierHash.h>

#include <functional>

//// This header contains common helper utilities and definitions
namespace MuonGMR4 {   
    
    /// Checks whether the linear part of the transformation rotates or stetches
    /// any of the basis vectors. Returns false that happens
    bool doesNotDeform(const Amg::Transform3D& trans);
    /// Checks whether the transformation is the Identity transformation
    bool isIdentity(const Amg::Transform3D& trans);


}  // namespace MuonGMR4

#endif