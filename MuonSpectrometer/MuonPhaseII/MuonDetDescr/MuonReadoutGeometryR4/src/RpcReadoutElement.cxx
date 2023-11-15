/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <AthenaBaseComps/AthCheckMacros.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <optional>
#ifndef SIMULATIONBASE
#  include "Acts/Surfaces/RectangleBounds.hpp"
#endif
using namespace ActsTrk;

namespace MuonGMR4 {
using parameterBook = RpcReadoutElement::parameterBook;
std::ostream& operator<<(std::ostream& ostr, const parameterBook& pars) {
   ostr<<"chamber width/length/thickness [mm]: "<<(2.*pars.halfWidth)<<"/";
   ostr<<(2.*pars.halfLength)<<"/"<<(2.*pars.halfThickness)<<std::endl;
   if (pars.etaDesign) ostr<<"Eta strips: "<<(*pars.etaDesign)<<std::endl;
   if (pars.phiDesign) ostr<<"Phi strips: "<<(*pars.phiDesign)<<std::endl;   
   return ostr;
}

RpcReadoutElement::RpcReadoutElement(defineArgs&& args)
    : MuonReadoutElement(std::move(args)),
      m_pars{std::move(args)} {
}

const parameterBook& RpcReadoutElement::getParameters() const { return m_pars; }

StatusCode RpcReadoutElement::initElement() {    
    ATH_MSG_DEBUG("Parameter book "<<parameterBook());
    if (m_pars.layers.empty()) {
       ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" doesn't have any layers defined");
       return StatusCode::FAILURE;
    }
#ifndef SIMULATIONBASE
    ATH_CHECK(planeSurfaceFactory(geoTransformHash(), m_pars.layerBounds->make_bounds(m_pars.halfWidth, 
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
      ATH_CHECK(planeSurfaceFactory(layHash, m_pars.layerBounds->make_bounds(design.halfWidth(),
                                                                             design.shortHalfHeight())));
#endif
    }
    m_gasThickness = (chamberStripPos(createHash(1, 2, 1, false)) - 
                      chamberStripPos(createHash(1, 1, 1, false))).mag();
#ifndef SIMULATIONBASE
    m_pars.layerBounds.reset();
#endif
    return StatusCode::SUCCESS;
}

Amg::Transform3D RpcReadoutElement::fromGapToChamOrigin(const IdentifierHash& hash) const{
   unsigned int layIdx = static_cast<unsigned int>(hash);
   if (layIdx < m_pars.layers.size()) return m_pars.layers[layIdx].toOrigin();
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.layers.size());
   return Amg::Transform3D::Identity();
}

Amg::Vector3D RpcReadoutElement::stripPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   unsigned int layIdx = static_cast<unsigned int>(lHash);
   if (layIdx < m_pars.layers.size()) {
      return localToGlobalTrans(ctx, lHash) * m_pars.layers[layIdx].localStripPos(stripNumber(measHash));
   }
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.layers.size());
   return Amg::Vector3D::Zero();
}
Amg::Vector3D RpcReadoutElement::rightStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const{
    const IdentifierHash lHash = layerHash(measHash);
    unsigned int layIdx = static_cast<unsigned int>(lHash);
    if (layIdx < m_pars.layers.size()) {
       return localToGlobalTrans(ctx, lHash) * m_pars.layers[layIdx].localStripLeftEdge(stripNumber(measHash));
    }
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.layers.size());
    return Amg::Vector3D::Zero();
}
Amg::Vector3D RpcReadoutElement::leftStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const {
    const IdentifierHash lHash = layerHash(measHash);
    unsigned int layIdx = static_cast<unsigned int>(lHash);
    if (layIdx < m_pars.layers.size()) {
       return localToGlobalTrans(ctx, lHash) * m_pars.layers[layIdx].localStripRightEdge(stripNumber(measHash));
    }
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.layers.size());
    return Amg::Vector3D::Zero();
}

Amg::Vector3D RpcReadoutElement::chamberStripPos(const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   unsigned int layIdx = static_cast<unsigned int>(lHash);
   if (layIdx < m_pars.layers.size()) {
      return  m_pars.layers[layIdx].stripPosition(stripNumber(measHash));
   }
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.layers.size());
   return Amg::Vector3D::Zero();
}

}