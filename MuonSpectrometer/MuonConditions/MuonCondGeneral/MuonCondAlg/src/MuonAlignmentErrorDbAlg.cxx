/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/MuonAlignmentErrorDbAlg.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include <GaudiKernel/EventIDRange.h>
#include <fstream>
#include <iterator>

MuonAlignmentErrorDbAlg::MuonAlignmentErrorDbAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonAlignmentErrorDbAlg::initialize() {
    ATH_MSG_DEBUG("initialize " << name());
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_writeKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentErrorDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());

    // Write Cond Handle

    SG::WriteCondHandle<MuonAlignmentErrorData> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<MuonAlignmentErrorData> writeCdo{std::make_unique<MuonAlignmentErrorData>()};

    std::string clobContent;
    EventIDRange rangeW;
    std::tie(clobContent, rangeW) = m_clobFileOverride.value().empty() ? getDbClobContent(ctx) : getFileClobContent();

    if (clobContent.empty()) {
        ATH_MSG_ERROR("Cannot retrieve alignment error CLOB");
        return StatusCode::FAILURE;
    }

    std::istringstream indata(clobContent);
    if (!indata) {
        ATH_MSG_ERROR("Alignment error configuration invalid");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("***********************************");
    ATH_MSG_DEBUG("PARSING LIST OF DEVIATIONS...");

    std::string line;
    std::vector<MuonAlignmentErrorData::Deviation> deviationVec;
    MuonAlignmentErrorData::Deviation aDev;
    while (getline(indata, line)) {
        // READING COMMENTS
        if (line.compare(0, 1,"#") == 0) {
            // ATH_MSG_DEBUG("Reading a commented line saying " << line);
            continue;
        }

        // READING FROM INPUT FILE:                                //
        std::string flag("");
        std::string name_sstring("");
        std::string multilayer_sstring("");
        double translation(0.);
        double rotation(0.);

        // GET INPUT FILE VERSION
        if (line.compare(0, 7, "version") == 0) {
            std::string clobVersion;
            std::istringstream(line) >> flag >> clobVersion;
            ATH_MSG_INFO("*****************************************");
            ATH_MSG_INFO("Input file version " << clobVersion);
            ATH_MSG_INFO("*****************************************");
            writeCdo->setClobVersion(std::move(clobVersion));
            continue;
        }

        // get the flag has_nsw_hits
        if (line.compare(0, 12, "has_nsw_hits") == 0) {
            bool hasNswHits{false};
            std::istringstream(line) >> flag >> hasNswHits;
            writeCdo->setHasNswHits(hasNswHits);
            continue;
        }

        // A FLAG CHARACTERIZING THE TYPE OF ERROR
        // A REGULAR EXPRESSION FOR THE STATION NAME (EX: BIL.*)   //
        // TWO DOUBLES FOR THE TRANSLATION AND ROTATION DEVIATIONS //
        if (std::istringstream(line) >> flag >> name_sstring >> multilayer_sstring >> translation >> rotation) {
            ATH_MSG_DEBUG(flag << " " << name_sstring << " " << multilayer_sstring << " " << translation << " " << rotation);

            // SAVING THE STATION DEVIATIONS IN THE STRUCT //
            aDev.stationName = name_sstring;
            aDev.multilayer = multilayer_sstring;
            aDev.translation = translation;
            aDev.rotation = rotation;

            // FILLING THE VECTOR OF STRUCT CONTAINING THE STATION DEVIATIONS //
            deviationVec.emplace_back(std::move(aDev));

        }  // check stream is not at the end

    }  // end of loop on input file lines

    if (deviationVec.empty()) {
        ATH_MSG_WARNING("Could not read any alignment error configuration");
        return StatusCode::FAILURE;
    }
    writeCdo->setDeviations(std::move(deviationVec));

    if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
        ATH_MSG_FATAL("Could not record MuonAlignmentErrorData " << writeHandle.key() << " with EventRange " << rangeW
                                                                 << " into Conditions Store");
        return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("recorded new " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

    return StatusCode::SUCCESS;
}

std::tuple<std::string, EventIDRange> MuonAlignmentErrorDbAlg::getDbClobContent(const EventContext& ctx) const {
    // Read Cond Handle
    SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey, ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    // const CondAttrListCollection* atrc(0);
    // readCdo = *readHandle;
    if (readCdo == nullptr) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return std::make_tuple(std::string(), EventIDRange());
    }

    EventIDRange rangeW;
    if (!readHandle.range(rangeW)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << readHandle.key());
        return std::make_tuple(std::string(), EventIDRange());
    }

    ATH_MSG_INFO("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_INFO("Range of input is " << rangeW);

    // like MuonAlignmentErrorDbTool::loadAlignmentError() after retrieving atrc (readCdo)

    std::string clobContent, dbClobVersion;
    CondAttrListCollection::const_iterator itr;
    for (itr = readCdo->begin(); itr != readCdo->end(); ++itr) {
        const coral::AttributeList& atr = itr->second;
        clobContent = *(static_cast<const std::string*>((atr["syserrors"]).addressOfData()));
        dbClobVersion = *(static_cast<const std::string*>((atr["version"]).addressOfData()));
    }
    return std::make_tuple(std::move(clobContent), rangeW);
}

std::tuple<std::string, EventIDRange> MuonAlignmentErrorDbAlg::getFileClobContent() const {
    ATH_MSG_INFO("Retrieving alignment error CLOB from file override " << m_clobFileOverride.value());

    std::ifstream in(m_clobFileOverride.value());
    if (!in) {
        ATH_MSG_ERROR("Failed to read CLOB file " << m_clobFileOverride.value());
        return std::make_tuple(std::string(), EventIDRange());
    }
    return std::make_tuple(std::string(std::istreambuf_iterator<char>(in), {}), IOVInfiniteRange::infiniteTime());
}

