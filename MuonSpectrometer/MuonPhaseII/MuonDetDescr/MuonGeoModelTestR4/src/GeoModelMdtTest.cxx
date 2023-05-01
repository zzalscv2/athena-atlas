
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelMdtTest.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include <fstream>
namespace MuonGMR4{
GeoModelMdtTest::GeoModelMdtTest(const std::string& name, ISvcLocator* pSvcLocator):
AthHistogramAlgorithm(name,pSvcLocator) {}

StatusCode GeoModelMdtTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    /// Disable the tracking geometry tool. We'll need it later
    ATH_CHECK(m_trackingGeometryTool.retrieve(DisableTool{true}));
    
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    for (const std::string& testCham : m_selectStat){
       if (testCham.size() !=6 ){
          ATH_MSG_FATAL("Wrong format give "<<testCham);
          return StatusCode::FAILURE;
       }
       /// Example string BIL1A3
       std::string statName = testCham.substr(0,3);
       unsigned int statEta = std::atoi(testCham.substr(3,1).c_str()) *(testCham[4] == 'A' ? 1 : -1);
       unsigned int statPhi = std::atoi(testCham.substr(5,1).c_str());
       bool is_valid{false};
       const Identifier eleId = id_helper.elementID(statName, statEta, statPhi, is_valid);
       if (!is_valid) {
          ATH_MSG_FATAL("Failed to deduce a station name for "<<statName);
          return StatusCode::FAILURE;
       }
       m_testStations.insert(eleId);
       /// Add the second multilayer if possible
       const Identifier secMl = id_helper.multilayerID(eleId,2,is_valid);
       if (is_valid) m_testStations.insert(secMl);    
    }
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}

