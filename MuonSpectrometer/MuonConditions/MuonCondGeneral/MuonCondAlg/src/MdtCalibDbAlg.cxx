/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/MdtCalibDbAlg.h"

#include "AthenaKernel/IIOVDbSvc.h"
#include "AthenaKernel/RNGWrapper.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeListSpecification.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "MdtCalibData/BFieldCorFunc.h"
#include "MdtCalibData/CalibFunc.h"
#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/IRtResolution.h"
#include "MdtCalibData/MdtCalibrationFactory.h"
#include "MdtCalibData/MdtCorFuncSetCollection.h"
#include "MdtCalibData/MdtFullCalibData.h"
#include "MdtCalibData/MdtRtRelationCollection.h"
#include "MdtCalibData/MdtSlewCorFuncHardcoded.h"
#include "MdtCalibData/MdtTubeCalibContainerCollection.h"
#include "MdtCalibData/RtFromPoints.h"
#include "MdtCalibData/RtResolutionFromPoints.h"
#include "MdtCalibData/WireSagCorFunc.h"
#include "MdtCalibSvc/MdtCalibrationRegionSvc.h"
#include "MdtCalibUtils/RtDataFromFile.h"
#include "MuonCalibIdentifier/MdtCalibCreationFlags.h"
#include "MuonCalibIdentifier/MuonFixedId.h"
#include "MuonCalibMath/SamplePoint.h"
#include "MuonCalibStl/ToString.h"
#include "MuonCalibTools/IdToFixedIdTool.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "PathResolver/PathResolver.h"
#include "SGTools/TransientAddress.h"

// TODO: use smart pointers
// TODO: avoid dynamic char array
// TODO: check if temporary things can be removed

#include <fstream>

#include "TFile.h"
#include "TSpline.h"

