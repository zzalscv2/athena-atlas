/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSTATIONLAYERSURFACESVC_IMuonStationLayerSurfaceSvc_H
#define MUONSTATIONLAYERSURFACESVC_IMuonStationLayerSurfaceSvc_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>

#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/INamedInterface.h>



namespace MuonGMR4{
    /**
     * The IMuonStationLayerSurfaceTool defines planes in the center of each muon chamber. The center is defined as 
     *      the half distance between the first and last tube layer in the cases of the Mdts, 
     *      the distance between the first and last layer of a NSW layer in the case of the NSW
     * 
     * The idea behind these common transformation is that these planes serve as a refernce frame to perform the Hough
     * seeding and segement finding in each muon chamber. 
     * 
     *  The instances of the tool are required to be public. i.e.
     *  Include it via PublicToolHandle<IMuonStationLayerSurfaceTool> m_surfTool{};
    */  
    class IMuonStationLayerSurfaceTool: virtual public IAlgTool {        
        public:
        virtual ~IMuonStationLayerSurfaceTool() = default;
        /// Declare interface to the framework
        DeclareInterfaceID( MuonGMR4::IMuonStationLayerSurfaceTool, 1, 0 );

        /// Returns the local -> global transformation for a given measurement Identifier
        virtual const Amg::Transform3D& chambCenterToGlobal(const ActsGeometryContext& gctx, 
                                                            const Identifier& id) const =0;
        /// Returns the global -> chamber center transformation for a given measurement Identifier
        virtual const Amg::Transform3D& globalToChambCenter(const ActsGeometryContext& gctx,
                                                            const Identifier& id) const = 0;

        /// Load the aligned transformations into the GeoModel store. To be called after the
        /// Muon stations are aligned. Returns the number of aligned objects
        virtual unsigned int storeAlignment(ActsTrk::RawGeomAlignStore& alignStore) const = 0;

    };
}
#endif
