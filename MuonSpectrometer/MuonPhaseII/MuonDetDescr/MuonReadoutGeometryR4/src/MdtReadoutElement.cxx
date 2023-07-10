/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <AthenaBaseComps/AthCheckMacros.h>
#include <Acts/Surfaces/PlaneSurface.hpp>
using namespace ActsTrk;

namespace MuonGMR4 {
std::ostream& operator<<(
    std::ostream& ostr,
    const MuonGMR4::MdtReadoutElement::parameterBook& pars) {
    ostr << std::endl;
    ostr << " //  tube half-length (min/max): "<<pars.shortHalfX<<"/"<<pars.longHalfX<<", chamber width "<<
        pars.halfY<<", multilayer height: "<<pars.halfHeight;
    ostr << " // Number of tube layers " << pars.tubeLayers.size()<< std::endl;
    ostr << " // Tube pitch: " << pars.tubePitch
         << " wall thickness: " << pars.tubeWall
         << " inner radius: " << pars.tubeInnerRad << std::endl;
    for (const MdtTubeLayer& layer : pars.tubeLayers) {
         ostr << "//   **** "<< Amg::toString(layer.tubeTransform(0).linear(),3)<<std::endl;
    }
    return ostr;
}
MdtReadoutElement::MdtReadoutElement(defineArgs&& args)
    : MuonReadoutElement(std::move(args)),
      m_pars{std::move(args)} {
}
const MdtReadoutElement::parameterBook& MdtReadoutElement::getParameters() const {return m_pars;}
Identifier MdtReadoutElement::measurementId(
    const IdentifierHash& measHash) const {
    return m_idHelper.channelID(identify(), multilayer(),
                                layerNumber(measHash) + 1,
                                tubeNumber(measHash) + 1);
}
StatusCode MdtReadoutElement::initElement() {
  if (m_init) return StatusCode::SUCCESS;
  /// First check whether we're having tubes
  if (!numLayers() || !numTubesInLay()) {
     ATH_MSG_FATAL("The readout element "<< idHelperSvc()->toStringDetEl(identify())<<" has no tubes. Please check "<<std::endl<<m_pars);
     return StatusCode::FAILURE;
  }
  if (m_pars.tubePitch<=tubeRadius()) {
     ATH_MSG_FATAL("The tubes of "<<idHelperSvc()->toStringDetEl(identify())<<" will fall together on a single point. Please check "<<std::endl<<m_pars);
     return StatusCode::FAILURE;
  }
  /// Coordinate system of the trapezoid is in the center while the tubes are defined 
  /// w.r.t. to the chamber edge. Move first tube into the proper position
  for (unsigned int lay =1 ; lay <= numLayers() ; ++lay){
     /// Cache the transformations to the chamber layers
     ATH_CHECK(insertTransform(measurementHash(lay,0), 
                [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                    return toStation(store) * toChamberLayer(hash); 
                }));
    /// Cache the transformations to the tube layers
    for (unsigned int tube = 1; tube <= numTubesInLay(); ++ tube) {
      ATH_CHECK(insertTransform(measurementHash(lay,tube),
                [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                    return toStation(store) * toTubeFrame(hash); 
                }));
    }
  }
  m_init = true;
  return StatusCode::SUCCESS;
}
const Acts::Surface& MdtReadoutElement::surface() const{
   static const std::shared_ptr<Acts::Surface> dummy{Acts::Surface::makeShared<Acts::PlaneSurface>(Amg::Vector3D::UnitX(), 
                                                                                                   Amg::Vector3D::UnitY())};
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<"I am a dummy method ");
   return *dummy;
}

Acts::Surface& MdtReadoutElement::surface() {
   std::shared_ptr<Acts::Surface> dummy{Acts::Surface::makeShared<Acts::PlaneSurface>(Amg::Vector3D::UnitX(), 
                                                                                      Amg::Vector3D::UnitY())};
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<"I am a dummy method ");
   return *dummy;
}

Amg::Vector3D MdtReadoutElement::globalTubePos(const ActsGeometryContext& ctx,
                                const IdentifierHash& hash) const {
   return localToGlobalTrans(ctx) * localTubePos(hash);
}

Amg::Vector3D MdtReadoutElement::localTubePos(const IdentifierHash& hash) const {
  return toTubeFrame(hash).translation();
}
Amg::Vector3D MdtReadoutElement::readOutPos(const ActsGeometryContext& ctx,
                                const IdentifierHash& hash) const {
   const unsigned int layer = layerNumber(hash);
   const unsigned int tube  = tubeNumber(hash);
   const MdtTubeLayer& zeroT{m_pars.tubeLayers[layer]};
   const double length = -zeroT.tubeHalfLength(tube);
   return localToGlobalTrans(ctx) * zeroT.tubeTransform(tube)*(length * Amg::Vector3D::UnitZ());
}

Amg::Transform3D MdtReadoutElement::toChamberLayer(const IdentifierHash& hash) const {   
   const unsigned int layer = layerNumber(hash);
   const MdtTubeLayer& zeroT{m_pars.tubeLayers[layer]};
   return zeroT.layerTransform();
}
Amg::Transform3D MdtReadoutElement::toTubeFrame(const IdentifierHash& hash) const {
   const unsigned int layer = layerNumber(hash);
   const unsigned int tube = tubeNumber(hash);
   const MdtTubeLayer& zeroT{m_pars.tubeLayers[layer]};
   return zeroT.tubeTransform(tube);  
}
double MdtReadoutElement::activeTubeLength(const IdentifierHash& hash) const {
   const unsigned int layer = layerNumber(hash);
   const unsigned int tube = tubeNumber(hash);
   const MdtTubeLayer& zeroT{m_pars.tubeLayers[layer]}; 
   return 2. * zeroT.tubeHalfLength(tube);
}
double MdtReadoutElement::tubeLength(const IdentifierHash& hash) const {
  return activeTubeLength(hash) + 2.*m_pars.deadLength;
}
double MdtReadoutElement::wireLength(const IdentifierHash& hash) const {
   return tubeLength(hash) - 2.*m_pars.endPlugLength;
}
        
}  // namespace MuonGMR4