MdtCalibDbAlg::MdtCalibDbAlg(const std::string &name, ISvcLocator *pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MdtCalibDbAlg::initialize() {
    ATH_MSG_DEBUG("initialize " << name());

    // if timeslew correction vector m_MeanCorrectionVsR has non-zero size then set
    // m_TsCorrectionT0=m_MeanCorrectionVsR[0] and subtract this each value in the vector.
    if (m_MeanCorrectionVsR.size()) {
        m_TsCorrectionT0 = m_MeanCorrectionVsR[0];
        for (float & it : m_MeanCorrectionVsR) {
            it -= m_TsCorrectionT0;
        }
    }

    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_regionSvc.retrieve());
    ATH_CHECK(m_idToFixedIdTool.retrieve());

    // initialize MdtRtRelationCollection
    // if COOL RT folder is called /MDT/RTUNIQUE then only read one RT from COOL and use for all chambers
    // Not sure this option has ever been used, perhaps could be used for simulated data.
    // Job option RtFolder would need to be set to "/MDT/RTUNIQUE" to make this work.
    if (m_readKeyRt.key() == "/MDT/RTUNIQUE") {
        m_regionSvc->remapRtRegions("OneRt");
    } else if (m_UseMLRt) {
        m_regionSvc->remapRtRegions("OnePerMultilayer");
    } else {
        m_regionSvc->remapRtRegions("OnePerChamber");
    }

    // initiallize random number generator if doing t0 smearing (for robustness studies)
    if (m_t0Spread != 0.) {
        ATH_CHECK(m_AthRNGSvc.retrieve());
        ATH_MSG_DEBUG(" initialize Random Number Service: running with t0 shift " << m_t0Shift << " spread " << m_t0Spread << " rt shift "
                                                                                  << m_rtShift);
        // getting our random numbers stream
        m_RNGWrapper = m_AthRNGSvc->getEngine(this, m_randomStream);
        if (!m_RNGWrapper) {
            ATH_MSG_ERROR("Could not get random number engine from AthRNGSvc. Abort.");
            return StatusCode::FAILURE;
        }
    }

    if (m_rtShift != 0. || m_rtScale != 1. || m_t0Shift != 0. || m_t0Spread != 0.) {
        ATH_MSG_INFO("************************************" << std::endl
                                                            << " Running with Calibration Deformations! " << std::endl
                                                            << " For performance studies only!" << std::endl
                                                            << " **************************************");
        ATH_MSG_DEBUG(" rt scale " << m_rtScale << " t0 shift " << m_t0Shift << " spread " << m_t0Spread << " rt shift " << m_rtShift);
    }

    ATH_CHECK(m_readKeyRt.initialize());
    ATH_CHECK(m_readKeyTube.initialize());
    ATH_CHECK(m_writeKeyRt.initialize());
    ATH_CHECK(m_writeKeyTube.initialize());
    ATH_CHECK(m_writeKeyCor.initialize(m_create_b_field_function || m_createWireSagFunction|| m_createSlewingFunction));

    if (m_useNewGeo) ATH_CHECK(detStore()->retrieve(m_r4detMgr));
    else ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());
    ATH_CHECK(loadRt(ctx));
    ATH_CHECK(loadTube(ctx));
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::defaultRt(MdtRtRelationCollection &writeCdoRt)  const {
    ATH_MSG_DEBUG("defaultRt " << name());

    // Build the transient structure in StoreGate and load default RT function read from a text file
    // In principle, a list of text files can be specified in the job options, and each text file
    // may potentially contain multiple RT functions.  However, only the first valid RT function in
    // the first text file is used as the default for all chambers.

    writeCdoRt.resize(m_regionSvc->numberOfRegions());
    ATH_MSG_DEBUG("Created new MdtRtRelationCollection size " << writeCdoRt.size());

    // Check that an RT text file has been specified in job options
    const std::vector<std::string>& fileNames {m_RTfileNames.value()};
    if (fileNames.empty()) {
        ATH_MSG_FATAL("No input RT file declared in jobOptions");
        return StatusCode::FAILURE;
    } else if (fileNames.size() > 1) {
        ATH_MSG_WARNING("Only first RT file declared in jobOptions will be used");
    }

    // Open the Ascii file with the RT relations
    std::string fileName = PathResolver::find_file(fileNames[0], "DATAPATH");
    if (fileName.empty()) { 
        ATH_MSG_ERROR("RT Ascii file \"" << fileNames[0] << "\" not found"); 
        return StatusCode::FAILURE;
    }
    std::ifstream inputFile(fileName);
    if (!inputFile) {
        ATH_MSG_ERROR("Unable to open RT Ascii file: " << fileName.c_str());
        return StatusCode::FAILURE;
    } 
    ATH_MSG_DEBUG("Opened RT Ascii file: " << fileName.c_str());
    

    // Read the RTs from the text file
    MuonCalib::RtDataFromFile rts;
    rts.read(inputFile);
    ATH_MSG_VERBOSE("File contains " << rts.nRts() << " RT relations ");

    // Loop over all RTs in the file (but the default file only has 1 RT)
    // Use the first valid RT found in the file as the default for all chambers.
    for (unsigned int n = 0; n < rts.nRts(); ++n) {
        std::unique_ptr<MuonCalib::RtDataFromFile::RtRelation> rt(rts.getRt(n));

        const MuonCalib::RtDataFromFile::RtRelation::DataVec &times = rt->times();
        const MuonCalib::RtDataFromFile::RtRelation::DataVec &radii = rt->radii();
        const MuonCalib::RtDataFromFile::RtRelation::DataVec &reso = rt->resolution();

        // check if rt contains data, at least two points on the rt are required
        if (times.size() < 2) {
            ATH_MSG_ERROR(" defaultRt rt table has too few entries");
            return StatusCode::FAILURE;
        }
        // check if all tables have same size
        if (times.size() != radii.size() || times.size() != reso.size()) {
            ATH_MSG_ERROR("defaultRt rt table size mismatch ");
            return StatusCode::FAILURE;
        }
        // check for negative time bins, i.e. decreasing time value with radius
        double t_min = times[0];
        double bin_size = times[1] - t_min;
        if (bin_size <= 0) {
            ATH_MSG_ERROR("defaultRt rt table negative binsize ");
            return StatusCode::FAILURE;
        }

        // create a vector to hold the r values,
        // we need two extra fields to store t_min and bin_size
        MuonCalib::CalibFunc::ParVec rtPars;
        rtPars.push_back(t_min);
        rtPars.push_back(bin_size);

        // copy r values into vector
        rtPars.insert(rtPars.end(), radii.begin(), radii.end());

        ATH_MSG_DEBUG("defaultRt new MuonCalib::IRtRelation");

        MuonCalib::CalibFunc::ParVec resoPars{t_min, bin_size};
        // copy r values into vector
        resoPars.insert(resoPars.end(), reso.begin(), reso.end());

        ATH_MSG_DEBUG("defaultRt new MuonCalib::IRtResolution");

        // create RT and resolution "I" objects
        std::unique_ptr<MuonCalib::IRtRelation> rtRel {MuonCalib::MdtCalibrationFactory::createRtRelation("RtRelationLookUp", rtPars)};
        if (!rtRel) ATH_MSG_WARNING("ERROR creating RtRelationLookUp ");

        std::unique_ptr<MuonCalib::IRtResolution> resoRel{MuonCalib::MdtCalibrationFactory::createRtResolution("RtResolutionLookUp", resoPars)};
        if (!resoRel) ATH_MSG_WARNING("ERROR creating RtResolutionLookUp ");

        // if either RT and resolution are not OK then delete both and try next RT in file
        if (!resoRel || !rtRel) {
            continue;
        }

        // Since the same RT is loaded for all chambers you might be tempted to create it once
        // and simply store the same pointer in writeCdoRt for all regions.
        // However it seems that when StoreGate clears writeCdoRt (which will happen in LoadRt
        // by detStore()->removeDataAndProxy) it will crash unless there are unique pointers/objects
        // for rtRel, resoRel, and MdtRtRelation

        // Loop over RT regions and store the default RT in each
        for (unsigned int iregion = 0; iregion < writeCdoRt.size(); iregion++) {
            ATH_MSG_DEBUG("Inserting default Rt for region " << iregion);
            // create RT and resolution "I" objects, again, so they can all be cleanly deleted later.
            std::unique_ptr<MuonCalib::IRtRelation> rtRelRegion{MuonCalib::MdtCalibrationFactory::createRtRelation("RtRelationLookUp", rtPars)};
            std::unique_ptr<MuonCalib::IRtResolution> resoRelRegion{MuonCalib::MdtCalibrationFactory::createRtResolution("RtResolutionLookUp", resoPars)};
            writeCdoRt[iregion] = std::make_unique<MuonCalib::MdtRtRelation>(std::move(rtRelRegion), std::move(resoRelRegion), 0.);
        }  // end loop over RT regions

        // if VERBOSE enabled print out RT function
        if (msgLvl(MSG::VERBOSE)) {
            int npoints = rtRel->nPar() - 2;
            ATH_MSG_VERBOSE("defaultRt npoints from rtRel=" << npoints);
            for (int ipt = 0; ipt < npoints; ++ipt) {
                double t = t_min + ipt * bin_size;
                ATH_MSG_VERBOSE(" " << ipt << " " << t << " " << rtRel->radius(t) << " " << resoRel->resolution(t));
            }
        }
        break;  // only need the first good RT from the text file

    }  // end loop over RTs in file

    return StatusCode::SUCCESS;
}
std::optional<double> MdtCalibDbAlg::getInnerTubeRadius(const Identifier& id) const {
    static std::atomic<bool> rtWarningPrinted = false;
            
    if (m_detMgr) {
        const MuonGM::MdtReadoutElement *detEl = m_detMgr->getMdtReadoutElement(id);        
        if (detEl) { return std::make_optional<double>(detEl->innerTubeRadius()); }
       
    } else if (m_r4detMgr) {
        const MuonGMR4::MdtReadoutElement* detEl = m_r4detMgr->getMdtReadoutElement(id);
        if (detEl) { return std::make_optional<double>(detEl->innerTubeRadius()); }
    }
    if (!rtWarningPrinted) {
        ATH_MSG_WARNING("getInnerTubeRadius() - no Muon station known under the name "
                            << m_idHelperSvc->toString(id));
        rtWarningPrinted = true;
    }
    return std::nullopt;
}

