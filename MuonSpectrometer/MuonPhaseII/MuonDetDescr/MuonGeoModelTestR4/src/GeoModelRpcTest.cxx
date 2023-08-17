
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelRpcTest.h"
#include <MuonReadoutGeometryR4/StringUtils.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <fstream>


namespace MuonGMR4{

GeoModelRpcTest::GeoModelRpcTest(const std::string& name, ISvcLocator* pSvcLocator):
AthHistogramAlgorithm(name,pSvcLocator) {}

StatusCode GeoModelRpcTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());    
    ATH_CHECK(m_surfaceProvTool.retrieve());
    /// Prepare the TTree dump
    if (m_dumpTree) ATH_CHECK(m_tree.init(this));

    
    const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};
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
        const Identifier eleId = id_helper.elementID(statName, statEta, statPhi, 1, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("Failed to deduce a station name for " << testCham);
            return StatusCode::FAILURE;
        }
        std::copy_if(id_helper.detectorElement_begin(), 
                     id_helper.detectorElement_end(), 
                     std::inserter(m_testStations, m_testStations.end()), 
                        [&](const Identifier& id) {
                            return id_helper.stationName(id) == id_helper.stationName(eleId) &&
                                   id_helper.stationEta(id) == id_helper.stationEta(eleId) &&
                                   id_helper.stationPhi(id) == id_helper.stationPhi(eleId);
                        });
    }
    /// Add all stations for testing if nothing has been specified
    if (m_testStations.empty()){
        std::copy(id_helper.detectorElement_begin(), 
                  id_helper.detectorElement_end(), 
                  std::inserter(m_testStations, m_testStations.end()));
    } else {
        std::stringstream sstr{};
        for (const Identifier& id : m_testStations){
            sstr<<" *** "<<m_idHelperSvc->toString(id)<<std::endl;
        }
        ATH_MSG_INFO("Test only the following stations "<<std::endl<<sstr.str());
    }
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
StatusCode GeoModelRpcTest::finalize() {
    if (m_dumpTree) ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelRpcTest::execute() {
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

    for (const Identifier& test_me : m_testStations) {
      ATH_MSG_DEBUG("Test retrieval of Rpc detector element "<<m_idHelperSvc->toStringDetEl(test_me));
      const RpcReadoutElement* reElement = m_detMgr->getRpcReadoutElement(test_me);
      if (!reElement) {
         ATH_MSG_WARNING("Detector element "<<m_idHelperSvc->toStringDetEl(test_me)<<" is invalid");
         continue;
      }
      /// Check that we retrieved the proper readout element
      if (reElement->identify() != test_me) {
         ATH_MSG_FATAL("Expected to retrieve "<<m_idHelperSvc->toStringDetEl(test_me)
                     <<". But got instead "<<m_idHelperSvc->toStringDetEl(reElement->identify()));
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
            ATH_MSG_FATAL("Closure test failed for "<<m_idHelperSvc->toStringDetEl(test_me)<<" and axis "<<Amg::toString(axis, 0)
            <<". Ended up with "<< Amg::toString(transClosure*axis) );
            return StatusCode::FAILURE;
         }         
      }
      const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};
      
      for (int doubPhi = 1; doubPhi <= 2; ++doubPhi) {
        for (int gasGap = 1; gasGap <= 2; ++gasGap) {
            for (bool measPhi: {false, true}) {
                for (unsigned int strip = 1; strip < 10 ; ++strip) {
                    bool isValid{false};
                    const Identifier chId = id_helper.channelID(reElement->identify(),
                                                                reElement->doubletZ(),
                                                                doubPhi, gasGap, measPhi, strip, isValid);
                    if (!isValid) {
                        continue;
                    }
                    /// Test the back and forth conversion of the Identifier
                    const IdentifierHash measHash = reElement->measurementHash(chId);

                    const Identifier backCnv = reElement->measurementId(measHash);
                    if (backCnv != chId) {
                        ATH_MSG_FATAL("The back and forth conversion of "<<m_idHelperSvc->toString(chId)
                                    <<" failed. Got "<<m_idHelperSvc->toString(backCnv));
                        return StatusCode::FAILURE;
                    }
                    if (reElement->layerHash(chId) != reElement->layerHash(measHash)) {
                        ATH_MSG_FATAL("Constructing the layer hash from the identifier "<<
                                    m_idHelperSvc->toString(chId)<<" leadds to different layer hashes "<<
                                    reElement->layerHash(chId)<<" vs. "<< reElement->layerHash(measHash));
                        return StatusCode::FAILURE;
                    }
                    ATH_MSG_INFO("Channel "<<m_idHelperSvc->toString(chId)<<" strip position "
                                            <<Amg::toString(reElement->stripPosition(gctx, measHash)));
                }                
            }
            return StatusCode::SUCCESS;
        }
    }
    if (outStream) dumpToFile(ctx, gctx, reElement, *outStream);        
   }
   return StatusCode::SUCCESS;
}
StatusCode GeoModelRpcTest::dumpToTree(const EventContext& ctx,
                                       const ActsGeometryContext& gctx, 
                                       const RpcReadoutElement* readoutEle){

   if (!m_dumpTree) return StatusCode::SUCCESS;
   
   m_stIndex    = readoutEle->stationIndex();
   m_stEta      = readoutEle->stationEta();
   m_stPhi      = readoutEle->stationPhi();
   m_doubletR   = readoutEle->doubletR();
   m_doubletZ   = readoutEle->doubletZ();
   m_doubletPhi = readoutEle->doubletPhi();
   

   /// Dump the local to global transformation of the readout element
   const Amg::Transform3D& transform{readoutEle->localToGlobalTrans(gctx)};
   m_readoutTransform.push_back(Amg::Vector3D{transform.translation()});
   m_readoutTransform.push_back(Amg::Vector3D{transform.linear()*Amg::Vector3D::UnitX()});
   m_readoutTransform.push_back(Amg::Vector3D{transform.linear()*Amg::Vector3D::UnitY()});
   m_readoutTransform.push_back(Amg::Vector3D{transform.linear()*Amg::Vector3D::UnitZ()});
  
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
void GeoModelRpcTest::dumpToFile(const EventContext& /*ctx*/,
                                 const ActsGeometryContext& gctx,
                                 const RpcReadoutElement* reElement, 
                                 std::ostream& sstr) {
   sstr<<"######################################################################################"<<std::endl;
   sstr<<"Found Readout element "<<m_idHelperSvc->toStringDetEl(reElement->identify())<<std::endl;
   sstr<<"######################################################################################"<<std::endl;
   /// location   
   const Amg::Transform3D localToGlob{reElement->localToGlobalTrans(gctx)};
   sstr<<"GeoModel transformation: "<<to_string(localToGlob)<<std::endl;
   sstr<<"Chamber center: "<<to_string(m_surfaceProvTool->chambCenterToGlobal(gctx, 
                                                               reElement->identify()))<<std::endl;
   
   sstr<<reElement->getParameters()<<std::endl;
}


}

