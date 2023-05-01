/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELR4_UTILS_H
#define MUONGEOMODELR4_UTILS_H


#include "GeoPrimitives/GeoPrimitives.h"
/// Load the Eigen definitions.

#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoShape.h"
#include "AthenaBaseComps/AthMessaging.h"
namespace MuonGMR4{

    /// Navigates throughs the volume to find a Box / Prd shape
    const GeoShape* extractShape(const PVConstLink& physVol, MsgStream& msg);
    const GeoShape* extractShape(const GeoShape* inShape, MsgStream& msg);
    // Navigates through the bolume to find the shifts / rotations etc.
    Amg::Transform3D extractShifts(const PVConstLink& physVol, MsgStream& msg);
    Amg::Transform3D extractShifts(const GeoShape* inShape, MsgStream& msg);

    

}
#endif