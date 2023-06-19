/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_MDTREADOUTELEMENT_H
#define MUONREADOUTGEOMETRYR4_MDTREADOUTELEMENT_H

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/MdtCutOut.h>


namespace MuonGMR4 {

class MdtReadoutElement : public MuonReadoutElement {

   public:
    
    /// Set of parameters to describe a MDT chamber
    struct parameterBook {
        /// Number of tubes per layer
        unsigned int numTubesPerLay{0};
        /// Vector defining the position of the first tube in each tube layer.
        /// The Size of the vector reflects the number of tube layers in the
        /// multi layer
        std::vector<Amg::Vector3D> firstTubePos{};
        /// Thickness of the tube walls
        double tubeWall{0.};
        /// Inner radius of the tubes
        double tubeInnerRad{0.};
        /// Distance between 2 tubes in the layer
        double tubePitch{0.};
        /// Tension parameter Used in the SaggedLine surfaces
        double wireTension{0.};
        /// Depth of the endplug into the active tube volume
        double endPlugLength{0.};
        /// 
        double deadLength{0.};
        /// Radiadtion length
        double radLengthX0{0.};
        /// The chambers have either a rectangular or a trapezoidal shape to first approximation.
        /// The former is mounted in the barrel while the latter can be found on the middle and outer
        /// big wheels. In Run 1 & Run 2, the inner wheel also consistet of MDT chambers.
        /// The local coordinate system is placed in the center of the chamber and the x-axis
        /// is paralell to the long & short edges of the trapezoid as illustrated in
        /// https://gitlab.cern.ch/atlas/athena/-/blob/master/docs/images/TrapezoidalBounds.gif
        /// For the rectengular barrel chambers, the X length is read from longHalfX
        double shortHalfX{0.};
        double longHalfX{0.};
        double halfY{0.};
        /// Height of the chamber ~ number of layers
        double halfHeight{0.};
        /// The trapezoidal shape of the endcap chambers is approximate. In fact, the trapezoid is segmented into
        /// steps, where nTubes have the same length.
        unsigned int tubesPerStep{0};
        /// set of cut outs
        MdtCutOuts cutouts{}; 
    };

    struct defineArgs : public MuonReadoutElement::defineArgs,
                        public parameterBook {};

    MdtReadoutElement(defineArgs&& args);

    const parameterBook& getParameters() const;
    /// Overload from the ActsTrk::IDetectorElement
    ActsTrk::DetectorType detectorType() const override final {
        return ActsTrk::DetectorType::Mdt;
    }
    /// Overload from the Acts::DetectorElement (2 * halfheight)
    double thickness() const override final;
    /// Overload from the Acts::DetectorElement (dummy implementation)
    const Acts::Surface& surface() const override final;
    Acts::Surface& surface() override final;

    StatusCode initElement() override final;
    /// Returns the multi layer of the MdtReadoutElement
    int multilayer() const;

    /// Returns the number of tube layer
    unsigned int numLayers() const;
    /// Returns the number of tubes per layer
    unsigned int numTubesInLay() const;

    /// Transforms the idenfier hash into a tube number ranging from (0-
    /// numTubesInLay()-1)
    static unsigned int tubeNumber(const IdentifierHash& hash);
    /// Transforms the identifier hash into a layer number ranging from
    /// (0-numLayers()-1)
    static  unsigned int layerNumber(const IdentifierHash& hash);
    /// Transform the layer and tube number to the measurementHash
    static IdentifierHash measurementHash(unsigned int layerNumber, unsigned int tubeNumber);


    /// Constructs the identifier hash from the full measurement Identifier. The
    /// hash is always defined w.r.t the specific detector element and used to
    /// access the information in memory quickly
    IdentifierHash measurementHash(const Identifier& measId) const override final;
    /// Transforms the Identifier into a layer hash
    IdentifierHash layerHash(const Identifier& measId) const override final;

    /// Converts the measurement hash back to the full Identifier
    Identifier measurementId(const IdentifierHash& measHash) const override final;

    /// States whether the chamber is built into the barrel or not
    bool isBarrel() const;
    /// Returns the inner tube radius
    double innerTubeRadius() const;
    /// Adds the thickness of the tube wall onto the radius
    double tubeRadius() const;
    
    /// Returns the global position of the tube center. 
    Amg::Vector3D globalTubePos(const ActsGeometryContext& ctx, 
                                const Identifier& measId) const;
    
    Amg::Vector3D globalTubePos(const ActsGeometryContext& ctx,
                                const IdentifierHash& hash) const;
    /// Returns the global position of the readout card
    Amg::Vector3D readOutPos(const ActsGeometryContext& ctx,
                             const Identifier& measId) const;

    Amg::Vector3D readOutPos(const ActsGeometryContext& ctx,
                             const IdentifierHash& measId) const;
    
    /// Returns the Mdt tube length including the dead areas of the endplugs & dead length
    double tubeLength(const IdentifierHash& hash) const;

   private:
        /// Returns the tube position in the chamber coordinate frame
        Amg::Vector3D localTubePos(const IdentifierHash& hash) const;
        /// Returns the transformation to go into the chamber layer
        ///  x-axis: Parallel to the wire layer
        ///  y-axis: Along the wire
        ///  z-axis: Pointing outwards the chamber
        Amg::Transform3D toChamberLayer(const IdentifierHash& hash) const;
        /// Returns the transformation into the rest frame of the tube
        /// x-axis: Pointing parallel to the wire layer 
        /// z-axis: Pointing along the wire
        Amg::Transform3D toTubeFrame(const IdentifierHash& hash) const;

        bool m_init{false};
        parameterBook m_pars{};
        const MdtIdHelper& m_idHelper{idHelperSvc()->mdtIdHelper()};
        /// Identifier index of the multilayer (1-2)
        int m_stML{m_idHelper.multilayer(identify())};
        /// Flag defining whether the chamber is barrel or not
        bool m_isBarrel{m_idHelper.isBarrel(identify())};
};

std::ostream& operator<<(
    std::ostream& ostr, const MuonGMR4::MdtReadoutElement::parameterBook& pars);
}  // namespace MuonGMR4

#include <MuonReadoutGeometryR4/MdtReadoutElement.icc>
#endif
