/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/MuonAlignmentCondAlg.h"

#include <fstream>
#include <map>
#include <string>

#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeListSpecification.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "MuonCondSvc/MdtStringUtils.h"
#include "MuonReadoutGeometry/GlobalUtilities.h"
#include "PathResolver/PathResolver.h"
#include "SGTools/TransientAddress.h"
#include "boost/algorithm/string/predicate.hpp"

MuonAlignmentCondAlg::MuonAlignmentCondAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthAlgorithm(name, pSvcLocator) {
   
}

StatusCode MuonAlignmentCondAlg::initialize() {
    ATH_MSG_INFO("Initilalizing");
    ATH_MSG_INFO("In initialize ---- # of folders registered is " << m_parlineFolder.size());

    // =======================
    // Loop on folders requested in configuration and check if /MUONALIGN/CSC/ILINES and /MUONALIGN/MDT/ASBUILTPARAMS are requested
    // =======================
    bool mmAsBuiltRequested{false};
    bool sTgcAsBuiltRequested{false};
    for (const std::string& currentFolderName : m_parlineFolder) {
        if (currentFolderName.find("ILINES") != std::string::npos) m_ILineRequested = true;
        if (currentFolderName.find("ASBUILTPARAMS") != std::string::npos) {
            if      (currentFolderName.find("MDT" ) != std::string::npos) m_MdtAsBuiltRequested = true;
            else if (currentFolderName.find("MM"  ) != std::string::npos) mmAsBuiltRequested    = true;
            else if (currentFolderName.find("STGC") != std::string::npos) sTgcAsBuiltRequested  = true;
        }
    }
    m_NswAsBuiltRequested = mmAsBuiltRequested && sTgcAsBuiltRequested;

    // Read Handles Keys
    ATH_CHECK(m_readMdtBarrelKey.initialize());
    ATH_CHECK(m_readMdtEndcapSideAKey.initialize());
    ATH_CHECK(m_readMdtEndcapSideCKey.initialize());
    ATH_CHECK(m_readTgcSideAKey.initialize());
    ATH_CHECK(m_readTgcSideCKey.initialize());
    ATH_CHECK(m_readCscILinesKey.initialize(m_idHelperSvc->hasCSC() && m_ILinesFromDb and m_ILineRequested));
    ATH_CHECK(m_readMdtAsBuiltParamsKey .initialize(m_MdtAsBuiltRequested));
    ATH_CHECK(m_readMmAsBuiltParamsKey  .initialize(m_NswAsBuiltRequested));
    ATH_CHECK(m_readSTgcAsBuiltParamsKey.initialize(m_NswAsBuiltRequested));

    // Write Handles
    ATH_CHECK(m_writeALineKey.initialize());
    ATH_CHECK(m_writeBLineKey.initialize());
    ATH_CHECK(m_writeILineKey.initialize(m_idHelperSvc->hasCSC() && m_ILinesFromDb and m_ILineRequested));
    ATH_CHECK(m_writeMdtAsBuiltKey.initialize(m_MdtAsBuiltRequested));
    ATH_CHECK(m_writeNswAsBuiltKey.initialize(m_NswAsBuiltRequested));
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::execute() {
    ATH_MSG_DEBUG("execute " << name());
    ATH_CHECK(loadParameters());
    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::loadParameters() {
    StatusCode sc = StatusCode::SUCCESS;

    ATH_MSG_DEBUG("In LoadParameters " << name());

    // =======================
    // Load ILine parameters if requested in the joboptions and /MUONALIGN/CSC/ILINES folder given in the joboptions
    // =======================
    if (m_idHelperSvc->hasCSC() && m_ILinesFromDb and m_ILineRequested) {
        sc = loadAlignILines("/MUONALIGN/CSC/ILINES");
    } else if (m_ILineRequested and not m_ILinesFromDb) {
        ATH_MSG_INFO("DB folder for I-Lines given in job options however loading disabled ");
        sc = StatusCode::SUCCESS;
    }
    if (sc.isFailure()) return StatusCode::FAILURE;

    // =======================
    // Load AsBuilt parameters if /MUONALIGN/MDT/ASBUILTPARAMS folder given in the joboptions
    // =======================
    if (m_MdtAsBuiltRequested) sc = loadMdtAlignAsBuilt("/MUONALIGN/MDT/ASBUILTPARAMS");
    if (sc.isFailure()) return StatusCode::FAILURE;

    // =======================
    // Load NSW AsBuilt parameters if both MM and STGC as-built folders are given in the joboptions
    // =======================
    if (m_NswAsBuiltRequested) sc = loadNswAlignAsBuilt("/MUONALIGN/ASBUILTPARAMS/MM", "/MUONALIGN/ASBUILTPARAMS/STGC");
    if (sc.isFailure()) return StatusCode::FAILURE;

    // =======================
    // Load A- and B-Lines
    // =======================
    sc = loadAlignABLines();
    if (sc.isFailure()) return StatusCode::FAILURE;

    return sc;
}

StatusCode MuonAlignmentCondAlg::loadAlignABLines() {

    StatusCode sc = StatusCode::SUCCESS;
    // =======================
    // Write ALine Cond Handle
    // =======================
    SG::WriteCondHandle<ALineMapContainer> writeALineHandle{m_writeALineKey};
    if (writeALineHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeALineHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<ALineMapContainer> writeALineCdo{std::make_unique<ALineMapContainer>()};

    // =======================
    // Write BLine Cond Handle
    // =======================
    SG::WriteCondHandle<BLineMapContainer> writeBLineHandle{m_writeBLineKey};
    if (writeBLineHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeBLineHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<BLineMapContainer> writeBLineCdo{std::make_unique<BLineMapContainer>()};

    // =======================
    // Loop on folders requested in configuration and process the ones that have A- and B-lines
    // =======================

    EventIDRange rangeALineW;
    EventIDRange rangeBLineW;
    for (const std::string& currentFolderName : m_parlineFolder) {
        if (currentFolderName.find("ILINES") != std::string::npos) continue;
        if (currentFolderName.find("ASBUILTPARAMS") != std::string::npos) continue;
        // Process the current folder for the A- and B-lines
        if (loadAlignABLines(currentFolderName, writeALineCdo.get(), writeBLineCdo.get(), rangeALineW, rangeBLineW).isSuccess()) {
            ATH_MSG_INFO("A- and B-Lines parameters from DB folder <" << currentFolderName << "> loaded");
        } else {
            ATH_MSG_ERROR("A- and B-Lines parameters from DB folder <" << currentFolderName << "> failed to load");
            return StatusCode::FAILURE;
        }
    }

    // After all the folders have been processed, if the A-Lines are empty we are on MC
    if (!m_aLinesFile.empty() && (int)writeALineCdo.get()->size() == 0) {
      sc = setALinesFromAscii(writeALineCdo.get());
      if (sc.isFailure()) return StatusCode::FAILURE;
    }

    if (writeALineHandle.record(rangeALineW, std::move(writeALineCdo)).isFailure()) {
        ATH_MSG_FATAL("Could not record ALineMapContainer " << writeALineHandle.key() << " with EventRange " << rangeALineW
                                                            << " into Conditions Store");
        return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("recorded new " << writeALineHandle.key() << " with range " << rangeALineW << " into Conditions Store");

    if (writeBLineHandle.record(rangeBLineW, std::move(writeBLineCdo)).isFailure()) {
        ATH_MSG_FATAL("Could not record BLineMapContainer " << writeBLineHandle.key() << " with EventRange " << rangeBLineW
                                                            << " into Conditions Store");
        return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("recorded new " << writeBLineHandle.key() << " with range " << rangeBLineW << " into Conditions Store");

    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::loadAlignABLinesData(const std::string& folderName, const std::string& data, nlohmann::json& json, bool hasBLine) {

    using namespace MuonCalib;
    // Parse corrections
    char delimiter = '\n';

    json = nlohmann::json::array();
    auto lines = MdtStringUtils::tokenize(data, delimiter);
    for (const std::string_view& blobline : lines) {
        nlohmann::json line;
        char delimiter = ':';
        const auto tokens = MdtStringUtils::tokenize(blobline, delimiter);

        // Check if tokens is not empty
        if (tokens.empty()) {
            ATH_MSG_FATAL("Empty string retrieved from DB in folder " << folderName);
            return StatusCode::FAILURE;
        }
        const std::string_view &type = tokens[0];
        // Parse line
        if (type[0] == '#') {
            // skip it
            continue;
        }
        if (type.compare(0, 4, "Corr") == 0) {
            //#: Corr line is counter typ,  jff,  jzz, job,                         * Chamber information
            //#:                       svalue,  zvalue, tvalue,  tsv,  tzv,  ttv,   * A lines
            //#:                       bz, bp, bn, sp, sn, tw, pg, tr, eg, ep, en   * B lines
            //#:                       chamber                                      * Chamber name
            //.... example
            // Corr: EMS  4   1  0     2.260     3.461    28.639 -0.002402 -0.002013  0.000482    -0.006    -0.013 -0.006000  0.000000
            // 0.000000     0.026    -0.353  0.000000  0.070000  0.012000    -0.012    EMS1A08

            constexpr char delimiter = ' ';
            auto tokens = MdtStringUtils::tokenize(blobline, delimiter);

            // Check if tokens has the right length
            // if (tokens.size() != 12 and tokens.size() != 23) {
            if (tokens.size() != 25) {
                ATH_MSG_FATAL("Invalid length in string retrieved from DB in folder " << folderName << " String length is "
                                                                                      << tokens.size());
                return StatusCode::FAILURE;
            }

            ATH_MSG_VERBOSE("Parsing Line = ");
            for (const std::string_view& token : tokens) ATH_MSG_VERBOSE(token << " | ");
            ATH_MSG_VERBOSE(" ");

            bool thisRowHasBLine = true;
            if (tokens.size() < 15) {
                // only A-lines ..... old COOL blob convention for barrel
                thisRowHasBLine = false;
                ATH_MSG_VERBOSE("(old COOL blob convention for barrel) skipping B-line decoding ");
            }

            // Start parsing
            int ival = 1;

            // Station Component identification
            int jff{0}, jzz{0}, job{0};
            std::string stationType = std::string(tokens[ival++]);
            line["typ"] = std::move(stationType);
            jff = MdtStringUtils::atoi(tokens[ival++]);
            line["jff"] = jff;
            jzz = MdtStringUtils::atoi(tokens[ival++]);
            line["jzz"] = jzz;
            job = MdtStringUtils::atoi(tokens[ival++]);
            line["job"] = job;

            // A-line
            float s{0.f}, z{0.f}, t{0.f}, ths{0.f}, thz{0.f}, tht{0.f};
            s = MdtStringUtils::stof(tokens[ival++]);
            line["svalue"] = s;
            z = MdtStringUtils::stof(tokens[ival++]);
            line["zvalue"] = z;
            t = MdtStringUtils::stof(tokens[ival++]);
            line["tvalue"] = t;
            ths = MdtStringUtils::stof(tokens[ival++]);
            line["tsv"] = ths;
            thz = MdtStringUtils::stof(tokens[ival++]);
            line["tzv"] = thz;
            tht = MdtStringUtils::stof(tokens[ival++]);
            line["ttv"] = tht;

            // B-line
            float bz{0.f}, bp{0.f}, bn{0.f}, sp{0.f}, sn{0.f}, tw{0.f}, pg{0.f}, tr{0.f}, eg{0.f}, ep{0.f}, en{0.f};
            float xAtlas{0.f}, yAtlas{0.f};

            if (hasBLine && thisRowHasBLine) {
                bz = MdtStringUtils::stof(tokens[ival++]);
                line["bz"] = bz;
                bp = MdtStringUtils::stof(tokens[ival++]);
                line["bp"] = bp;
                bn = MdtStringUtils::stof(tokens[ival++]);
                line["bn"] = bn;
                sp = MdtStringUtils::stof(tokens[ival++]);
                line["sp"] = sp;
                sn = MdtStringUtils::stof(tokens[ival++]);
                line["sn"] = sn;
                tw = MdtStringUtils::stof(tokens[ival++]);
                line["tw"] = tw;
                pg = MdtStringUtils::stof(tokens[ival++]);
                line["pg"] = pg;
                tr = MdtStringUtils::stof(tokens[ival++]);
                line["tr"] = tr;
                eg = MdtStringUtils::stof(tokens[ival++]);
                line["eg"] = eg;
                ep = MdtStringUtils::stof(tokens[ival++]);
                line["ep"] = ep;
                en = MdtStringUtils::stof(tokens[ival++]);
                line["en"] = en;

                xAtlas = MdtStringUtils::stof(tokens[ival++]);
                line["xAtlas"] = xAtlas;
                yAtlas = MdtStringUtils::stof(tokens[ival++]);
                line["yAtlas"] = yAtlas;

                // ChamberName (hardware convention)
                line["hwElement"] = std::string(tokens[ival++]);
            }
        }
        if (line.empty()) continue;
        json.push_back(std::move(line));
    }
    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::loadAlignABLines(const std::string& folderName, ALineMapContainer* writeALineCdo,
                                                  BLineMapContainer* writeBLineCdo, EventIDRange& rangeALineW, EventIDRange& rangeBLineW) {
    ATH_MSG_INFO("Load alignment parameters from DB folder <" << folderName << ">");

    bool hasBLine = true;
    if (folderName.find("TGC") != std::string::npos) {
        hasBLine = false;
        ATH_MSG_INFO("No BLines decoding will be attempted for folder named " << folderName);
    }

    // =======================
    // Read Cond Handles and ranges for current folder
    // =======================
    EventIDRange rangeALinesTemp;
    EventIDRange rangeBLinesTemp;
    const CondAttrListCollection* readCdo = nullptr;
    if (folderName.find("/MUONALIGN/MDT/BARREL") != std::string::npos) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readMdtBarrelKey};
        readCdo = *readHandle;
        if (readCdo == nullptr) {
            ATH_MSG_ERROR("Null pointer to the read ALINES conditions object");
            return StatusCode::FAILURE;
        }

        if (!readHandle.range(rangeALinesTemp)) {
            ATH_MSG_ERROR("Failed to retrieve ALines validity range for " << readHandle.key());
            return StatusCode::FAILURE;
        }
        if (!readHandle.range(rangeBLinesTemp)) {
            ATH_MSG_ERROR("Failed to retrieve BLines validity range for " << readHandle.key());
            return StatusCode::FAILURE;
        }

        ATH_MSG_INFO("Size of " << folderName << " CondAttrListCollection " << readHandle.fullKey()
                                << " readCdo->size()= " << readCdo->size());
        ATH_MSG_INFO("Range of " << folderName << " input is, ALines: " << rangeALinesTemp << " BLines: " << rangeBLinesTemp);

    } else if (folderName.find("/MUONALIGN/MDT/ENDCAP/SIDEA") != std::string::npos) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readMdtEndcapSideAKey};
        readCdo = *readHandle;
        if (readCdo == nullptr) {
            ATH_MSG_ERROR("Null pointer to the read ALINES conditions object");
            return StatusCode::FAILURE;
        }

        if (!readHandle.range(rangeALinesTemp)) {
            ATH_MSG_ERROR("Failed to retrieve ALines validity range for " << readHandle.key());
            return StatusCode::FAILURE;
        }
        if (!readHandle.range(rangeBLinesTemp)) {
            ATH_MSG_ERROR("Failed to retrieve BLines validity range for " << readHandle.key());
            return StatusCode::FAILURE;
        }

        ATH_MSG_INFO("Size of " << folderName << " CondAttrListCollection " << readHandle.fullKey()
                                << " readCdo->size()= " << readCdo->size());
        ATH_MSG_INFO("Range of " << folderName << " input is, ALines: " << rangeALinesTemp << " BLines: " << rangeBLinesTemp);

    } else if (folderName.find("/MUONALIGN/MDT/ENDCAP/SIDEC") != std::string::npos) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readMdtEndcapSideCKey};
        readCdo = *readHandle;
        if (readCdo == nullptr) {
            ATH_MSG_ERROR("Null pointer to the read ALINES conditions object");
            return StatusCode::FAILURE;
        }

        if (!readHandle.range(rangeALinesTemp)) {
            ATH_MSG_ERROR("Failed to retrieve ALines validity range for " << readHandle.key());
            return StatusCode::FAILURE;
        }
        if (!readHandle.range(rangeBLinesTemp)) {
            ATH_MSG_ERROR("Failed to retrieve BLines validity range for " << readHandle.key());
            return StatusCode::FAILURE;
        }

        ATH_MSG_INFO("Size of " << folderName << " CondAttrListCollection " << readHandle.fullKey()
                                << " readCdo->size()= " << readCdo->size());
        ATH_MSG_INFO("Range of " << folderName << " input is, ALines: " << rangeALinesTemp << " BLines: " << rangeBLinesTemp);

    } else if (folderName.find("/MUONALIGN/TGC/SIDEA") != std::string::npos) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readTgcSideAKey};
        readCdo = *readHandle;
        if (readCdo == nullptr) {
            ATH_MSG_ERROR("Null pointer to the read ALINES conditions object");
            return StatusCode::FAILURE;
        }

        if (!readHandle.range(rangeALinesTemp)) {
            ATH_MSG_ERROR("Failed to retrieve ALines validity range for " << readHandle.key());
            return StatusCode::FAILURE;
        }

        // !!!!!!!! NO BLINES FOR TGCs !!!!!!!!!!!!!!

        ATH_MSG_INFO("Size of " << folderName << " CondAttrListCollection " << readHandle.fullKey()
                                << " readCdo->size()= " << readCdo->size());
        ATH_MSG_INFO("Range of " << folderName << " input is, ALines: " << rangeALinesTemp);

    } else if (folderName.find("/MUONALIGN/TGC/SIDEC") != std::string::npos) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readTgcSideCKey};
        readCdo = *readHandle;
        if (readCdo == nullptr) {
            ATH_MSG_ERROR("Null pointer to the read ALINES conditions object");
            return StatusCode::FAILURE;
        }

        if (!readHandle.range(rangeALinesTemp)) {
            ATH_MSG_ERROR("Failed to retrieve ALines validity range for " << readHandle.key());
            return StatusCode::FAILURE;
        }
        // !!!!!!!! NO BLINES FOR TGCs !!!!!!!!!!!!!!

        ATH_MSG_INFO("Size of " << folderName << " CondAttrListCollection " << readHandle.fullKey()
                                << " readCdo->size()= " << readCdo->size());
        ATH_MSG_INFO("Range of " << folderName << " input is, ALines: " << rangeALinesTemp);
    }

    // Combine the validity ranges for ALines
    EventIDRange rangeIntersection{EventIDRange::intersect(rangeALineW, rangeALinesTemp)};
    ATH_MSG_DEBUG("rangeIntersection: " << rangeIntersection << " Previous range: " << rangeALineW
                                        << " Current range : " << rangeALinesTemp);
    if (rangeIntersection.stop().isValid() and rangeIntersection.start() > rangeIntersection.stop()) {
        ATH_MSG_FATAL("Invalid intersection range: " << rangeIntersection);
        return StatusCode::FAILURE;
    }
    rangeALineW = rangeIntersection;

    // Combine the validity ranges for BLines
    if (hasBLine) {
        EventIDRange rangeIntersection{EventIDRange::intersect(rangeBLineW, rangeBLinesTemp)};
        ATH_MSG_DEBUG("rangeIntersection: " << rangeIntersection << " Previous range: " << rangeALineW
                                            << " Current range: " << rangeALinesTemp);
        if (rangeIntersection.stop().isValid() and rangeIntersection.start() > rangeIntersection.stop()) {
            ATH_MSG_FATAL("Invalid intersection range: " << rangeIntersection);
            return StatusCode::FAILURE;
        }
        rangeBLineW = rangeIntersection;
    }

    // =======================
    // retreive the collection of strings read out from the DB
    // =======================

    // unpack the strings in the collection and update the
    // ALlineContainer in TDS
    int nLines = 0;
    int nDecodedLines = 0;
    int nNewDecodedALines = 0;
    int nNewDecodedBLines = 0;
    CondAttrListCollection::const_iterator itr;
    for (itr = readCdo->begin(); itr != readCdo->end(); ++itr) {
        const coral::AttributeList& atr = itr->second;

        std::string data;
        if (atr["data"].specification().type() == typeid(coral::Blob)) {
            ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
            if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                return StatusCode::FAILURE;
            }
        } else {
            data = *(static_cast<const std::string*>((atr["data"]).addressOfData()));
        }
        // nlohmann::json lines = nlohmann::json::array();
        nlohmann::json lines;

        // new format -----------------------------------
        if (m_newFormat2020) {
            nlohmann::json j = nlohmann::json::parse(data);
            lines = j["corrections"];
        }

        // old format -----------------------------------
        else {
            if (loadAlignABLinesData(folderName, data, lines, hasBLine).isFailure()) return StatusCode::FAILURE;
        }

        // loop over corrections ------------------------
        for (auto& corr : lines.items()) {
            ++nLines;
            nlohmann::json line = corr.value();

            // Station Component identification
            int jff = line["jff"];
            int jzz = line["jzz"];
            int job = line["job"];
            std::string stationType = line["typ"];

            // A-line
            float s = line["svalue"];
            float z = line["zvalue"];
            float t = line["tvalue"];
            float ths = line["tsv"];
            float thz = line["tzv"];
            float tht = line["ttv"];

            // B-line
            bool thisRowHasBLine = false;
            float bz, bp, bn, sp, sn, tw, pg, tr, eg, ep, en;
            std::string ChamberHwName = "";
            if (line.find("bz") != line.end()) {
                bz = line["bz"];
                bp = line["bp"];
                bn = line["bn"];
                sp = line["sp"];
                sn = line["sn"];
                tw = line["tw"];
                pg = line["pg"];
                tr = line["tr"];
                eg = line["eg"];
                ep = line["ep"];
                en = line["en"];
                ChamberHwName = static_cast<std::string>(line["hwElement"]);
                thisRowHasBLine = true;
            }

            ATH_MSG_VERBOSE("Station type " << stationType);
            ATH_MSG_VERBOSE("jff / jzz / job " << jff << " / " << jzz << " / " << job);
            if (hasBLine) {
                ATH_MSG_VERBOSE(" HardwareChamberName " << ChamberHwName);
            } else
                ATH_MSG_VERBOSE(" ");
            ATH_MSG_VERBOSE("A-line: s,z,t " << s << " " << z << " " << t);
            ATH_MSG_VERBOSE(" ts,tz,tt " << ths << " " << thz << " " << tht);
            if (hasBLine) {
                if (thisRowHasBLine) {
                    ATH_MSG_VERBOSE("B-line:  bz,bp,bn " << bz << " " << bp << " " << bn);
                    ATH_MSG_VERBOSE(" sp,sn " << sp << " " << sn << " tw,pg,tr " << tw << " " << pg << " " << tr << " eg,ep,en " << eg
                                              << " " << ep << " " << en);
                } else
                    ATH_MSG_VERBOSE("No B-line found");
            }

            int stationName = m_idHelperSvc->mdtIdHelper().stationNameIndex(stationType);
            Identifier id;
            ATH_MSG_VERBOSE("stationName  " << stationName);
            if (stationType.at(0) == 'M') {
                // micromegas case
                if(!m_idHelperSvc->hasMM()) continue;  // skip if geometry does not include MMs (e.g. RUN1 or RUN2 data or RUN3 muon calibration stream)
                id = m_idHelperSvc->mmIdHelper().elementID(stationName, jzz, jff);
                id = m_idHelperSvc->mmIdHelper().multilayerID(id, job);
                ATH_MSG_VERBOSE("identifier being assigned is " << m_idHelperSvc->mmIdHelper().show_to_string(id));
            } else if (stationType.at(0) == 'S') {
                // sTGC case
                if(!m_idHelperSvc->hasSTGC()) continue;  // skip if geometry does not include MMs (e.g. RUN1 or RUN2 data or RUN3 muon calibration stream)
                id = m_idHelperSvc->stgcIdHelper().elementID(stationName, jzz, jff);
                id = m_idHelperSvc->stgcIdHelper().multilayerID(id, job);
                ATH_MSG_VERBOSE("identifier being assigned is " << m_idHelperSvc->stgcIdHelper().show_to_string(id)); 
            } else if (stationType.at(0) == 'T') {
                // tgc case
                int stPhi = MuonGM::stationPhiTGC(stationType, jff, jzz);  // !!!!! The stationPhiTGC implementation in this package is NOT used !!!!!
                int stEta = 1;
                if (jzz < 0) stEta = -1;
                if (job != 0) {
                    // this should become the default now
                    stEta = job;
                    if (jzz < 0) stEta = -stEta;
                }
                id = m_idHelperSvc->tgcIdHelper().elementID(stationName, stEta, stPhi);
                ATH_MSG_VERBOSE("identifier being assigned is " << m_idHelperSvc->tgcIdHelper().show_to_string(id));
            } else if (stationType.substr(0, 1) == "C") {
                // csc case
  	             if(!m_idHelperSvc->hasCSC()) continue; //skip if geometry doesn't include CSCs
                id = m_idHelperSvc->cscIdHelper().elementID(stationName, jzz, jff);
                ATH_MSG_VERBOSE("identifier being assigned is " << m_idHelperSvc->cscIdHelper().show_to_string(id));
            } else if (stationType.substr(0, 3) == "BML" && abs(jzz) == 7) {
                // rpc case
                id = m_idHelperSvc->rpcIdHelper().elementID(stationName, jzz, jff, 1);
                ATH_MSG_VERBOSE("identifier being assigned is " << m_idHelperSvc->rpcIdHelper().show_to_string(id));
            } else {
                id = m_idHelperSvc->mdtIdHelper().elementID(stationName, jzz, jff);
                ATH_MSG_VERBOSE("identifier being assigned is " << m_idHelperSvc->mdtIdHelper().show_to_string(id));
            }

            // new Aline
            ++nDecodedLines;
            ++nNewDecodedALines;
            ALinePar newALine;
            newALine.setAmdbId(stationType, jff, jzz, job);
            newALine.setParameters(s, z, t, ths, thz, tht);
            newALine.isNew(true);
            if (!writeALineCdo->insert_or_assign(id, std::move(newALine)).second) {
                ATH_MSG_WARNING("More than one (A-line) entry in folder " << folderName << " for  " << stationType << " at Jzz/Jff " << jzz
                                                                          << "/" << jff << " --- keep the latest one");
                --nNewDecodedALines;
            }

            if (hasBLine && thisRowHasBLine) {
                // new Bline
                ++nNewDecodedBLines;
                BLinePar newBLine;
                newBLine.setAmdbId(stationType, jff, jzz, job);
                newBLine.setParameters(bz, bp, bn, sp, sn, tw, pg, tr, eg, ep, en);
                newBLine.isNew(true);
                if (!writeBLineCdo->insert_or_assign(id, std::move(newBLine)).second) {
                    ATH_MSG_WARNING("More than one (B-line) entry in folder " << folderName << " for  " << stationType << " at Jzz/Jff "
                                                                              << jzz << "/" << jff << " --- keep the latest one");
                    --nNewDecodedBLines;
                }
            }
        }
    }
    ATH_MSG_DEBUG("In folder < " << folderName << "> # lines/decodedLines/newDecodedALines/newDecodedBLines= " << nLines << "/"
                                 << nDecodedLines << "/" << nNewDecodedALines << "/" << nNewDecodedBLines);

    // set A-lines from ASCII file
    StatusCode sc = StatusCode::SUCCESS;
    if (!m_aLinesFile.empty() && (int)writeALineCdo->size() > 0) {
      sc = setALinesFromAscii(writeALineCdo);
      if (sc.isFailure()) return StatusCode::FAILURE;
    }

    // dump A-lines to log file
    if (m_dumpALines && (int)writeALineCdo->size() > 0) dumpALines(folderName, writeALineCdo);

    // dump B-lines to log file
    if (m_dumpBLines && (int)writeBLineCdo->size() > 0) dumpBLines(folderName, writeBLineCdo);

    // !!!!!!!!!!!!!! In the MuonAlignmentDbSvc this was here. I moved it to loadMdtAlignAsBuilt
    // if ( m_asBuiltFile!="" ) setAsBuiltFromAscii();

    ATH_MSG_VERBOSE("Collection CondAttrListCollection CLID " << readCdo->clID());

    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::loadAlignILines(const std::string& folderName) {
    // =======================
    // Write ILine Cond Handle
    // =======================
    SG::WriteCondHandle<CscInternalAlignmentMapContainer> writeHandle{m_writeILineKey};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<CscInternalAlignmentMapContainer> writeCdo{std::make_unique<CscInternalAlignmentMapContainer>()};

    ATH_MSG_INFO("Load alignment parameters from DB folder <" << folderName << ">");

    // =======================
    // Read CSC/ILINES Cond Handle
    // =======================
    SG::ReadCondHandle<CondAttrListCollection> readCscILinesHandle{m_readCscILinesKey};
    const CondAttrListCollection* readCscILinesCdo{*readCscILinesHandle};
    if (!readCscILinesCdo) {
        ATH_MSG_ERROR("Null pointer to the read CSC/ILINES conditions object");
        return StatusCode::FAILURE;
    }

    EventIDRange rangeCscILinesW;
    if (!readCscILinesHandle.range(rangeCscILinesW)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << readCscILinesHandle.key());
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Size of CSC/ILINES CondAttrListCollection " << readCscILinesHandle.fullKey()
                                                              << " readCscILinesCdo->size()= " << readCscILinesCdo->size());
    ATH_MSG_INFO("Range of CSC/ILINES input is " << rangeCscILinesW);

    // =======================
    // Retrieve the collection of strings read out from the DB
    // =======================
    // unpack the strings in the collection and update the
    // ALlineContainer in TDS
    int nLines = 0;
    int nDecodedLines = 0;
    int nNewDecodedILines = 0;
    CondAttrListCollection::const_iterator itr;
    for (itr = readCscILinesCdo->begin(); itr != readCscILinesCdo->end(); ++itr) {
        const coral::AttributeList& atr = itr->second;

        std::string data;
        if (atr["data"].specification().type() == typeid(coral::Blob)) {
            ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
            if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                return StatusCode::FAILURE;
            }
        } else {
            data = *(static_cast<const std::string*>((atr["data"]).addressOfData()));
        }
        ATH_MSG_DEBUG("Data load is " << data << " FINISHED HERE ");
        nlohmann::json lines = nlohmann::json::array();

        // new format -----------------------------------
        if (m_newFormat2020) {
            nlohmann::json j = nlohmann::json::parse(data);
            lines = j["corrections"];
        }

        // old format -----------------------------------
        else if (m_idHelperSvc->hasCSC() && loadAlignILinesData(folderName, data, lines).isFailure()) return StatusCode::FAILURE;
        

        // loop over corrections ------------------------
        for (auto& corr : lines.items()) {
            ++nLines;
            nlohmann::json line = corr.value();

            // Station Component identification
            int jff = line["jff"];
            int jzz = line["jzz"];
            int job = line["job"];
            int jlay = line["jlay"];
            std::string stationType = line["typ"];

            // I-line
            float tras = line["tras"];
            float traz = line["traz"];
            float trat = line["trat"];
            float rots = line["rots"];
            float rotz = line["rotz"];
            float rott = line["rott"];

            ATH_MSG_VERBOSE("Station type " << stationType);
            ATH_MSG_VERBOSE("jff / jzz / job / jlay " << jff << " / " << jzz << " / " << job << " / " << jlay);
            ATH_MSG_VERBOSE("I-line: tras,traz,trat " << tras << " " << traz << " " << trat);
            ATH_MSG_VERBOSE(" rots,rotz,rott " << rots << " " << rotz << " " << rott);

            int stationName = m_idHelperSvc->cscIdHelper().stationNameIndex(stationType);
            Identifier id;
            ATH_MSG_VERBOSE("stationName  " << stationName);
            // if (stationType.substr(0,1)=="C") {
            if (stationType.at(0) == 'C') {
                // csc case
                int chamberLayer = 2;
                if (job != 3) ATH_MSG_WARNING("job = " << job << " is not 3 => chamberLayer should be 1 - not existing ! setting 2");
                id = m_idHelperSvc->cscIdHelper().channelID(stationType, jzz, jff, chamberLayer, jlay, 0, 1);
                ATH_MSG_VERBOSE("identifier being assigned is " << m_idHelperSvc->cscIdHelper().show_to_string(id));
            } else {
                ATH_MSG_ERROR("There is a non CSC chamber in the list of CSC internal alignment parameters.");
            }
            // new Iline
            ++nDecodedLines;
            ++nNewDecodedILines;
            CscInternalAlignmentPar newILine;
            newILine.setAmdbId(stationType, jff, jzz, job, jlay);
            newILine.setParameters(tras, traz, trat, rots, rotz, rott);
            newILine.isNew(true);
            if (!writeCdo->insert_or_assign(id, newILine).second) {
                ATH_MSG_WARNING("More than one (I-line) entry in folder " << folderName << " for  " << stationType << " at Jzz/Jff/Jlay "
                                                                          << jzz << "/" << jff << "/" << jlay
                                                                          << " --- keep the latest one");
                --nNewDecodedILines;
            }
        }
    }
    ATH_MSG_DEBUG("In folder <" << folderName << "> # lines/decodedLines/newDecodedILines= " << nLines << "/" << nDecodedLines << "/"
                                << nNewDecodedILines << "/");

    // dump I-lines to log file TBA
    if (m_dumpILines && (int)writeCdo->size() > 0) dumpILines(folderName, writeCdo.get());

    if (writeHandle.record(rangeCscILinesW, std::move(writeCdo)).isFailure()) {
        ATH_MSG_FATAL("Could not record CscInternalAlignmentMapContainer " << writeHandle.key() << " with EventRange " << rangeCscILinesW
                                                                           << " into Conditions Store");
        return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("recorded new " << writeHandle.key() << " with range " << rangeCscILinesW << " into Conditions Store");

    ATH_MSG_VERBOSE("Collection CondAttrListCollection CLID " << readCscILinesCdo->clID());

    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::loadAlignILinesData(const std::string& folderName, const std::string& data, nlohmann::json& json) {
    using namespace MuonCalib;
    // Parse corrections
    char delimiter = '\n';

    json = nlohmann::json::array();
    auto lines = MuonCalib::MdtStringUtils::tokenize(data, delimiter);
    for (const std::string_view& blobline : lines) {
        nlohmann::json line;
        char delimiter = ':';
        auto tokens = MuonCalib::MdtStringUtils::tokenize(blobline, delimiter);
        // Check if tokens is not empty
        if (tokens.empty()) {
            ATH_MSG_FATAL("Empty string retrieved from DB in folder " << folderName);
            return StatusCode::FAILURE;
        }
        const std::string_view &type = tokens[0];
        // Parse line
        if ('#' == type[0]) {
            // skip it
            continue;
        }
        if (type.compare(0, 4, "Corr") == 0) {
            //# Amdb like clob for ilines using geometry tag ISZT-R06-02
            //# ISZT_DATA_ID VERS TYP JFF JZZ JOB JLAY TRAS TRAZ TRAT ROTS ROTZ ROTT
            //
            //.... example
            // Corr:  CSL 1 -1 3 1 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000

            char delimiter = ' ';
            auto tokens = MuonCalib::MdtStringUtils::tokenize(blobline, delimiter);
            if (tokens.size() != 12) {
                ATH_MSG_FATAL("Invalid length in string retrieved from DB in folder " << folderName << " String length is "
                                                                                      << tokens.size());
                return StatusCode::FAILURE;
            }

            ATH_MSG_VERBOSE("Parsing Line = ");
            for (const std::string_view& token : tokens) ATH_MSG_VERBOSE(token << " | ");
            ATH_MSG_VERBOSE(" ");

            // Start parsing
            int ival = 1;

            // Station Component identification
            int jff{0}, job{0}, jlay{0}, jzz{0};
            std::string stationType = std::string(tokens[ival++]);
            line["typ"] = std::move(stationType);
            jff = MdtStringUtils::atoi(tokens[ival++]);
            line["jff"] = jff;
            jzz = MdtStringUtils::atoi(tokens[ival++]);
            line["jzz"] = jzz;
            job = MdtStringUtils::atoi(tokens[ival++]);
            line["job"] = job;
            jlay = MdtStringUtils::atoi(tokens[ival++]);
            line["jlay"] = jlay;

            // I-line
            float tras{0.f}, traz{0.f}, trat{0.f}, rots{0.f}, rotz{0.f}, rott{0.f};
            tras = MdtStringUtils::stof(tokens[ival++]);
            line["tras"] = tras;
            traz = MdtStringUtils::stof(tokens[ival++]);
            line["traz"] = traz;
            trat = MdtStringUtils::stof(tokens[ival++]);
            line["trat"] = trat;
            rots = MdtStringUtils::stof(tokens[ival++]);
            line["rots"] = rots;
            rotz = MdtStringUtils::stof(tokens[ival++]);
            line["rotz"] = rotz;
            rott = MdtStringUtils::stof(tokens[ival++]);
            line["rott"] = rott;
        }
        if (line.empty()) continue;
        json.push_back(std::move(line));
    }
    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::loadMdtAlignAsBuilt(const std::string& folderName) {
    // =======================
    // Write AsBuilt Cond Handle
    // =======================
    SG::WriteCondHandle<MdtAsBuiltMapContainer> writeHandle{m_writeMdtAsBuiltKey};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<MdtAsBuiltMapContainer> writeCdo{std::make_unique<MdtAsBuiltMapContainer>()};

    ATH_MSG_INFO("Load alignment parameters from DB folder <" << folderName << ">");

    // =======================
    // Read MDT/ASBUILTPARAMS Cond Handle
    // =======================
    SG::ReadCondHandle<CondAttrListCollection> readMdtAsBuiltHandle{m_readMdtAsBuiltParamsKey};
    const CondAttrListCollection* readMdtAsBuiltCdo{*readMdtAsBuiltHandle};
    if (readMdtAsBuiltCdo == nullptr) {
        ATH_MSG_ERROR("Null pointer to the read MDT/ASBUILTPARAMS conditions object");
        return StatusCode::FAILURE;
    }

    EventIDRange rangeMdtAsBuiltW;
    if (!readMdtAsBuiltHandle.range(rangeMdtAsBuiltW)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << readMdtAsBuiltHandle.key());
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Size of MDT/ASBUILTPARAMS CondAttrListCollection " << readMdtAsBuiltHandle.fullKey()
                                                                     << " ->size()= " << readMdtAsBuiltCdo->size());
    ATH_MSG_INFO("Range of MDT/ASBUILTPARAMS input is " << rangeMdtAsBuiltW);

    // =======================
    // Retrieve the collection of strings read out from the DB
    // =======================
    // unpack the strings in the collection and update the
    // AsBuiltContainer in TDS
    int nLines = 0;
    int nDecodedLines = 0;
    int nNewDecodedAsBuilt = 0;
    MdtAsBuiltPar xPar;
    xPar.isNew(true);
    CondAttrListCollection::const_iterator itr;
    for (itr = readMdtAsBuiltCdo->begin(); itr != readMdtAsBuiltCdo->end(); ++itr) {
        const coral::AttributeList& atr = itr->second;
        std::string data;
        data = *(static_cast<const std::string*>((atr["data"]).addressOfData()));

        ATH_MSG_DEBUG("Data load is " << data << " FINISHED HERE ");

        // Parse corrections
        char delimiter = '\n';

        auto lines = MuonCalib::MdtStringUtils::tokenize(data, delimiter);
        for (const std::string_view& blobline : lines) {
            ++nLines;

            char delimiter = ':';
            auto tokens = MuonCalib::MdtStringUtils::tokenize(blobline, delimiter);
            // Check if tokens is not empty
            if (tokens.empty()) {
                ATH_MSG_FATAL("Empty string retrieved from DB in folder " << folderName);
                return StatusCode::FAILURE;
            }
            const std::string_view &type = tokens[0];
            // Parse line
            if (type[0] == '#') {
                // skip it
                continue;
            }

            if (type.compare(0, 4, "Corr") == 0) {
                if (!xPar.setFromAscii(std::string(blobline))) {
                    ATH_MSG_ERROR("Unable to parse AsBuilt params from Ascii line: " << blobline);
                    continue;
                }

                std::string stationType = "XXX";
                int jff {0},  jzz{0}, job{0};
                xPar.getAmdbId(stationType, jff, jzz, job);
                Identifier id = m_idHelperSvc->mdtIdHelper().elementID(stationType, jzz, jff);

                ATH_MSG_VERBOSE("Station type jff jzz " << stationType << jff << " " << jzz);
                ++nDecodedLines;
                ++nNewDecodedAsBuilt;
                if (!writeCdo->insert_or_assign(id, xPar).second) {
                    ATH_MSG_WARNING("More than one (As-built) entry in folder " << folderName << " for  " << stationType << " at Jzz/Jff "
                                                                                << jzz << "/" << jff << " --- keep the latest one");
                    --nNewDecodedAsBuilt;
                }
            }
        }
    }
    ATH_MSG_DEBUG("In folder <" << folderName << "> # lines/decodedLines/newDecodedILines= " << nLines << "/" << nDecodedLines << "/"
                                << nNewDecodedAsBuilt << "/");

    // !!!!!!!!!!!!!! In the MuonAlignmentDbTool this was in loadAlignABLines. I moved it here
    if (!m_asBuiltFile.empty()) setAsBuiltFromAscii(writeCdo.get());

    if (writeHandle.record(rangeMdtAsBuiltW, std::move(writeCdo)).isFailure()) {
        ATH_MSG_FATAL("Could not record MdtAsBuiltMapContainer " << writeHandle.key() << " with EventRange " << rangeMdtAsBuiltW
                                                                 << " into Conditions Store");
        return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("recorded new " << writeHandle.key() << " with range " << rangeMdtAsBuiltW << " into Conditions Store");

    ATH_MSG_VERBOSE("Collection CondAttrListCollection CLID " << readMdtAsBuiltCdo->clID());

    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::loadNswAlignAsBuilt(const std::string& mmFolderName, const std::string& sTgcFolderName) {
    // =======================
    // Write AsBuilt Cond Handle
    // =======================
    SG::WriteCondHandle<NswAsBuiltDbData> writeHandle{m_writeNswAsBuiltKey};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<NswAsBuiltDbData> writeCdo{std::make_unique<NswAsBuiltDbData>()};

    ATH_MSG_INFO("Load alignment parameters from DB folders <" << mmFolderName << "> and <" << sTgcFolderName << ">");

    // =======================
    // Read MM/ASBUILTPARAMS Cond Handle
    // =======================
    SG::ReadCondHandle<CondAttrListCollection> readMmAsBuiltHandle{m_readMmAsBuiltParamsKey};
    const CondAttrListCollection* readMmAsBuiltCdo{*readMmAsBuiltHandle};
    if (readMmAsBuiltCdo == nullptr) {
        ATH_MSG_ERROR("Null pointer to the read MM/ASBUILTPARAMS conditions object");
        return StatusCode::FAILURE;
    }

    EventIDRange rangeMmAsBuiltW;
    if (!readMmAsBuiltHandle.range(rangeMmAsBuiltW)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << readMmAsBuiltHandle.key());
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Size of MM/ASBUILTPARAMS CondAttrListCollection " << readMmAsBuiltHandle.fullKey()
                                                                     << " ->size()= " << readMmAsBuiltCdo->size());
    ATH_MSG_INFO("Range of MM/ASBUILTPARAMS input is " << rangeMmAsBuiltW);

    // =======================
    // Read STGC/ASBUILTPARAMS Cond Handle
    // =======================
    SG::ReadCondHandle<CondAttrListCollection> readSTgcAsBuiltHandle{m_readSTgcAsBuiltParamsKey};
    const CondAttrListCollection* readSTgcAsBuiltCdo{*readSTgcAsBuiltHandle};
    if (readSTgcAsBuiltCdo == nullptr) {
        ATH_MSG_ERROR("Null pointer to the read STGC/ASBUILTPARAMS conditions object");
        return StatusCode::FAILURE;
    }

    EventIDRange rangeSTgcAsBuiltW;
    if (!readSTgcAsBuiltHandle.range(rangeSTgcAsBuiltW)) {
        ATH_MSG_ERROR("Failed to retrieve validity range for " << readSTgcAsBuiltHandle.key());
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Size of STGC/ASBUILTPARAMS CondAttrListCollection " << readSTgcAsBuiltHandle.fullKey()
                                                                     << " ->size()= " << readSTgcAsBuiltCdo->size());
    ATH_MSG_INFO("Range of STGC/ASBUILTPARAMS input is " << rangeSTgcAsBuiltW);

    // =======================
    // Retrieve the collection of strings read out from the DB
    // =======================
    unsigned int nLines = 0;
    CondAttrListCollection::const_iterator itr;
    for(itr = readMmAsBuiltCdo->begin(); itr != readMmAsBuiltCdo->end(); ++itr) {
        const coral::AttributeList& atr = itr->second;
        std::string data;
        data = *(static_cast<const std::string*>((atr["data"]).addressOfData()));
        ATH_MSG_DEBUG("Data load is " << data << " FINISHED HERE ");
        writeCdo->setMmData(data);
        nLines++;
    }
    
    if(nLines>1) ATH_MSG_WARNING(nLines << " data objects were loaded for MM/ASBUILTPARAMS! Expected only one for this validity range!");
    
    nLines = 0;
    for(itr = readSTgcAsBuiltCdo->begin(); itr != readSTgcAsBuiltCdo->end(); ++itr) {
        const coral::AttributeList& atr = itr->second;
        std::string data;
        data = *(static_cast<const std::string*>((atr["data"]).addressOfData()));
        ATH_MSG_DEBUG("Data load is " << data << " FINISHED HERE ");
        writeCdo->setSTgcData(data);
        nLines++;
    }
    if(nLines>1) ATH_MSG_WARNING(nLines << " data objects were loaded for STGC/ASBUILTPARAMS! Expected only one for this validity range!");

    EventIDRange rangeIntersection{EventIDRange::intersect(rangeMmAsBuiltW, rangeSTgcAsBuiltW)};
    if (writeHandle.record(rangeIntersection, std::move(writeCdo)).isFailure()) {
        ATH_MSG_FATAL("Could not record NswAsBuiltMapContainer " << writeHandle.key() << " with EventRange " << rangeMmAsBuiltW << " into Conditions Store");
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

void MuonAlignmentCondAlg::dumpALines(const std::string& folderName, ALineMapContainer* writeALineCdo) {
    ATH_MSG_INFO("dumping A-lines for folder " << folderName);
    ATH_MSG_INFO("A type jff jzz job s(cm)  z(cm)  t(cm)  ths(rad)  thz(rad)  tht(rad)  ID");

    for (const auto& [ALineId, ALine] : *writeALineCdo) {
        std::string stationType;
        int jff, jzz, job;
        ALine.getAmdbId(stationType, jff, jzz, job);
        float s, z, t, ths, thz, tht;
        ALine.getParameters(s, z, t, ths, thz, tht);

        ATH_MSG_INFO("A " << std::setiosflags(std::ios::fixed | std::ios::right) << std::setw(4) << stationType << " " << std::setw(2)
                          << jff << "  " << std::setw(2) << jzz << " " << std::setw(2) << job << " " << std::setw(6) << std::setprecision(3)
                          << s << " "                                          // here cm !
                          << std::setw(6) << std::setprecision(3) << z << " "  // here cm !
                          << std::setw(6) << std::setprecision(3) << t << " "  // here cm !
                          << std::setw(6) << std::setprecision(6) << ths << " " << std::setw(6) << std::setprecision(6) << thz << " "
                          << std::setw(6) << std::setprecision(6) << tht << " " << ALineId);
    }
}

void MuonAlignmentCondAlg::dumpBLines(const std::string& folderName, BLineMapContainer* writeBLineCdo) {
    ATH_MSG_INFO("dumping B-lines for folder " << folderName);
    ATH_MSG_INFO(
        "B type jff jzz job bs       bp        bn        sp        sn        tw        pg        tr        eg        ep        en        "
        "ID");

    for (const auto& [BLineId, BLine] : *writeBLineCdo) {
        std::string stationType;
        int jff, jzz, job;
        BLine.getAmdbId(stationType, jff, jzz, job);
        float bs, bp, bn, sp, sn, tw, pg, tr, eg, ep, en;
        BLine.getParameters(bs, bp, bn, sp, sn, tw, pg, tr, eg, ep, en);

        ATH_MSG_INFO("B " << std::setiosflags(std::ios::fixed | std::ios::right) << std::setw(4) << stationType << " " << std::setw(2)
                          << jff << " " << std::setw(2) << jzz << " " << std::setw(2) << job << "  " << std::setw(6) << std::setprecision(6)
                          << bs << " " << std::setw(6) << std::setprecision(6) << bp << " " << std::setw(6) << std::setprecision(6) << bn
                          << " " << std::setw(6) << std::setprecision(6) << sp << " " << std::setw(6) << std::setprecision(6) << sn << " "
                          << std::setw(6) << std::setprecision(6) << tw << " " << std::setw(6) << std::setprecision(6) << pg << " "
                          << std::setw(6) << std::setprecision(6) << tr << " " << std::setw(6) << std::setprecision(6) << eg << " "
                          << std::setw(6) << std::setprecision(6) << ep << " " << std::setw(6) << std::setprecision(6) << en << " "
                          << BLineId);
    }
}

void MuonAlignmentCondAlg::dumpILines(const std::string& folderName, CscInternalAlignmentMapContainer* writeCdo) {
    ATH_MSG_INFO("dumping I-lines for folder " << folderName);
    ATH_MSG_INFO("I \ttype\tjff\tjzz\tjob\tjlay\ttras\ttraz\ttrat\trots\trotz\trott");

    for (const auto& [ILineId, ILine] : *writeCdo) {
        std::string stationType;
        int jff, jzz, job, jlay;
        ILine.getAmdbId(stationType, jff, jzz, job, jlay);
        float tras, traz, trat, rots, rotz, rott;
        ILine.getParameters(tras, traz, trat, rots, rotz, rott);

        ATH_MSG_INFO("I\t" << std::setiosflags(std::ios::fixed | std::ios::right) << std::setw(4) << stationType << "\t" << std::setw(2)
                           << jff << "\t" << std::setw(2) << jzz << "\t" << std::setw(2) << job << "\t" << std::setw(2) << jlay << "\t"
                           << std::setw(6) << std::setprecision(6) << tras << "\t" << std::setw(6) << std::setprecision(6) << traz << "\t"
                           << std::setw(6) << std::setprecision(6) << trat << "\t" << std::setw(6) << std::setprecision(6) << rots << "\t"
                           << std::setw(6) << std::setprecision(6) << rotz << "\t" << std::setw(6) << std::setprecision(6) << rott << "\t"
                           << ILineId);
    }
}

StatusCode MuonAlignmentCondAlg::setALinesFromAscii(ALineMapContainer* writeALineCdo) const {
  ATH_MSG_INFO(" Set alignment constants from text file " << m_aLinesFile);

  std::string file_in = PathResolver::find_file(m_aLinesFile, "DATAPATH");

  std::ifstream infile;
  if (!file_in.empty()) {
    infile.open(file_in.c_str());
  }
  else {
    ATH_MSG_ERROR("MuonAlignmentCondAlg::setALinesFromAscii Could not find Ascii file " << m_aLinesFile <<  " containing A lines");
    return StatusCode::FAILURE;
  }
  if (infile.bad()) {
    ATH_MSG_ERROR("MuonAlignmentCondAlg::setALinesFromAscii Could not open file " << file_in) ;
    return StatusCode::FAILURE;
  }

  char line[512];
  ATH_MSG_DEBUG("reading file");

  //If the ALine map is empty we are running on MC. This is for after the
  //loop over all folders, where for MC we will have an empty map.
  bool emptyALineMap = false;
  if ((int)writeALineCdo->size() == 0) {emptyALineMap = true;}

  while (infile.getline(line, 512)) {
    std::istringstream is(line);

    char AlineMarker[2];
    std::string name;
    int jff, jzz, obj;
    float tras, traz, trat, rots, rotz, rott;
    if (is >> AlineMarker >> name >> jff >> jzz >> obj >> tras >> traz >> trat >> rots >> rotz >> rott) {
      ATH_MSG_DEBUG("SUCCESSFULLY read line: " << line);
    } else {
      ATH_MSG_ERROR("ERROR reading line: " << line);
    }

    if (AlineMarker[0] == '\0') {
      ATH_MSG_DEBUG("read empty line!");
    } else {
      if (!emptyALineMap) {

	// loop through A-line container and find the correct one
	std::string testStationType;
	for (auto& [id, ALine] : *writeALineCdo) {
	  int testJff, testJzz, testJob;
	  ALine.getAmdbId(testStationType, testJff, testJzz, testJob);
	  if (testStationType == name && testJff == jff && testJzz == jzz) {
	    // set parameter if you found it
	    ALine.setParameters(tras, traz, trat, rots, rotz, rott);
	    break;
	  }
	}
      } else { // Set A lines for MC

	ALinePar newALine;
	newALine.setAmdbId(name, jff, jzz, obj);
	newALine.setParameters(tras, traz, trat, rots, rotz, rott);
	newALine.isNew(true);

	int stationName;
	Identifier id;
	if (name[0] == 'M') {
	  // micromegas case
      if(!m_idHelperSvc->hasMM()) continue;  // skip if geometry does not include MMs (e.g. RUN1 or RUN2 data or RUN3 muon calibration stream)
	  stationName = m_idHelperSvc->mmIdHelper().stationNameIndex(name);
	  id = m_idHelperSvc->mmIdHelper().elementID(stationName, jzz, jff);
	  id = m_idHelperSvc->mmIdHelper().multilayerID(id, obj);
	} else if (name[0] == 'S') {
	  // sTGC case
      if(!m_idHelperSvc->hasSTGC()) continue;  // skip if geometry does not include MMs (e.g. RUN1 or RUN2 data or RUN3 muon calibration stream)
	  stationName = m_idHelperSvc->stgcIdHelper().stationNameIndex(name);
	  id = m_idHelperSvc->stgcIdHelper().elementID(stationName, jzz, jff);
	  id = m_idHelperSvc->stgcIdHelper().multilayerID(id, obj);
	} else if (name[0] == 'T') {
	  // tgc case
	  int stPhi = MuonGM::stationPhiTGC(name, jff, jzz);  // !!!!! The stationPhiTGC implementation in this package is NOT used !!!!!
	  int stEta = 1;
	  if (jzz < 0) stEta = -1;
	  if (obj != 0) {
	    // this should become the default now
	    stEta = obj;
	    if (jzz < 0) stEta = -stEta;
	  }
	  stationName = m_idHelperSvc->tgcIdHelper().stationNameIndex(name);
	  id = m_idHelperSvc->tgcIdHelper().elementID(stationName, stEta, stPhi);
	} else if (name.substr(0, 1) == "C") {
	  // csc case
	  if(!m_idHelperSvc->hasCSC()) continue; //skip if geometry doesn't include CSCs
	  stationName = m_idHelperSvc->cscIdHelper().stationNameIndex(name);
	  id = m_idHelperSvc->cscIdHelper().elementID(stationName, jzz, jff);
	} else if (name.substr(0, 3) == "BML" && abs(jzz) == 7) {
	  // rpc case
	  stationName = m_idHelperSvc->rpcIdHelper().stationNameIndex(name);
	  id = m_idHelperSvc->rpcIdHelper().elementID(stationName, jzz, jff, 1);
	} else {
	  stationName = m_idHelperSvc->mdtIdHelper().stationNameIndex(name);
	  id = m_idHelperSvc->mdtIdHelper().elementID(stationName, jzz, jff);
	}
	  
	ATH_MSG_VERBOSE("stationName  " << stationName);
	ATH_MSG_VERBOSE("identifier being assigned is " << m_idHelperSvc->toString(id));

	if (!writeALineCdo->insert_or_assign(id, std::move(newALine)).second) {
	  ATH_MSG_WARNING("More than one (A-line) entry in file " << m_aLinesFile << " for  " <<  name << " at Jzz/Jff " << jzz << "/" << jff << " --- keep the latest one");
	}
      }
    }
  }
  
  return StatusCode::SUCCESS; 
}

void MuonAlignmentCondAlg::setAsBuiltFromAscii(MdtAsBuiltMapContainer* writeCdo) const {
    ATH_MSG_INFO(" Set alignment constants from text file " << m_asBuiltFile);

    std::ifstream fin(m_asBuiltFile);
    std::string line;
    MdtAsBuiltPar xPar;
    xPar.isNew(true);
    int count = 0;
    while (getline(fin, line)) {
        if (boost::starts_with (line, "Corr:")) {
            if (!xPar.setFromAscii(line)) {
                ATH_MSG_ERROR("Unable to parse AsBuilt params from Ascii line: " << line);
            } else {
                std::string stName = "XXX";
                int jff = 0;
                int jzz = 0;
                int job = 0;
                xPar.getAmdbId(stName, jff, jzz, job);
                Identifier id = m_idHelperSvc->mdtIdHelper().elementID(stName, jzz, jff);
                if (!id.is_valid()) {
                    ATH_MSG_ERROR("Invalid MDT identifiers: sta=" << stName << " eta=" << jzz << " phi=" << jff);
                    continue;
                }
                if (writeCdo->insert_or_assign(id, xPar).second) {
                    ATH_MSG_DEBUG("New entry in AsBuilt container for Station " << stName << " at Jzz/Jff " << jzz << "/" << jff
                                                                                << " --- in the container with key "
                                                                                << m_idHelperSvc->mdtIdHelper().show_to_string(id));
                } else {
                    ATH_MSG_DEBUG("Updating existing entry in AsBuilt container for Station " << stName << " at Jzz/Jff " << jzz << "/"
                                                                                              << jff);
                    ATH_MSG_DEBUG("That is strange since it's read from ASCII so this station is listed twice!");
                }
                ++count;
            }
        }
    }
    ATH_MSG_INFO("Parsed AsBuilt parameters: " << count);
}
