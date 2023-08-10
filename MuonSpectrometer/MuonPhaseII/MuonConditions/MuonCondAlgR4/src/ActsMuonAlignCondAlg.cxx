/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsMuonAlignCondAlg.h"

#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteCondHandle.h"

using namespace MuonGMR4;

ActsMuonAlignCondAlg::ActsMuonAlignCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm{name, pSvcLocator}{}

StatusCode ActsMuonAlignCondAlg::initialize() {
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    ATH_CHECK(m_surfaceProvTool.retrieve());

    m_techs = m_detMgr->getDetectorTypes();
    if (m_techs.empty()) {
        ATH_MSG_FATAL("The detector manager does not contain any elements");
        return StatusCode::FAILURE;
    }
    for (const ActsTrk::DetectorType det : m_techs) {
        m_writeKeys.emplace_back(ActsTrk::to_string(det) + m_keyToken);
        ATH_MSG_INFO("Register new alignment container "<<m_writeKeys.back().fullKey());
    }
    ATH_CHECK(m_writeKeys.initialize());
    return StatusCode::SUCCESS;
}

StatusCode ActsMuonAlignCondAlg::execute(const EventContext& ctx) const {
    SG::ReadCondHandle<ALineContainer> aLineContainer{m_readKey, ctx};
    if (!aLineContainer.isValid()){
        ATH_MSG_FATAL("Failed to retrieve the Muon A-lines "<<m_readKey.fullKey());
        return StatusCode::FAILURE;
    }
    std::vector<const MuonReadoutElement*> readoutEles = m_detMgr->getAllReadoutElements();
    /// Temporary store to cache the alignable transforms
    
    std::unordered_map<const GeoAlignableTransform*, std::shared_ptr<const Amg::Transform3D>> transStore{};

    std::map<ActsTrk::DetectorType, std::set<const GeoAlignableTransform*>> procTrans{};
    ATH_MSG_INFO("Load the alignment of "<<readoutEles.size()<<" detector elements");
    for (const MuonReadoutElement* re : readoutEles) {
        const GeoAlignableTransform* alignTrans = re->alignableTransform();
        if (!alignTrans) {
            ATH_MSG_WARNING("The readout element "<<m_idHelperSvc->toStringDetEl(re->identify())
                            <<" has no alignable transform.");
            continue;
        }
        std::shared_ptr<const Amg::Transform3D>& cached = transStore[alignTrans];
        if (cached) {
            ATH_MSG_DEBUG("The alignable transformation for "<<m_idHelperSvc->toStringChamber(re->identify())
                         <<" has been cached before. ");
            continue;
        }
        /// Construct the identifier to search for the proper Aline transformation 
        Identifier stationId{};
        /// In case of the Mdts it's the chamber Identifier        
        if (re->detectorType() == ActsTrk::DetectorType::Mdt || re->detectorType() == ActsTrk::DetectorType::Tgc) {
            stationId = m_idHelperSvc->chamberId(re->identify());
        } else if (re->detectorType() == ActsTrk::DetectorType::Rpc) {
            /// The BML eta 7 stations have their own alignment. 
            if (std::abs(re->stationEta()) == 7 && m_idHelperSvc->stationNameString(re->identify()) == "BML") {
                stationId = m_idHelperSvc->rpcIdHelper().elementID(re->stationIndex(), re->stationEta(), re->stationPhi(), 1);
            /// The rest shares the same alignmnet constants with the Mdts
            } else {
                stationId = m_idHelperSvc->mdtIdHelper().elementID(re->stationIndex(), re->stationEta(), re->stationPhi());
            }
        }
        ALineContainer::const_iterator aLineItr = aLineContainer->find(stationId);
        if (aLineItr == aLineContainer->end()) {
            ATH_MSG_INFO("No Alines were stored for "<<m_idHelperSvc->toString(re->identify())
                        <<". Used "<<m_idHelperSvc->toString(stationId)<<" as station Identifier");
            continue;
        }
        /// Store the alignable transformation
        cached = std::make_shared<Amg::Transform3D>(aLineItr->delta());
        procTrans[re->detectorType()].insert(alignTrans);
    }
    /// Create the condition handles
    unsigned int numAligned{0};
    for (size_t det =0 ; det < m_techs.size(); ++det) {
        const SG::WriteCondHandleKey<ActsTrk::RawGeomAlignStore>& key = m_writeKeys[det];
        const ActsTrk::DetectorType subDet = m_techs[det];

        SG::WriteCondHandle<ActsTrk::RawGeomAlignStore> writeHandle{key, ctx};
        if (writeHandle.isValid()) {
            ATH_MSG_FATAL("The alignment constants for "<<ActsTrk::to_string(subDet)
                          <<" is still valid. That should not happen at this stage");
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(aLineContainer);        
        std::unique_ptr<ActsTrk::RawGeomAlignStore> writeCdo = std::make_unique<ActsTrk::RawGeomAlignStore>();
        writeCdo->detType = subDet;

        const std::set<const GeoAlignableTransform*>& toStore =  procTrans[subDet];
        /// Append the alignable transformations to the conditions object
        for (const GeoAlignableTransform* alignable : toStore) {
           const std::shared_ptr<const Amg::Transform3D>& cached = transStore[alignable];
           if (!cached) continue;
           writeCdo->geoModelAlignment->setDelta(alignable, transStore[alignable]);
        }
        /// Propagate the cache throughout the geometry
        for (const MuonReadoutElement* re : readoutEles){
            numAligned+= re->storeAlignment(*writeCdo);
        }
        m_surfaceProvTool->storeAlignment(*writeCdo);
        ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    }
    /// Check that all readout elements were properly aligned
    if (numAligned != readoutEles.size()){
        ATH_MSG_WARNING("Only "<<numAligned<<" out of "<<readoutEles.size()<<" were picked up by the alignment cutalg");
    }
    return StatusCode::SUCCESS;
}
