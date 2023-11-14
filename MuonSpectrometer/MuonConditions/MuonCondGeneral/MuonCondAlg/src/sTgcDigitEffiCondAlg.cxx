/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/sTgcDigitEffiCondAlg.h"

#include <StoreGate/WriteCondHandle.h>
#include <AthenaKernel/IOVInfiniteRange.h>
#include <PathResolver/PathResolver.h>
#include <fstream>

sTgcDigitEffiCondAlg::sTgcDigitEffiCondAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

// Initialize
StatusCode sTgcDigitEffiCondAlg::initialize() {
    ATH_MSG_DEBUG("initializing " << name());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    if (m_readKeyDb.empty() && m_readFromJSON.value().empty()){
        ATH_MSG_FATAL("No data source is given to load the energy thresholds from. Please provide either a COOL folder or a json file");
        return StatusCode::FAILURE;
    } else if (m_readKeyDb.empty()) {
        ATH_MSG_INFO("Load the energy thresholds from a JSON file "<<m_readFromJSON);
    } else {
        ATH_MSG_INFO("Load the energy thresholds list from COOL "<<m_readKeyDb.fullKey());
    }
    return StatusCode::SUCCESS;
}

// execute
StatusCode sTgcDigitEffiCondAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());
    // launching Write Cond Handle
    SG::WriteCondHandle<DigitEffiData> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << " In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    std::unique_ptr<DigitEffiData> writeCdo{std::make_unique<DigitEffiData>(m_idHelperSvc.get())};
    if (!m_readKeyDb.empty()) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKeyDb, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to initialize the COOL folder "<<m_readKeyDb.fullKey());
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readHandle);
        for (CondAttrListCollection::const_iterator itr = readHandle->begin(); itr != readHandle->end(); ++itr) {
            const coral::AttributeList& atr = itr->second;
            const std::string data{*(static_cast<const std::string*>((atr["data"]).addressOfData()))};
            nlohmann::json lines = nlohmann::json::parse(data);           
            ATH_CHECK(parseDataFromJSON(lines, *writeCdo)); 
        }
    } else {
       std::ifstream inStream{PathResolverFindCalibFile(m_readFromJSON)};
        if (!inStream.good()) {
            ATH_MSG_FATAL("No such file or directory");
            return StatusCode::FAILURE;
        }
        nlohmann::json lines;
        inStream >> lines;
        ATH_CHECK(parseDataFromJSON(lines, *writeCdo));          
    }
    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    ATH_MSG_DEBUG("Recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");
    return StatusCode::SUCCESS;
}
StatusCode sTgcDigitEffiCondAlg::parseDataFromJSON(const nlohmann::json& lines,
                                                   DigitEffiData& effiData) const {
    for (auto& corr : lines.items()) {
        nlohmann::json line = corr.value();    
         /// Station Component identification
        const std::string stationType = line["station"];
        const int stationPhi = line["phi"];
        const int stationEta = line["eta"];
        const int multiLayer = line["multiLayer"];
        const int gasGap = line["gasGap"];
        const double efficiency = line["efficiency"];
        bool is_valid{false};
        const Identifier id = m_idHelperSvc->stgcIdHelper().channelID(stationType, stationEta, stationPhi, 
                                                                      multiLayer, gasGap, sTgcIdHelper::Strip, 1, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("The Identifier identifier "<<stationType<<", "<<stationEta<<", "<<stationPhi
                        << ", "<<multiLayer<<", "<<gasGap<<" is invalid");
            return StatusCode::FAILURE;
        }
        ATH_CHECK(effiData.setEfficiency(id, efficiency));
    }
    return StatusCode::SUCCESS;
}