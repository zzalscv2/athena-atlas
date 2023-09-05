
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsMuonChamberTool.h"
///
#include <MuonReadoutGeometryR4/StringUtils.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <Acts/Geometry/CuboidVolumeBounds.hpp>


namespace {
    constexpr double tolerance = 10 * Gaudi::Units::micrometer;
}
namespace MuonGMR4{
  using defineArgs = MuonChamber::defineArgs;
  using ReadoutSet = MuonChamber::ReadoutSet;

  ActsMuonChamberTool::ActsMuonChamberTool( const std::string& type, 
                                                            const std::string& name, 
                                                            const IInterface* parent ):
        AthAlgTool{type,name, parent} {
    declareInterface<IActsMuonChamberTool>(this);
  }
  
  StatusCode ActsMuonChamberTool::initialize() {    
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    ATH_CHECK(m_geoUtilTool.retrieve());     
    return StatusCode::SUCCESS;
  }
  ChamberSet ActsMuonChamberTool::buildChambers() const {
     ChamberSet chambers{};
     ActsGeometryContext gctx{};
     /// First access the detector manager to ask for the list of all readout elements
     std::vector<const MuonReadoutElement*> allElements = m_detMgr->getAllReadoutElements();
     /// Next loop over all volumes and group them by their mother
     std::map<PVConstLink, ReadoutSet> chamberConstituents{};
     for (const MuonReadoutElement* readOut: allElements) {
        chamberConstituents[readOut->getMaterialGeom()->getParent()].push_back(readOut);
     }

     for (auto& [motherVol, readoutEles] : chamberConstituents){
        /// Sort the chambers within the mother such that the one that's closest
        /// to the IP comes first (The x-axis points outwards R or Z)
        
        const Amg::Transform3D motherTrans = motherVol->getX().inverse();        
        std::sort(readoutEles.begin(),readoutEles.end(), 
                    [&](const MuonReadoutElement*a, const MuonReadoutElement* b) {
                      const Amg::Vector3D aPos = motherTrans * a->center(gctx);
                      const Amg::Vector3D bPos = motherTrans * b->center(gctx);
                      if (std::abs(aPos.x() - bPos.x()) > tolerance) {
                          return aPos.x() < bPos.x();
                      }
                      return aPos.z() < bPos.z(); 
                });
        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream sstr{};
            for (const MuonReadoutElement* readOut: readoutEles) {
                sstr<<"*** "<<m_idHelperSvc->toString(readOut->identify())
                    <<" "<<Amg::toString(motherTrans * readOut->center(gctx))<<std::endl;                
            }
            ATH_MSG_VERBOSE(m_geoUtilTool->dumpVolume(motherVol)<<std::endl<<
                            "Associated readout elements: "<<std::endl<<sstr.str());
        }
        const MuonReadoutElement* primRE = readoutEles[0];
        /// Origin of the volume is shifted to the center of the edge closest to the IP
        const GeoShape* shape = m_geoUtilTool->extractShape(motherVol);
        if (!shape) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" There is no shape for chamber "
                                  <<m_idHelperSvc->toStringChamber(primRE->identify()));
            throw std::logic_error("Shapeless volume");
        }
        double xMin{0.}, xMax{0.}, yMin{0.}, yMax{0.}, zMin{0.}, zMax{0.};
        /// The shape knows how to find its smallest surrouding box. 
        /// Function deployed with GeoModel 4.5. Keep it here for the moment.
        // shape->extent(xMin, xMax, yMin, yMax, zMin, zMax);        
        defineArgs define{};
        define.halfX = (xMax - xMin) /2.;
        define.halfY = (yMax - yMin) /2.;
        define.halfZ = (zMax - zMin) /2.;
        const Amg::Vector3D boxCenter {xMin + define.halfX, 
                                       yMin + define.halfY, 
                                       zMin + define.halfZ};
        define.readoutEles = std::move(readoutEles);
        ATH_MSG_DEBUG("Chamber "<<m_idHelperSvc->toStringChamber(primRE->identify())<<" has volume "
                                <<m_geoUtilTool->dumpShape(shape)<<". Extracted minimal box at "
                                <<Amg::toString(boxCenter, 2)<<" with dimensions (halfX/ halfY/ halfZ): "
                                <<define.halfX<<"/"<<define.halfY<<"/"<<define.halfZ);
        
        define.centerTrans = primRE->globalToLocalTrans(gctx) * motherTrans.inverse() * Amg::Translation3D{boxCenter};
        MuonChamber muonChamb{std::move(define)};
        chambers.insert(std::move(muonChamb));
    }
    return chambers;
  }

  

}