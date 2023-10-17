/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <AthenaBaseComps/AthCheckMacros.h>
#include <Acts/Surfaces/PlaneSurface.hpp>
using namespace ActsTrk;

namespace MuonGMR4 {
std::ostream& operator<<(std::ostream& ostr, const MuonGMR4::sTgcReadoutElement::parameterBook& /*pars*/) {
    return ostr;

}
sTgcReadoutElement::sTgcReadoutElement(defineArgs&& args)
    : MuonReadoutElement(std::move(args)),
      m_pars{std::move(args)} {
}
const sTgcReadoutElement::parameterBook& sTgcReadoutElement::getParameters() const {return m_pars;}
Identifier sTgcReadoutElement::measurementId(
    const IdentifierHash& /*measHash*/) const {
    return Identifier{};
}
StatusCode sTgcReadoutElement::initElement() {
  if (m_init) return StatusCode::SUCCESS;
  m_init = true;
  return StatusCode::SUCCESS;
}

IdentifierHash sTgcReadoutElement::measurementHash(const Identifier& /*measId*/) const { return IdentifierHash{};}
IdentifierHash sTgcReadoutElement::layerHash(const Identifier& /*measId*/) const { return  IdentifierHash{};}

        
}  // namespace MuonGMR4
