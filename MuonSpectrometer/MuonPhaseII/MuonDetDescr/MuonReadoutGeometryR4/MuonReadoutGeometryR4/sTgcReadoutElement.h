/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_STGCREADOUTELEMENT_H
#define MUONREADOUTGEOMETRYR4_STGCREADOUTELEMENT_H

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/MdtTubeLayer.h>

namespace MuonGMR4 {

class sTgcReadoutElement : public MuonReadoutElement {

   public:
    
    /// Set of parameters to describe a MDT chamber
    struct parameterBook {
         unsigned int cake{0};
    };

    struct defineArgs : public MuonReadoutElement::defineArgs,
                        public parameterBook {};

    sTgcReadoutElement(defineArgs&& args);

    const parameterBook& getParameters() const;
    /// Overload from the ActsTrk::IDetectorElement
    ActsTrk::DetectorType detectorType() const override final {
        return ActsTrk::DetectorType::sTgc;
    }
    /// Overload from the Acts::DetectorElement (2 * halfheight)
    double thickness() const override final {return 0.;};
    /// Overload from the Acts::DetectorElement (dummy implementation)
    const Acts::Surface& surface() const override final;
    Acts::Surface& surface() override final;

    StatusCode initElement() override final;
    /// Returns the multi layer of the sTgcReadoutElement
    unsigned int multilayer() const;

    /// Returns the number of tube layer
    unsigned int numLayers() const;

    // static unsigned int layerNumber(const IdentifierHash& hash);

    /// Constructs the identifier hash from the full measurement Identifier. The
    /// hash is always defined w.r.t the specific detector element and used to
    /// access the information in memory quickly
    IdentifierHash measurementHash(const Identifier& measId) const override final;
    /// Transforms the Identifier into a layer hash
    IdentifierHash layerHash(const Identifier& measId) const override final;

    /// Converts the measurement hash back to the full Identifier
    Identifier measurementId(const IdentifierHash& measHash) const override final;



   private:

        bool m_init{false};
        parameterBook m_pars{};
        const sTgcIdHelper& m_idHelper{idHelperSvc()->stgcIdHelper()};
        /// Identifier index of the multilayer (1-2)
        int m_stML{m_idHelper.multilayer(identify())};
};

std::ostream& operator<<(
    std::ostream& ostr, const MuonGMR4::sTgcReadoutElement::parameterBook& pars);
}  // namespace MuonGMR4

#endif
