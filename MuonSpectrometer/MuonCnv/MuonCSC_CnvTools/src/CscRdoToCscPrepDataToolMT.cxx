/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CscRdoToCscPrepDataToolMT.h"

#include "EventPrimitives/EventPrimitives.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "MuonIdHelpers/CscIdHelper.h"
#include "MuonRDO/CscRawData.h"
#include "MuonRDO/CscRawDataCollection.h"
#include "MuonRDO/CscRawDataContainer.h"
#include "MuonReadoutGeometry/CscReadoutElement.h"
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/SurfaceBounds.h"

using namespace MuonGM;
using namespace Trk;
using namespace Muon;

CscRdoToCscPrepDataToolMT::CscRdoToCscPrepDataToolMT(const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type, name, parent) {}


StatusCode CscRdoToCscPrepDataToolMT::initialize() {
    ATH_CHECK(m_cscCalibTool.retrieve());
    ATH_CHECK(m_cscRdoDecoderTool.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_cabling.retrieve());
    // check if initializing of DataHandle objects success
    ATH_CHECK(m_rdoContainerKey.initialize());
    ATH_CHECK(m_outputCollectionKey.initialize());
    ATH_CHECK(m_muDetMgrKey.initialize());
    ATH_CHECK(m_prdContainerCacheKey.initialize(!m_prdContainerCacheKey.key().empty()));
    return StatusCode::SUCCESS;
}

void CscRdoToCscPrepDataToolMT::printPrepDataImpl(const Muon::CscStripPrepDataContainer* outputCollection) const {
    ATH_MSG_INFO("***************************************************************");
    ATH_MSG_INFO("****** Listing Csc(Strip)PrepData collections content *********");

    if (outputCollection->empty())
        ATH_MSG_INFO("No Csc(Strip)PrepRawData collections found");
    else {
        ATH_MSG_INFO("Number of Csc(Strip)PrepRawData collections found in this event is " << outputCollection->size());

        int ict = 0;
        int ncoll = 0;
        ATH_MSG_INFO("-------------------------------------------------------------");
        for (IdentifiableContainer<Muon::CscStripPrepDataCollection>::const_iterator icscColl = outputCollection->begin();
             icscColl != outputCollection->end(); ++icscColl) {
            const Muon::CscStripPrepDataCollection* cscColl = *icscColl;

            if (cscColl->empty()) continue;

            ATH_MSG_INFO("PrepData Collection ID " << m_idHelperSvc->cscIdHelper().show_to_string(cscColl->identify())
                                                   << " with size = " << cscColl->size());
            CscStripPrepDataCollection::const_iterator it_cscStripPrepData;
            int icc = 0;
            int iccphi = 0;
            int icceta = 0;
            for (it_cscStripPrepData = cscColl->begin(); it_cscStripPrepData != cscColl->end(); ++it_cscStripPrepData) {
                icc++;
                ict++;
                if (m_idHelperSvc->cscIdHelper().measuresPhi((*it_cscStripPrepData)->identify()))
                    iccphi++;
                else
                    icceta++;

                ATH_MSG_INFO(ict << " in this coll. " << icc
                                 << " prepData id = " << m_idHelperSvc->cscIdHelper().show_to_string((*it_cscStripPrepData)->identify()));
            }
            ncoll++;
            ATH_MSG_INFO("*** Collection " << ncoll << " Summary: " << iccphi << " phi hits / " << icceta << " eta hits ");
            ATH_MSG_INFO("-------------------------------------------------------------");
        }
    }
}

