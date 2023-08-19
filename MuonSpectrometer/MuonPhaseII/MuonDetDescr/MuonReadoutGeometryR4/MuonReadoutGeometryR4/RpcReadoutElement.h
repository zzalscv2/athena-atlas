/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_RPCREADOUTELEMENT_H
#define MUONREADOUTGEOMETRYR4_RPCREADOUTELEMENT_H

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/StripDesign.h>
#include <MuonReadoutGeometryR4/StripLayer.h>


namespace MuonGMR4 {

class RpcReadoutElement : public MuonReadoutElement {

   public:
    
    /// Set of parameters to describe a RPC chamber
    struct parameterBook {
        double halfX{0.};
        double halfY{0.};
        double halfZ{0.};
        
        
        /// The number of gas gaps (along the radial direction)
        /// in the RPC chamber (2 or 3) 
        unsigned int nGasGaps{0};
        /// Each gas gap is usually subdivided into 2 phi panels
        /// which is actually the sector granularity of the Rpc trigger
        int nGapsInPhi{0};
        
        bool hasPhiStrips{true};
        std::vector<StripLayer> layers{};

        StripDesign etaDesign{};
        StripDesign phiDesign{};

    };

    struct defineArgs : public MuonReadoutElement::defineArgs,
                        public parameterBook {};

    RpcReadoutElement(defineArgs&& args);

    const parameterBook& getParameters() const;
    /// Overload from the ActsTrk::IDetectorElement
    ActsTrk::DetectorType detectorType() const override final {
        return ActsTrk::DetectorType::Rpc;
    }
    /// Overload from the Acts::DetectorElement (2 * halfheight)
    double thickness() const override final;
    /// Overload from the Acts::DetectorElement (dummy implementation)
    const Acts::Surface& surface() const override final;
    Acts::Surface& surface() override final;

    StatusCode initElement() override final;

    /// Returns the doublet Z field of the MuonReadoutElement identifier
    unsigned int doubletZ() const;
    /// Returns the doublet R field of the MuonReadoutElement identifier
    unsigned int doubletR() const;
    /// Returns the doublet Phi field of the MuonReadoutElement identifier
    int doubletPhi() const;

    /// Returns the number of gasgaps described by this ReadOutElement (usally 2 or 3)
    int nGasGaps() const;
    /// Returns the number of phi panels
    int nPhiPanels() const;


    unsigned int numEtaStrips() const;
    unsigned int numPhiStrips() const;


    Amg::Vector3D stripPosition(const ActsGeometryContext& ctx, const Identifier& measId) const;
    Amg::Vector3D stripPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const;



    /// Constructs the identifier hash from the full measurement Identifier. The
    /// hash is always defined w.r.t the specific detector element and used to
    /// access the information in memory quickly
    IdentifierHash measurementHash(const Identifier& measId) const override final;
    
    IdentifierHash layerHash(const Identifier& measId) const override final;
    IdentifierHash layerHash(const IdentifierHash& measHash) const;

    Identifier measurementId(const IdentifierHash& measHash) const override final;

   private:
        IdentifierHash createHash(const int strip, const int gasGap, const int doubPhi, const bool measPhi) const;

        unsigned int stripNumber(const IdentifierHash& measHash) const;
        unsigned int gasGapNumber(const IdentifierHash& measHash) const;
        unsigned int doubletPhiNumber(const IdentifierHash& measHash) const;
        static bool measuresPhi(const IdentifierHash& measHash);

        Amg::Transform3D fromGapToChamOrigin(const IdentifierHash& layerHash) const;


        parameterBook m_pars{};
        const RpcIdHelper& m_idHelper{idHelperSvc()->rpcIdHelper()};
        /// doublet R -> 1: chamber is mounted below the Mdts 
        //            -> 2: chamber is mounted on top of the Mdts
        const int m_doubletR{m_idHelper.doubletR(identify())};
        /// Associated doublet Z (Ranges from  1-3)
        /// If doubletZ is 3, there's generally the possibility that the module is
        /// additionally split according to doublet Phi
        const int m_doubletZ{m_idHelper.doubletZ(identify())};
        const int m_doubletPhi{m_idHelper.doubletPhi(identify())};

        /// Auxillary variables to translate the Identifier to a measurement hash and back
        const unsigned int m_hashShiftDbl{m_pars.hasPhiStrips ? 1u :0u};
        const unsigned int m_hashShiftGap{m_hashShiftDbl + (nPhiPanels() <= m_doubletPhi ? 0u : 1u)};
        const unsigned int m_hashShiftStr{m_hashShiftGap + MuonGM::maxBit(nGasGaps()) + 1};
       
};
std::ostream& operator<<(std::ostream& ostr, const RpcReadoutElement::parameterBook& pars);
}  // namespace MuonGMR4


#include <MuonReadoutGeometryR4/RpcReadoutElement.icc>
#endif
