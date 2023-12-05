/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_MMREADOUTELEMENT_H
#define MUONREADOUTGEOMETRYR4_MMREADOUTELEMENT_H

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/StripDesign.h>
#include <MuonReadoutGeometryR4/StripLayer.h>

#ifndef SIMULATIONBASE
#   include "Acts/Surfaces/TrapezoidBounds.hpp"
#endif


namespace MuonGMR4 {

class MmReadoutElement : public MuonReadoutElement {

   public:
    
    /// Set of parameters to describe a RPC chamber
    struct parameterBook {


      ///Trapezoid dimensions of MicroMegas
      
      /// half-thickness along z-axis
      double halfThickness{0.};
      /// width of the lower edge 
      double halfShortWidth{0.};
      /// width of the upper edge
      double halfLongWidth{0.};
      /// length in the radial direction
      double halfHeight{0.};
      /// number of gasGaps
      unsigned int nGasGaps{0};

      std::vector<StripLayer> layers{};
#ifndef SIMULATIONBASE
        ActsTrk::SurfaceBoundSetPtr<Acts::TrapezoidBounds> layerBounds{};
#endif


    };

    struct defineArgs : public MuonReadoutElement::defineArgs,
                        public parameterBook {};

    MmReadoutElement(defineArgs&& args);

    const parameterBook& getParameters() const;
    /// Overload from the ActsTrk::IDetectorElement
    ActsTrk::DetectorType detectorType() const override final {
        return ActsTrk::DetectorType::Mm;
    }

    
    /// Overload from the Acts::DetectorElement (2 * halfheight)
    double thickness() const override final;
    
    /// Returns the multi layer of the element [1-2]
    int multilayer() const;
    /// Returns the height along the z-axis
    double moduleHeight() const;
    /// Returns the width at the short edge
    double moduleWidthS() const;
    /// Returns the width at the top edge
    double moduleWidthL() const;
    /// Returns the module thickness
    double moduleThickness() const;
    /// Returns the number of gas gaps
    unsigned int nGasGaps() const;

    StatusCode initElement() override final;

    /// Constructs the identifier hash from the full measurement Identifier. The
    /// hash is always defined w.r.t the specific detector element and used to
    /// access the information in memory quickly
    IdentifierHash measurementHash(const Identifier& measId) const override final;
    IdentifierHash layerHash(const Identifier& measId) const override final;
    Identifier measurementId(const IdentifierHash& measHash) const override final;

    static IdentifierHash createHash(const int strip, const int gasGap);
   private:
       
    

    static unsigned int gasGapNumber(const IdentifierHash& measHash);
    static unsigned int stripNumber(const IdentifierHash& measHash);

    parameterBook m_pars{};        

    const MmIdHelper& m_idHelper{idHelperSvc()->mmIdHelper()};

    const int m_multilayer{m_idHelper.multilayer(identify())};

    Amg::Transform3D fromGapToChamOrigin(const IdentifierHash& layerHash) const;
       
};
std::ostream& operator<<(std::ostream& ostr, const MmReadoutElement::parameterBook& pars);
}  // namespace MuonGMR4


#include <MuonReadoutGeometryR4/MmReadoutElement.icc>
#endif