StatusCode MdtCalibDbAlg::loadRt(const EventContext& ctx) const {
    ATH_MSG_DEBUG("loadRt " << name());

    SG::WriteCondHandle<MdtRtRelationCollection> writeHandleRt{m_writeKeyRt, ctx};
    if (writeHandleRt.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandleRt.fullKey() << " is already valid.");
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<MdtRtRelationCollection> writeCdoRt{std::make_unique<MdtRtRelationCollection>()};

    // like MdtCalibDbCoolStrTool::loadRt()
    // m_rtData is writeCdoRt here
    // atrc is readCdoRt here

    // tr-relation creators
    MuonCalib::RtFromPoints rt_fromPoints;

    ATH_CHECK(defaultRt(*writeCdoRt));

    // Read Cond Handle
    SG::ReadCondHandle<CondAttrListCollection> readHandleRt{m_readKeyRt, ctx};
    if (!readHandleRt.isValid()) {
        ATH_MSG_ERROR("readCdoRt==nullptr");
        return StatusCode::FAILURE;
    }
    writeHandleRt.addDependency(readHandleRt);
    
    ATH_MSG_INFO("Size of CondAttrListCollection " << readHandleRt.fullKey() << " readCdoRt->size()= " << readHandleRt->size());
    ATH_MSG_INFO("Range of input is " << readHandleRt.getRange());

    // read new-style format 2020
    std::vector<coral::AttributeList> dataPerChannel;
    if (m_newFormat2020) {
        CondAttrListCollection::const_iterator itr;
        for (itr = readHandleRt->begin(); itr != readHandleRt->end(); ++itr) {
            const coral::AttributeList &atr = itr->second;
            std::string data;
            if (atr["data"].specification().type() == typeid(coral::Blob)) {
                ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
                if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                    ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                    return StatusCode::FAILURE;
                }
            } else {
                ATH_MSG_VERBOSE("Loading data as a STRING");
                data = *(static_cast<const std::string *>((atr["data"]).addressOfData()));
            }

            // unwrap the json and build the data vector
            nlohmann::json yy = nlohmann::json::parse(data);
            for (auto &it : yy.items()) {
                nlohmann::json yx = it.value();
                for (auto &jt : yx.items()) {
                    coral::AttributeList al;
                    al.extend("tech", "int");
                    al.extend("file", "string");
                    al.extend("data", "blob");
                    al["tech"].data<int>() = jt.value()[1];
                    al["file"].data<std::string>() = static_cast<std::string>(jt.value()[2]);
                    std::string data = jt.value()[3];
                    if (!CoralUtilities::writeBlobFromString(data, al["data"].data<coral::Blob>())) {
                        ATH_MSG_FATAL("Cannot compress BLOB!");
                        return StatusCode::FAILURE;
                    }
                    dataPerChannel.push_back(al);
                }
            }
        }
    }
    // read old-style format
    else {
        for (CondAttrListCollection::const_iterator itr = readHandleRt->begin(); 
             itr != readHandleRt->end(); ++itr) {
            dataPerChannel.push_back(itr->second);
        }
    }

    // unpack the strings in the collection and update the writeCdoRt
    for (const auto& atr : dataPerChannel) {
        bool rt_ts_applied = (atr["tech"].data<int>() & MuonCalib::TIME_SLEWING_CORRECTION_APPLIED);
        std::string header = "", payload = "", trailer = "";
        // if BLOB data
        if (atr["data"].specification().type() == typeid(coral::Blob)) {
            std::string istr;
            ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
            if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), istr)) {
                ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                return StatusCode::FAILURE;
            }
            ATH_CHECK(extractString(istr, header, "\n"));
            ATH_CHECK(extractString(istr, payload, "\n"));
            if (!istr.empty()) ATH_CHECK(extractString(istr, trailer, "\n"));
        } else {  // else CLOB data
            std::string data;
            data = *(static_cast<const std::string *>((atr["data"]).addressOfData()));
            ATH_MSG_VERBOSE("Data load is " << data);
            // interpret as string stream
            std::string istr(data);
            ATH_CHECK(extractString(istr, header, " "));
            ATH_CHECK(extractString(istr, payload, " "));
            if (!istr.empty()) ATH_CHECK(extractString(istr, trailer, " "));
        }
        ATH_MSG_VERBOSE("Read header:" << header << " payload:" << payload << " trailer:" << trailer);

        // the header contains the muonfixedid rather than the hash
        std::unique_ptr<char[]> parameters{new char[header.size() + 1]};
        strncpy(parameters.get(), header.c_str(), header.size() + 1);
        parameters[header.size()] = '\0';
        unsigned int regionId, npoints(0);
        Identifier athenaId;
        char *saveptr;
        char *pch = strtok_r(parameters.get(), " _,",&saveptr);
        regionId = atoi(pch);
        // Long ago the hash was put in the RT file header, but now (2016)
        // the muonfixedid of the chamber is in the header.  Hence the "if" below will always be true.
        if (regionId > m_regionIdThreshold) {
            MuonCalib::MuonFixedId id(regionId);
            if (!id.is_mdt()) {
                ATH_MSG_WARNING("Found non-MDT MuonFixedId, continuing...");
                continue;
            }
            if (!m_idHelperSvc->hasCSC()) {
                // in case there are no CSCs, there must be 2 NSWs, and accordingly no EIS/EIL1-3 MDTs
                std::string stationName = id.stationNumberToFixedStationString(id.stationName());
                if (stationName.find("EIS") != std::string::npos ||
                    (std::abs(id.eta()) < 4 && stationName.find("EIL") != std::string::npos)) {
                    static std::atomic<bool> eisWarningPrinted = false;
                    if (!eisWarningPrinted) {
                        ATH_MSG_WARNING("Found EIS/EIL1-3 MuonFixedId, although NSWs should be present, continuing...");
                        eisWarningPrinted.store(true, std::memory_order_relaxed);
                    }
                    continue;
                }
            }
            athenaId = m_idToFixedIdTool->fixedIdToId(id);
            if (!m_idHelperSvc->isMuon(athenaId)){
                ATH_MSG_WARNING("The translation from the calibration ID with station: "
                                <<id.stationNameString()<<"("<<id.stationName()<<") "
                                <<" eta:"<<id.eta()<<" phi: "<<id.phi()
                                <<" failed. Please check carefully the geometry tag provided and whether the previous printout also makes sense");
                
                continue;
            }
            // If using chamber RTs skip RTs for ML2 -- use ML1 RT for entire chamber
            if (m_regionSvc->RegionType() == ONEPERCHAMBER && m_idHelperSvc->mdtIdHelper().multilayer(athenaId) == 2) {
                ATH_MSG_VERBOSE("MdtCalibDbAlg::loadRt Ignore ML2 RT for region "
                    << regionId << " " << m_idHelperSvc->toString(athenaId));  // TEMP
                continue;
            }
            IdentifierHash hash{0};  // chamber hash
            if (m_idHelperSvc->mdtIdHelper().get_module_hash(athenaId, hash)) {
                ATH_MSG_ERROR("Could not retrieve module hash for identifier " << athenaId.get_compact());
                return StatusCode::FAILURE;
            }
            ATH_MSG_VERBOSE("Fixed region Id " << regionId << " converted into athena Id " << athenaId << " and then into hash " << hash);
            regionId = hash;  // reset regionId to chamber hash
        }
        if (regionId >= writeCdoRt->size()) {
            ATH_MSG_ERROR("loadRt() - regionId=" << regionId << " larger than size of MdtRtRelationCollection, skipping...");
            return StatusCode::FAILURE;
        }
        // extract npoints in RT function
        pch = strtok_r(nullptr, "_,",&saveptr);
        npoints = atoi(pch);
        MuonCalib::CalibFunc::ParVec rtPars;
        MuonCalib::CalibFunc::ParVec resoPars;

        MuonCalib::SamplePoint tr_point, ts_point;  // pairs of numbers; tr = (time,radius); ts = (time,sigma)  [sigma=resolution]
        std::vector<MuonCalib::SamplePoint> tr_points(0),
            ts_points(0);  // all the points in time,radius [RT] and time,sigma [resolution func]
        float multilayer_tmax_diff(-9e9);

        std::optional<double> innerTubeRadius = getInnerTubeRadius(m_idHelperSvc->mdtIdHelper().multilayerID(athenaId, 1));
        if (!innerTubeRadius) continue;

        std::unique_ptr<char[]> RTPar{new char[payload.size() + 1]};
        strncpy(RTPar.get(), payload.c_str(), payload.size() + 1);
        RTPar[payload.size()] =
            '\0';  // terminate string (not sure this is really needed because payload.c_str() should be terminated in \0)
        char *saveptr1;
        char *pch1 = strtok_r(RTPar.get(), ",",&saveptr1);
        unsigned int n = 0;
        // loop over RT function payload (triplets of radius,time,sigma(=resolution) )
        for (int k = 1; pch1 != nullptr && n <= npoints; pch1 = strtok_r(nullptr, ", ",&saveptr1), k++) {
            if (k == 1) {  // radius point
                float radius = atof(pch1);
                if (m_rtShift != 0.) {
                    float oldradius = radius;
                    // TODO: What is this magic number
                    float rshift = m_rtShift * 1.87652e-2 * radius * (radius - *innerTubeRadius);
                    radius = oldradius + rshift;
                    ATH_MSG_DEBUG("DEFORM RT: old radius " << oldradius << " new radius " << radius << " shift " << rshift << " max shift "
                                                           << m_rtShift);
                }

                if (m_rtScale != 1.) {
                    radius = radius * m_rtScale;
                    ATH_MSG_DEBUG("DEFORM RT: old radius " << radius << " new radius " << radius << " scale factor " << m_rtScale);
                }

                tr_point.set_x2(radius);
            } else if (k == 2) {  // time
                float time = atof(pch1);
                tr_point.set_x1(time);
                ts_point.set_x1(time);
            } else if (k == 3) {  // sigma or resolution
                float sigma = atof(pch1);
                ts_point.set_x2(sigma);
                ts_point.set_error(1.0);
                tr_point.set_error(1.0);
                if (tr_point.x2() < -99) {  // if radius is < -99 then treat time as ML Tmax difference
                    multilayer_tmax_diff = tr_point.x1();
                } else if (n == 0 || (tr_points[n - 1].x1() < tr_point.x1() && tr_points[n - 1].x2() < tr_point.x2())) {
                    tr_points.push_back(tr_point);
                    ts_points.push_back(ts_point);
                    n++;  // count points in RT
                }
                k = 0;
            }
        }  // end loop over RT function payload (triplets of radius,time,resolution)

        // Must have at least 3 points to have a valid RT
        if (ts_points.size() < 3) {
            ATH_MSG_FATAL("Rt relation broken!");
            ATH_MSG_FATAL("file='" << atr["file"].data<std::string>() << "'");
            ATH_MSG_FATAL("header='" << header << "'");
            return StatusCode::FAILURE;
        }

        if (rt_ts_applied != m_TimeSlewingCorrection) {
            float sign(rt_ts_applied ? -1.0 : 1.0);
            float slice_width = (*innerTubeRadius) / static_cast<float>(m_MeanCorrectionVsR.size());
            for (auto & tr_point : tr_points) {
                int slice_number = static_cast<int>(std::floor(tr_point.x2() / slice_width));
                if (slice_number < 0) slice_number = 0;
                if (slice_number >= static_cast<int>(m_MeanCorrectionVsR.size()))
                    slice_number = static_cast<int>(m_MeanCorrectionVsR.size()) - 1;
                tr_point.set_x1(tr_point.x1() + sign * m_MeanCorrectionVsR[slice_number]);
            }
        }

        // Create resolution function from ts_points
        std::unique_ptr<MuonCalib::IRtResolution> reso = getRtResolutionInterpolation(ts_points);
        if (msgLvl(MSG::VERBOSE)) {
            ATH_MSG_VERBOSE("Resolution points :");
            for (std::vector<MuonCalib::SamplePoint>::const_iterator it = tr_points.begin(); it != tr_points.end(); ++it) {
                ATH_MSG_VERBOSE(it->x1() << "|" << it->x2() << "|" << it->error());
            }

            ATH_MSG_DEBUG("Resolution parameters :");
            for (unsigned int i = 0; i < reso->nPar(); i++) { ATH_MSG_VERBOSE(i << " " << reso->par(i)); }
        }

        // Create RT function from tr_points and load RT and resolution functions
        try {
            std::unique_ptr<MuonCalib::IRtRelation> rt = std::make_unique<MuonCalib::RtRelationLookUp>(MuonCalib::RtFromPoints::getRtRelationLookUp(tr_points));
            if (reso && rt) {
                if (regionId >= writeCdoRt->size()) {                   
                    ATH_MSG_ERROR("Illegal regionId " << regionId);
                    return StatusCode::FAILURE;
                } else {
                    if (rt->par(1) == 0.) {
                        ATH_MSG_WARNING("Bin size is 0");
                        for (const MuonCalib::SamplePoint& it: tr_points)
                            ATH_MSG_WARNING(it.x1() << " " << it.x2() << " " << it.error());
                    }
                    // Save ML difference if it is available
                    if (multilayer_tmax_diff > -8e8) { rt->SetTmaxDiff(multilayer_tmax_diff); }
                    // Store RT and resolution functions for this region
                    std::unique_ptr<MuonCalib::MdtRtRelation> rt_rel = std::make_unique<MuonCalib::MdtRtRelation>(std::move(rt), std::move(reso), 0.);
                    if (m_regionSvc->RegionType() == ONERT) {
                        (*writeCdoRt)[0] = std::move(rt_rel);
                        break;  // only read one RT from COOL for ONERT option.
                        // If doing ML2 RTs, and this is a ML2 RT function then add it to the end of writeCdoRt
                    } else if (m_regionSvc->RegionType() == ONEPERMULTILAYER && m_idHelperSvc->mdtIdHelper().multilayer(athenaId) == 2) {
                        ATH_MSG_VERBOSE("MdtCalibDbAlg::loadRt Load ML2 RT for region "
                                        << regionId << " " << m_idHelperSvc->toString(athenaId));
                        (*writeCdoRt).push_back(std::move(rt_rel));
                        IdentifierHash mlHash{0};
                        m_idHelperSvc->mdtIdHelper().get_detectorElement_hash(athenaId, mlHash);
                        m_regionSvc->setRegionHash(mlHash);
                    } else {  // store RT for chamber or ML1 if doing ONEPERMULTILAYER
                        (*writeCdoRt)[regionId] = std::move(rt_rel);
                        // TODO: add setter method to the container to check if you are not overwriting an existing pointer
                    }
                }  // end else regionId is OK
            }      // end if reso && rt
        }          // end try
        catch (int i) {
            ATH_MSG_FATAL("Error in creating rt-relation!");
            ATH_MSG_FATAL("npoints=" << tr_points.size());
            ATH_MSG_FATAL("Offending input: header=" << header);
            ATH_MSG_FATAL("Offending input: payload=" << payload);
            return StatusCode::FAILURE;
        }

    }  // end loop over itr (strings read from COOL)
    ATH_MSG_INFO("MdtCalibDbAlg::loadRt Read " << m_regionSvc->numberOfRegions() << "RTs from COOL");

    // like MdtCalibrationDbSvc
    // for corData in loadRt

    // finally record writeCdo

    if (writeCdoRt->empty()) {
        ATH_MSG_ERROR("writeCdoRt.size()==0");
        return StatusCode::FAILURE;
    }
    const MdtRtRelationCollection *writeCdoRtPtr = writeCdoRt.get();
    ATH_CHECK(writeHandleRt.record(std::move(writeCdoRt)));
    ATH_MSG_INFO("recorded new " << writeHandleRt.key() << " with range " << writeHandleRt.getRange() << " into Conditions Store");

    if (m_writeKeyCor.empty()){
        return StatusCode::SUCCESS;
    }
    SG::WriteCondHandle<MdtCorFuncSetCollection> writeHandleCor{m_writeKeyCor, ctx};
    writeHandleCor.addDependency(readHandleRt);
    
    std::unique_ptr<MdtCorFuncSetCollection> writeCdoCor{std::make_unique<MdtCorFuncSetCollection>()};
    ATH_MSG_DEBUG("Initializing " << writeCdoCor->size() << " b-field functions");
    for (const MuonCalib::MdtRtRelation* rtRelation :  * writeCdoRtPtr) {
        std::unique_ptr<MuonCalib::MdtCorFuncSet> corrFuncSet = std::make_unique<MuonCalib::MdtCorFuncSet>(); 
        if (m_create_b_field_function) initialize_B_correction(*corrFuncSet, *rtRelation);
        if (m_createWireSagFunction) initializeSagCorrection(*corrFuncSet);
        if (m_createSlewingFunction) {
            corrFuncSet->setSlewing(std::make_unique<MuonCalib::MdtSlewCorFuncHardcoded>(MuonCalib::CalibFunc::ParVec()));
        }
        writeCdoCor->push_back(std::move(corrFuncSet));
    }

    ATH_CHECK(writeHandleCor.record(std::move(writeCdoCor)));   
    ATH_MSG_INFO("recorded new " << writeHandleCor.key() << " with range " << writeHandleCor.getRange() << " into Conditions Store");
    

    return StatusCode::SUCCESS;
}