void CscRdoToCscPrepDataToolMT::printInputRdo(const EventContext&) const { }
StatusCode CscRdoToCscPrepDataToolMT::decode(const EventContext&, const std::vector<uint32_t>&) const { 
   ATH_MSG_FATAL("ROB based decoding is not supported....");
   return StatusCode::FAILURE;
}
StatusCode CscRdoToCscPrepDataToolMT::provideEmptyContainer(const EventContext& ctx) const{
    /// Recording the PRD container in StoreGate
    SG::WriteHandle<Muon::CscStripPrepDataContainer> outputHandle(m_outputCollectionKey, ctx);

    // Caching of PRD container
    if (m_prdContainerCacheKey.key().empty()) {
        // without the cache we just record the container
        ATH_CHECK(outputHandle.record(std::make_unique<Muon::CscStripPrepDataContainer>(m_idHelperSvc->cscIdHelper().module_hash_max())));
        ATH_MSG_DEBUG("Created container " << m_outputCollectionKey.key());
    } else {
        // use the cache to get the container
        SG::UpdateHandle<CscStripPrepDataCollection_Cache> update(m_prdContainerCacheKey, ctx);
        if (!update.isValid()) {
            ATH_MSG_FATAL("Invalid UpdateHandle " << m_prdContainerCacheKey.key());
            return StatusCode::FAILURE;
        }
        ATH_CHECK(outputHandle.record(std::make_unique<Muon::CscStripPrepDataContainer>(update.ptr())));
        ATH_MSG_DEBUG("Created container using cache for " << m_prdContainerCacheKey.key());
    }
    return StatusCode::SUCCESS;
}
StatusCode CscRdoToCscPrepDataToolMT::decode(const EventContext& ctx, std::vector<IdentifierHash>& givenIdhs, std::vector<IdentifierHash>& decodedIdhs) const {
    // WARNING : Trigger Part is not finished.
    unsigned int sizeVectorRequested = givenIdhs.size();
    ATH_MSG_DEBUG("decode for " << sizeVectorRequested << " offline collections called");

    // clear output vector of selected data collections containing data
    decodedIdhs.clear();

    /// Recording the PRD container in StoreGate
    SG::WriteHandle<Muon::CscStripPrepDataContainer> outputHandle(m_outputCollectionKey, ctx);

    // Caching of PRD container
    if (m_prdContainerCacheKey.key().empty()) {
        // without the cache we just record the container
        ATH_CHECK(outputHandle.record(std::make_unique<Muon::CscStripPrepDataContainer>(m_idHelperSvc->cscIdHelper().module_hash_max())));
        ATH_MSG_DEBUG("Created container " << m_outputCollectionKey.key());
    } else {
        // use the cache to get the container
        SG::UpdateHandle<CscStripPrepDataCollection_Cache> update(m_prdContainerCacheKey, ctx);
        if (!update.isValid()) {
            ATH_MSG_FATAL("Invalid UpdateHandle " << m_prdContainerCacheKey.key());
            return StatusCode::FAILURE;
        }
        ATH_CHECK(outputHandle.record(std::make_unique<Muon::CscStripPrepDataContainer>(update.ptr())));
        ATH_MSG_DEBUG("Created container using cache for " << m_prdContainerCacheKey.key());
    }
    // Pass the container from the handle
    Muon::CscStripPrepDataContainer* outputCollection = outputHandle.ptr();

    // retrieve the pointer to the RDO container
    // this will just get the pointer from memory if the container is already recorded in SG
    // or
    // will activate the TP converter for reading from pool root the RDO container and recording it in SG

    auto rdoContainerHandle = SG::makeHandle(m_rdoContainerKey, ctx);
    if (!rdoContainerHandle.isValid()) {
        ATH_MSG_WARNING("No CSC RDO container in StoreGate!");
        return StatusCode::SUCCESS;
    }
    const CscRawDataContainer* rdoContainer = rdoContainerHandle.cptr();

    ATH_MSG_DEBUG("Retrieved " << rdoContainer->size() << " CSC RDOs.");
    // here the RDO container is in SG and its pointer rdoContainer is initialised
    // decoding
    if (sizeVectorRequested) {
        // seeded decoding
        for (unsigned int i = 0; i < sizeVectorRequested; ++i) {
            if (decodeImpl(outputCollection, rdoContainer, givenIdhs[i], decodedIdhs).isFailure()) {
                ATH_MSG_ERROR("Unable to decode CSC RDO " << i << "th into CSC PrepRawData");
                return StatusCode::FAILURE;
            }
        }
    } else {
        // unseeded decoding
        if (decodeImpl(outputCollection, rdoContainer, decodedIdhs).isFailure()) {
            ATH_MSG_ERROR("Unable to decode CSC RDO ");
            return StatusCode::FAILURE;
        }
    }

    return StatusCode::SUCCESS;
}

