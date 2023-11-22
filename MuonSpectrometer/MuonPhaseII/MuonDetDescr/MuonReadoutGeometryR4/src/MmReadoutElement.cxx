/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <AthenaBaseComps/AthCheckMacros.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <optional>

#ifndef SIMULATIONBASE
#   include "Acts/Surfaces/TrapezoidBounds.hpp"
#endif

using namespace ActsTrk;

namespace MuonGMR4 {
using parameterBook = MmReadoutElement::parameterBook;
std::ostream& operator<<(std::ostream& ostr, const parameterBook& pars) {
   ostr<<"chamber shortWidth/longWidth/length [mm]: "<<(2.*pars.halfShortWidth)<<"/";
   ostr<<(2.*pars.halfLongWidth)<<"/"<<(2.*pars.halfLength)<<std::endl;
   return ostr;
}

MmReadoutElement::MmReadoutElement(defineArgs&& args)
    : MuonReadoutElement(std::move(args)),
      m_pars{std::move(args)} {
}

const parameterBook& MmReadoutElement::getParameters() const { return m_pars; }

StatusCode MmReadoutElement::initElement() {    
    ATH_MSG_DEBUG("Parameter book "<<parameterBook());
    if (m_pars.layers.empty()) {
       ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" doesn't have any layers defined");
       return StatusCode::FAILURE;
    }
#ifndef SIMULATIONBASE
    ATH_CHECK(planeSurfaceFactory(geoTransformHash(), m_pars.layerBounds->make_bounds(m_pars.halfShortWidth, 
                                                                            m_pars.halfLongWidth, 
                                                                            m_pars.halfLength)));
#endif
    for (unsigned int layer = 0; layer < m_pars.layers.size(); ++layer) {
      IdentifierHash layHash{layer};
      if (m_pars.layers[layer].hash() != layHash) {
         ATH_MSG_FATAL("Layer "<<m_pars.layers[layer]<<" has a very strange hash. Expect "<<layer);
         return StatusCode::FAILURE;
      }
      ATH_CHECK(insertTransform(layHash, 
                                 [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                                    return toStation(store) * fromGapToChamOrigin(hash); 
                                 }));
#ifndef SIMULATIONBASE
      const StripDesign& design{m_pars.layers[layer].design()};
      ATH_CHECK(planeSurfaceFactory(layHash, m_pars.layerBounds->make_bounds(design.shortHalfHeight(),
                                                                                   design.longHalfHeight(),
                                                                                   design.halfWidth())));
#endif
    }
    // m_gasThickness = (chamberStripPos(createHash(1, 2, 1, false)) - 
    //                   chamberStripPos(createHash(1, 1, 1, false))).mag();
#ifndef SIMULATIONBASE
    m_pars.layerBounds.reset();
#endif
    return StatusCode::SUCCESS;
}

Amg::Transform3D MmReadoutElement::fromGapToChamOrigin(const IdentifierHash& layHash) const {
      const unsigned layIdx{static_cast<unsigned>(layHash)};
      return m_pars.layers[layIdx].toOrigin();
}

}