// build the transient structure and load some defaults for T0s
StatusCode MdtCalibDbAlg::defaultT0s(MdtTubeCalibContainerCollection &writeCdoTube) const {
   

    // like MdtCalibDbCoolStrTool::defaultT0s()
    // m_tubeData is writeCdoTube here
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    writeCdoTube.resize(id_helper.module_hash_max());
    ATH_MSG_DEBUG(" Created new MdtTubeCalibContainerCollection size " << writeCdoTube.size());

    // Inverse of wire propagation speed
    float inversePropSpeed = 1. / (Gaudi::Units::c_light * m_prop_beta);

    // loop over modules (MDT chambers) and create an MdtTubeContainer for each
    MdtIdHelper::const_id_iterator it = id_helper.module_begin();
    MdtIdHelper::const_id_iterator it_end = id_helper.module_end();
    for (; it != it_end; ++it) {
        // create an MdtTubeContainer
        std::unique_ptr<MuonCalib::MdtTubeCalibContainer> tubes = buildMdtTubeCalibContainer(*it);
        if (!tubes) {
            ATH_MSG_VERBOSE("Failed to create tube container for "<<m_idHelperSvc->toString(*it));
            continue;
        }
        // is tubes ever 0?  how could that happen?
       
        std::string rName = tubes->regionKey();
        double t0 = m_defaultT0;

        unsigned int nml = tubes->numMultilayers();
        unsigned int nlayers = tubes->numLayers();
        unsigned int ntubes = tubes->numTubes();
        int size = nml * nlayers * ntubes;
        ATH_MSG_VERBOSE("Adding chamber " << m_idHelperSvc->toString(*it));
        ATH_MSG_VERBOSE(" size " << size << " ml " << nml << " l " << nlayers << " t " << ntubes);
        for (unsigned int ml = 0; ml < nml; ++ml) {
            for (unsigned int l = 0; l < nlayers; ++l) {
                for (unsigned int t = 0; t < ntubes; ++t) {
                    MuonCalib::MdtTubeCalibContainer::SingleTubeCalib data;
                    data.t0 = t0;
                    data.adcCal = 1.;
                    data.inversePropSpeed = inversePropSpeed;
                    tubes->setCalib(ml, l, t, data);
                }
            }
        }
        
        ATH_MSG_VERBOSE(" set t0's done ");
        IdentifierHash hash{0};
        if (m_idHelperSvc->mdtIdHelper().get_module_hash(*it, hash)) {
            ATH_MSG_ERROR("Could not retrieve module hash for identifier " << m_idHelperSvc->toString(*it));
            return StatusCode::FAILURE;
        }

        if (hash < writeCdoTube.size()) {
            writeCdoTube[hash] = std::move(tubes);
            ATH_MSG_VERBOSE(" adding tubes at " << hash << " current size " << writeCdoTube.size());
            // write out string for chamberlist
            if (tubes) {
                int nml = tubes->numMultilayers();
                int nlayers = tubes->numLayers();
                int ntubes = tubes->numTubes();
                ATH_MSG_VERBOSE("CHAMBERLIST: " << m_idHelperSvc->toStringChamber(*it) << " "
                                << " " << nml * nlayers * ntubes << " " << nml << " " << nlayers << " " << ntubes << " dummy " << hash);
            }
        } else {            
            ATH_MSG_ERROR(" HashId out of range " << hash << " max " << writeCdoTube.size());
            return StatusCode::FAILURE;
        }
    }
    ATH_MSG_DEBUG(" Done defaultT0s " << writeCdoTube.size());

    return StatusCode::SUCCESS;
}  // end MdtCalibDbAlg::defaultT0s

