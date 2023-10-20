/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <MuonReadoutGeometryR4/StringUtils.h>
#include <AthenaBaseComps/AthCheckMacros.h>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <GaudiKernel/SystemOfUnits.h>
#include <optional>
using namespace ActsTrk;




namespace MuonGMR4 {
using parameterBook = TgcReadoutElement::parameterBook;
std::ostream& operator<<(std::ostream& ostr, const parameterBook& pars) {
   ostr<<"chamber dimensions --- ";
   ostr<<"thickness: "<<pars.halfThickness<<" ";
   ostr<<"height: "<<pars.halfHeight<<" ";
   ostr<<"shortWidth: "<<pars.halfWidthShort<<" ";
   ostr<<"longWidth: "<<pars.halfWidthLong<<" --- ";
   return ostr;
}

TgcReadoutElement::TgcReadoutElement(defineArgs&& args)
    : MuonReadoutElement(std::move(args)),
      m_pars{std::move(args)} {
}

const parameterBook& TgcReadoutElement::getParameters() const { return m_pars; }
StatusCode TgcReadoutElement::initElement() {
    /// Check that the readoutelement has sensor layouts
    bool hasSensor{false};
    for (size_t s = 0; s < m_pars.sensorLayouts.size(); ++s) {
       const StripLayerPtr& layPtr{m_pars.sensorLayouts[s]};
       if (!layPtr) continue;
       if (layPtr->hash() != s) {
          ATH_MSG_FATAL("Layer "<<(*layPtr)<<" has an unexpected hash "<<s);
          return StatusCode::FAILURE;
       }
       hasSensor = true;
    }
    if (!hasSensor) {
         ATH_MSG_FATAL("No active layer is defined for "<<idHelperSvc()->toStringDetEl(identify()));
         return StatusCode::FAILURE;
    }
    if (std::find_if(m_pars.sensorLayouts.begin(), m_pars.sensorLayouts.end(),
                     [](const StripLayerPtr & ptr)->bool{ return ptr; }) == m_pars.sensorLayouts.end()) {
         ATH_MSG_FATAL("No sensor structure is provided to "<<idHelperSvc()->toStringDetEl(identify()));
         return StatusCode::FAILURE;
    }
    for (unsigned int gap = 1; gap <= nGasGaps(); ++gap){
         if (numWireGangs(gap)) {
            ATH_CHECK(insertTransform(constructHash(0, gap, false), 
                                 [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                                    return toStation(store) * fromGapToChamOrigin(hash); 
                                 }));
         }
         if (numStrips(gap)) {
            ATH_CHECK(insertTransform(constructHash(0, gap, true), 
                                 [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                                    return toStation(store) * fromGapToChamOrigin(hash); 
                                 }));
         }
    }
    return StatusCode::SUCCESS;
}

Amg::Transform3D TgcReadoutElement::fromGapToChamOrigin(const IdentifierHash& layHash) const {
      const unsigned layIdx{static_cast<unsigned>(layHash)};
      return m_pars.sensorLayouts[layIdx]->toOrigin();
}
Amg::Vector3D TgcReadoutElement::channelPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const { 
   const StripLayerPtr& layDesign{sensorLayout(gasGapNumber(measHash), isStrip(measHash))};
   if (!layDesign) {
       ATH_MSG_WARNING("The gasGap "<<gasGapNumber(measHash)<<" & strip:"<<isStrip(measHash)<<" is unknown");
       return Amg::Vector3D::Zero();
   }
   return localToGlobalTrans(ctx, layerHash(measHash)) * layDesign->localStripPos(channelNumber(measHash));
}
}