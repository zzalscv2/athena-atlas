/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELR4_MUONREADOUTELEMENT_H
#define MUONGEOMODELR4_MUONREADOUTELEMENT_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
////
#include <AthenaBaseComps/AthMessaging.h>
#include <GaudiKernel/ServiceHandle.h>
#include <GeoModelKernel/GeoVDetectorElement.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonReadoutGeometryR4/MuonTransformCache.h>
#include <ActsGeometryInterfaces/IDetectorElement.h>
#include <mutex>
namespace MuonGMR4 {
///   The MuonReadoutElement is an abstract class representing the geometry
///   representing the muon detector. The segmentation of the detectors varies
///   along the MS subsystems and is documented further in the specific sub
///   detector classes. As rule of thumb, detectors sitting on at a different MS
///   layer or station Index, stationEta or station phi are represented by at
///   least one MuonReadoutElement. The MuonReadout elements are constructed
///   from the RAW geometry provided by GeoModelSvc. The class below is pure
///   virtual and implements the minimal set of methods shared by all muon
///   detector technolgies.

class MuonReadoutElement : public GeoVDetectorElement, public AthMessaging, public ActsTrk::IDetectorElement {

   public:
    /// Helper struct to ship the defining arguments of the detector element
    /// around
    struct defineArgs {
        /// Pointer to the underlying physical volume in GeoModel
        GeoVFullPhysVol* physVol{nullptr};
        /// chamber design name (see below for explanation)
        std::string chambDesign{""};
        /// ATLAS identifier
        Identifier detElId{0};        
        /// Basic transformation to be applied on top in order 
        /// to reach the center of the volume from the GeoModel transform
        Amg::Transform3D toVolCenter{Amg::Transform3D::Identity()};

    };
    

    MuonReadoutElement(defineArgs&& args);
    MuonReadoutElement()=delete;
    MuonReadoutElement(const MuonReadoutElement&)=delete;
    
    /// Element initialization
    virtual StatusCode initElement() = 0;
    /// Cache the alignment
    bool storeAlignment(ActsTrk::RawGeomAlignStore& store) const;
    /// Return the athena identifier.
    ///  The Identifier is identical with the first measurment channel in
    ///  readout element (E.g. Strip 1 in Layer 1 in the NSW)
    Identifier identify() const override final;

    /// Returns the Identifier has of the Element that is Identical to the
    /// detElHash from the id_helper class
    IdentifierHash identHash() const;
    /// Returns the stationName (BIS, BOS, etc) encoded into the integer
    int stationIndex() const;
    /// Returns the stationEta (positive A site, negative O site)
    int stationEta() const;
    /// Returns the stationPhi (1-8) -> sector (2*phi - (isSmall))
    int stationPhi() const;

    /// Constructs the identifier hash from the full measurement Identifier. The
    /// hash is always defined w.r.t the specific detector element and used to
    /// access the information in memory quickly
    virtual IdentifierHash measurementHash(const Identifier& measId) const = 0;

    virtual IdentifierHash layerHash(const Identifier& measId) const = 0;
    /// Converts the measurement hash back to the full Identifier
    virtual Identifier measurementId(const IdentifierHash& measHash) const = 0;


    /// The chamber design refers to the construction parameters of a readout
    /// element. Used for the retrieval of the chamber parameters
    ///   E.g. the chambers BOL1A8 & BOL2A8 are identical in terms of number of
    ///   tubes, dimensions etc.
    std::string chamberDesign() const;

    /// Returns the pointer to the muonIdHelperSvc
    const Muon::IMuonIdHelperSvc* idHelperSvc() const;

    /// Returns the detector center (Which is the same as the detector center of
    /// the first measurement layer)
    Amg::Vector3D center(const ActsGeometryContext& ctx) const;
    /// Returns the center of a given detector layer using the complete
    /// Identifier of the measurement
    Amg::Vector3D center(const ActsGeometryContext& ctx,
                         const Identifier& id) const;
    /// Returns the center of a given detector layer using the Identifier hash
    /// of the measurement
    Amg::Vector3D center(const ActsGeometryContext& ctx,
                         const IdentifierHash& hash) const;

    ///   Transformations to translate between local <-> global coordinates.
    ///   They follow the common ATLAS conventations that the origin is located
    ///   in the center of the detector layer
    ///     --- x_axis: Points along the primary coordinate
    ///     --- y_axis:
    ///     --- z_axis: Points towards the surface normal
    ///   The transformations always include the corrections from the A-Lines of
    ///   the alignment system
    /// Returns the global to local transformation into the rest frame of the
    /// detector (Coincides with the first measurement layer)
    const Amg::Transform3D& globalToLocalTrans(const ActsGeometryContext& ctx) const;
    /// Returns the global to local transformation into the rest frame of a
    /// given measurement layer
    const Amg::Transform3D& globalToLocalTrans(const ActsGeometryContext& ctx,
                                               const Identifier& id) const;
    /// Returns the global to local transformation into the rest frame of a
    /// given measurement layer
    const Amg::Transform3D& globalToLocalTrans(const ActsGeometryContext& ctx, 
                                               const IdentifierHash& hash) const;

    /// Returns the local to global transformation into the ATLAS coordinate
    /// system
    const Amg::Transform3D& localToGlobalTrans(const ActsGeometryContext& ctx) const;
    const Amg::Transform3D& localToGlobalTrans(const ActsGeometryContext& ctx,
                                               const Identifier& id) const;
    const Amg::Transform3D& localToGlobalTrans(const ActsGeometryContext& ctx,
                                               const IdentifierHash& id) const;

     
     const Acts::Transform3& transform(const Acts::GeometryContext& gctx) const override final;
     /// Returns the transformation from the origin given by GeoModel to 
     /// the center of the volume
     const Amg::Transform3D& toCenterTrans() const;
     
   protected:
     using TransformMaker = MuonTransformCache::TransformMaker;
      
      /// Inserts a transfomration for caching
      StatusCode insertTransform(const IdentifierHash& hash,
                                 TransformMaker make);

      /// Returns the transformation into the center of the readout volume
      Amg::Transform3D toStation(ActsTrk::RawGeomAlignStore* alignStore) const;
   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        "Muon::MuonIdHelperSvc/MuonIdHelperSvc", "MuonReadoutElement"};

    const defineArgs m_args{};
    IdentifierHash m_detElHash{0};
    /// Cache the station index of the identifier
    int m_stIdx{-1};
    /// Cache the station eta of the identifier
    int m_stEta{-1};
    /// Cache the station phi of the identifier
    int m_stPhi{-1};

    /// Cache all global to local transformations
    MuonTransformSet m_globalToLocalCaches{};
    /// Cache all local to gloabl transformations
    MuonTransformSet m_localToGlobalCaches{};
};
}  // namespace MuonGMR4
#include <MuonReadoutGeometryR4/MuonReadoutElement.icc>
#endif