StatusCode MdtCalibDbAlg::loadTube(const EventContext& ctx) const {
    ATH_MSG_DEBUG("loadTube " << name());

    SG::WriteCondHandle<MdtTubeCalibContainerCollection> writeHandleTube{m_writeKeyTube, ctx};
    if (writeHandleTube.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandleTube.fullKey() << " is already valid.");
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<MdtTubeCalibContainerCollection> writeCdoTube{std::make_unique<MdtTubeCalibContainerCollection>()};

    // like MdtCalibDbCoolStrTool::loadTube()
    // m_tubeData is writeCdoTube here
    // atrc is readCdoTube here
    ATH_CHECK(defaultT0s(*writeCdoTube));

    // Read Cond Handle
    SG::ReadCondHandle<CondAttrListCollection> readHandleTube{m_readKeyTube, ctx};
    writeHandleTube.addDependency(readHandleTube);
    ATH_MSG_INFO("Size of CondAttrListCollection " << readHandleTube.fullKey() << " readCdoTube->size()= " << readHandleTube->size());
    ATH_MSG_INFO("Range of input is " << readHandleTube.getRange());

    // read new-style format 2020
    std::vector<coral::AttributeList> dataPerChannel;
    if (m_newFormat2020) {
        CondAttrListCollection::const_iterator itr;
        for (itr = readHandleTube->begin(); itr != readHandleTube->end(); ++itr) {
            const coral::AttributeList &atr = itr->second;
            std::string data;
            if (atr["data"].specification().type() == typeid(coral::Blob)) {
                ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
                if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                    ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                    return StatusCode::FAILURE;
                }
            } else {
                ATH_MSG_VERBOSE("Loading data as a STRING");
                data = *(static_cast<const std::string *>((atr["data"]).addressOfData()));
            }

            // unwrap the json and build the data vector
            nlohmann::json yy = nlohmann::json::parse(data);
            for (auto &it : yy.items()) {
                nlohmann::json yx = it.value();
                for (auto &jt : yx.items()) {
                    coral::AttributeList al;
                    al.extend("tech", "int");
                    al.extend("file", "string");
                    al.extend("data", "blob");
                    al["tech"].data<int>() = jt.value()[1];
                    al["file"].data<std::string>() = static_cast<std::string>(jt.value()[2]);
                    std::string data = jt.value()[3];
                    if (!CoralUtilities::writeBlobFromString(data, al["data"].data<coral::Blob>())) {
                        ATH_MSG_FATAL("Cannot compress BLOB!");
                        return StatusCode::FAILURE;
                    }
                    dataPerChannel.push_back(al);
                }
            }
        }
    }
    // read old-style format
    else {
         for (CondAttrListCollection::const_iterator itr = readHandleTube->begin(); 
                    itr != readHandleTube->end(); ++itr) {
             dataPerChannel.push_back(itr->second);
        }
    }

    // Inverse of wire propagation speed
    float inversePropSpeed = 1. / (Gaudi::Units::c_light * m_prop_beta);

    // unpack the strings in the collection and update the
    // MdtTubeCalibContainers in TDS
    for (auto atr : dataPerChannel) {
        std::string header = "", payload = "", trailer = "";

        bool t0_ts_applied = (atr["tech"].data<int>() & MuonCalib::TIME_SLEWING_CORRECTION_APPLIED);
        // If BLOB data then uncompress
        if (atr["data"].specification().type() == typeid(coral::Blob)) {
            std::string istr;
            ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
            if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), istr)) {
                ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                return StatusCode::FAILURE;
            }
            ATH_CHECK(extractString(istr, header, "\n"));
            ATH_CHECK(extractString(istr, payload, "\n"));
            if (!istr.empty()) ATH_CHECK(extractString(istr, trailer, "\n"));
        } else {  // else is uncompressed CLOB (no longer used)
            std::string data;
            data = *(static_cast<const std::string *>((atr["data"]).addressOfData()));
            ATH_MSG_VERBOSE("Data load is " << data);

            // interpret as string stream
            std::string istr(data);
            ATH_CHECK(extractString(istr, header, "\n"));
            ATH_CHECK(extractString(istr, payload, "\n"));
            if (!istr.empty()) ATH_CHECK(extractString(istr, trailer, "\n"));
        }
        ATH_MSG_VERBOSE("Read header:" << header << " payload:" << payload << " trailer:" << trailer);

        // Extract info from the header line, chamber name, number of tubes.
        int ieta = -99, iphi = -99, region = -99, ntubes = -99;
        //    std::string rName;

        // parameters for the MdtTubeContainer
        // header filename,version,region,tubes
        std::unique_ptr<char[]> parameters{new char[header.size() + 1]};
        strncpy(parameters.get(), header.c_str(), header.size() + 1);
        parameters[header.size()] = '\0';             // terminate string
        char *saveptr;
        char *pch = strtok_r(parameters.get(), " _," , &saveptr);  // split using delimiters "_" and ","
        std::string name(pch, 2, 3);                  // extract 3-character station to "name" (e.g. BIL)
        // Split header line and extract phi, eta, region, ntubes
        pch = strtok_r(nullptr, "_,",&saveptr);
        for (int i = 1; pch != nullptr; pch = strtok_r(nullptr, "_,",&saveptr), i++) {
            std::istringstream is(pch);
            if (i == 1) {
                is >> iphi;
            } else if (i == 2) {
                is >> ieta;
            } else if (i == 4) {
                is >> region;
            } else if (i == 5) {
                is >> ntubes;
            }
        }

        // need to check validity of Identifier since database contains all Run 2 MDT chambers, e.g. also EI chambers which are
        // potentially replaced by NSW
        bool isValid = true;  // the elementID takes a bool pointer to check the validity of the Identifier
        Identifier chId = m_idHelperSvc->mdtIdHelper().elementID(name, ieta, iphi, isValid);
        if (!isValid) {
            static std::atomic<bool> idWarningPrinted = false;
            if (!idWarningPrinted) {
                ATH_MSG_WARNING("Element Identifier " << chId.get_compact() << " retrieved for station name " << name
                                                      << " is not valid, skipping");
                idWarningPrinted.store(true, std::memory_order_relaxed);
            }
            continue;
        }

        MuonCalib::MdtTubeCalibContainer *tubes = nullptr;

        // get chamber hash
        IdentifierHash hash{0};
        if (m_idHelperSvc->mdtIdHelper().get_module_hash(chId, hash))
            ATH_MSG_WARNING("Retrieving module hash for Identifier " << m_idHelperSvc->toString(chId) << " failed");

        // we have to check whether the retrieved Identifier is valid. The problem is that the is_valid() function of the Identifier class
        // does only check for the size of the number, not for the physical validity. The get_detectorElement_hash function of the
        // MuonIdHelper however returns an error in case the Identifier is not part of the vector of physically valid Identifiers (the check
        // could also be done using the module hash) It is important that the methods from MuonIdHelper are called which are not overwritten
        // by the MdtIdHelper
        IdentifierHash detElHash{0};
        if (m_idHelperSvc->mdtIdHelper().MuonIdHelper::get_detectorElement_hash(chId, detElHash)) {
            ATH_MSG_WARNING("Retrieving detector element hash for Identifier "
                            << chId.get_compact() << " failed, thus Identifier (original information was name=" << name << ", eta=" << ieta
                            << ", phi=" << iphi << ") is not valid, skipping...");
            continue;
        }

        if (msgLvl(MSG::VERBOSE)) {
            ATH_MSG_VERBOSE("name of chamber is " << pch << " station name is " << name);
            ATH_MSG_VERBOSE("phi value is " << iphi);
            ATH_MSG_VERBOSE("eta value is " << ieta);
            ATH_MSG_VERBOSE("region value is " << region);
            ATH_MSG_VERBOSE("ntubes value is " << ntubes);
            ATH_MSG_VERBOSE("station name is " << name << " chamber ID  is " << chId);
            ATH_MSG_VERBOSE("corresponding hash is " << hash);
        }

        if (hash >= writeCdoTube->size()) {
            ATH_MSG_ERROR("Illegal station (1)! (" << name << "," << iphi << "," << ieta << ")");
            return StatusCode::FAILURE;
        }

        // retrieve the existing one (created by defaultt0() )
        tubes = (*writeCdoTube)[hash];

        if (!tubes) {
            ATH_MSG_INFO("Illegal station (2)! (" << name << "," << iphi << "," << ieta << ")");
            continue;
        }

        int nml = tubes->numMultilayers();
        int nlayers = tubes->numLayers();
        int ntubesLay = tubes->numTubes();
        int size = nml * nlayers * ntubesLay;

        if (size != ntubes) {
            if(m_checkTubes) {
                ATH_MSG_ERROR("Mismatch between number of tubes in MdtTubeCalibContainer for chamber "
                              << name << "," << iphi << "," << ieta << " (" << size << ") and COOL DB (" << ntubes << ")");
                return StatusCode::FAILURE;
            } else {
                ATH_MSG_WARNING("Mismatch between number of tubes in MdtTubeCalibContainer for chamber "
                              << name << "," << iphi << "," << ieta << " (" << size << ") and COOL DB (" << ntubes << ")");                
            }
        }

        // Extract T0, ADCcal, valid flag for each tube from payload.
        MuonCalib::MdtTubeCalibContainer::SingleTubeCalib datatube;
        std::unique_ptr<char[]> TubePar{new char[payload.size() + 1]};
        strncpy(TubePar.get(), payload.c_str(), payload.size() + 1);
        TubePar[payload.size()] = '\0';

        // Loop over payload
        char *saveptr1;
        char *pch1 = strtok_r(TubePar.get(), ",", &saveptr1);
        int ml = 1, l = 1, t = 1;
        for (int k = 1; pch1 != nullptr; pch1 = strtok_r(nullptr, ", ", &saveptr1), ++k) {
            if (k == 1) {
                double tzero = atof(pch1);
                if (m_t0Shift != 0.) {
                    tzero += m_t0Shift;
                    ATH_MSG_VERBOSE("T0 shift " << m_t0Shift << " t0 " << tzero << " id " << ml << " " << l << " " << t);
                }
                if (m_t0Spread != 0.) {
                    CLHEP::HepRandomEngine *engine = m_RNGWrapper->getEngine(ctx);
                    double sh = CLHEP::RandGaussZiggurat::shoot(engine, 0., m_t0Spread);
                    tzero += sh;
                    ATH_MSG_VERBOSE("T0 spread " << sh << " t0 " << tzero << " id " << ml << " " << l << " " << t);
                }
                if (!t0_ts_applied && m_TimeSlewingCorrection) { tzero += m_TsCorrectionT0; }
                if (t0_ts_applied && !m_TimeSlewingCorrection) { tzero -= m_TsCorrectionT0; }
                datatube.t0 = tzero;
            } else if (k == 2) {
                datatube.statusCode = atoi(pch1);
            } else if (k == 3) {
                datatube.adcCal = atof(pch1);
                datatube.inversePropSpeed = inversePropSpeed;
                tubes->setCalib(ml - 1, l - 1, t - 1, datatube);
                ATH_MSG_VERBOSE("Loading T0s " << ml << " " << l << " " << t << " " << datatube.t0);
                t++;
                k = 0;
                if (t > ntubesLay) {
                    l++;
                    t = 1;
                }
                if (l > nlayers) {
                    ml++;
                    l = 1;
                }
            }
        }
    }  // end loop over readCdoTube

    // finally record writeCdo

    if (writeCdoTube->empty()) {
        ATH_MSG_WARNING("writeCdoTube->size()==0");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(writeHandleTube.record(std::move(writeCdoTube)));
    ATH_MSG_INFO("recorded new " << writeHandleTube.key() << " with range " << writeHandleTube.getRange() << " into Conditions Store");

    return StatusCode::SUCCESS;
}
MdtCalibDbAlg::chamberDim MdtCalibDbAlg::getChamberDimension(const Identifier& id) const {
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    chamberDim dim{};
    dim.multilayers = id_helper.numberOfMultilayers(id);
    if (m_detMgr) {
        const MuonGM::MdtReadoutElement* detEl = m_detMgr->getMdtReadoutElement(id_helper.multilayerID(id, 1));
        const MuonGM::MdtReadoutElement* detEl2 = dim.multilayers == 2 ? 
                                                   m_detMgr->getMdtReadoutElement(id_helper.multilayerID(id, 2)) : nullptr;
        if (!detEl) return dim;
        dim.layers = std::max(detEl->getNLayers(), detEl2 ? detEl2->getNLayers() : 0);
        dim.tubes = std::max(detEl->getNtubesperlayer(), detEl2 ? detEl2->getNtubesperlayer() : 0);
    } else if (m_r4detMgr) {
        const MuonGMR4::MdtReadoutElement* detEl = m_r4detMgr->getMdtReadoutElement(id_helper.multilayerID(id, 1));
        const MuonGMR4::MdtReadoutElement* detEl2 = dim.multilayers == 2 ? 
                                                   m_r4detMgr->getMdtReadoutElement(id_helper.multilayerID(id, 2)) : nullptr;
        if (!detEl) return dim;
        dim.layers = std::max(detEl->numLayers(), detEl2 ? detEl2->numLayers() : 0);
        dim.tubes = std::max(detEl->numTubesInLay(), detEl2 ? detEl2->numTubesInLay() : 0);
    }
    return dim;
}

