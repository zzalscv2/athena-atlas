
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelsTgcTest.h"
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <fstream>

using namespace ActsTrk;
namespace MuonGMR4{

GeoModelsTgcTest::GeoModelsTgcTest(const std::string& name, ISvcLocator* pSvcLocator):
AthHistogramAlgorithm(name,pSvcLocator) {}

StatusCode GeoModelsTgcTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());    
    ATH_CHECK(m_surfaceProvTool.retrieve());
    /// Prepare the TTree dump
    ATH_CHECK(m_tree.init(this));

    
    const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
    for (const std::string& testCham : m_selectStat) {
        if (testCham.size() != 6) {
            ATH_MSG_FATAL("Wrong format given " << testCham);
            return StatusCode::FAILURE;
        }
        /// Example string STS3A3
        std::string statName = testCham.substr(0, 3);
        unsigned int statEta = std::atoi(testCham.substr(3, 1).c_str()) *
                               (testCham[4] == 'A' ? 1 : -1);
        unsigned int statPhi = std::atoi(testCham.substr(5, 1).c_str());
        bool is_valid{false};
        const Identifier eleId = id_helper.elementID(statName, statEta, statPhi, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("Failed to deduce a station name for " << testCham);
            return StatusCode::FAILURE;
        }
        std::copy_if(id_helper.detectorElement_begin(), 
                     id_helper.detectorElement_end(), 
                     std::inserter(m_testStations, m_testStations.end()), 
                        [&](const Identifier& id) {
                            return id_helper.elementID(id) == eleId;
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
StatusCode GeoModelsTgcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelsTgcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<ActsGeometryContext> geoContextHandle{m_geoCtxKey, ctx};
    if (!geoContextHandle.isValid()){
      ATH_MSG_FATAL("Failed to retrieve "<<m_geoCtxKey.fullKey());
      return StatusCode::FAILURE;
    }
    const ActsGeometryContext& gctx{**geoContextHandle};

    for (const Identifier& test_me : m_testStations) {
      ATH_MSG_DEBUG("Test retrieval of sTgc detector element "<<m_idHelperSvc->toStringDetEl(test_me));
      const sTgcReadoutElement* reElement = m_detMgr->getsTgcReadoutElement(test_me);
      if (!reElement) {         
         continue;
      }
      /// Check that we retrieved the proper readout element
      if (m_idHelperSvc->stgcIdHelper().elementID(reElement->identify()) != m_idHelperSvc->stgcIdHelper().elementID(test_me)) {
         ATH_MSG_FATAL("Expected to retrieve "<<m_idHelperSvc->toString(test_me)
                      <<". But got instead "<<m_idHelperSvc->toString(reElement->identify()));
         return StatusCode::FAILURE;
      }
      ATH_CHECK(dumpToTree(ctx,gctx,reElement));
      const Amg::Transform3D& globToLocal{reElement->globalToLocalTrans(gctx)};
      const Amg::Transform3D& localToGlob{reElement->localToGlobalTrans(gctx)};
      /// Closure test that the transformations actually close
      const Amg::Transform3D transClosure = globToLocal * localToGlob;
      for (Amg::Vector3D axis :{Amg::Vector3D::UnitX(),Amg::Vector3D::UnitY(),Amg::Vector3D::UnitZ()}) {
         const double closure_mag = std::abs( (transClosure*axis).dot(axis) - 1.);
         if (closure_mag > std::numeric_limits<float>::epsilon() ) {
            ATH_MSG_FATAL("Closure test failed for "<<m_idHelperSvc->toStringDetEl(test_me)<<" and axis "<<Amg::toString(axis, 0)
            <<". Ended up with "<< Amg::toString(transClosure*axis) );
            return StatusCode::FAILURE;
         }         
      }
      const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
      for (int layer = 1; layer <= reElement->numLayers(); ++layer) {
        for (int chType = sTgcIdHelper::sTgcChannelTypes::Strip/*Pad*/; chType <= sTgcIdHelper::sTgcChannelTypes::Strip/*Wire*/; ++chType) {

            unsigned int numStrip = reElement->numStrips();
            for (unsigned int strip = 1; strip < numStrip ; ++strip) {
                bool isValid{false};
                const Identifier chId = id_helper.channelID(reElement->identify(),
                                                                reElement->multilayer(),
                                                                layer, chType, strip, isValid);
                if (!isValid) {
                    continue;
                }
                /// Test the back and forth conversion of the Identifier
                const IdentifierHash measHash = reElement->measurementHash(chId);
                const IdentifierHash layHash = reElement->layerHash(chId);
                ATH_MSG_VERBOSE("layer: "<<layer<<", chType: "<<chType
                               <<" --> layerHash: "<<static_cast<unsigned>(layHash));
                const Identifier backCnv = reElement->measurementId(measHash);
                if (backCnv != chId) {
                    ATH_MSG_FATAL("The back and forth conversion of "<<m_idHelperSvc->toString(chId)
                                    <<" failed. Got "<<m_idHelperSvc->toString(backCnv));
                    return StatusCode::FAILURE;
                }
                if (layHash != reElement->layerHash(measHash)) {
                    ATH_MSG_FATAL("Constructing the layer hash from the identifier "<<
                                m_idHelperSvc->toString(chId)<<" leads to different layer hashes "<<
                                layHash<<" vs. "<< reElement->layerHash(measHash));
                    return StatusCode::FAILURE;
                }
                ATH_MSG_VERBOSE("Channel "<<m_idHelperSvc->toString(chId)<<" strip position "
                                        <<Amg::toString(reElement->stripPosition(gctx, measHash)));
            }
        }
      }
    
    }   
    return StatusCode::SUCCESS;
}

StatusCode GeoModelsTgcTest::dumpToTree(const EventContext& ctx,
                                       const ActsGeometryContext& gctx, 
                                       const sTgcReadoutElement* reElement){
    
   
    m_stIndex    = reElement->stationName();
    m_stEta      = reElement->stationEta();
    m_stPhi      = reElement->stationPhi();
    m_stML       = reElement->multilayer();
    m_chamberDesign = reElement->chamberDesign();
    ///
    m_numLayers = reElement->numLayers();
    m_yCutout = reElement->yCutout();
    m_gasTck = reElement->gasGapPitch();
    ///
    m_sChamberLength = reElement->sChamberLength();
    m_lChamberLength = reElement->lChamberLength();
    m_chamberHeight = reElement->chamberHeight();
    ///
    m_sGapLength = reElement->sGapLength();
    m_lGapLength = reElement->lGapLength();
    m_gapHeight = reElement->gapHeight();
    ///
    m_numStrips = reElement->numStrips();
    m_stripPitch = reElement->stripPitch();
    m_stripWidth = reElement->stripWidth();   
    ///Wires and Pads will also come here 
 
   /// Dump the local to global transformation of the readout element
   const Amg::Transform3D& transform{reElement->localToGlobalTrans(gctx)};
   m_readoutTransform = transform;
   const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
      
   for (int layer = 1; layer <= reElement->numLayers(); ++layer) {
        for (int chType = sTgcIdHelper::sTgcChannelTypes::Pad; chType <= sTgcIdHelper::sTgcChannelTypes::Wire; ++chType) {
            switch (chType) {
                case sTgcIdHelper::sTgcChannelTypes::Strip:
                    unsigned int numStrip = reElement->numStrips();
                    for (unsigned int strip = 1; strip <= numStrip ; ++strip) {

                        bool isValid{false};
                        const Identifier stripID = id_helper.channelID(reElement->identify(), 
                                                                   reElement->multilayer(),
                                                                    layer, chType, strip, isValid);
                    if (!isValid) {
                        ATH_MSG_WARNING("Invalid Identifier detected for readout element "
                                       <<m_idHelperSvc->toStringDetEl(reElement->identify())
                                       <<" layer: "<<layer<<" strip: "<<strip<<" channelType: "<<chType);
                        continue;
                    }
                    m_globalStripPos.push_back(reElement->stripPosition(gctx, stripID));
                    m_stripGasGap.push_back(layer);
                    m_stripNum.push_back(strip);
                    m_stripLengths.push_back(reElement->stripLength(strip));

                    if (strip != 1) continue;
                    const Amg::Transform3D locToGlob = reElement->localToGlobalTrans(gctx, stripID);
                    ATH_MSG_ALWAYS("The local to global transformation on layers is: " << Amg::toString(locToGlob));
                    m_stripRot.push_back(locToGlob);
                    m_stripRotGasGap.push_back(layer);
                
                }
            }
        }
   }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

}

