
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelMdtTest.h"
#include <MuonReadoutGeometryR4/StringUtils.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <fstream>


namespace MuonGMR4{

GeoModelMdtTest::GeoModelMdtTest(const std::string& name, ISvcLocator* pSvcLocator):
AthHistogramAlgorithm(name,pSvcLocator) {}

StatusCode GeoModelMdtTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());    
    ATH_CHECK(m_surfaceProvTool.retrieve());
    /// Prepare the TTree dump
    if (m_dumpTree) ATH_CHECK(m_tree.init(this));

    
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    for (const std::string& testCham : m_selectStat) {
        if (testCham.size() != 6) {
            ATH_MSG_FATAL("Wrong format given " << testCham);
            return StatusCode::FAILURE;
        }
        /// Example string BIL1A3
        std::string statName = testCham.substr(0, 3);
        unsigned int statEta = std::atoi(testCham.substr(3, 1).c_str()) *
                               (testCham[4] == 'A' ? 1 : -1);
        unsigned int statPhi = std::atoi(testCham.substr(5, 1).c_str());
        bool is_valid{false};
        const Identifier eleId =
            id_helper.elementID(statName, statEta, statPhi, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("Failed to deduce a station name for " << testCham);
            return StatusCode::FAILURE;
        }
        m_testStations.insert(eleId);
        /// Add the second multilayer if possible
        const Identifier secMl = id_helper.multilayerID(eleId, 2, is_valid);
        if (is_valid)
            m_testStations.insert(secMl);
    }
    /// Add all stations for testing
    if (m_testStations.empty()){
        for(auto itr = id_helper.detectorElement_begin();
                 itr!= id_helper.detectorElement_end();++itr){
           if (!id_helper.isBMG(*itr)) m_testStations.insert(*itr);
        }
    }
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMdtTest::finalize() {
    if (m_dumpTree) ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMdtTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<ActsGeometryContext> geoContextHandle{m_geoCtxKey, ctx};
    if (!geoContextHandle.isValid()){
      ATH_MSG_FATAL("Failed to retrieve "<<m_geoCtxKey.fullKey());
      return StatusCode::FAILURE;
    }
    const ActsGeometryContext& gctx{**geoContextHandle};

    std::optional<std::fstream> outStream{};
    if (!m_outputTxt.empty()) {
        outStream = std::make_optional<std::fstream>(m_outputTxt, std::ios_base::out);
        if (!outStream->good()) {
            ATH_MSG_FATAL("Failed to create output file " << m_outputTxt);
            return StatusCode::FAILURE;
        }
    }

    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    for (const Identifier& test_me : m_testStations) {
      const int ml = id_helper.multilayer(test_me);
      const std::string detStr = m_idHelperSvc->toStringDetEl(test_me);
      ATH_MSG_DEBUG("Test retrieval of Mdt detector element "<<detStr);
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
      ATH_CHECK(dumpToTree(ctx,gctx,reElement));
      const Amg::Transform3D& globToLocal{reElement->globalToLocalTrans(gctx)};
      const Amg::Transform3D& localToGlob{reElement->localToGlobalTrans(gctx)};
      /// Closure test that the transformations actually close
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
         }
      }
      if (outStream) dumpToFile(ctx, gctx, reElement, *outStream);        
   }
   return StatusCode::SUCCESS;
}
StatusCode GeoModelMdtTest::dumpToTree(const EventContext& ctx,
                                       const ActsGeometryContext& gctx, 
                                       const MdtReadoutElement* readoutEle){

   if (!m_dumpTree) return StatusCode::SUCCESS;
   
   m_stIndex = readoutEle->stationIndex();
   m_stEta = readoutEle->stationEta();
   m_stPhi = readoutEle->stationPhi();
   m_stML = readoutEle->multilayer();
   
   m_numLayers = readoutEle->numLayers();
   m_numTubes = readoutEle->numTubesInLay();
   
   m_tubeRad = readoutEle->innerTubeRadius();
   m_tubePitch = readoutEle->tubePitch();

   /// Dump the local to global transformation of the readout element
   const Amg::Transform3D& transform {readoutEle->getMaterialGeom()->getAbsoluteTransform()};
   m_readoutTransform.push_back(Amg::Vector3D(transform.translation()));
   m_readoutTransform.push_back(Amg::Vector3D(transform.linear()*Amg::Vector3D::UnitX()));
   m_readoutTransform.push_back(Amg::Vector3D(transform.linear()*Amg::Vector3D::UnitY()));
   m_readoutTransform.push_back(Amg::Vector3D(transform.linear()*Amg::Vector3D::UnitZ()));
   
   /// Loop over the tubes
   for (unsigned int lay = 1; lay <= readoutEle->numLayers(); ++lay) {
      for (unsigned int tube = 1; tube <= readoutEle->numTubesInLay(); ++tube) {
         const IdentifierHash measHash{readoutEle->measurementHash(lay,tube)};
         const Amg::Transform3D& tubeTransform{readoutEle->localToGlobalTrans(gctx,measHash)};
         m_tubeLay.push_back(lay);
         m_tubeNum.push_back(tube);         
         m_tubeTransformTran.push_back(Amg::Vector3D(tubeTransform.translation()));
         m_tubeTransformColX.push_back(Amg::Vector3D(tubeTransform.linear()*Amg::Vector3D::UnitX()));
         m_tubeTransformColY.push_back(Amg::Vector3D(tubeTransform.linear()*Amg::Vector3D::UnitY()));
         m_tubeTransformColZ.push_back(Amg::Vector3D(tubeTransform.linear()*Amg::Vector3D::UnitZ()));
         m_roPos.push_back(readoutEle->readOutPos(gctx, measHash));
         m_tubeLength.push_back(readoutEle->tubeLength(measHash));
         m_activeTubeLength.push_back(readoutEle->activeTubeLength(measHash));
         m_wireLength.push_back(readoutEle->wireLength(measHash));
      }
   }

   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
void GeoModelMdtTest::dumpToFile(const EventContext& /*ctx*/,
                                 const ActsGeometryContext& gctx,
                                 const MdtReadoutElement* reElement, 
                                 std::ostream& sstr) {
   const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
   sstr<<"######################################################################################"<<std::endl;
   sstr<<"Found Readout element "<<m_idHelperSvc->toStringDetEl(reElement->identify())<<std::endl;
   sstr<<"######################################################################################"<<std::endl;
   /// location   
   const Amg::Transform3D localToGlob{reElement->localToGlobalTrans(gctx)};
   sstr<<"GeoModel transformation: "<<to_string(localToGlob)<<std::endl;
   sstr<<"Chamber center: "<<to_string(m_surfaceProvTool->chambCenterToGlobal(gctx, 
                                                               reElement->identify()))<<std::endl;
   
   sstr<<reElement->getParameters()<<std::endl;
  
   for (unsigned int lay = 1 ; lay <= reElement->numLayers() ; ++lay ) {
      for (unsigned int tube = 1; tube <=reElement->numTubesInLay(); ++tube ){
         const Identifier tube_id = id_helper.channelID(reElement->identify(),reElement->multilayer(),lay,tube);
         const IdentifierHash measHash = reElement->measurementHash(tube_id);      
         if (tube == 1) {
            const IdentifierHash layHash = reElement->layerHash(tube_id);
            const Amg::Transform3D& layTrans{reElement->localToGlobalTrans(gctx, layHash)};
            sstr<<"Layer "<<lay<<" : "<<to_string(layTrans)<<std::endl;
         }
         sstr<< " *** (" << std::setfill('0') << std::setw(2) << lay
             << ", " << std::setfill('0') << std::setw(3) << tube << ")    "; 
         sstr<<to_string(reElement->localToGlobalTrans(gctx, measHash))<<", ";
         sstr<<Amg::toString(reElement->getParameters().tubeLayers[lay-1].tubePosInLayer(tube -1),2);
         sstr<<", activeTube: "<<reElement->activeTubeLength(measHash);
         sstr<<", tubeLength: "<<reElement->tubeLength(measHash);
         sstr<<", wireLength: "<<reElement->wireLength(measHash);
         sstr<<std::endl;            
      }
   }
}


}