// Build a MuonCalib::MdtTubeCalibContainer for a given Identifier
std::unique_ptr<MuonCalib::MdtTubeCalibContainer> MdtCalibDbAlg::buildMdtTubeCalibContainer(const Identifier &id) const {
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    const chamberDim dim = getChamberDimension(id);
    if (!dim) {
        static std::atomic<bool> warningPrinted = false;
        if (!warningPrinted) {
            ATH_MSG_WARNING("buildMdtTubeCalibContainer() - Ignoring nonexistant station in calibration DB: "
                            << m_idHelperSvc->toString(id) << ", cf. ATLASRECTS-6035");
            warningPrinted = true;
        }
        return nullptr;
    }
    // build the region name in the format STATION_ETA_PHI    
    int stName = id_helper.stationName(id);
    int stPhi = id_helper.stationPhi(id);
    int stEta = id_helper.stationEta(id);

    std::stringstream rName{};
    rName<<id_helper.stationNameString(stName)<<"_";
    rName<<stPhi<<"_"<<stEta;
    return std::make_unique<MuonCalib::MdtTubeCalibContainer>(rName.str(), dim.multilayers, dim.layers, dim.tubes);
}  // end MdtCalibDbAlg::buildMdtTubeCalibContainer

std::unique_ptr<MuonCalib::RtResolutionLookUp> MdtCalibDbAlg::getRtResolutionInterpolation(
    const std::vector<MuonCalib::SamplePoint> &sample_points) {
    ///////////////
    // VARIABLES //
    ///////////////
    std::vector<Double_t> x(sample_points.size(),0);
    std::vector<Double_t> y(sample_points.size(),0);

    for (unsigned int i = 0; i < sample_points.size(); i++) {
        x[i] = sample_points[i].x1();
        y[i] = sample_points[i].x2();
    }
    TSpline3 sp("Rt Res Tmp", x.data(), y.data(), sample_points.size());
    ///////////////////////////////////////////////////////////////////
    // CREATE AN RtRelationLookUp OBJECT WITH THE CORRECT PARAMETERS //
    ///////////////////////////////////////////////////////////////////
    unsigned int nb_points(100);
    std::vector<double> res_param(nb_points + 2);  // r-t parameters
    Double_t bin_width = (x[sample_points.size() - 1] - x[0]) / static_cast<Double_t>(nb_points);

    res_param[0] = x[0];
    res_param[1] = bin_width;
    for (unsigned int k = 0; k < nb_points; k++) {
      Double_t xx = x[0] + k * bin_width;
      res_param[k + 2] = sp.Eval(xx);
      if (std::isnan(res_param[k + 2])) {
        TFile outf("kacke.root", "RECREATE");
        sp.Write("kacke");
        throw std::runtime_error("MdtCalibDbAlg::getRtResolutionInterpolation "
                                 "encountered nan element");
      }
    }
    return std::make_unique<MuonCalib::RtResolutionLookUp>(std::move(res_param));
}

