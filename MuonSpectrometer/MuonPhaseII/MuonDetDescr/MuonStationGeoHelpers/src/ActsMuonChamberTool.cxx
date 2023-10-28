
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsMuonChamberTool.h"
///
#include <GaudiKernel/SystemOfUnits.h>


namespace {
    constexpr double tolerance = 10 * Gaudi::Units::micrometer;
}
using namespace ActsTrk;
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
        /// Sort the chambers within the mother such that mdts are sorted first by distance to IP
        /// then the rpcs are sorted by distance to IP
        const Amg::Transform3D motherTrans = motherVol->getX().inverse();        
        std::sort(readoutEles.begin(),readoutEles.end(), 
                    [&motherTrans, &gctx](const MuonReadoutElement*a, const MuonReadoutElement* b) {
                      if (a->detectorType() != b->detectorType()) {
                        return static_cast<unsigned> (a->detectorType()) < static_cast<unsigned> (b->detectorType());
                      }
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
        double xMin{10e5}, xMax{-10e5}, yMinS{10e5}, yMaxS{-10e5}, yMinL{10.e5}, yMaxL{-10.e5}, zMin{10e5}, zMax{-10e5};
        defineArgs define{};

        /// Record the thickness for the top and bottom of the chamber
        for(const MuonReadoutElement* ele: readoutEles){
          const Amg::Vector3D center{motherTrans * ele->center(gctx)};
          ATH_MSG_DEBUG("Center of readout element: "<<m_idHelperSvc->toStringDetEl(ele->identify())<<Amg::toString(center, 2));
          if(ele->detectorType() == ActsTrk::DetectorType::Mdt){
            const MdtReadoutElement* mdtReadoutEle = static_cast<const MdtReadoutElement*>(ele);
            const auto parameters = mdtReadoutEle->getParameters();
            xMax = std::max(center.x() + 2.*parameters.halfHeight, xMax);
            xMin = std::min(center.x(), xMin);
            yMinS = std::min(center.y() - parameters.shortHalfX, yMinS);
            yMaxS = std::max(center.y() + parameters.shortHalfX, yMaxS);
            yMinL = std::min(center.y() - parameters.longHalfX, yMinL);
            yMaxL = std::max(center.y() + parameters.longHalfX, yMaxL);
            zMin =  std::min(center.z(), zMin);
            zMax = std::max(center.z() + 2.*parameters.halfY, zMax);
          } else if(ele->detectorType() == ActsTrk::DetectorType::Rpc){
              const RpcReadoutElement* rpcReadoutEle = static_cast<const RpcReadoutElement*>(ele);
              const auto parameters = rpcReadoutEle->getParameters();
              if(parameters.isUpsideDown) {
                xMax = std::max(center.x(), xMax);
                xMin = std::min(center.x() - 2.* parameters.halfThickness, xMin);
                yMinS = std::min(center.y() + parameters.halfLength, yMinS);
                yMaxS = std::max(center.y() - parameters.halfLength, yMaxS);
                yMinL = yMinS;
                yMaxL = yMaxS;
                zMin =  std::min(center.z(), zMin);
                zMax = std::max(center.z() + 2.*parameters.halfWidth, zMax);
              } else{
                xMax = std::max(center.x() + 2.* parameters.halfThickness, xMax);
                xMin = std::min(center.x(), xMin);
                yMinS = std::min(center.y() - parameters.halfLength, yMinS);
                yMaxS = std::max(center.y() + parameters.halfLength, yMaxS);
                yMinL = yMinS;
                yMaxL = yMaxS;
                zMin =  std::min(center.z(), zMin);
                zMax = std::max(center.z() + 2.*parameters.halfWidth, zMax);
              }
          } else {
              ATH_MSG_FATAL("Only MDT chambers can be used right now.");
              throw std::logic_error("Detector type not supported");
          }
        }
        ATH_MSG_VERBOSE("Chamber dimensions: xMin: "<<xMin<<" xMax: "<<xMax
                      <<" yMinS: "<<yMinS<<" yMaxS: "<<yMaxS<<" yMinL: "<<yMinL<<" yMaxL: "<<yMaxL<<" zMin: "<<zMin<<" zMax: "<<zMax);
        constexpr double tolerance = 0.1 * Gaudi::Units::mm;
        define.halfX = ((xMax - xMin) / 2.);
        define.halfYShort = ((yMaxS - yMinS) / 2.);
        define.halfYLong = ((yMaxL - yMinL) / 2.);
        define.halfZ = ((zMax - zMin) / 2.);

        const Amg::Vector3D boxCenter {xMin + define.halfX, 
                                       yMinS + define.halfYShort, 
                                       zMin + define.halfZ};
        define.halfX += tolerance;
        define.halfYShort += tolerance;
        define.halfYLong += tolerance;
        define.halfZ += tolerance;
        std::swap(define.halfX, define.halfZ);
        define.readoutEles = std::move(readoutEles);
        ATH_MSG_DEBUG("Chamber "<<m_idHelperSvc->toStringChamber(primRE->identify())<<" has volume "
                                <<m_geoUtilTool->dumpShape(shape)<<". Extracted minimal box at "
                                <<Amg::toString(boxCenter, 2)<<" with dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                <<define.halfYShort<<"/"<<define.halfYLong<<"/"<<define.halfX<<"/"<<define.halfZ);
        
        define.centerTrans = primRE->globalToLocalTrans(gctx) * motherTrans.inverse() * Amg::Translation3D{boxCenter} * Amg::getRotateZ3D(-M_PI_2) * Amg::getRotateX3D(M_PI_2);
        MuonChamber muonChamb{std::move(define)};
        /// Internal consistency check that we did not mess up the transformations
        ATH_MSG_VERBOSE("Chamber center in global frame "<<Amg::toString(motherTrans.inverse() * boxCenter)<<" from the box: "<<Amg::toString(muonChamb.localToGlobalTrans(gctx) * Amg::Vector3D::Zero()));
        chambers.insert(std::move(muonChamb));
    }
    return chambers;
  }

  

}   