StatusCode GeoModelMdtTest::execute() {
    
    ActsGeometryContext gctx{};
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    std::stringstream sstr{};
    for (const Identifier& test_me : m_testStations) {
           const int ml = id_helper.multilayer(test_me);
           const std::string detStr = m_idHelperSvc->toStringDetEl(test_me);
           ATH_MSG_ALWAYS("Test retrieval of Mdt detector element "<<detStr);
           const MdtReadoutElement* reElement = m_detMgr->getMdtReadoutElement(test_me);
           if (!reElement) {
              ATH_MSG_DEBUG("Detector element is invalid");
              continue;
           }
           /// Check that we retrieved the proper readout element
           if (reElement->identify() != test_me) {
              ATH_MSG_FATAL("Expected to retrieve "<<detStr<<". But got instead "<<m_idHelperSvc->toStringDetEl(reElement->identify()));
              return StatusCode::FAILURE;
           }
           sstr<<"######################################################################################"<<std::endl;
           sstr<<"Found Readout element "<<detStr<<std::endl;
           sstr<<"######################################################################################"<<std::endl;
           /// location
           const Amg::Transform3D& globToLocal{reElement->globalToLocalTrans(gctx)};
           const Amg::Transform3D& localToGlob{reElement->localToGlobalTrans(gctx)};
           sstr<<reElement->getParameters()<<std::endl;
           sstr<<"Displacement:       "<<Amg::toString(localToGlob.translation(),3)<<std::endl;
           sstr<<"x-Axis orientation: "<<Amg::toString(localToGlob.linear()*Amg::Vector3D::UnitX(),3)<<std::endl;
           sstr<<"y-Axis orientation: "<<Amg::toString(localToGlob.linear()*Amg::Vector3D::UnitY(),3)<<std::endl;
           sstr<<"z-Axis orientation: "<<Amg::toString(localToGlob.linear()*Amg::Vector3D::UnitZ(),3)<<std::endl;
           ///Closure test that the transformations actually close
           const Amg::Transform3D transClosure = globToLocal * localToGlob;
           for (Amg::Vector3D axis :{Amg::Vector3D::UnitX(),Amg::Vector3D::UnitY(),Amg::Vector3D::UnitZ()}){
              const double closure_mag = std::abs( (transClosure*axis).dot(axis) - 1.);
              if (closure_mag > std::numeric_limits<float>::epsilon() ) {
                 ATH_MSG_FATAL("Closure test failed for "<<detStr<<" and axis "<<Amg::toString(axis)
                 <<". Ended up with "<< Amg::toString(transClosure*axis) );
                 return StatusCode::FAILURE;
              }
           }
           for (unsigned int lay = 1 ; lay <= reElement->numLayers() ; ++lay ) {
              for (unsigned int tube = 1; tube <=reElement->numTubesInLay(); ++tube ){
                  const Identifier tube_id = id_helper.channelID(test_me,ml,lay,tube);
                  /// Test the forward -> backward conversion
                  const IdentifierHash measHash = reElement->measurementHash(tube_id);
                  const Identifier cnv_tube_id  = reElement->measurementId(measHash);
                  if (tube_id != cnv_tube_id) {
                     ATH_MSG_FATAL("Failed to convert "<<m_idHelperSvc->toString(tube_id)<<" back and forth "<<m_idHelperSvc->toString(cnv_tube_id));
                     return StatusCode::FAILURE;
                  }
                  if (tube == 0){
                     const IdentifierHash layHash = reElement->layerHash(tube_id);
                     const Amg::Transform3D& layTrans{reElement->localToGlobalTrans(gctx, layHash)};
                     const Amg::Transform3D& tubeTrans{reElement->localToGlobalTrans(gctx, measHash)};
                     sstr<<"Layer"<<lay<<" displacement        : "<<Amg::toString(layTrans.translation(),3)<<std::endl;;
                     sstr<<"Layer"<<lay<<" x-Axis orientation  : "<<Amg::toString(layTrans.linear()*Amg::Vector3D::UnitX(), 3)<<std::endl;
                     sstr<<"Layer"<<lay<<" y-Axis orientation  : "<<Amg::toString(layTrans.linear()*Amg::Vector3D::UnitY(), 3)<<std::endl;
                     sstr<<"Layer"<<lay<<" z-Axis orientation  : "<<Amg::toString(layTrans.linear()*Amg::Vector3D::UnitZ(), 3)<<std::endl;
                     sstr<<"Tube "<<tube<<" displacement        : "<<Amg::toString(tubeTrans.translation(),3)<<std::endl;;
                     sstr<<"Tube "<<tube<<" x-Axis orientation  : "<<Amg::toString(tubeTrans.linear()*Amg::Vector3D::UnitX(), 3)<<std::endl;
                     sstr<<"Tube "<<tube<<" y-Axis orientation  : "<<Amg::toString(tubeTrans.linear()*Amg::Vector3D::UnitY(), 3)<<std::endl;
                     sstr<<"Tube "<<tube<<" z-Axis orientation  : "<<Amg::toString(tubeTrans.linear()*Amg::Vector3D::UnitZ(), 3)<<std::endl;
                  }
                  sstr<<" *** ("
                      <<std::setfill('0') << std::setw(2)<<lay<<", "
                      <<std::setfill('0') << std::setw(3)<<tube<<")    "
                      <<Amg::toString(reElement->readOutPos(gctx, tube_id), 3)<< " / "
                      <<Amg::toString(globToLocal*reElement->readOutPos(gctx, tube_id), 3)<<"  ---> "
                      <<Amg::toString(reElement->globalTubePos(gctx, tube_id), 3)<< " / "
                      <<Amg::toString(globToLocal*reElement->globalTubePos(gctx, tube_id), 3)
                      <<std::endl;
              }
           }
        
    }

    if (!m_outputTxt.empty()) {
       std::fstream outStream{m_outputTxt, std::ios_base::out};
       if (!outStream.good()){
          ATH_MSG_FATAL("Failed to create output file "<<m_outputTxt);
          return StatusCode::FAILURE;
       }
       outStream<<sstr.str();

    }
   return StatusCode::SUCCESS;
}
}
