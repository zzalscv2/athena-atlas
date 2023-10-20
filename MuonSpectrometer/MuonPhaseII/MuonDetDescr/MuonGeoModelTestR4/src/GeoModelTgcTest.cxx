
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelTgcTest.h"
#include <MuonReadoutGeometryR4/StringUtils.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <fstream>


namespace MuonGMR4{

GeoModelTgcTest::GeoModelTgcTest(const std::string& name, ISvcLocator* pSvcLocator):
AthHistogramAlgorithm(name,pSvcLocator) {}

StatusCode GeoModelTgcTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());
    /// Prepare the TTree dump
    ATH_CHECK(m_tree.init(this));

    const TgcIdHelper& id_helper{m_idHelperSvc->tgcIdHelper()};
    for (const std::string& testCham : m_selectStat) {
        if (testCham.size() != 7) {
            ATH_MSG_FATAL("Wrong format given " << testCham);
            return StatusCode::FAILURE;
        }
        /// Example string T1F1A03
        std::string statName = testCham.substr(0, 3);
        unsigned int statEta = std::atoi(testCham.substr(3, 1).c_str()) *
                               (testCham[4] == 'A' ? 1 : -1);
        unsigned int statPhi = std::atoi(testCham.substr(5, 2).c_str());
        bool is_valid{false};
        const Identifier eleId{id_helper.elementID(statName, statEta, statPhi, is_valid)};
        if (!is_valid) {
            ATH_MSG_FATAL("Failed to deduce a station name for " << testCham);
            return StatusCode::FAILURE;
        }
        m_testStations.insert(eleId);
    }
    /// Add all stations for testing if nothing has been specified
    if (m_testStations.empty()){
        std::copy(id_helper.detectorElement_begin(), 
                  id_helper.detectorElement_end(), 
                  std::inserter(m_testStations, m_testStations.end()));
    } else {
        std::stringstream sstr{};
        for (const Identifier& id : m_testStations) {
            sstr<<" *** "<<m_idHelperSvc->toString(id)<<std::endl;
        }
        ATH_MSG_INFO("Test only the following stations "<<std::endl<<sstr.str());
    }
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<ActsGeometryContext> geoContextHandle{m_geoCtxKey, ctx};
    if (!geoContextHandle.isValid()){
      ATH_MSG_FATAL("Failed to retrieve "<<m_geoCtxKey.fullKey());
      return StatusCode::FAILURE;
    }
    const ActsGeometryContext& gctx{**geoContextHandle};

    for (const Identifier& test_me : m_testStations) {
      ATH_MSG_DEBUG("Test retrieval of Tgc detector element "<<m_idHelperSvc->toStringDetEl(test_me));
      const TgcReadoutElement* reElement = m_detMgr->getTgcReadoutElement(test_me);
      if (!reElement) {
         continue;
      }
      /// Check that we retrieved the proper readout element
      if (reElement->identify() != test_me) {
         ATH_MSG_FATAL("Expected to retrieve "<<m_idHelperSvc->toStringDetEl(test_me)
                      <<". But got instead "<<m_idHelperSvc->toStringDetEl(reElement->identify()));
         return StatusCode::FAILURE;
      }
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
      const TgcIdHelper& id_helper{m_idHelperSvc->tgcIdHelper()};
      for (unsigned int gasGap = 1; gasGap <= reElement->nGasGaps(); ++gasGap) {
        for (bool isStrip : {false, true}) {
            const unsigned int nChan = isStrip ? reElement->numStrips(gasGap)
                                               : reElement->numWireGangs(gasGap);
            for (unsigned int chan = 1; chan <= nChan ; ++chan) {
                bool isValid{0};
                const Identifier channelId = id_helper.channelID(reElement->identify(),
                                                                 gasGap, isStrip, chan, isValid);
                if (!isValid) {
                    ATH_MSG_DEBUG("No valid Identifier constructed from the fields "
                                <<m_idHelperSvc->toStringDetEl(reElement->identify())
                                <<"isStrip: "<<(isStrip ? "yay" : "nay")<<" gasGap: "<<gasGap<<
                                " channel: "<<chan);
                    continue;
                }
                const IdentifierHash measHash{reElement->measurementHash(channelId)};
                const Identifier backCnv = reElement->measurementId(measHash);
                if (backCnv != channelId) {
                    ATH_MSG_FATAL("Forward-backward conversion of the Identifier "<<m_idHelperSvc->toString(channelId)
                                <<"failed. Got instead "<<m_idHelperSvc->toString(backCnv));
                    return StatusCode::FAILURE;
                }
                if (reElement->layerHash(channelId) != reElement->layerHash(measHash)) {
                    ATH_MSG_FATAL("The cosntruction of the layer hash from the Identifier "<<m_idHelperSvc->toString(channelId)
                    <<" gave something else than doing it from the measurement hash "<<measHash<<". "<<
                    reElement->layerHash(channelId)<<" vs. "<<reElement->layerHash(measHash));
                }
            }
        }        
    } 
    ATH_CHECK(dumpToTree(ctx, gctx, reElement));  
   }
   return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::dumpToTree(const EventContext& ctx,
                                       const ActsGeometryContext& gctx, 
                                       const TgcReadoutElement* reElement) {
   
   m_stIndex    = reElement->stationName();
   m_stEta      = reElement->stationEta();
   m_stPhi      = reElement->stationPhi();
   m_stLayout   = reElement->chamberDesign();
   m_readoutTransform = reElement->localToGlobalTrans(gctx);

   m_shortWidth = reElement->moduleWidthS();
   m_longWidth = reElement->moduleWidthL();
   m_height = reElement->moduleHeight();
   m_thickness = reElement->moduleThickness();

   const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};   
   for (unsigned int gap = 1; gap <= reElement->nGasGaps(); ++gap) {
        /// Loop over all strips dump their respective information
        for (unsigned int strip = 1 ; strip <= reElement->numStrips(gap); ++strip) {
            const Identifier measId = idHelper.channelID(reElement->identify(),gap, true, strip);
            const RadialStripDesign& layout{reElement->stripLayout(gap)};

            if (strip == 1) {
                m_layTans.push_back(reElement->localToGlobalTrans(gctx,measId));
                m_layMeasPhi.push_back(true);
                m_layNumber.push_back(gap);
                m_layShortWidth.push_back(2.*layout.shortHalfHeight());
                m_layLongWidth.push_back(2.*layout.longHalfHeight());
                m_layHeight.push_back(2.*layout.halfWidth());
                m_layNumWires.push_back(0);
            }
        }
        /// Loop over all wire gangs dump their respective information
        for (unsigned int gang = 1; gang <= reElement->numWireGangs(gap); ++gang) {
            const Identifier measId = idHelper.channelID(reElement->identify(),gap, false, gang);
            const WireGroupDesign& layout{reElement->wireGangLayout(gap)};
            if (gang == 1) {
                m_layTans.push_back(reElement->localToGlobalTrans(gctx,measId));
                m_layMeasPhi.push_back(false);
                m_layNumber.push_back(gap);
                m_layShortWidth.push_back(2.*layout.shortHalfHeight());
                m_layLongWidth.push_back(2.*layout.longHalfHeight());
                m_layHeight.push_back(2.*layout.halfWidth());
                m_layNumWires.push_back(layout.nAllWires());
            }
            m_gangNum.push_back(gang);
            m_gangGasGap.push_back(gap);
            m_gangCenter.push_back(reElement->channelPosition(gctx, measId));
            m_gangNumWires.push_back(layout.numWiresInGroup(gang));
            m_locGangPos.push_back(layout.center(gang).value_or(Amg::Vector2D::Zero()));
            m_gangLength.push_back(layout.stripLength(gang));
        }
   }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

}

