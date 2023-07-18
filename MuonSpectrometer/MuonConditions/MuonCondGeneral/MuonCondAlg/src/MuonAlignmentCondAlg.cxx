/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/MuonAlignmentCondAlg.h"

#include <fstream>
#include <map>
#include <string>
#include "AthenaKernel/IOVInfiniteRange.h"
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
    for (const std::string& currentFolderName : m_parlineFolder) {
        if (currentFolderName.find("ILINES") != std::string::npos) m_ILineRequested = true;
        if (currentFolderName.find("ASBUILTPARAMS") != std::string::npos && 
            currentFolderName.find("MDT" ) != std::string::npos) m_MdtAsBuiltRequested = true;
    }
    // Read Handles Keys
    ATH_CHECK(m_alignKeys.initialize(m_readFromJSON.value().empty()));
    ATH_CHECK(m_readCscILinesKey.initialize(m_idHelperSvc->hasCSC() && m_ILinesFromDb and m_ILineRequested));
    ATH_CHECK(m_readMdtAsBuiltParamsKey .initialize(m_MdtAsBuiltRequested));

    // Write Handles
    ATH_CHECK(m_writeALineKey.initialize());
    ATH_CHECK(m_writeBLineKey.initialize());
    ATH_CHECK(m_writeILineKey.initialize(m_idHelperSvc->hasCSC() && m_ILinesFromDb and m_ILineRequested));
    ATH_CHECK(m_writeMdtAsBuiltKey.initialize(m_MdtAsBuiltRequested));
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::execute() {
    ATH_MSG_DEBUG("execute " << name());

    ATH_MSG_DEBUG("In LoadParameters " << name());
    StatusCode sc = StatusCode::SUCCESS;
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
    // Load A- and B-Lines
    // =======================
    const EventContext& ctx = Gaudi::Hive::currentContext();

    SG::WriteCondHandle<ALineContainer> writeALineHandle{m_writeALineKey, ctx};
    if (writeALineHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeALineHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    ///
    // =======================
    // Write BLine Cond Handle
    // =======================
    SG::WriteCondHandle<BLineContainer> writeBLineHandle{m_writeBLineKey, ctx};
    if (writeBLineHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeBLineHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    /// Declare the dependencies on the alignment constant data
    ATH_CHECK(attachDependencies(ctx, writeALineHandle, writeBLineHandle));
    /// Create the containers
    std::unique_ptr<ALineContainer> writeALineCdo{std::make_unique<ALineContainer>()};
    std::unique_ptr<BLineContainer> writeBLineCdo{std::make_unique<BLineContainer>()};

    for (const SG::ReadCondHandleKey<CondAttrListCollection>& key:  m_alignKeys){ 
        ATH_CHECK(loadCoolFolder(ctx, key, *writeALineCdo, *writeBLineCdo));
    }
    if (!m_readFromJSON.value().empty()) {
        std::ifstream inStream{PathResolverFindCalibFile(m_readFromJSON)};
        if (!inStream.good()) {
            ATH_MSG_FATAL("No such file or directory");
            return StatusCode::FAILURE;
        }
        nlohmann::json lines;
        inStream >> lines;
        ATH_CHECK(parseDataFromJSON(lines, *writeALineCdo, *writeBLineCdo));        
    }
    ATH_CHECK(writeALineHandle.record(std::move(writeALineCdo)));
    ATH_CHECK(writeBLineHandle.record(std::move(writeBLineCdo)));

    return StatusCode::SUCCESS;
}
StatusCode MuonAlignmentCondAlg::attachDependencies(const EventContext& ctx,
                                  SG::WriteCondHandle<ALineContainer>& alines,
                                  SG::WriteCondHandle<BLineContainer>& blines ) const {
    alines.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    blines.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    /// Loop over all input folder and attach their IOVs to the output conditions
    for (const SG::ReadCondHandleKey<CondAttrListCollection>& key : m_alignKeys) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{key, ctx};
        if (!readHandle.isValid()){
            ATH_MSG_FATAL("Failed to load alignment folder "<<key.fullKey());
            return StatusCode::FAILURE;
        }
        ATH_MSG_INFO("Attach new dependency from <"<<readHandle.key()<<"> to the A & B lines. IOV: "<<readHandle.getRange());
        alines.addDependency(readHandle);
        blines.addDependency(readHandle);
    }
    return StatusCode::SUCCESS;
}
StatusCode MuonAlignmentCondAlg::loadCoolFolder(const EventContext& ctx,
                                                  const SG::ReadCondHandleKey<CondAttrListCollection>& key,
                                                  ALineContainer& writeALineCdo, 
                                                  BLineContainer& writeBLineCdo) const {
    
    SG::ReadCondHandle<CondAttrListCollection> readHandle{key, ctx};
    if (!readHandle.isValid()){
        ATH_MSG_FATAL("Failed to load alignment folder "<<key.fullKey());
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Load constants from folder "<<key.key());
    // unpack the strings in the collection and update the
    // ALlineContainer in TDS
    for (CondAttrListCollection::const_iterator itr = readHandle->begin(); itr != readHandle->end(); ++itr) {
        const coral::AttributeList& atr = itr->second;
        std::string data{};
        if (atr["data"].specification().type() == typeid(coral::Blob)) {
            ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
            if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                return StatusCode::FAILURE;
            }
        } else {
            data = *(static_cast<const std::string*>((atr["data"]).addressOfData()));
        }
        nlohmann::json lines;

        // new format -----------------------------------
        if (m_newFormat2020) {
            nlohmann::json j = nlohmann::json::parse(data);
            lines = j["corrections"];
        } 
        // old format -----------------------------------
        else {
            ATH_CHECK(loadDataFromLegacy(data, lines, true));
        }
        ATH_CHECK(parseDataFromJSON(lines, writeALineCdo, writeBLineCdo));
    }    
    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::parseDataFromJSON(const nlohmann::json& lines,
                                                  ALineContainer& writeALineCdo, 
                                                  BLineContainer& writeBLineCdo) const{
    // loop over corrections ------------------------
    for (auto& corr : lines.items()) {
        nlohmann::json line = corr.value();

        /// Station Component identification
        const std::string stationType = line["typ"];
        const int stationPhi = line["jff"];
        const int stationEta = line["jzz"];
        const int multiLayer = line["job"];
        Identifier id{0};
        /// Micromega case
        if (stationType[0] == 'M') {
            // micromegas case
            if(!m_idHelperSvc->hasMM()) {
                ATH_MSG_WARNING("Micromega alignment parameter is part of the database "<<stationType<<","<<stationEta<<","<<stationPhi<<","<<multiLayer);
                continue;  // skip if geometry does not include MMs (e.g. RUN1 or RUN2 data or RUN3 muon calibration stream)
            }
            id = m_idHelperSvc->mmIdHelper().channelID(stationType, stationEta, stationPhi, multiLayer, 1, 1);
        } else if (stationType[0] == 'S') {
            // sTGC case
            if(!m_idHelperSvc->hasSTGC()) {
                ATH_MSG_WARNING("sTgc alignment parameter is part of the database "<<stationType<<","<<stationEta<<","<<stationPhi<<","<<multiLayer);
                continue;  // skip if geometry does not include MMs (e.g. RUN1 or RUN2 data or RUN3 muon calibration stream)
            }
            id = m_idHelperSvc->stgcIdHelper().elementID(stationType, stationEta, stationPhi);
            id = m_idHelperSvc->stgcIdHelper().multilayerID(id, multiLayer);
        } else if (stationType[0] == 'T') {
            /// Tgc case
            int stPhi = MuonGM::stationPhiTGC(stationType, stationPhi, stationEta);
            int stEta = stationEta > 0 ? 1 : -1;
            if (multiLayer != 0) {
                // this should become the default now
                stEta = stationEta > 0 ?  multiLayer: - multiLayer;
            }
            id = m_idHelperSvc->tgcIdHelper().elementID(stationType, stEta, stPhi);
        } else if (stationType[0] == 'C') {
            // csc case
            if(!m_idHelperSvc->hasCSC()) {
                ATH_MSG_WARNING("Csc alignment parameter is part of the database "<<stationType<<","<<stationEta<<","<<stationPhi<<","<<multiLayer);               
                continue; //skip if geometry doesn't include CSCs
            }
            id = m_idHelperSvc->cscIdHelper().elementID(stationType, stationEta, stationPhi);
        } else if (stationType.substr(0, 3) == "BML" && std::abs(stationEta) == 7) {
            // rpc case
            id = m_idHelperSvc->rpcIdHelper().elementID(stationType, stationEta, stationPhi, 1);
        } else {
            id = m_idHelperSvc->mdtIdHelper().elementID(stationType, stationEta, stationPhi);
        }
        ALinePar newALine{};
        newALine.setIdentifier(id);
        newALine.setAmdbId(stationType, stationEta, stationPhi, multiLayer);
        newALine.setParameters(line["svalue"], line["zvalue"], line["tvalue"], 
                               line["tsv"], line["tzv"], line["ttv"]);
        auto aLineInsert = writeALineCdo.insert(newALine);
        if (newALine && !aLineInsert.second) {
            ATH_MSG_WARNING("Failed to insert  A line "<<newALine<<" for "<<m_idHelperSvc->toString(id)
                            <<" because "<<(*aLineInsert.first)<<" has been added before");
        }
        ATH_MSG_VERBOSE("Inserted new a Line "<<newALine<<" "<<m_idHelperSvc->toString(id));
    
        if (line.find("bz") == line.end()) {
            continue;
        }
        BLinePar newBLine{};
        newBLine.setParameters(line["bz"], line["bp"], line["bn"], 
                               line["sp"], line["sn"], line["tw"], 
                               line["pg"], line["tr"], line["eg"], 
                               line["ep"], line["en"]);
        newBLine.setIdentifier(id);
        newBLine.setAmdbId(stationType, stationEta, stationPhi, multiLayer);
        ATH_MSG_VERBOSE(" HardwareChamberName " <<  static_cast<std::string>(line["hwElement"]));
        auto bLineInsert = writeBLineCdo.insert(newBLine);
        if (newBLine && !bLineInsert.second){
            ATH_MSG_WARNING("Failed to insert B line "<<newBLine<<" for "<<m_idHelperSvc->toString(id)
                            <<" because "<<(*bLineInsert.first)<<" has been added before.");
        }
    }    
    return StatusCode::SUCCESS;
}

StatusCode MuonAlignmentCondAlg::loadDataFromLegacy(const std::string& data, nlohmann::json& json,
                                                      bool loadBLines) const {

    using namespace MuonCalib;
    // Parse corrections
    constexpr char delimiter = '\n';

    json = nlohmann::json::array();
    auto lines = MdtStringUtils::tokenize(data, delimiter);
    for (const std::string_view& blobline : lines) {
        nlohmann::json line;
        constexpr char delimiter = ':';
        const auto tokens = MdtStringUtils::tokenize(blobline, delimiter);

        // Check if tokens is not empty
        if (tokens.empty()) {
            ATH_MSG_FATAL("Empty string retrieved from DB in folder ");
            return StatusCode::FAILURE;
        }
        const std::string_view &type = tokens[0];
        // Parse line
        if (type[0] == '#') {
            continue;
        }
        //#: Corr line is counter typ,  jff,  jzz, job,                         * Chamber information
        //#:                       svalue,  zvalue, tvalue,  tsv,  tzv,  ttv,   * A lines
        //#:                       bz, bp, bn, sp, sn, tw, pg, tr, eg, ep, en   * B lines
        //#:                       chamber                                      * Chamber name
        //.... example
        // Corr: EMS  4   1  0     2.260     3.461    28.639 -0.002402 -0.002013  0.000482    -0.006    -0.013 -0.006000  0.000000
        // 0.000000     0.026    -0.353  0.000000  0.070000  0.012000    -0.012    EMS1A08
        
        if (type.compare(0, 4, "Corr") == 0) {
            constexpr char delimiter = ' ';
            auto tokens = MdtStringUtils::tokenize(blobline, delimiter);
            if (tokens.size() != 25) {
                ATH_MSG_FATAL("Invalid length in string retrieved. String length is " << tokens.size());
                return StatusCode::FAILURE;
            }
            // Start parsing
            int ival = 1;
            // Station Component identification
            line["typ"] = std::string(tokens[ival++]);
            line["jff"] = MdtStringUtils::atoi(tokens[ival++]);
            line["jzz"] = MdtStringUtils::atoi(tokens[ival++]);
            line["job"] = MdtStringUtils::atoi(tokens[ival++]);
            
            // A-line
            line["svalue"] = MdtStringUtils::stof(tokens[ival++]);
            line["zvalue"] = MdtStringUtils::stof(tokens[ival++]);
            line["tvalue"] = MdtStringUtils::stof(tokens[ival++]);
            
            line["tsv"] = MdtStringUtils::stof(tokens[ival++]);
            line["tzv"] = MdtStringUtils::stof(tokens[ival++]);
            line["ttv"] = MdtStringUtils::stof(tokens[ival++]);

            // B-line
            if (loadBLines) {
                line["bz"] = MdtStringUtils::stof(tokens[ival++]);
                line["bp"] = MdtStringUtils::stof(tokens[ival++]);
                line["bn"] = MdtStringUtils::stof(tokens[ival++]);
                line["sp"] = MdtStringUtils::stof(tokens[ival++]);
                line["sn"] = MdtStringUtils::stof(tokens[ival++]);
                line["tw"] = MdtStringUtils::stof(tokens[ival++]);
                line["pg"] = MdtStringUtils::stof(tokens[ival++]);
                line["tr"] = MdtStringUtils::stof(tokens[ival++]);
                line["eg"] = MdtStringUtils::stof(tokens[ival++]);
                line["ep"] = MdtStringUtils::stof(tokens[ival++]);
                line["en"] = MdtStringUtils::stof(tokens[ival++]);

                line["xAtlas"] = MdtStringUtils::stof(tokens[ival++]);
                line["yAtlas"] = MdtStringUtils::stof(tokens[ival++]);

                // ChamberName (hardware convention)
                line["hwElement"] = std::string(tokens[ival++]);
            }
            json.push_back(std::move(line));  
        }
    }
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
                const std::string stationType = xPar.AmdbStation();
                const int jff = xPar.AmdbPhi();
                const int jzz = xPar.AmdbEta();
                Identifier id = m_idHelperSvc->mdtIdHelper().elementID(stationType, jzz, jff);

                ATH_MSG_VERBOSE("Station type jff jzz " << xPar);
                ++nDecodedLines;
                ++nNewDecodedAsBuilt;
                if (!writeCdo->insert_or_assign(id, xPar).second) {
                    ATH_MSG_WARNING("More than one (As-built) entry in folder " << folderName << " for  " << stationType << " at Jzz/Jff "
                                                                                << jzz << "/" << jff <<" "<< " --- keep the latest one");
                    --nNewDecodedAsBuilt;
                    return StatusCode::FAILURE;
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


void MuonAlignmentCondAlg::dumpILines(const std::string& folderName, CscInternalAlignmentMapContainer* writeCdo) {
    ATH_MSG_INFO("dumping I-lines for folder " << folderName);
    ATH_MSG_INFO("I \ttype\tjff\tjzz\tjob\tjlay\ttras\ttraz\ttrat\trots\trotz\trott");

    for (const auto& [ILineId, ILine] : *writeCdo) {
        std::string stationType{};
        int jff{0}, jzz{0}, job{0}, jlay{0};
       
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
                const std::string stName = xPar.AmdbStation();
                const int jff = xPar.AmdbPhi();
                const int jzz = xPar.AmdbEta();                
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