StatusCode CscRdoToCscPrepDataToolMT::decodeImpl(Muon::CscStripPrepDataContainer* outputCollection, const CscRawDataContainer* rdoContainer,
                                                 IdentifierHash givenHashId, std::vector<IdentifierHash>& decodedIdhs) const {
    IdContext cscContext = m_idHelperSvc->cscIdHelper().module_context();
    SG::ReadCondHandle<MuonGM::MuonDetectorManager> muDetMgrHandle{m_muDetMgrKey};
    const MuonGM::MuonDetectorManager* muDetMgr = muDetMgrHandle.cptr();

    // if CSC decoding is switched off stop here
    if (!m_decodeData) {
        ATH_MSG_DEBUG("Stored empty container; Decoding CSC RDO into CSC PrepRawData is switched off");
        return StatusCode::SUCCESS;
    }

    // These collections can be empty for the trigger
    if (!outputCollection || outputCollection->empty()) {
        ATH_MSG_DEBUG("Stored empty collection.");
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("Decoding CSC RDO into CSC PrepRawData");
    /// create the CSC RDO decoder
    //**********************************************
    // retrieve specific collection for the givenID
    uint16_t idColl = 0xffff;
    m_cabling->hash2CollectionId(givenHashId, idColl);
    const CscRawDataCollection* rawCollection = rdoContainer->indexFindPtr(idColl);
    if (nullptr == rawCollection) {
        ATH_MSG_DEBUG("Specific CSC RDO collection retrieving failed for collection hash = " << idColl);
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("Retrieved " << rawCollection->size() << " CSC RDOs.");
    // return if the input raw collection is empty (can happen for seeded decoding in trigger)
    if (rawCollection->empty()) return StatusCode::SUCCESS;

    //************************************************
    IdentifierHash cscHashId;

    unsigned int samplingTime = rawCollection->rate();
    unsigned int numSamples = rawCollection->numSamples();
    bool samplingPhase = rawCollection->samplingPhase();
    std::vector<float> charges;
    charges.reserve(4);
    std::vector<uint16_t> samples;
    samples.reserve(4);

    if (int(samplingTime) != int(m_cscCalibTool->getSamplingTime())) {
        ATH_MSG_WARNING(" CSC sampling time from Collection is NOT consistant to calibTool parameter!!!!!!! ");
    }

    // For each Rdo, loop over RawData, converter RawData to PrepRawData
    // Retrieve/create PrepRawData collection, and insert PrepRawData into collection
    CscRawDataCollection::const_iterator itD = rawCollection->begin();
    CscRawDataCollection::const_iterator itD_e = rawCollection->end();

    // Each RDO Collection goes into a PRD Collection, but we need to know what hash the PRD container is
    // and this is determined by the stationID, no the RDO hash ID
    ATH_MSG_DEBUG("CSC RDO ID " << rawCollection->identify() << " with hashID " << rawCollection->identifyHash());
    // Just use the first iterator entry as stationID does not change between data inside a single container
    Identifier stationId = m_cscRdoDecoderTool->stationIdentifier((const CscRawData*)(*itD), &m_idHelperSvc->cscIdHelper());
    if (m_idHelperSvc->cscIdHelper().get_hash(stationId, cscHashId, &cscContext)) {
        ATH_MSG_WARNING("Unable to get CSC digiti collection hash id "
                        << "context begin_index = " << cscContext.begin_index() << " context end_index  = " << cscContext.end_index()
                        << " the identifier is ");
        stationId.show();
    }
    ATH_MSG_DEBUG("Create CSC PRD Collection with hash " << cscHashId << " (givenHashId is " << givenHashId << ")");
    std::unique_ptr<CscStripPrepDataCollection> collection = nullptr;
    CscStripPrepDataContainer::IDC_WriteHandle lock = outputCollection->getWriteHandle(cscHashId);
    // Note that if the hash check above works, we should never reach this step where the lock is present
    if (lock.alreadyPresent()) {
        ATH_MSG_DEBUG("CSC PRD collection already exist with collection hash = " << cscHashId << " collection filling is skipped!");
        decodedIdhs.push_back(givenHashId);
        return StatusCode::SUCCESS;
    } else {
        ATH_MSG_DEBUG("CSC PRD collection does not exist - creating a new one with hash = " << cscHashId);
        collection = std::make_unique<CscStripPrepDataCollection>(cscHashId);
        collection->setIdentifier(stationId);
    }

    for (; itD != itD_e; ++itD) {
        const CscRawData* data = (*itD);
        uint16_t width = data->width();
        uint16_t totalSamples = (data->samples()).size();
        uint32_t hashOffset = data->hashId();

        ATH_MSG_DEBUG(" Size of online cluster in this RawData: "
                      << " Width = " << width << " Samples = " << totalSamples << " stationId : " << stationId
                      << "  hashOffset : " << hashOffset);

        for (unsigned int j = 0; j < width; ++j) {
            const Identifier channelId = m_cscRdoDecoderTool->channelIdentifier(data, &m_idHelperSvc->cscIdHelper(), j);
            ATH_MSG_DEBUG("        LOOP over width  " << j << " " << channelId);

            const CscReadoutElement* descriptor = muDetMgr->getCscReadoutElement(channelId);
            // calculate local positions on the strip planes
            if (!descriptor) {
                ATH_MSG_WARNING("Invalid descriptor for " << m_idHelperSvc->cscIdHelper().show_to_string(channelId)
                                                          << " Skipping channel ");
                continue;
            } else if (!descriptor->containsId(channelId)) {
                ATH_MSG_WARNING("Identifier from the cabling service <"
                                << m_idHelperSvc->cscIdHelper().show_to_string(channelId)
                                << "> inconsistent with the geometry of detector element <"
                                << m_idHelperSvc->cscIdHelper().show_to_string(descriptor->identify()) << ">  =>>ignore this hit");
                continue;
            }

            float timeOfFirstSample = 0.0;
            bool extractSamples = data->samples(j, numSamples, samples);
            if (!extractSamples) {
                ATH_MSG_WARNING("Unable to extract samples for strip " << j << " Online Cluster width = " << width
                                                                       << " for number of Samples = " << numSamples << " continuing ...");
                continue;
            }

            IdentifierHash stripHash;
            if (m_idHelperSvc->cscIdHelper().get_channel_hash(channelId, stripHash)) {
                ATH_MSG_WARNING("Unable to get CSC strip hash id");
                channelId.show();
            }

            bool adctocharge = m_cscCalibTool->adcToCharge(samples, stripHash, charges);
            if (!adctocharge) {
                ATH_MSG_WARNING(" CSC conversion ADC to Charge failed "
                                << "CSC PrepData not build ... skipping ");
                continue;
            }
            if (samples.size() >= 4)
                ATH_MSG_DEBUG("ADC: " << m_idHelperSvc->cscIdHelper().show_to_string(channelId) << " " << samples[0] << " " << samples[1]
                                      << " " << samples[2] << " " << samples[3] << " Charges: "
                                      << " " << charges[0] << " " << charges[1] << " " << charges[2] << " " << charges[3]);

            int measuresPhi = m_idHelperSvc->cscIdHelper().measuresPhi(channelId);

            Amg::Vector2D localWirePos1(descriptor->xCoordinateInTrackingFrame(channelId), 0.);

            int chamberLayer = m_idHelperSvc->cscIdHelper().chamberLayer(channelId);
            float stripWidth = descriptor->cathodeReadoutPitch(chamberLayer, measuresPhi);
            double errPos = stripWidth / sqrt(12.0);

            AmgSymMatrix(2) covariance;
            covariance.setIdentity();
            covariance *= errPos * errPos;
            Amg::MatrixX errClusterPos = Amg::MatrixX(covariance);

            /** new CscStripPrepRawData */
            CscStripPrepData* newPrepData = new CscStripPrepData(channelId, cscHashId, localWirePos1, std::move(errClusterPos), descriptor,
                                                                 charges, timeOfFirstSample, samplingTime);

            if (samplingPhase) newPrepData->set_samplingPhase();
            newPrepData->setHashAndIndex(collection->identifyHash(), collection->size());
            collection->push_back(newPrepData);
        }
    }
    // Record the container
    StatusCode status_lock = lock.addOrDelete(std::move(collection));
    if (status_lock.isFailure()) {
        ATH_MSG_ERROR("Could not insert CscStripPrepdataCollection into CscStripPrepdataContainer...");
        return StatusCode::FAILURE;
    } else {
        decodedIdhs.push_back(cscHashId);
    }
    return StatusCode::SUCCESS;
}

//************** Process for all in case of Offline
StatusCode CscRdoToCscPrepDataToolMT::decodeImpl(Muon::CscStripPrepDataContainer* outputCollection, const CscRawDataContainer* rdoContainer,
                                                 std::vector<IdentifierHash>& decodedIdhs) const {
    typedef CscRawDataContainer::const_iterator collection_iterator;

    IdContext cscContext = m_idHelperSvc->cscIdHelper().module_context();
    SG::ReadCondHandle<MuonGM::MuonDetectorManager> muDetMgrHandle{m_muDetMgrKey};
    const MuonGM::MuonDetectorManager* muDetMgr = muDetMgrHandle.cptr();

    // if CSC decoding is switched off stop here
    if (!m_decodeData) {
        ATH_MSG_DEBUG("Stored empty container. "
                      << "Decoding CSC RDO into CSC PrepRawData is switched off");
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG("Decoding CSC RDO into CSC PrepRawData");

    collection_iterator rdoColl = rdoContainer->begin();
    collection_iterator lastRdoColl = rdoContainer->end();
    std::vector<float> charges;
    charges.reserve(4);
    std::vector<uint16_t> samples;
    samples.reserve(4);

    IdentifierHash cscHashId;
    for (; rdoColl != lastRdoColl; ++rdoColl) {
        if (!(*rdoColl)->empty()) {
            ATH_MSG_DEBUG(" Number of RawData in this rdo " << (*rdoColl)->size());

            const CscRawDataCollection* cscCollection = *rdoColl;
            unsigned int samplingTime = cscCollection->rate();
            unsigned int numSamples = cscCollection->numSamples();
            bool samplingPhase = cscCollection->samplingPhase();

            if (int(samplingTime) != int(m_cscCalibTool->getSamplingTime())) {
                ATH_MSG_WARNING(" CSC sampling time from Collection is NOT consistant to calibTool parameter!!!!!!! ");
            }
            // For each Rdo, loop over RawData, converter RawData to PrepRawData
            // Retrieve/create PrepRawData collection, and insert PrepRawData into collection
            CscRawDataCollection::const_iterator itD = cscCollection->begin();
            CscRawDataCollection::const_iterator itD_e = cscCollection->end();

            // Each RDO Collection goes into a PRD Collection, but we need to know what hash the PRD container is
            // and this is determined by the stationID, no the RDO hash ID
            ATH_MSG_DEBUG("CSC RDO ID " << (*rdoColl)->identify() << " with hashID " << (*rdoColl)->identifyHash());
            // Just use the first iterator entry as stationID does not change between data inside a single container
            Identifier stationId = m_cscRdoDecoderTool->stationIdentifier((const CscRawData*)(*itD), &m_idHelperSvc->cscIdHelper());
            if (m_idHelperSvc->cscIdHelper().get_hash(stationId, cscHashId, &cscContext)) {
                ATH_MSG_WARNING("Unable to get CSC digiti collection hash id "
                                << "context begin_index = " << cscContext.begin_index()
                                << " context end_index  = " << cscContext.end_index() << " the identifier is ");
                stationId.show();
            }

            ATH_MSG_DEBUG("Create CSC PRD Collection with hash " << cscHashId);
            std::unique_ptr<CscStripPrepDataCollection> collection = nullptr;
            CscStripPrepDataContainer::IDC_WriteHandle lock = outputCollection->getWriteHandle(cscHashId);
            if (lock.alreadyPresent()) {
                ATH_MSG_DEBUG("CSC PRD collection already exist with collection hash = " << cscHashId << " collection filling is skipped!");
                continue;
            } else {
                ATH_MSG_DEBUG("CSC PRD collection does not exist - creating a new one with hash = " << cscHashId);
                collection = std::make_unique<CscStripPrepDataCollection>(cscHashId);
                collection->setIdentifier(stationId);
            }

            // This loops over the RDO data, decodes and puts into the PRD collection
            for (; itD != itD_e; ++itD) {
                const CscRawData* data = (*itD);
                uint16_t width = data->width();
                uint16_t totalSamples = (data->samples()).size();
                uint32_t hashOffset = data->hashId();

                ATH_MSG_DEBUG("DecodeAll*Size of online cluster in this RawData: "
                              << " Width = " << width << " Samples = " << totalSamples << " stationId : " << stationId
                              << "  hashOffset : " << hashOffset);

                for (unsigned int j = 0; j < width; ++j) {
                    const Identifier channelId = m_cscRdoDecoderTool->channelIdentifier(data, &m_idHelperSvc->cscIdHelper(), j);
                    ATH_MSG_DEBUG("DecodeAll**LOOP over width  " << j << " " << channelId);

                    const CscReadoutElement* descriptor = muDetMgr->getCscReadoutElement(channelId);
                    // calculate local positions on the strip planes
                    if (!descriptor) {
                        ATH_MSG_WARNING("Invalid descriptor for " << m_idHelperSvc->cscIdHelper().show_to_string(channelId)
                                                                  << " Skipping channel ");
                        continue;
                    } else if (!descriptor->containsId(channelId)) {
                        ATH_MSG_WARNING("Identifier from the cabling service <"
                                        << m_idHelperSvc->cscIdHelper().show_to_string(channelId)
                                        << "> inconsistent with the geometry of detector element <"
                                        << m_idHelperSvc->cscIdHelper().show_to_string(descriptor->identify()) << ">  =>>ignore this hit");
                        continue;
                    }

                    float timeOfFirstSample = 0.0;
                    bool extractSamples = data->samples(j, numSamples, samples);
                    if (!extractSamples) {
                        ATH_MSG_WARNING("Unable to extract samples for strip " << j << " Online Cluster width = " << width
                                                                               << " for number of Samples = " << numSamples
                                                                               << " continuing ...");
                        continue;
                    }

                    IdentifierHash stripHash;
                    if (m_idHelperSvc->cscIdHelper().get_channel_hash(channelId, stripHash)) {
                        ATH_MSG_WARNING("Unable to get CSC strip hash id");
                        channelId.show();
                    }

                    Identifier channelIdFromHash;
                    m_idHelperSvc->cscIdHelper().get_id(stripHash, channelIdFromHash, &cscContext);

                    bool adctocharge = m_cscCalibTool->adcToCharge(samples, stripHash, charges);
                    if (!adctocharge) {
                        ATH_MSG_WARNING(" CSC conversion ADC to Charge failed "
                                        << "CSC PrepData not build ... skipping ");
                        continue;
                    }
                    if (samples.size() >= 4)
                        ATH_MSG_DEBUG("DecodeAll*** ADC: "
                                      << m_idHelperSvc->cscIdHelper().show_to_string(channelId) << " " << (int)stripHash << " "
                                      << m_idHelperSvc->cscIdHelper().show_to_string(channelIdFromHash) << " " << samples[0] << " "
                                      << samples[1] << " " << samples[2] << " " << samples[3] << " Charges: "
                                      << " " << charges[0] << " " << charges[1] << " " << charges[2] << " " << charges[3]);
                    if (m_idHelperSvc->cscIdHelper().get_hash(stationId, cscHashId, &cscContext)) {
                        ATH_MSG_WARNING("Unable to get CSC hash id from CSC RDO collection "
                                        << "context begin_index = " << cscContext.begin_index()
                                        << " context end_index  = " << cscContext.end_index() << " the identifier is ");
                        stationId.show();
                    }

                    // Check if this strip is already decoded.. Then we don't have to decode it again
                    bool IsThisStripDecoded = 0;
                    for (CscStripPrepDataCollection::const_iterator idig = collection->begin(); idig != collection->end(); ++idig) {
                        const CscStripPrepData& dig = **idig;
                        Identifier did = dig.identify();
                        if (did == channelId) {
                            IsThisStripDecoded = 1;
                            break;
                        }
                    }
                    if (IsThisStripDecoded) continue;

                    int measuresPhi = m_idHelperSvc->cscIdHelper().measuresPhi(channelId);

                    Amg::Vector2D localWirePos1(descriptor->xCoordinateInTrackingFrame(channelId), 0.);

                    int chamberLayer = m_idHelperSvc->cscIdHelper().chamberLayer(channelId);
                    float stripWidth = descriptor->cathodeReadoutPitch(chamberLayer, measuresPhi);
                    double errPos = stripWidth / sqrt(12.0);

                    AmgSymMatrix(2) covariance;
                    covariance.setIdentity();
                    covariance *= errPos * errPos;
                    auto errClusterPos = Amg::MatrixX(covariance);

                    /** new CscPrepRawData */
                    CscStripPrepData* newPrepData = new CscStripPrepData(channelId, cscHashId, localWirePos1, std::move(errClusterPos),
                                                                         descriptor, charges, timeOfFirstSample, samplingTime);

                    if (samplingPhase) newPrepData->set_samplingPhase();
                    newPrepData->setHashAndIndex(collection->identifyHash(), collection->size());
                    collection->push_back(newPrepData);
                }
            }
            // Record the container after looping through all the RDO data in this RDO collection
            StatusCode status_lock = lock.addOrDelete(std::move(collection));
            if (status_lock.isFailure()) {
                ATH_MSG_ERROR("Could not insert CscStripPrepdataCollection into CscStripPrepdataContainer...");
                return StatusCode::FAILURE;
            }
            decodedIdhs.push_back(cscHashId);
        }
    }
    return StatusCode::SUCCESS;
}

void CscRdoToCscPrepDataToolMT::printPrepData(const EventContext& ctx) const {
    SG::ReadHandleKey<Muon::CscStripPrepDataContainer> k(m_prdContainerCacheKey.key());
    k.initialize().ignore();
    printPrepDataImpl(SG::makeHandle(k, ctx).get());
}
