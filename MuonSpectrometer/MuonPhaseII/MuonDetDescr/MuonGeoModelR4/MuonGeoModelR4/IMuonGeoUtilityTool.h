/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELR4_IMUONGEOUTILITYTOOL_H
#define MUONGEOMODELR4_IMUONGEOUTILITYTOOL_H


#include "GeoPrimitives/GeoPrimitives.h"
/// Load the Eigen definitions.

#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoAlignableTransform.h"

#include "GaudiKernel/IAlgTool.h"

namespace MuonGMR4{

class IMuonGeoUtilityTool : virtual public IAlgTool {
    public:
         /// Gaudi interface ID
        DeclareInterfaceID(IMuonGeoUtilityTool, 1, 0);
       
        /// Abbriviate the type definitions of the maps produced by the SqLite publisher
        using alignNodeMap = std::map<std::string, GeoAlignableTransform*>;
        using physNodeMap = std::map<std::string, GeoFullPhysVol*>;
        using alignedPhysNodes = std::map<PVConstLink, const GeoAlignableTransform*>;
        /// Loops over the publised alignable transforms and the published alignable nodes and produces a map
        /// of full phys vols that have alignable transformations attached
        virtual alignedPhysNodes selectAlignableVolumes(const physNodeMap& publishedPhysVols, 
                                                       const alignNodeMap& publishedAlignNodes) const = 0;

        /// Returns the next alignable transformation in the GeoModel tree hierachy upstream
        virtual const GeoAlignableTransform* findAlignableTransform(const PVConstLink& physVol,
                                                                    const alignedPhysNodes& alignNodes) const = 0;
                                                             
        
        /// Navigates throughs the volume to find a Box / Prd shape
        virtual const GeoShape* extractShape(const PVConstLink& physVol) const = 0;
        virtual const GeoShape* extractShape(const GeoShape* inShape) const = 0;
    
        // Navigates through the bolume to find the shifts / rotations etc.
        virtual Amg::Transform3D extractShifts(const PVConstLink& physVol) const = 0;
        virtual Amg::Transform3D extractShifts(const GeoShape* inShape) const = 0;

        ///     
        virtual std::string dumpShape(const GeoShape* inShape) const = 0;
        ///
        virtual std::string dumpVolume(const PVConstLink& physVol) const = 0;
 
};

}
#endif