StatusCode MdtCalibDbAlg::extractString(std::string &input, std::string &output, const std::string& separator) const {
    unsigned long int pos = 0;
    std::string::size_type start = input.find_first_not_of(separator.c_str(), pos);
    if (start == std::string::npos) {
        ATH_MSG_ERROR("MdtCalibDbAlg::extractString: Cannot extract string in a proper way!");
        return StatusCode::FAILURE;
    }
    std::string::size_type stop = input.find_first_of(separator.c_str(), start + 1);
    if (stop == std::string::npos) stop = input.size();
    output = input.substr(start, stop - start);
    input.erase(pos, stop - pos);

    return StatusCode::SUCCESS;
}

// like MdtCalibrationDbSvc
// for corData in loadRt
void MdtCalibDbAlg::initialize_B_correction(MuonCalib::MdtCorFuncSet& funcSet, const MuonCalib::MdtRtRelation& rt_rel) const {
    ATH_MSG_VERBOSE("initialize_B_correction...");
    std::vector<double> corr_params(2);
    corr_params[0] = 3080.0;  // high voltage (not correct for sMDT which use 2730V!)
    corr_params[1] = 0.11;    // epsilon parameter
    funcSet.setBField(std::make_unique<MuonCalib::BFieldCorFunc>("medium", corr_params, rt_rel.rt()));
}

void MdtCalibDbAlg::initializeSagCorrection(MuonCalib::MdtCorFuncSet& funcSet) const {
    ATH_MSG_VERBOSE("initializeSagCorrection...");
    std::vector<double> corr_params(0);
    funcSet.wireSag(std::make_unique<MuonCalib::WireSagCorFunc>(corr_params));
}
