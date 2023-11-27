/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// MuonStationTypeBuilder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef MUONTRACKINGGEOMETRY_MUONSTATIONTYPEBUILDER_H
#define MUONTRACKINGGEOMETRY_MUONSTATIONTYPEBUILDER_H
// Amg
#include "GeoPrimitives/GeoPrimitives.h" //Amg stuff
// Trk
#include "TrkDetDescrInterfaces/ITrackingVolumeArrayCreator.h" //in tool handle template
#include "TrkDetDescrUtils/SharedObject.h" //see the typedef for LayTr
#include "TrkGeometry/TrackingVolume.h" //also for LayerArray typedef
#include "TrkDetDescrGeoModelCnv/GeoMaterialConverter.h"

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h" //base class
#include "GaudiKernel/ToolHandle.h" //member

//stl
#include <utility>  //for std::pair

class GeoVPhysVol;
class GeoShape;
class Identifier;

namespace Trk {
    class Volume;
    class Layer;
    class CuboidVolumeBounds;
    class TrapezoidVolumeBounds;
    class DoubleTrapezoidVolumeBounds;
    class PlaneLayer;
    class Material;
    class MaterialProperties;
}  // namespace Trk

namespace MuonGM {
    class MuonDetectorManager;
    class MuonStation;
}  // namespace MuonGM

namespace Muon {

    typedef std::pair<Trk::SharedObject<const Trk::Layer>, const Amg::Transform3D*> LayTr;

    /** @class MuonStationTypeBuilder

        The Muon::MuonStationTypeBuilder retrieves components of muon stations from
       Muon Geometry Tree, builds 'prototype' object (TrackingVolume with NameType)

        by Sarka.Todorova@cern.ch
      */

    class MuonStationTypeBuilder : public AthAlgTool {
    public:
        struct Cache {
            std::unique_ptr<Trk::MaterialProperties> m_mdtTubeMat{};
            std::unique_ptr<Trk::MaterialProperties> m_rpcLayer{};
            std::vector<std::unique_ptr<Trk::MaterialProperties>> m_mdtFoamMat{};
            std::unique_ptr<Trk::MaterialProperties> m_rpc46{};
            std::vector<std::unique_ptr<Trk::MaterialProperties>> m_rpcDed{};
            std::unique_ptr<Trk::MaterialProperties> m_rpcExtPanel{};
            std::unique_ptr<Trk::MaterialProperties> m_rpcMidPanel{};
            std::unique_ptr<Trk::MaterialProperties> m_matCSC01{};
            std::unique_ptr<Trk::MaterialProperties> m_matCSCspacer1{};
            std::unique_ptr<Trk::MaterialProperties> m_matCSC02{};
            std::unique_ptr<Trk::MaterialProperties> m_matCSCspacer2{};
            std::unique_ptr<Trk::MaterialProperties> m_matTGC01{};
            std::unique_ptr<Trk::MaterialProperties> m_matTGC06{};
        };
        /** Constructor */
        MuonStationTypeBuilder(const std::string&, const std::string&, const IInterface*);
        /** Destructor */
        virtual ~MuonStationTypeBuilder() = default;
        /** AlgTool initailize method.*/
        StatusCode initialize();
        /** AlgTool finalize method */
        StatusCode finalize();
        /** Interface methode */
        static const InterfaceID& interfaceID(){
            static const InterfaceID IID_IMuonStationTypeBuilder("MuonStationTypeBuilder", 1, 0);
            return IID_IMuonStationTypeBuilder;
        }
        /** steering routine */
        Trk::TrackingVolumeArray* processBoxStationComponents(const GeoVPhysVol* cv, Trk::CuboidVolumeBounds* envBounds,
                                                              Cache&) const;

        Trk::TrackingVolumeArray* processTrdStationComponents(const GeoVPhysVol* cv, Trk::TrapezoidVolumeBounds* envBounds,
                                                              Cache&) const;

        Trk::TrackingVolume* processCscStation(const GeoVPhysVol* cv, const std::string& name, Cache&) const;

        std::vector<Trk::TrackingVolume*> processTgcStation(const GeoVPhysVol* cv, Cache&) const;

        /** components */
        Trk::TrackingVolume* processMdtBox(Trk::Volume&, 
                                           const GeoVPhysVol* , 
                                           Amg::Transform3D, double, Cache&) const;

        Trk::TrackingVolume* processMdtTrd(Trk::Volume&, 
                                           const GeoVPhysVol*, 
                                           Amg::Transform3D, Cache&) const;

        Trk::TrackingVolume* processRpc(Trk::Volume&, 
                                        const std::vector<const GeoVPhysVol*>&, 
                                        const std::vector<Amg::Transform3D>&, Cache&) const;

        Trk::TrackingVolume* processSpacer(Trk::Volume&, 
                                          std::vector<const GeoVPhysVol*>, 
                                          std::vector<Amg::Transform3D>) const;

        Trk::TrackingVolume* processNSW(const MuonGM::MuonDetectorManager* muonDetMgr, const std::vector<Trk::Layer*>&) const;

        Trk::LayerArray* processCSCTrdComponent(const GeoVPhysVol*, 
                                                Trk::TrapezoidVolumeBounds&, 
                                                Amg::Transform3D&, Cache&) const;

        Trk::LayerArray* processCSCDiamondComponent(const GeoVPhysVol*, 
                                                    Trk::DoubleTrapezoidVolumeBounds&, 
                                                    Amg::Transform3D&,
                                                    Cache&) const;

        Trk::LayerArray* processTGCComponent(const GeoVPhysVol*, 
                                             Trk::TrapezoidVolumeBounds&, 
                                             Amg::Transform3D&, 
                                             Cache&) const;

        std::pair<Trk::Layer*, const std::vector<Trk::Layer*>*> createLayerRepresentation(
            Trk::TrackingVolume* trVol) const;

        Trk::Layer* createLayer(const MuonGM::MuonDetectorManager* detMgr, Trk::TrackingVolume* trVol, Trk::MaterialProperties*,
                                Amg::Transform3D&) const;

        static Identifier identifyNSW(const MuonGM::MuonDetectorManager* muonDetMgr, const std::string&, const Amg::Transform3D&) ;

        void printChildren(const GeoVPhysVol*) const;
        // used to be private ..
        double get_x_size(const GeoVPhysVol*) const;
        double decodeX(const GeoShape*) const;
        double getVolume(const GeoShape*) const;
        Trk::MaterialProperties getAveragedLayerMaterial(const GeoVPhysVol*, double, double) const;
        void collectMaterial(const GeoVPhysVol*, Trk::MaterialProperties&, double) const;
        Trk::MaterialProperties collectStationMaterial(const Trk::TrackingVolume* trVol, double) const;

    private:
        /** Private method to fill default material */
        // void fillDefaultServiceMaterial();

        Gaudi::Property<bool> m_multilayerRepresentation{this, "BuildMultilayerRepresentation", true};
        Gaudi::Property<bool> m_resolveSpacer{this, "ResolveSpacerBeams", false};

        ToolHandle<Trk::ITrackingVolumeArrayCreator> m_trackingVolumeArrayCreator{
            this, "TrackingVolumeArrayCreator",
            "Trk::TrackingVolumeArrayCreator/TrackingVolumeArrayCreator"};  //!< Helper Tool to create TrackingVolume Arrays

        Trk::Material m_muonMaterial{10e10, 10e10, 0., 0., 0.};  //!< the material
        Trk::GeoMaterialConverter m_materialConverter{};
    };

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONSTATIONTYPEBUILDER_H
