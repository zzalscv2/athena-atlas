/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtRdoToPrepDataToolCore.h"

#include <algorithm>
#include <vector>

#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "GeoModelUtilities/GeoGetIds.h"
#include "MdtCalibSvc/MdtCalibrationSvcInput.h"
#include "MdtCalibSvc/MdtCalibrationTool.h"
#include "MdtRDO_Decoder.h"
#include "MuonCalibEvent/MdtCalibHit.h"
#include "MuonPrepRawData/MdtTwinPrepData.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"

using namespace MuonGM;
using namespace Trk;
using namespace Muon;

namespace {
    static constexpr double inverseSpeedOfLight = 1 / Gaudi::Units::c_light;  // need 1/299.792458
    // the tube number of a tube in a tubeLayer is encoded in the GeoSerialIdentifier (modulo maxNTubesPerLayer)
    static constexpr unsigned int maxNTubesPerLayer = MdtIdHelper::maxNTubesPerLayer;
}  // namespace

namespace Muon {

    MdtPrepDataCollection* MdtRdoToPrepDataToolCore::ModfiablePrdColl::createCollection(const Identifier& elementId,
                                                                                        const MdtIdHelper& id_helper, MsgStream& msg) {
        IdentifierHash mdtHashId{0};
        if (id_helper.get_module_hash(elementId, mdtHashId)) {
            msg << MSG::ERROR << "Module hash creation failed. " << elementId << endmsg;
            return nullptr;
        }
        PrdCollMap::iterator itr = addedCols.find(mdtHashId);
        if (itr != addedCols.end()) return itr->second.get();
        MdtPrepDataContainer::IDC_WriteHandle lock = prd_cont->getWriteHandle(mdtHashId);
        if (lock.alreadyPresent()) {
            if (msg.level() <= MSG::DEBUG) {
                msg << MSG::DEBUG << "MdtPrepDataCollection already contained in IDC " << elementId << " " << mdtHashId << endmsg;
            }
            return nullptr;
        }
        std::unique_ptr<MdtPrepDataCollection> newColl = std::make_unique<MdtPrepDataCollection>(mdtHashId);
        newColl->setIdentifier(id_helper.elementID(elementId));
        return addedCols.insert(std::make_pair(mdtHashId, std::move(newColl))).first->second.get();
    }
    StatusCode MdtRdoToPrepDataToolCore::ModfiablePrdColl::finalize(std::vector<IdentifierHash>& prdHashes, MsgStream& msg) {
        for (auto& to_insert : addedCols) {
            if (to_insert.second->empty()) continue;
            MdtPrepDataContainer::IDC_WriteHandle lock = prd_cont->getWriteHandle(to_insert.first);
            if (lock.addOrDelete(std::move(to_insert.second)).isFailure()) {
                msg << MSG::ERROR << " Failed to add prep data collection " << to_insert.first << endmsg;
                return StatusCode::FAILURE;
            }
            prdHashes.emplace_back(to_insert.first);
        }
        return StatusCode::SUCCESS;
    }

    MdtRdoToPrepDataToolCore::MdtRdoToPrepDataToolCore(const std::string& t, const std::string& n, const IInterface* p) :
        base_class(t, n, p) {
        // - TWIN TUBE
        declareProperty("TimeWindowLowerBound", m_mdtCalibSvcSettings->windowLowerBound);
        declareProperty("TimeWindowUpperBound", m_mdtCalibSvcSettings->windowUpperBound);
        declareProperty("TimeWindowSetting", m_mdtCalibSvcSettings->windowSetting = 2);
        declareProperty("DoTofCorrection", m_mdtCalibSvcSettings->doTof = true);
        declareProperty("DoPropagationCorrection", m_mdtCalibSvcSettings->doProp = false);
        /// Update stuff
        declareProperty("MdtPrdContainerCacheKey", m_prdContainerCacheKey, "Optional external cache for the MDT PRD container");
    }

