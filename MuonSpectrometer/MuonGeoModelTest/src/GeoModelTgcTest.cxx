/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelTgcTest.h"

#include <fstream>
#include <iostream>

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "StoreGate/ReadCondHandle.h"
#include "GaudiKernel/SystemOfUnits.h"


namespace MuonGM {

template<typename VType> bool isEqual(const std::vector<VType>& a,
                                      const std::vector<VType>&b){
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t k =0 ; k < a.size() ; ++k) {
        if ( std::abs(a[k] - b[k]) > std::numeric_limits<VType>::epsilon()){
            return false;
        }
    }
    return true;
}
template <typename VType> std::ostream& operator<<(std::ostream& ostr, const std::vector<VType>& v){
    for (size_t k = 0 ; k <v.size(); ++k){
        ostr<<v[k];
        if ( k+1 != v.size())ostr<<",";
    } 
    return ostr;
}
template <typename VType> std::ostream& operator<<(std::ostream& ostr, const std::set<VType>& s){
    unsigned int k=1;
    for (const VType& ele : s){
        ostr<<ele;
        if (k != s.size()) ostr<<";";
        ++k;
    } 
    return ostr;
}
struct TgcChamberLayout {
    Identifier gasGap{};
    std::string techType{};
    std::vector<double> botStripPos{};
    std::vector<double> topStripPos{};
    std::vector<int> wireGangLayout{};
    double wirePitch{0.};

    bool operator==(const TgcChamberLayout& other) const{
        return isEqual(botStripPos, other.botStripPos) &&
               isEqual(topStripPos, other.topStripPos) && 
               isEqual(wireGangLayout, other.wireGangLayout) &&
               std::abs(wirePitch - other.wirePitch) < std::numeric_limits<float>::epsilon();
    }
};
struct ChamberGrp {
    ChamberGrp(const TgcChamberLayout& grp):
            m_lay{grp} {m_gaps[grp.techType].insert(grp.gasGap);}
    bool addChamber(const TgcChamberLayout& lay){
        if (m_lay == lay) { 
            m_gaps[lay.techType].insert(lay.gasGap);
            return true;
        }
        return false;
    }
    const std::map<std::string, std::set<Identifier>>& allGaps() const{ return m_gaps; }
    const TgcChamberLayout& layout() const { return m_lay; }
    private:
        TgcChamberLayout m_lay{};
        std::map<std::string, std::set<Identifier>> m_gaps{};
    
};

GeoModelTgcTest::GeoModelTgcTest(const std::string& name, ISvcLocator* pSvcLocator):
    AthHistogramAlgorithm{name, pSvcLocator} {}

StatusCode GeoModelTgcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tree.init(this));
    const TgcIdHelper& id_helper{m_idHelperSvc->tgcIdHelper()};
    for (const std::string& testCham : m_selectStat) {
        if (testCham.size() != 7) {
            ATH_MSG_FATAL("Wrong format given " << testCham);
            return StatusCode::FAILURE;
        }
        /// Example string BIL1A3
        std::string statName = testCham.substr(0, 3);
        unsigned int statEta = std::atoi(testCham.substr(3, 1).c_str()) *
                               (testCham[4] == 'A' ? 1 : -1);
        unsigned int statPhi = std::atoi(testCham.substr(5, 2).c_str());
        bool is_valid{false};
        const Identifier eleId =
            id_helper.elementID(statName, statEta, statPhi, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("Failed to deduce a station name for " << testCham);
            return StatusCode::FAILURE;
        }
        m_testStations.insert(eleId);
    }
    /// Add all stations for testing
    if (m_testStations.empty()){
        for(auto itr = id_helper.detectorElement_begin();
                 itr!= id_helper.detectorElement_end();++itr){
            m_testStations.insert(*itr);
        }
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<MuonDetectorManager> detMgr{m_detMgrKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve MuonDetectorManager "
                      << m_detMgrKey.fullKey());
        return StatusCode::FAILURE;
    }
    dumpReadoutXML(**detMgr);
    for (const Identifier& test_me : m_testStations) {
        ATH_MSG_VERBOSE("Test retrieval of Mdt detector element " 
                        << m_idHelperSvc->toStringDetEl(test_me));
        const TgcReadoutElement* reElement = detMgr->getTgcReadoutElement(test_me);
        if (!reElement) {
            ATH_MSG_VERBOSE("Detector element is invalid");
            continue;
        }
        /// Check that we retrieved the proper readout element
        if (reElement->identify() != test_me) {
            ATH_MSG_FATAL("Expected to retrieve "
                          << m_idHelperSvc->toStringDetEl(test_me) << ". But got instead "
                          << m_idHelperSvc->toStringDetEl(reElement->identify()));
            return StatusCode::FAILURE;
        }
       
        const Identifier prevId = reElement->getStationPhi() > 1 ? m_idHelperSvc->tgcIdHelper().elementID(m_idHelperSvc->stationNameString(test_me),
                                                                                                          reElement->getStationEta(),
                                                                                                          reElement->getStationPhi() - 1) : test_me;
        const TgcReadoutElement* prevRE = detMgr->getTgcReadoutElement(prevId);
        const Amg::Vector3D center = reElement->center();
        ATH_MSG_DEBUG("Tgc element "<<m_idHelperSvc->toString(reElement->identify())
                        <<" position "<<Amg::toString(center, 2)
                        <<" perp: "<<center.perp()
                        <<" phi: "<<(center.phi() / Gaudi::Units::deg)
                        <<" theta: "<<(center.theta() / Gaudi::Units::deg)
                        <<" rSize: "<<reElement->getRsize()<<"/"<<reElement->getLongRsize()
                        <<" sSize: "<<reElement->getSsize()<<"/"<<reElement->getLongSsize()
                        <<" zSize: "<<reElement->getZsize()<<"/"<<reElement->getLongZsize()
                        <<" dPhi: "<<(prevRE->center().deltaPhi(center) / Gaudi::Units::deg));        
        ATH_CHECK(dumpToTree(ctx, reElement));
    }
    return StatusCode::SUCCESS;
}
StatusCode GeoModelTgcTest::dumpToTree(const EventContext& ctx, const TgcReadoutElement* readoutEle) {
    m_stIndex = readoutEle->getStationIndex();
    m_stEta   = readoutEle->getStationEta();
    m_stPhi   = readoutEle->getStationPhi();
    m_nGasGaps = readoutEle->Ngasgaps();
    ATH_MSG_DEBUG("Dump readout element "<<m_idHelperSvc->toString(readoutEle->identify()));

    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};

    const Amg::Transform3D& trans{readoutEle->transform()};
    m_readoutTransform = trans;
    m_shortWidth = readoutEle->shortWidth();
    m_longWidth = readoutEle->longWidth();
    m_height = readoutEle->length();
    m_thickness = readoutEle->thickness();

   const MuonGM::MuonStation* station = readoutEle->parentMuonStation();
   if (station->hasALines()){ 
        m_ALineTransS = station->getALine_tras();
        m_ALineTransT = station->getALine_traz();
        m_ALineTransZ = station->getALine_trat();
        m_ALineRotS   = station->getALine_rots();
        m_ALineRotT   = station->getALine_rotz();
        m_ALineRotZ   = station->getALine_rott();
    }

    for (bool isStrip : {false, true}) {        
        for (int layer = 1 ; layer <= readoutEle->numberOfLayers(isStrip); ++layer){
            const unsigned int nChan = isStrip ? readoutEle->getNStrips(layer) :
                                                 readoutEle->nGangs(layer);
            if (!nChan) continue;
            const Identifier layerId = idHelper.channelID(readoutEle->identify(),layer, isStrip, 1);
            m_layTans.push_back(readoutEle->surface(layerId).transform());
            m_layMeasPhi.push_back(isStrip);
            m_layNumber.push_back(layer);
            m_layHeight.push_back(readoutEle->length());
            m_layShortWidth.push_back(readoutEle->shortWidth() /*- readoutEle->frameXwidth() * 2. */);
            m_layLongWidth.push_back(readoutEle->longWidth()   /*- readoutEle->frameXwidth() * 2. */);
            unsigned int numWires = !isStrip ? readoutEle->getTotalWires(layer) : 0;
            m_layNumWires.push_back(numWires);
            
            if (isStrip) {
                /// The last strip is for one reason always 0
                for (int strip = 1; strip < readoutEle->getNStrips(layer); ++strip) {
                    bool is_valid{false};
                    const Identifier stripId = idHelper.channelID(readoutEle->identify(), layer, isStrip, strip, is_valid);
                    if (!is_valid) continue;
                    
                    /// Strip center
                    const Amg::Vector3D globStripPos = readoutEle->channelPos(stripId);
                    Amg::Vector2D locStripPos{Amg::Vector2D::Zero()};
                    const Trk::Surface& surf{readoutEle->surface(stripId)};
                    if (!surf.globalToLocal(globStripPos, Amg::Vector3D::Zero(), locStripPos)){
                        ATH_MSG_FATAL("Failed to build local strip position "<<m_idHelperSvc->toString(stripId));
                        return StatusCode::FAILURE;
                    }
                    m_locStripCenter.push_back(locStripPos);
                    m_stripCenter.push_back(globStripPos);
                    /// Strip bottom & top edges
                    const double stripHalfLength = readoutEle->stripLength(layer, strip) / 2.;

                    const Amg::Vector2D locStripBot{readoutEle->getStripPositionOnShortBase(strip), -stripHalfLength};                    
                    const Amg::Vector2D locStripTop{readoutEle->getStripPositionOnLargeBase(strip), stripHalfLength};
                    const Amg::Vector3D globStripBot{surf.localToGlobal(locStripBot)};
                    const Amg::Vector3D globStripTop{surf.localToGlobal(locStripTop)};
                    
                    m_stripBottom.push_back(globStripBot);
                    m_stripTop.push_back(globStripTop);
    
                    m_locStripBottom.push_back(locStripBot);
                    m_locStripTop.push_back(locStripTop);
  
                    m_stripGasGap.push_back(layer);
                    m_stripNum.push_back(strip);
                    m_stripLength.push_back(stripHalfLength * 2.);
                    m_stripPitch.push_back(readoutEle->stripPitch(layer,strip));
                    m_stripShortWidth.push_back(readoutEle->stripShortWidth(layer, strip));
                    m_stripLongWidth.push_back(readoutEle->stripLongWidth(layer, strip));
                }
            } else {
                /// The last gang is for one reason always 0
                for (int gang = 1; gang < readoutEle->nGangs(layer); ++gang) {
                       const Identifier gangId{idHelper.channelID(readoutEle->identify(), layer, isStrip, gang)};
                       const Trk::Surface& surf{readoutEle->surface(gangId)};
                       const Amg::Vector3D globPos{readoutEle->gangPos(layer, gang)};
                       Amg::Vector2D locPos{Amg::Vector2D::Zero()};
                       if (!surf.globalToLocal(globPos,Amg::Vector3D::Zero(),locPos)) {
                           ATH_MSG_FATAL("Failed to extract local position "<<m_idHelperSvc->toString(gangId));
                           return StatusCode::FAILURE;
                       }
                       m_locGangPos.push_back(locPos);
                       m_gangCenter.push_back(globPos);
                       m_gangGasGap.push_back(layer);
                       m_gangNum.push_back(gang);
                       m_gangNumWires.push_back(readoutEle->getNWires(layer, gang));
                       m_gangLength.push_back(0.5 *(readoutEle->gangShortWidth(layer, gang) + 
                                                    readoutEle->gangLongWidth(layer, gang) ));
                }
            }
        }    
   }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

void GeoModelTgcTest::dumpReadoutXML(const MuonGM::MuonDetectorManager& detMgr) {
    if (m_readoutXML.empty()) {
        return;
    }
    std::ofstream xmlStream{m_readoutXML};
    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
    std::map<Identifier, TgcChamberLayout> allLayouts{};
    for (TgcIdHelper::const_id_iterator itr = idHelper.module_begin();
                                        itr != idHelper.module_end(); ++itr) {
        const MuonGM::TgcReadoutElement* reEle = detMgr.getTgcReadoutElement(*itr);
        if (!reEle) continue;
        for (bool isStrip : {false, true}) {
            for (int layer = 1 ; layer <= reEle->numberOfLayers(isStrip); ++layer){
                const Identifier layerId = idHelper.channelID(reEle->identify(), layer, isStrip, 1);
                TgcChamberLayout& chambLayout = allLayouts[m_idHelperSvc->gasGapId(layerId)];
                chambLayout.gasGap = m_idHelperSvc->gasGapId(layerId);
                chambLayout.techType = reEle->getTechnologyName();
                if (isStrip) {
                   for (int strip = 1; strip < reEle->getNStrips(layer); ++strip) {
                        /// Note the slight shift in the coordinate system given that the positions in the legacy
                        /// are given w.r.t. strip center while for the new geometry we need them w.r.t. strip edge
                        chambLayout.botStripPos.push_back(reEle->getStripPositionOnShortBase(strip) + 0.5*reEle->getSsize());
                        chambLayout.topStripPos.push_back(reEle->getStripPositionOnLargeBase(strip) + 0.5*reEle->getLongSsize());
                   }
                } else {
                    unsigned int accumlWires{0};
                    chambLayout.wirePitch = reEle->WirePitch(layer);
                    /// Another reason to love AMDB. Summing up the number of wires in a gang does not match the
                    /// number of wires in the gasgap, because the last gang has always 0 entries. However, the total
                    /// number of wires is used in the legacy geometry to calculate the position of the first wire. I am
                    /// amazed about the precision to get the N/2 wire right into the center of the chamber. Anyhow, let's 
                    /// insert this hack to have a proper number of wires in the last gang.
                    for (int gang = 1; gang <= reEle->getNGangs(layer); ++gang) {
                        unsigned int nWires = reEle->getNWires(layer , gang);
                        accumlWires+=nWires;
                        if (nWires) {
                            chambLayout.wireGangLayout.push_back(nWires);
                        } else {
                            chambLayout.wireGangLayout.push_back(reEle->getTotalWires(layer) - accumlWires);
                            break;
                        }
                    }
                }
            }
        }
    }
    /// Select the set of all layouts that are belonging together
    std::vector<ChamberGrp> groupies{};
    for (const auto& lay : allLayouts) {
        bool added{false};
        for (ChamberGrp& grp : groupies) {
            if (grp.addChamber(lay.second)){
                added = true;
                break;
            }
        }
        if (!added) groupies.emplace_back(lay.second);
    }
    allLayouts.clear();
    std::stable_sort(groupies.begin(),groupies.end(), 
                    [](const ChamberGrp& a, const ChamberGrp& b){
                        return a.layout().techType < b.layout().techType;
                    });
    /// All added
    ATH_MSG_INFO("Found in total "<<groupies.size()<<" different chamber layouts");
    xmlStream<<"<Table name=\"TgcSensorLayout\">"<<std::endl;
    unsigned int counter{1};
    for (const ChamberGrp& grp : groupies) {
        for (const auto& [tech_type, gapIds]: grp.allGaps() ){
            std::set<int> gaps{};            
            for (const Identifier gapId : gapIds) {
                gaps.insert(m_idHelperSvc->gasGap(gapId));
            }
            xmlStream<<"    <Row ";
            xmlStream<<"TGCSENSORLAYOUT_DATA_ID=\""<<counter<<"\" ";
            xmlStream<<"technology=\""<<tech_type<<"\" ";
            xmlStream<<"gasGap=\""<<gaps<<"\" ";
            xmlStream<<"wirePitch=\""<<grp.layout().wirePitch<<"\" ";
            xmlStream<<"wireGangs=\""<<grp.layout().wireGangLayout<<"\" ";
            xmlStream<<"bottomStrips=\""<<grp.layout().botStripPos<<"\" ";
            xmlStream<<"topStrips=\""<<grp.layout().topStripPos<<"\" ";
            xmlStream<<" />"<<std::endl;
            ++counter;
        }
    }
    xmlStream<<"</Table> "<<std::endl;
}


}
