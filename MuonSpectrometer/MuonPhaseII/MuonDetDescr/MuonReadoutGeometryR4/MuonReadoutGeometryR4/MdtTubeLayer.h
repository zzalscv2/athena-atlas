/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_MDTTUBELAYER_H
#define MUONREADOUTGEOMETRYR4_MDTTUBELAYER_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <GeoModelKernel/GeoVFullPhysVol.h>
#include <GeoModelKernel/GeoTransform.h>
#include <GeoModelKernel/GeoSerialTransformer.h>



namespace MuonGMR4{
    /**
     * Helper struct to retrieve the tube lengths and the tube centers directly from the GeoModel tree
    */
    class MdtTubeLayer{
        /**
         * Constructor taking the GeoModel parent node of the tube nodes
        */
    public:
        MdtTubeLayer(const PVConstLink layer);
        ///@brief Returns the number of tubes in the layer
        unsigned int nTubes() const ;
        ///@brief: Returns the transformation from the layer to the muon station
        const Amg::Transform3D layerTransform() const;
        ///@brief Returns the transformation of the tube to the muon station
        ///       Index counting [0 - nTubes()-1]
        const Amg::Transform3D tubeTransform(unsigned int tube) const;
        ///@brief Returns the half-length of the given tube 
        double tubeHalfLength(unsigned int tube) const;
    private:
        PVConstLink m_layerNode{nullptr};       
        PVConstLink getTubeNode(unsigned int tube) const;
        const GeoSerialTransformer* m_serialTrans{nullptr};
    };
}
#endif