    StatusCode MdtRdoToPrepDataToolCore::initialize() {
        ATH_CHECK(AthAlgTool::initialize());
        const MuonGM::MuonDetectorManager* muDetMgr = nullptr;
        ATH_CHECK(detStore()->retrieve(muDetMgr));
        ATH_CHECK(m_calibrationTool.retrieve());
        ATH_MSG_VERBOSE("MdtCalibrationTool retrieved with pointer = " << m_calibrationTool);
        ATH_CHECK(m_prdContainerCacheKey.initialize(!m_prdContainerCacheKey.key().empty()));
        ATH_CHECK(m_idHelperSvc.retrieve());
        // Retrieve the RDO decoder
        ATH_CHECK(m_mdtDecoder.retrieve());

        // + TWIN TUBES
        // make an array of [multilayer][layer][twin-pair]; 2 multilayers, 3 layer per multilayer, 36 twin-pairs per layer
        if (m_useTwin) {
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 3; j++) {
                    for (int k = 0; k < 36; k++) {
                        // fill m_twin_chamber array with unique numbers
                        m_twin_chamber[i][j][k] = 1000 * i + 100 * j + k;
                        // for secondary hits we need to make a second array with unique numbers
                        // (i+1 is used in the expression, so numbers are always different from m_twin_chamber array)
                        m_secondaryHit_twin_chamber[i][j][k] = 10000 * (i + 1) + 100 * j + k;
                    }
                }
            }
        }  // end if(m_useTwin){
        // - TWIN TUBES

        m_mdtCalibSvcSettings->initialize();

        m_BMGid = m_idHelperSvc->mdtIdHelper().stationNameIndex("BMG");
        m_BMGpresent = m_BMGid != -1;
        if (m_BMGpresent) {
            ATH_MSG_INFO("Processing configuration for layouts with BMG chambers.");

            for (int phi = 6; phi < 8; phi++) {                 // phi sectors
                for (int eta = 1; eta < 4; eta++) {             // eta sectors
                    for (int side = -1; side < 2; side += 2) {  // side
                        if (!muDetMgr->getMuonStation("BMG", side * eta, phi)) continue;
                        for (int roe = 1; roe <= (muDetMgr->getMuonStation("BMG", side * eta, phi))->nMuonReadoutElements();
                             roe++) {  // iterate on readout elemets
                            const MdtReadoutElement* mdtRE = dynamic_cast<const MdtReadoutElement*>(
                                (muDetMgr->getMuonStation("BMG", side * eta, phi))->getMuonReadoutElement(roe));  // has to be an MDT
                            if (mdtRE) initDeadChannels(mdtRE);
                        }
                    }
                }
            }
        }

        // check if initializing of DataHandle objects success
        ATH_CHECK(m_rdoContainerKey.initialize());
        ATH_CHECK(m_mdtPrepDataContainerKey.initialize());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_muDetMgrKey.initialize());
        return StatusCode::SUCCESS;
    }

    StatusCode MdtRdoToPrepDataToolCore::decode(const std::vector<uint32_t>& robIds) const {
        const EventContext& ctx = Gaudi::Hive::currentContext();
        SG::ReadCondHandle<MuonMDT_CablingMap> readHandle{m_readKey, ctx};
        const MuonMDT_CablingMap* readCdo{*readHandle};
        if (!readCdo) {
            ATH_MSG_ERROR("nullptr to the read conditions object");
            return StatusCode::FAILURE;
        }
        return decode(ctx, readCdo->getMultiLayerHashVec(robIds, msgStream()));
    }

    const MdtCsmContainer* MdtRdoToPrepDataToolCore::getRdoContainer(const EventContext& ctx) const {
        SG::ReadHandle<MdtCsmContainer> rdoContainerHandle{m_rdoContainerKey, ctx};
        if (rdoContainerHandle.isValid()) {
            ATH_MSG_DEBUG("MdtgetRdoContainer success");
            return rdoContainerHandle.cptr();
        }
        ATH_MSG_WARNING("Retrieval of Mdt RDO container failed !");
        return nullptr;
    }

    StatusCode MdtRdoToPrepDataToolCore::decode(const EventContext& ctx, const std::vector<IdentifierHash>& multiLayerHashInRobs) const {
        // setup output container
        ModfiablePrdColl mdtPrepDataContainer = setupMdtPrepDataContainer(ctx);
        if (!mdtPrepDataContainer.prd_cont) { return StatusCode::FAILURE; }

        if (!m_decodeData) {
            ATH_MSG_DEBUG("Stored empty container. Decoding MDT RDO into MDT PrepRawData is switched off");
            return StatusCode::SUCCESS;
        }

        // left unused, needed by other decode function and further down the code.
        std::vector<IdentifierHash> idWithDataVect;
        processPRDHashes(ctx, mdtPrepDataContainer, multiLayerHashInRobs);
        ATH_CHECK(mdtPrepDataContainer.finalize(idWithDataVect, msgStream()));
        return StatusCode::SUCCESS;
    }  // end decode

    void MdtRdoToPrepDataToolCore::processPRDHashes(const EventContext& ctx, ModfiablePrdColl& mdtPrepDataContainer,
                                                    const std::vector<IdentifierHash>& multiLayerHashInRobs) const {
        for (const IdentifierHash& hash : multiLayerHashInRobs) {
            if (!handlePRDHash(ctx, mdtPrepDataContainer, hash)) { ATH_MSG_DEBUG("Failed to process hash " << hash); }
        }  // ends loop over chamberhash
    }

    bool MdtRdoToPrepDataToolCore::handlePRDHash(const EventContext& ctx, ModfiablePrdColl& mdtPrepDataContainer,
                                                 IdentifierHash rdoHash) const {
        const MdtCsmContainer* rdoContainer{getRdoContainer(ctx)};

        if (!rdoContainer->size()) {
            ATH_MSG_DEBUG("The container is empty");
            return true;
        }
        const MdtCsm* rdoColl = rdoContainer->indexFindPtr(rdoHash);
        if (!rdoColl) {
            ATH_MSG_DEBUG("The rdo container does not have the hash " << rdoHash);
            return true;
        }
        SG::ReadCondHandle<MuonGM::MuonDetectorManager> muDetMgr{m_muDetMgrKey, ctx};
        if (!muDetMgr.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the Muon detector manager " << m_muDetMgrKey.fullKey());
            return false;
        }

        if (processCsm(mdtPrepDataContainer, rdoColl, *muDetMgr).isFailure()) {
            ATH_MSG_WARNING("processCsm failed for RDO id " << m_idHelperSvc->toString(rdoColl->identify()));
            return false;
        }
        return true;
    }

    StatusCode MdtRdoToPrepDataToolCore::decode(std::vector<IdentifierHash>& idVect, std::vector<IdentifierHash>& idWithDataVect) const {
        // clear output vector of selected data collections containing data
        idWithDataVect.clear();
        const EventContext& ctx = Gaudi::Hive::currentContext();
        ATH_MSG_DEBUG("decodeMdtRDO for " << idVect.size() << " offline collections called");

        // setup output container
        ModfiablePrdColl mdtPrepDataContainer = setupMdtPrepDataContainer(ctx);
        if (!mdtPrepDataContainer.prd_cont) { return StatusCode::FAILURE; }

        if (!m_decodeData) {
            ATH_MSG_DEBUG("Stored empty container. Decoding MDT RDO into MDT PrepRawData is switched off");
            return StatusCode::SUCCESS;
        }
        // seeded or unseeded decoding
        if (!idVect.empty()) {
            processPRDHashes(ctx, mdtPrepDataContainer, idVect);
        } else {
            /// Construct the hashes from the existing RDOs
            std::vector<IdentifierHash> rdoHashes{};
            const MdtCsmContainer* rdoContainer = getRdoContainer(Gaudi::Hive::currentContext());
            if (!rdoContainer || !rdoContainer->size()) return StatusCode::SUCCESS;
            rdoHashes.reserve(rdoContainer->size());
            for (const MdtCsm* csm : *rdoContainer) rdoHashes.push_back(csm->identifyHash());

            processPRDHashes(ctx, mdtPrepDataContainer, rdoHashes);
        }
        ATH_CHECK(mdtPrepDataContainer.finalize(idWithDataVect, msgStream()));

        return StatusCode::SUCCESS;
    }

    // dump the RDO in input
    void MdtRdoToPrepDataToolCore::printInputRdo() const {
        ATH_MSG_DEBUG("******************************************************************************************");
        ATH_MSG_DEBUG("***************** Listing MdtCsmContainer collections content ********************************");

        const MdtCsmContainer* rdoContainer = getRdoContainer(Gaudi::Hive::currentContext());

        if (!rdoContainer->size()) ATH_MSG_DEBUG("MdtCsmContainer is Empty");

        ATH_MSG_DEBUG("-----------------------------------------------------------------------------");

        unsigned int ncsm{0}, namt{0};
        // loop on the MdtCsm collections
        for (const MdtCsm* mdtColl : *rdoContainer) {
            ++ncsm;
            ATH_MSG_DEBUG("**** MdtCsm with online Id: subdetector: " << MSG::hex << mdtColl->SubDetId() << MSG::dec
                                                                      << "  mrod: " << MSG::hex << mdtColl->MrodId() << MSG::dec
                                                                      << "  csmid: " << MSG::hex << mdtColl->CsmId() << MSG::dec);
            ATH_MSG_DEBUG("  number of mdt hits: " << mdtColl->size());

            // loop on the hits of the CSM
            for (const MdtAmtHit* amtHit : *mdtColl) {
                ++namt;
                ATH_MSG_DEBUG(">> AmtHit in tdc: " << MSG::hex << amtHit->tdcId() << MSG::dec << "  channel: " << MSG::hex
                                                   << amtHit->channelId() << MSG::dec << "  fine time: " << amtHit->fine()
                                                   << "  coarse time: " << amtHit->coarse() << "  width: " << amtHit->width());
            }
        }

        ATH_MSG_DEBUG("*** Event Summary: csm collections:" << ncsm << "  amt hits: " << namt);

        return;
    }

    void MdtRdoToPrepDataToolCore::printPrepDataImpl(const MdtPrepDataContainer* mdtPrepDataContainer) const {
        // Dump info about PRDs
        ATH_MSG_DEBUG("******************************************************************************************");
        ATH_MSG_DEBUG("***************** Listing MdtPrepData collections content ********************************");

        if (mdtPrepDataContainer->size() <= 0) ATH_MSG_DEBUG("No MdtPrepRawData collections found");
        int ncoll = 0;
        int nhits = 0;
        ATH_MSG_DEBUG("--------------------------------------------------------------------------------------------");
        for (const MdtPrepDataCollection* mdtColl : *mdtPrepDataContainer) {
            int nhitcoll = 0;
            if (mdtColl->size() > 0) {
                ATH_MSG_DEBUG("PrepData Collection ID " << m_idHelperSvc->toString(mdtColl->identify()));
                for (const MdtPrepData* prepData : *mdtColl) {
                    nhitcoll++;
                    nhits++;
                    ATH_MSG_DEBUG(" in this coll. " << nhitcoll << " prepData id = " << m_idHelperSvc->toString(prepData->identify())
                                                    << " tdc/adc =" << prepData->tdc() << "/" << prepData->adc());
                }
                ncoll++;
                ATH_MSG_DEBUG("*** Collection " << ncoll << " Summary: N. hits = " << nhitcoll);
                ATH_MSG_DEBUG("--------------------------------------------------------------------------------------------");
            }
        }
        ATH_MSG_DEBUG("*** Event  Summary: " << ncoll << " Collections / " << nhits << " hits  ");
        ATH_MSG_DEBUG("--------------------------------------------------------------------------------------------");
    }

    StatusCode MdtRdoToPrepDataToolCore::processCsm(ModfiablePrdColl& prepDataContainer, const MdtCsm* rdoColl,
                                                    const MuonGM::MuonDetectorManager* muDetMgr) const {
        const MdtIdHelper& id_helper = m_idHelperSvc->mdtIdHelper();
        // first handle the case of twin tubes
        if (m_useTwin) {
            // two chambers in ATLAS are installed with Twin Tubes; in detector coordinates BOL4A13 & BOL4C13; only INNER multilayer(=1) is
            // with twin tubes implement twin tube writing to prepData either for all BOL (m_useAllBOLTwin = true) _OR_ only for two
            // chambers really installed
            Identifier elementId = rdoColl->identify();
            MuonStationIndex::ChIndex chIndex = m_idHelperSvc->chamberIndex(elementId);
            if (chIndex == MuonStationIndex::BOL &&
                (m_useAllBOLTwin || (std::abs(id_helper.stationEta(elementId)) == 4 && id_helper.stationPhi(elementId) == 7))) {
                return processCsmTwin(prepDataContainer, rdoColl, muDetMgr);
            }
        }

        ATH_MSG_DEBUG(" ***************** Start of processCsm");

        /// MDT hit context
        const Identifier elementId = id_helper.parentID(rdoColl->identify());

        uint16_t subdetId = rdoColl->SubDetId();
        uint16_t mrodId = rdoColl->MrodId();
        uint16_t csmId = rdoColl->CsmId();
        ATH_MSG_VERBOSE("Identifier = " << m_idHelperSvc->toString(elementId) << " subdetId/ mrodId/ csmId = " << subdetId << " / "
                                        << mrodId << " / " << csmId);

        // for each Csm, loop over AmtHit, converter AmtHit to digit
        // retrieve/create digit collection, and insert digit into collection
        int mc = 0;
        for (const MdtAmtHit* amtHit : *rdoColl) {
            mc++;

            // FIXME: Still use the digit class.
            ATH_MSG_VERBOSE("Amt Hit n. " << mc << " tdcId = " << amtHit->tdcId());
            std::unique_ptr<MdtDigit> newDigit{m_mdtDecoder->getDigit(amtHit, subdetId, mrodId, csmId)};

            if (!newDigit) {
                ATH_MSG_WARNING("Found issue MDT RDO decoder for subdetId/mrodId/csmId "
                                << subdetId << "/" << mrodId << "/" << csmId << " amtHit channelId/tdcId =" << amtHit->channelId() << "/"
                                << amtHit->tdcId());
                continue;
            }
            // Do something with it
            Identifier channelId = newDigit->identify();
            if (deadBMGChannel(channelId)) continue;

            // Retrieve the proper PRD container. Note that there are cases where one CSM is either split into 2 chambers (BEE / BIS78
            // legacy) or 2 CSMs are split into one chamber
            MdtPrepDataCollection* driftCircleColl = prepDataContainer.createCollection(channelId, id_helper, msgStream());
            if (!driftCircleColl) {
                ATH_MSG_DEBUG("Corresponding multi layer " << m_idHelperSvc->toString(channelId) << " is already decoded.");
                continue;
            }

            // check if the module ID of this channel is different from the CSM one
            // If it's the first case, create the additional collection

            ATH_MSG_VERBOSE("got digit with id ext / hash " << m_idHelperSvc->toString(channelId) << " / "
                                                            << driftCircleColl->identifyHash());

            double radius{0.}, errRadius{0.};
            MdtDriftCircleStatus digitStatus = MdtStatusDriftTime;

            // do lookup once
            const MdtReadoutElement* descriptor = muDetMgr->getMdtReadoutElement(channelId);
            if (!descriptor) {
                ATH_MSG_WARNING("Detector Element not found for Identifier from the cabling service <" << m_idHelperSvc->toString(channelId)
                                                                                                       << ">  =>>ignore this hit");
                continue;
            }
            if (!descriptor->containsId(channelId)) {
                ATH_MSG_WARNING("Detector Element " << m_idHelperSvc->toString(descriptor->identify())
                                                    << " does not contains candidate prd Identifier <" << m_idHelperSvc->toString(channelId)
                                                    << ">  =>>ignore this hit");
                continue;
            }

            // Rescale ADC/TDC of chambers using HPTDC digitization chip
            // Must create a new digit from the old one, because MdtDigit has no methods to set ADC/TDC
            if (m_idHelperSvc->hasHPTDC(channelId)) {
                int adc = newDigit->adc() / 4;
                int tdc = newDigit->tdc() / 4;
                int mask = newDigit->is_masked();
                newDigit = std::make_unique<MdtDigit>(channelId, tdc, adc, mask);
                ATH_MSG_DEBUG("Change HPTDC ADC/TDC " << m_idHelperSvc->toString(channelId) << " Old ADC/TDC=" << adc * 4 << " " << tdc * 4
                                                      << " New=" << adc << " " << tdc);
            }

            if (newDigit->is_masked()) {
                digitStatus = MdtStatusMasked;
            } else {
                digitStatus = getMdtDriftRadius(*newDigit, radius, errRadius, descriptor, muDetMgr);
                if (radius < -999) {
                    ATH_MSG_WARNING("MDT PrepData with very large, negative radius "
                                    << " Id is: " << m_idHelperSvc->toString(channelId));
                }
            }

            Amg::Vector2D driftRadius(radius, 0);
            Amg::MatrixX cov(1, 1);
            (cov)(0, 0) = errRadius * errRadius;
            // Create new PrepData

            MdtPrepData* newPrepData = new MdtPrepData(channelId, driftCircleColl->identifyHash(), driftRadius, cov, descriptor,
                                                       newDigit->tdc(), newDigit->adc(), digitStatus);

            newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
            driftCircleColl->push_back(newPrepData);
        }
        return StatusCode::SUCCESS;
    }
    bool MdtRdoToPrepDataToolCore::deadBMGChannel(const Identifier& channelId) const {
        const MdtIdHelper& id_helper = m_idHelperSvc->mdtIdHelper();
        if (id_helper.stationName(channelId) != m_BMGid || !m_BMGpresent) return false;
        std::map<Identifier, std::vector<Identifier>>::const_iterator myIt = m_DeadChannels.find(id_helper.multilayerID(channelId));
        if (myIt == m_DeadChannels.end()) { return false; }
        if (std::find((myIt->second).begin(), (myIt->second).end(), channelId) != (myIt->second).end()) {
            ATH_MSG_DEBUG("deadBMGChannel : Deleting BMG digit with identifier" << m_idHelperSvc->toString(channelId));
            return true;
        }
        return false;
    }
    StatusCode MdtRdoToPrepDataToolCore::processCsmTwin(ModfiablePrdColl& prepDataContainer, const MdtCsm* rdoColl,
                                                        const MuonGM::MuonDetectorManager* muDetMgr) const {
        const MdtIdHelper& id_helper = m_idHelperSvc->mdtIdHelper();
        ATH_MSG_DEBUG(" ***************** Start of processCsmTwin");
        ATH_MSG_DEBUG(" Number of AmtHit in this Csm " << rdoColl->size());
        /// MDT hit context
        Identifier elementId = id_helper.parentID(rdoColl->identify());

        uint16_t subdetId = rdoColl->SubDetId();
        uint16_t mrodId = rdoColl->MrodId();
        uint16_t csmId = rdoColl->CsmId();
        ATH_MSG_VERBOSE("Identifier = " << m_idHelperSvc->toString(elementId) << " subdetId/ mrodId/ csmId = " << rdoColl->SubDetId()
                                        << " / " << rdoColl->MrodId() << " / " << rdoColl->CsmId());

        // for each Csm, loop over AmtHit, converter AmtHit to digit
        // retrieve/create digit collection, and insert digit into collection

        // make a map to be filled for every twin-pair
        //   std::map<int, std::vector<MdtDigit*> > mdtDigitColl;

        using twin_digit = std::pair<std::unique_ptr<MdtDigit>, std::unique_ptr<MdtDigit>>;
        std::map<int, twin_digit> mdtDigitColl;

        for (const MdtAmtHit* amtHit : *rdoColl) {
            std::unique_ptr<MdtDigit> newDigit{m_mdtDecoder->getDigit(amtHit, subdetId, mrodId, csmId)};

            if (!newDigit) {
                ATH_MSG_WARNING("Error in MDT RDO decoder for subdetId/mrodId/csmId "
                                << subdetId << "/" << mrodId << "/" << csmId << " amtHit channelId/tdcId =" << amtHit->channelId() << "/"
                                << amtHit->tdcId());
                continue;
            }

            // make an Identifier
            Identifier channelId = newDigit->identify();
            // IdentifierHash channelHash = newDigit->identifyHash();
            if (deadBMGChannel(channelId)) continue;

            // get tube params
            int tube = id_helper.tube(channelId);
            int layer = id_helper.tubeLayer(channelId);
            int multilayer = id_helper.multilayer(channelId);

            // find the correct twin-pair (tube-1 & tube-3 are twin pair 1, tube-2 & tube-4 are twin pair 2)
            int twinPair = -1;
            if (tube % 4 == 1) {
                twinPair = (tube + 1) / 2;
            } else if (tube % 4 == 3) {
                twinPair = (tube - 1) / 2;
            } else if (tube % 4 == 2) {
                twinPair = (tube + 2) / 2;
            } else {
                twinPair = tube / 2;
            }  // tube%4 == 0

            // fill the digitColl map
            twin_digit& pair = mdtDigitColl[m_twin_chamber[multilayer - 1][layer - 1][twinPair - 1]];
            if (!pair.first) {
                pair.first = std::move(newDigit);
            } else if (!pair.second) {
                pair.second = std::move(newDigit);
            }
            // if a secondary hit appears in a tube add it to mdtDigitColl, unless m_discardSecondaryHitTwin flag is true
            else {
                ATH_MSG_VERBOSE(" TWIN TUBES: found a secondary(not twin) hit in a twin tube");
                twin_digit& secondPair = mdtDigitColl[m_secondaryHit_twin_chamber[multilayer - 1][layer - 1][twinPair - 1]];
                if (!m_discardSecondaryHitTwin) {
                    if (!secondPair.first) {
                        secondPair.first = std::move(newDigit);
                    } else if (!secondPair.second) {
                        secondPair.second = std::move(newDigit);
                    } else {
                        ATH_MSG_VERBOSE(" TWIN TUBES: found a tertiary hit in a twin tube in one RdoCollection for "
                                        << m_idHelperSvc->toString(channelId) << " with adc  = " << newDigit->adc()
                                        << "  tdc = " << newDigit->tdc());
                    }
                }  // end --   if(!m_discardSecondaryHitTwin){
                else {
                    ATH_MSG_DEBUG(
                        " TWIN TUBES: discarding secondary(non-twin) hit in a twin tube as flag m_discardSecondaryHitTwin is set to true");
                }
            }
        }  // end for-loop over rdoColl

        // iterate over mdtDigitColl
        for (std::pair<const int, twin_digit>& digitPair : mdtDigitColl) {
            // get the twin hits from mdtDigitColl
            std::unique_ptr<MdtDigit>& digit = digitPair.second.first;
            std::unique_ptr<MdtDigit>& second_digit = digitPair.second.second;

            if (!digit) {
                ATH_MSG_FATAL("nullptr to a digit ");
                return StatusCode::FAILURE;
            }

            // Do something with it
            Identifier channelId = digit->identify();
            int multilayer = id_helper.multilayer(channelId);

            MdtPrepDataCollection* driftCircleColl = prepDataContainer.createCollection(channelId, id_helper, msgStream());
            if (!driftCircleColl) {
                ATH_MSG_DEBUG("Corresponding multi layer " << m_idHelperSvc->toString(channelId) << " is already decoded.");
                continue;
            }

            // check if the hit is in multilayer=1
            // two chambers in ATLAS are installed with Twin Tubes; in detector coordinates BOL4A13 & BOL4C13; only INNER multilayer(=1) is
            // with twin tubes
            if (multilayer == 1) {
                // if no twin hit present in data, use standard PRD making
                if (!second_digit) {
                    ATH_MSG_VERBOSE("got digit with id ext / hash " << m_idHelperSvc->toString(channelId) << " / "
                                                                    << driftCircleColl->identifyHash());

                    double radius{0.}, errRadius{0.};
                    MdtDriftCircleStatus digitStatus = MdtStatusDriftTime;

                    // do lookup once
                    const MdtReadoutElement* descriptor = muDetMgr->getMdtReadoutElement(channelId);
                    if (!descriptor) {
                        ATH_MSG_WARNING("Detector Element not found for Identifier from the cabling service <"
                                        << m_idHelperSvc->toString(channelId) << ">  =>>ignore this hit");
                        continue;
                    }

                    if (digit->is_masked()) {
                        digitStatus = MdtStatusMasked;
                    } else {
                        digitStatus = getMdtDriftRadius(*digit, radius, errRadius, descriptor, muDetMgr);
                        if (radius < -999) {
                            ATH_MSG_WARNING("MDT PrepData with very large, negative radius "
                                            << " Id is: " << m_idHelperSvc->toString(channelId));
                        }
                    }

                    Amg::Vector2D driftRadius(radius, 0);
                    Amg::MatrixX cov(1, 1);
                    (cov)(0, 0) = errRadius * errRadius;

                    // Create new PrepData
                    MdtPrepData* newPrepData = new MdtPrepData(channelId, driftCircleColl->identifyHash(), driftRadius, cov, descriptor,
                                                               digit->tdc(), digit->adc(), digitStatus);

                    newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                    driftCircleColl->push_back(newPrepData);

                    ATH_MSG_DEBUG(" MADE ORIGINAL PREPDATA " << m_idHelperSvc->toString(channelId) << "  radius = " << radius << " +- "
                                                             << errRadius);

                }  // end if(!second_digit){
                else {
                    // define twin position and error
                    double zTwin{0.}, errZTwin{0.}, radius{0.}, errRadius{0.};
                    bool secondHitIsPrompt(false);
                    // define drift-radius and error
                    MdtDriftCircleStatus digitStatus = MdtStatusDriftTime;
                    // call the function to calculate radii and twin coordinate
                    digitStatus =
                        getMdtTwinPosition(*digit, *second_digit, radius, errRadius, zTwin, errZTwin, secondHitIsPrompt, muDetMgr);
                    if (zTwin < -99999) {
                        ATH_MSG_WARNING("MDT Twin PrepData with very large, negative twin coordinate "
                                        << zTwin << " Id is: " << m_idHelperSvc->toString(digit->identify())
                                        << " Twin Id is: " << m_idHelperSvc->toString(second_digit->identify()));
                    }

                    // set the properties of PrepData-object to the tube that was PROMPT (= hit by the muon)

                    if (secondHitIsPrompt) { std::swap(digit, second_digit); }
                    std::unique_ptr<MdtDigit>& promptHit_Digit = digit;
                    std::unique_ptr<MdtDigit>& twinHit_Digit = second_digit;
                    Identifier promptHit_channelId = digit->identify();

                    // do lookup once
                    const MdtReadoutElement* descriptor = muDetMgr->getMdtReadoutElement(promptHit_channelId);
                    if (!descriptor) {
                        ATH_MSG_WARNING("Detector Element not found for Identifier from the DetManager <"
                                        << m_idHelperSvc->toString(promptHit_channelId) << ">  =>>ignore this hit");
                        continue;
                    }

                    // check if digit is masked
                    if (promptHit_Digit->is_masked()) {
                        digitStatus = MdtStatusMasked;
                    } else if (radius < -999) {
                        ATH_MSG_WARNING("MDT Twin PrepData with very large, negative radius "
                                        << " Id is: " << m_idHelperSvc->toString(promptHit_channelId));
                    }

                    Amg::Vector2D driftRadiusZTwin(radius, zTwin);
                    // make a 2x2 matrix with all values initialized at 0
                    Amg::MatrixX cov(2, 2);
                    (cov)(0, 0) = errRadius * errRadius;
                    (cov)(1, 1) = errZTwin * errZTwin;
                    (cov)(0, 1) = 0;
                    (cov)(1, 0) = 0;

                    // Create new PrepData either w/ or w/o twin hit info depending on m_use1DPrepDataTwin flag
                    if (!m_use1DPrepDataTwin) {
                        MdtTwinPrepData* twin_newPrepData =
                            new MdtTwinPrepData(promptHit_channelId,
                                                // promptHit_channelHash,
                                                driftCircleColl->identifyHash(), driftRadiusZTwin, cov, descriptor, promptHit_Digit->tdc(),
                                                promptHit_Digit->adc(), twinHit_Digit->tdc(), twinHit_Digit->adc(), digitStatus);

                        ATH_MSG_DEBUG(" MADE A 2D TWINPREPDATA " << m_idHelperSvc->toString(promptHit_channelId) << "  zTwin = " << zTwin
                                                                 << " +- " << errZTwin << "  radius = " << radius << " +- " << errRadius);

                        Amg::Vector3D gpos_centertube = twin_newPrepData->globalPosition();
                        const MdtReadoutElement* detEl = muDetMgr->getMdtReadoutElement(promptHit_channelId);
                        Amg::Vector3D locpos_centertube = zTwin * Amg::Vector3D::UnitZ();
                        const Amg::Vector3D gpos_twin = detEl->localToGlobalTransf(promptHit_channelId)*locpos_centertube;

                        ATH_MSG_DEBUG(" global pos center tube  x = " << gpos_centertube.x() << " y = " << gpos_centertube.y()
                                                                      << " z = " << gpos_centertube.z());
                        ATH_MSG_DEBUG(" local pos center tube w/ TWIN INFO  x = "
                                      << locpos_centertube.x() << " y = " << locpos_centertube.y() << " z = " << locpos_centertube.z());
                        ATH_MSG_DEBUG(" global pos w/ TWIN INFO  x = " << gpos_twin.x() << " y = " << gpos_twin.y()
                                                                       << " z = " << gpos_twin.z());

                        twin_newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                        driftCircleColl->push_back(twin_newPrepData);

                    }  // end if(!m_use1DPrepDataTwin){
                    else {
                        Amg::Vector2D driftRadius(radius, 0);
                        // make a 2x2 matrix with all values initialized at 0
                        Amg::MatrixX cov(1, 1);
                        (cov)(0, 0) = errRadius * errRadius;

                        MdtPrepData* twin_newPrepData =
                            new MdtPrepData(promptHit_channelId, driftCircleColl->identifyHash(), driftRadius, cov, descriptor,
                                            promptHit_Digit->tdc(), promptHit_Digit->adc(), digitStatus);

                        ATH_MSG_DEBUG(" MADE A 1D(=original) PREPDATA OUT OF TWINPAIR "
                                      << "   TWIN COORDINATE IS NOT STORED IN PREPDATA " << m_idHelperSvc->toString(promptHit_channelId)
                                      << "  zTwin = " << zTwin << " +- " << errZTwin << "  radius = " << radius << " +- " << errRadius);

                        twin_newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                        driftCircleColl->push_back(twin_newPrepData);

                    }  // end else --  if(!m_use1DPrepDataTwin){

                }  // end else --  if(!second_digit){
            }      // end -- if(multilayer==1)
            else if (multilayer == 2) {
                // if multilayer=2, then treat every hit as a separate hit, no twin hit should be present here as the hardware is not
                // installed

                if (!second_digit) {
                    ATH_MSG_VERBOSE("got digit with id ext / hash " << m_idHelperSvc->toString(channelId) << " / "
                                                                    << driftCircleColl->identifyHash());

                    double radius{0.}, errRadius{0.};
                    MdtDriftCircleStatus digitStatus = MdtStatusDriftTime;

                    // do lookup once
                    const MdtReadoutElement* descriptor = muDetMgr->getMdtReadoutElement(channelId);
                    if (!descriptor) {
                        ATH_MSG_WARNING("Detector Element not found for Identifier from the cabling service <"
                                        << m_idHelperSvc->toString(channelId) << ">  =>>ignore this hit");
                        continue;
                    }

                    if (digit->is_masked()) {
                        digitStatus = MdtStatusMasked;
                    } else {
                        digitStatus = getMdtDriftRadius(*digit, radius, errRadius, descriptor, muDetMgr);
                        if (radius < -999) {
                            ATH_MSG_WARNING("MDT PrepData with very large, negative radius "
                                            << " Id is: " << m_idHelperSvc->toString(channelId));
                        }
                    }

                    Amg::Vector2D driftRadius(radius, 0);
                    Amg::MatrixX cov(1, 1);
                    (cov)(0, 0) = errRadius * errRadius;

                    // Create new PrepData
                    MdtPrepData* newPrepData = new MdtPrepData(channelId, driftCircleColl->identifyHash(), driftRadius, cov, descriptor,
                                                               digit->tdc(), digit->adc(), digitStatus);

                    newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                    driftCircleColl->push_back(newPrepData);

                    ATH_MSG_DEBUG(" MADE ORIGINAL PREPDATA " << m_idHelperSvc->toString(channelId) << "  radius = " << radius << " +- "
                                                             << errRadius);

                }  // end  --  if(!second_digit){
                else {
                    // Do something with second_digit
                    Identifier second_channelId = second_digit->identify();
                    ATH_MSG_VERBOSE("got digit with id ext / hash " << m_idHelperSvc->toString(channelId) << " / "
                                                                    << driftCircleColl->identifyHash());

                    // second_digit
                    ATH_MSG_VERBOSE("got second_digit with id ext / hash " << m_idHelperSvc->toString(second_channelId) << " / "
                                                                           << driftCircleColl->identifyHash());

                    // Calculate radius
                    double radius{0.}, errRadius{0.};
                    MdtDriftCircleStatus digitStatus = MdtStatusDriftTime;

                    // second_digit
                    double second_radius{0.}, second_errRadius{0.};
                    MdtDriftCircleStatus second_digitStatus = MdtStatusDriftTime;

                    // do lookup once
                    const MdtReadoutElement* descriptor = muDetMgr->getMdtReadoutElement(channelId);
                    if (!descriptor) {
                        ATH_MSG_WARNING("Detector Element not found for Identifier from the cabling service <"
                                        << m_idHelperSvc->toString(channelId) << ">  =>>ignore this hit");
                        continue;
                    }

                    if (digit->is_masked()) {
                        digitStatus = MdtStatusMasked;
                    } else {
                        digitStatus = getMdtDriftRadius(*digit, radius, errRadius, descriptor, muDetMgr);
                        if (radius < -999) {
                            ATH_MSG_WARNING("MDT PrepData with very large, negative radius "
                                            << " Id is: " << m_idHelperSvc->toString(channelId));
                        }
                    }

                    const MdtReadoutElement* second_descriptor = muDetMgr->getMdtReadoutElement(second_channelId);
                    if (!second_descriptor) {
                        ATH_MSG_WARNING("Detector Element not found for Identifier from the cabling service <"
                                        << m_idHelperSvc->toString(second_channelId) << ">  =>>ignore this hit");
                        continue;
                    }

                    // second_digit
                    if (second_digit->is_masked()) {
                        second_digitStatus = MdtStatusMasked;
                    } else {
                        second_digitStatus = getMdtDriftRadius(*second_digit, second_radius, second_errRadius, second_descriptor, muDetMgr);
                        if (second_radius < -999) {
                            ATH_MSG_WARNING("MDT PrepData with very large, negative radius "
                                            << " Id is: " << m_idHelperSvc->toString(second_channelId));
                        }
                    }

                    Amg::Vector2D driftRadius(radius, 0);
                    Amg::MatrixX cov(1, 1);
                    (cov)(0, 0) = errRadius * errRadius;

                    // second_digit
                    Amg::Vector2D second_driftRadius(second_radius, 0);
                    auto cov2 = Amg::MatrixX(1, 1);
                    (cov2)(0, 0) = second_errRadius * second_errRadius;

                    // Create new PrepData
                    MdtPrepData* newPrepData = new MdtPrepData(channelId, driftCircleColl->identifyHash(), driftRadius, cov, descriptor,
                                                               digit->tdc(), digit->adc(), digitStatus);

                    newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                    driftCircleColl->push_back(newPrepData);

                    // second_digit
                    // Create new PrepData
                    MdtPrepData* second_newPrepData =
                        new MdtPrepData(second_channelId, driftCircleColl->identifyHash(), second_driftRadius, cov2, second_descriptor,
                                        second_digit->tdc(), second_digit->adc(), second_digitStatus);

                    second_newPrepData->setHashAndIndex(driftCircleColl->identifyHash(), driftCircleColl->size());
                    driftCircleColl->push_back(second_newPrepData);

                    ATH_MSG_DEBUG(" MADE ORIGINAL PREPDATA " << m_idHelperSvc->toString(channelId) << "  radius = " << radius << " +- "
                                                             << errRadius);

                    // second_digit
                    ATH_MSG_DEBUG(" MADE ORIGINAL PREPDATA FOR SECOND DIGIT " << m_idHelperSvc->toString(second_channelId) << "  radius = "
                                                                              << second_radius << " +- " << second_errRadius);
                }  // end  --  else -- if(!second_digit){
            } else {
                ATH_MSG_DEBUG("Something strange in MdtRdoToPrepDataToolCore, MDT multilayer (must be 1 or 2)= " << multilayer);
            }

        }  // end for( iter_map = mdtDigitColl.begin(); iter_map != mdtDigitColl.end(); iter_map++ )
        return StatusCode::SUCCESS;
    }

    MdtDriftCircleStatus MdtRdoToPrepDataToolCore::getMdtDriftRadius(const MdtDigit& digit, double& radius, double& errRadius,
                                                                     const MuonGM::MdtReadoutElement* descriptor,
                                                                     const MuonGM::MuonDetectorManager* muDetMgr) const {
        ATH_MSG_DEBUG("in getMdtDriftRadius()");

        if (m_calibratePrepData) {
            Identifier channelId = digit.identify();

            // here check validity
            // if invalid, reset flags
            if (!descriptor->containsId(channelId)) {
                radius = -1000.;
                ATH_MSG_WARNING("Identifier from the cabling service <"
                                << m_idHelperSvc->toString(channelId) << "> inconsistent with the geometry of detector element <"
                                << m_idHelperSvc->toString(descriptor->identify()) << ">  =>>ignore this hit");
                return MdtStatusUnDefined;
            }

            // use center (cached) to get the tube position instead of tubepos
            const Amg::Vector3D& position = descriptor->center(channelId);
            MdtCalibHit calibHit(channelId, digit.tdc(), digit.adc(), position, descriptor);

            MdtCalibrationSvcInput inputData;
            double signedTrackLength = position.mag();
            inputData.tof = signedTrackLength * inverseSpeedOfLight;

            calibHit.setGlobalPointOfClosestApproach(position);

            bool drift_ok = m_calibrationTool->driftRadiusFromTime(calibHit, inputData, *m_mdtCalibSvcSettings, false);
            if (!drift_ok) {
                if (calibHit.driftTime() < 0.)
                    return MdtStatusBeforeSpectrum;
                else
                    return MdtStatusAfterSpectrum;
            }
            radius = calibHit.driftRadius();
            errRadius = calibHit.sigmaDriftRadius();
            ATH_MSG_VERBOSE("Calibrated drift radius is " << radius << "+/-" << errRadius);
        } else {
            Identifier channelId = digit.identify();
            radius = 0.;
            errRadius = muDetMgr->getMdtReadoutElement(channelId)->innerTubeRadius() / std::sqrt(12);  // 14.6/sqrt(12)
        }
        return MdtStatusDriftTime;
    }

    MdtDriftCircleStatus MdtRdoToPrepDataToolCore::getMdtTwinPosition(const MdtDigit& digit, const MdtDigit& second_digit, double& radius,
                                                                      double& errRadius, double& zTwin, double& errZTwin,
                                                                      bool& secondHitIsPrompt,
                                                                      const MuonGM::MuonDetectorManager* muDetMgr) const {
        ATH_MSG_DEBUG("in getMdtTwinPosition()");

        if (m_calibratePrepData) {
            // start digit
            Identifier channelId = digit.identify();
            const MdtReadoutElement* descriptor = muDetMgr->getMdtReadoutElement(channelId);

            // here check validity
            // if invalid, reset flags
            if (!descriptor) {
                ATH_MSG_WARNING("getMdtTwinPosition(): Detector Element not found for Identifier from the cabling service <"
                                << m_idHelperSvc->toString(channelId) << ">  =>>ignore this hit");
                zTwin = -100000.;
                return MdtStatusUnDefined;
            } else if (!descriptor->containsId(channelId)) {
                zTwin = -100000.;
                ATH_MSG_WARNING("getMdtTwinPosition(): Identifier from the cabling service <"
                                << m_idHelperSvc->toString(channelId) << "> inconsistent with the geometry of detector element <"
                                << m_idHelperSvc->toString(descriptor->identify()) << ">  =>>ignore this hit");
                return MdtStatusUnDefined;
            }

            Amg::Vector3D position = descriptor->tubePos(channelId);
            double measured_perp = position.perp();
            if (descriptor->getStationS() != 0.) {
                measured_perp = std::sqrt(measured_perp * measured_perp - descriptor->getStationS() * descriptor->getStationS());
            }
            double measured_x = measured_perp * std::cos(position.phi());
            double measured_y = measured_perp * std::sin(position.phi());
            const Amg::Vector3D measured_position(measured_x, measured_y, position.z());
            MdtCalibHit calib_hit = MdtCalibHit(channelId, digit.tdc(), digit.adc(), measured_position, descriptor);
            calib_hit.setGlobalPointOfClosestApproach(measured_position);
            double signedTrackLength = measured_position.mag();

            // start second digit
            Identifier second_channelId = second_digit.identify();
            const MdtReadoutElement* second_descriptor = muDetMgr->getMdtReadoutElement(second_channelId);

            // here check validity
            // if invalid, reset flags
            if (!second_descriptor) {
                ATH_MSG_WARNING("getMdtTwinPosition(): Detector Element not found for Identifier from the cabling service <"
                                << m_idHelperSvc->toString(second_channelId) << ">  =>>ignore this hit");
                zTwin = -100000.;
                return MdtStatusUnDefined;
            } else if (!second_descriptor->containsId(second_channelId)) {
                zTwin = -100000.;
                ATH_MSG_WARNING("getMdtTwinPosition(): Identifier from the cabling service <"
                                << m_idHelperSvc->toString(second_channelId) << "> inconsistent with the geometry of detector element <"
                                << m_idHelperSvc->toString(second_descriptor->identify()) << ">  =>>ignore this hit");
                return MdtStatusUnDefined;
            }

            Amg::Vector3D second_position = second_descriptor->tubePos(second_channelId);
            double second_measured_perp = second_position.perp();
            if (second_descriptor->getStationS() != 0.) {
                second_measured_perp = std::sqrt(second_measured_perp * second_measured_perp - 
                                                 second_descriptor->getStationS() * second_descriptor->getStationS());
            }
            double second_measured_x = second_measured_perp * std::cos(second_position.phi());
            double second_measured_y = second_measured_perp * std::sin(second_position.phi());
            const Amg::Vector3D second_measured_position(second_measured_x, second_measured_y, second_position.z());
            MdtCalibHit second_calib_hit =
                MdtCalibHit(second_channelId, second_digit.tdc(), second_digit.adc(), second_measured_position, second_descriptor);
            double second_signedTrackLength = second_measured_position.mag();

            // calculate and calibrate radius for both hits and calculate twin position
            second_calib_hit.setGlobalPointOfClosestApproach(second_measured_position);

            bool second_ok = m_calibrationTool->twinPositionFromTwinHits(calib_hit, second_calib_hit, signedTrackLength,
                                                                         second_signedTrackLength, secondHitIsPrompt);
            if (!second_ok) {
                if (calib_hit.driftTime() < 0. || second_calib_hit.driftTime() < 0.)
                    return MdtStatusBeforeSpectrum;
                else
                    return MdtStatusAfterSpectrum;
            }

            // set radius and error to the prompt tube (tube that was actually hit by the muon)
            radius = calib_hit.driftRadius();
            errRadius = calib_hit.sigmaDriftRadius();
            if (secondHitIsPrompt) {
                radius = second_calib_hit.driftRadius();
                errRadius = second_calib_hit.sigmaDriftRadius();
            }

            zTwin = second_calib_hit.xtwin();
            errZTwin = second_calib_hit.sigmaXtwin();

            ATH_MSG_VERBOSE(" Calibrated drift radius of prompt hit (of twin pair) is " << radius << "+/-" << errRadius);
            ATH_MSG_VERBOSE(" Calibrated twin coordinate = " << zTwin << "+/-" << errZTwin);

        }  // end if(m_calibratePrepData)
        else {
            Identifier channelId = digit.identify();
            radius = 0.;
            const MuonGM::MdtReadoutElement* reElem = muDetMgr->getMdtReadoutElement(channelId); 
            errRadius = reElem->innerTubeRadius() / std::sqrt(12);  // 14.6/sqrt(12)
            zTwin = 0.;
            double tubelength = reElem->tubeLength(channelId);
            errZTwin = tubelength / 2.;
        }

        return MdtStatusDriftTime;
    }

    void MdtRdoToPrepDataToolCore::initDeadChannels(const MuonGM::MdtReadoutElement* mydetEl) {
        PVConstLink cv = mydetEl->getMaterialGeom();  // it is "Multilayer"
        int nGrandchildren = cv->getNChildVols();
        if (nGrandchildren <= 0) return;

        std::vector<int> tubes;
        geoGetIds([&](int id) { tubes.push_back(id); }, &*cv);
        std::sort(tubes.begin(), tubes.end());

        Identifier detElId = mydetEl->identify();

        int name = m_idHelperSvc->mdtIdHelper().stationName(detElId);
        int eta = m_idHelperSvc->mdtIdHelper().stationEta(detElId);
        int phi = m_idHelperSvc->mdtIdHelper().stationPhi(detElId);
        int ml = m_idHelperSvc->mdtIdHelper().multilayer(detElId);
        std::vector<Identifier> deadTubes;

        std::vector<int>::iterator it = tubes.begin();
        for (int layer = 1; layer <= mydetEl->getNLayers(); layer++) {
            for (int tube = 1; tube <= mydetEl->getNtubesperlayer(); tube++) {
                int want_id = layer * maxNTubesPerLayer + tube;
                if (it != tubes.end() && *it == want_id) {
                    ++it;
                } else {
                    it = std::lower_bound(tubes.begin(), tubes.end(), want_id);
                    if (it != tubes.end() && *it == want_id) {
                        ++it;
                    } else {
                        Identifier deadTubeId = m_idHelperSvc->mdtIdHelper().channelID(name, eta, phi, ml, layer, tube);
                        deadTubes.push_back(deadTubeId);
                        ATH_MSG_VERBOSE("adding dead tube (" << tube << "), layer(" << layer << "), phi(" << phi << "), eta(" << eta
                                                             << "), name(" << name << "), multilayerId(" << ml << ") and identifier "
                                                             << deadTubeId << " .");
                    }
                }
            }
        }
        std::sort(deadTubes.begin(), deadTubes.end());
        m_DeadChannels[detElId] = std::move(deadTubes);
    }
    MdtRdoToPrepDataToolCore::ModfiablePrdColl MdtRdoToPrepDataToolCore::setupMdtPrepDataContainer(const EventContext& ctx) const {
        SG::WriteHandle<MdtPrepDataContainer> handle{m_mdtPrepDataContainerKey, ctx};

        // Caching of PRD container
        if (m_prdContainerCacheKey.key().empty()) {
            // without the cache we just record the container
            StatusCode status = handle.record(std::make_unique<MdtPrepDataContainer>(m_idHelperSvc->mdtIdHelper().module_hash_max()));
            if (status.isFailure() || !handle.isValid()) {
                ATH_MSG_FATAL("Could not record container of MDT PrepData Container at " << m_mdtPrepDataContainerKey.key());
                return ModfiablePrdColl{};
            }
            ATH_MSG_DEBUG("Created container " << m_mdtPrepDataContainerKey.key());
        } else {
            // use the cache to get the container
            SG::UpdateHandle<MdtPrepDataCollection_Cache> update{m_prdContainerCacheKey, ctx};
            if (!update.isValid()) {
                ATH_MSG_FATAL("Invalid UpdateHandle " << m_prdContainerCacheKey.key());
                return ModfiablePrdColl{};
            }
            StatusCode status = handle.record(std::make_unique<MdtPrepDataContainer>(update.ptr()));
            if (status.isFailure() || !handle.isValid()) {
                ATH_MSG_FATAL("Could not record container of MDT PrepData Container using cache " << m_prdContainerCacheKey.key() << " - "
                                                                                                  << m_mdtPrepDataContainerKey.key());
                return ModfiablePrdColl{};
            }
            ATH_MSG_DEBUG("Created container using cache for " << m_prdContainerCacheKey.key());
        }
        // Pass the container from the handle
        return ModfiablePrdColl{handle.ptr()};
    }
}  // namespace Muon
