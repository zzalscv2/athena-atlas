/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcDigitizationTool.h"

#include "AthenaKernel/RNGWrapper.h"
#include "AthenaKernel/errorcheck.h"

// Other includes
#include "GeneratorObjects/HepMcParticleLink.h"
#include "Identifier/Identifier.h"

// TGC includes
#include "MuonIdHelpers/TgcIdHelper.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "MuonSimEvent/TgcHitIdHelper.h"
#include "PileUpTools/IPileUpTool.h"  // for SubEventIterator
#include "TgcDigitMaker.h"
#include "xAODEventInfo/EventInfo.h"

// run number from geometry DB
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

TgcDigitizationTool::TgcDigitizationTool(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent)
    : PileUpToolBase(type, name, parent) {}

//--------------------------------------------
StatusCode TgcDigitizationTool::initialize() {
    // retrieve MuonDetctorManager from DetectorStore
    ATH_CHECK(detStore()->retrieve(m_mdManager));
    ATH_MSG_DEBUG("Retrieved MuonDetectorManager from DetectorStore.");

    if (m_onlyUseContainerName) {
        ATH_CHECK(m_mergeSvc.retrieve());
    }

    // initialize the TgcIdHelper
    m_idHelper = m_mdManager->tgcIdHelper();
    if (!m_idHelper) {
        ATH_MSG_WARNING("tgcIdHelper could not be retrieved.");
        return StatusCode::FAILURE;
    }

    // TgcHitIdHelper
    m_hitIdHelper = TgcHitIdHelper::GetHelper();

    // check the input object name
    if (m_hitsContainerKey.key().empty()) {
        ATH_MSG_FATAL("Property InputObjectName not set !");
        return StatusCode::FAILURE;
    }
    if (m_onlyUseContainerName)
        m_inputHitCollectionName = m_hitsContainerKey.key();
    ATH_MSG_DEBUG("Input objects in container : '" << m_inputHitCollectionName
                                                   << "'");

    // Initialize Read(Cond)HandleKey
    ATH_CHECK(m_hitsContainerKey.initialize(true));
    ATH_CHECK(m_readCondKey_ASDpos.initialize(!m_readCondKey_ASDpos.empty()));
    ATH_CHECK(
        m_readCondKey_TimeOffset.initialize(!m_readCondKey_TimeOffset.empty()));
    ATH_CHECK(
        m_readCondKey_Crosstalk.initialize(!m_readCondKey_Crosstalk.empty()));

    // initialize the output WriteHandleKeys
    ATH_CHECK(m_outputDigitCollectionKey.initialize());
    ATH_CHECK(m_outputSDO_CollectionKey.initialize());
    ATH_MSG_DEBUG("Output Digits: '" << m_outputDigitCollectionKey.key()
                                     << "'");

    ATH_MSG_DEBUG("IncludePileUpTruth: " << m_includePileUpTruth);
    ATH_MSG_DEBUG("VetoPileUpTruthLinks: " << m_vetoPileUpTruthLinks);

    const IGeoModelSvc* geoModel = nullptr;
    CHECK(service("GeoModelSvc", geoModel));
    std::string atlasVersion = geoModel->atlasVersion();

    IRDBAccessSvc* rdbAccess = nullptr;
    CHECK(service("RDBAccessSvc", rdbAccess));

    IRDBRecordset_ptr atlasCommonRec =
        rdbAccess->getRecordsetPtr("AtlasCommon", atlasVersion, "ATLAS");
    unsigned int runperiod = 1;
    if (atlasCommonRec->size() == 0)
        runperiod = 1;
    else {
        std::string configVal = (*atlasCommonRec)[0]->getString("CONFIG");
        if (configVal == "RUN1")
            runperiod = 1;
        else if (configVal == "RUN2")
            runperiod = 2;
        else if (configVal == "RUN3")
            runperiod =
                3;  // currently runperiod 3 means no masking => ok for upgrade
        else if (configVal == "RUN4")
            runperiod =
                3;  // currently runperiod 3 means no masking => ok for upgrade
        else {
            ATH_MSG_FATAL(
                "Unexpected value for geometry config read from the database: "
                << configVal);
            return StatusCode::FAILURE;
        }
    }

    // initialize class to execute digitization
    m_digitizer = new TgcDigitMaker(m_hitIdHelper, m_mdManager, runperiod,
                                    m_doFourBunchDigitization);
    m_digitizer->setLevel(static_cast<MSG::Level>(msgLevel()));
    ATH_CHECK(m_rndmSvc.retrieve());

    ATH_CHECK(m_digitizer->initialize());

    return StatusCode::SUCCESS;
}

//--------------------------------------------
StatusCode TgcDigitizationTool::prepareEvent(const EventContext& /*ctx*/,
                                             unsigned int) {
    m_TGCHitCollList.clear();
    m_thpcTGC = new TimedHitCollection<TGCSimHit>();

    return StatusCode::SUCCESS;
}

//--------------------------------------------
StatusCode TgcDigitizationTool::processBunchXing(int bunchXing,
                                                 SubEventIterator bSubEvents,
                                                 SubEventIterator eSubEvents) {
    ATH_MSG_DEBUG("TgcDigitizationTool::processBunchXing() " << bunchXing);

    typedef PileUpMergeSvc::TimedList<TGCSimHitCollection>::type
        TimedHitCollList;
    TimedHitCollList hitCollList;

    if (!(m_mergeSvc
              ->retrieveSubSetEvtData(m_inputHitCollectionName, hitCollList,
                                      bunchXing, bSubEvents, eSubEvents)
              .isSuccess()) &&
        hitCollList.empty()) {
        ATH_MSG_ERROR("Could not fill TimedHitCollList");
        return StatusCode::FAILURE;
    } else {
        ATH_MSG_VERBOSE(hitCollList.size()
                        << " TGCSimHitCollection with key "
                        << m_inputHitCollectionName << " found");
    }

    TimedHitCollList::iterator iColl(hitCollList.begin());
    TimedHitCollList::iterator endColl(hitCollList.end());

    // Iterating over the list of collections
    for (; iColl != endColl; ++iColl) {

        TGCSimHitCollection* hitCollPtr =
            new TGCSimHitCollection(*iColl->second);
        PileUpTimeEventIndex timeIndex(iColl->first);

        ATH_MSG_DEBUG("TGCSimHitCollection found with " << hitCollPtr->size()
                                                        << " hits");
        ATH_MSG_VERBOSE("time index info. time: "
                        << timeIndex.time() << " index: " << timeIndex.index()
                        << " type: " << timeIndex.type());

        m_thpcTGC->insert(timeIndex, hitCollPtr);
        m_TGCHitCollList.push_back(hitCollPtr);
    }

    return StatusCode::SUCCESS;
}

//--------------------------------------------
StatusCode TgcDigitizationTool::mergeEvent(const EventContext& ctx) {
    ATH_MSG_DEBUG("TgcDigitizationTool::mergeEvent()");

    ATH_CHECK(digitizeCore(ctx));
    // reset the pointer (delete null pointer should be safe)
    delete m_thpcTGC;
    m_thpcTGC = nullptr;

    std::list<TGCSimHitCollection*>::iterator TGCHitColl =
        m_TGCHitCollList.begin();
    std::list<TGCSimHitCollection*>::iterator TGCHitCollEnd =
        m_TGCHitCollList.end();
    while (TGCHitColl != TGCHitCollEnd) {
        delete (*TGCHitColl);
        ++TGCHitColl;
    }
    m_TGCHitCollList.clear();

    return StatusCode::SUCCESS;
}

//_____________________________________________________________________________
StatusCode TgcDigitizationTool::processAllSubEvents(const EventContext& ctx) {
    ATH_MSG_DEBUG("TgcDigitizationTool::processAllSubEvents()");
    // merging of the hit collection in getNextEvent method
    if (!m_thpcTGC) {
        ATH_CHECK(getNextEvent(ctx));
    }
    ATH_CHECK(digitizeCore(ctx));
    // reset the pointer (delete null pointer should be safe)
    delete m_thpcTGC;
    m_thpcTGC = nullptr;
    return StatusCode::SUCCESS;
}

//_____________________________________________________________________________
StatusCode TgcDigitizationTool::finalize() {
    ATH_MSG_DEBUG("finalize.");

    delete m_digitizer;
    m_digitizer = nullptr;

    return StatusCode::SUCCESS;
}

//_____________________________________________________________________________
// Get next event and extract collection of hit collections:
StatusCode TgcDigitizationTool::getNextEvent(const EventContext& ctx) {
    // initialize pointer
    m_thpcTGC = nullptr;

    //  get the container(s)
    using TimedHitCollList =
        PileUpMergeSvc::TimedList<TGCSimHitCollection>::type;

    // In case of single hits container just load the collection using read
    // handles
    if (!m_onlyUseContainerName) {
        SG::ReadHandle<TGCSimHitCollection> hitCollection(m_hitsContainerKey,
                                                          ctx);
        if (!hitCollection.isValid()) {
            ATH_MSG_ERROR("Could not get TGCSimHitCollection container "
                          << hitCollection.name() << " from store "
                          << hitCollection.store());
            return StatusCode::FAILURE;
        }

        // create a new hits collection
        m_thpcTGC = new TimedHitCollection<TGCSimHit>{1};
        m_thpcTGC->insert(0, hitCollection.cptr());
        ATH_MSG_DEBUG("TGCSimHitCollection found with " << hitCollection->size()
                                                        << " hits");

        return StatusCode::SUCCESS;
    }

    // this is a list<pair<time_t, DataLink<TGCSimHitCollection> > >
    TimedHitCollList hitCollList;

    if (!(m_mergeSvc->retrieveSubEvtsData(m_inputHitCollectionName, hitCollList)
              .isSuccess())) {
        ATH_MSG_FATAL("Could not fill TimedHitCollList");
        return StatusCode::FAILURE;
    }
    if (hitCollList.empty()) {
        ATH_MSG_FATAL("TimedHitCollList has size 0");
        return StatusCode::FAILURE;
    } else {
        ATH_MSG_DEBUG(hitCollList.size()
                      << " TGCSimHitCollections with key "
                      << m_inputHitCollectionName << " found");
    }

    // create a new hits collection
    m_thpcTGC = new TimedHitCollection<TGCSimHit>();

    // now merge all collections into one
    TimedHitCollList::iterator iColl = hitCollList.begin();
    TimedHitCollList::iterator endColl = hitCollList.end();
    while (iColl != endColl) {
        const TGCSimHitCollection* p_collection = iColl->second;
        m_thpcTGC->insert(iColl->first, p_collection);
        ATH_MSG_DEBUG("TGCSimHitCollection found with "
                      << p_collection->size()
                      << " hits");  // loop on the hit collections
        ++iColl;
    }
    return StatusCode::SUCCESS;
}

StatusCode TgcDigitizationTool::digitizeCore(const EventContext& ctx) {

    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this);
    rngWrapper->setSeed(name(), ctx);
    CLHEP::HepRandomEngine* rndmEngine = rngWrapper->getEngine(ctx);

    // create and record the Digit container in StoreGate
    SG::WriteHandle<TgcDigitContainer> digitContainer(
        m_outputDigitCollectionKey, ctx);
    ATH_CHECK(digitContainer.record(
        std::make_unique<TgcDigitContainer>(m_idHelper->module_hash_max())));
    ATH_MSG_DEBUG("TgcDigitContainer recorded in StoreGate.");

    // Create and record the SDO container in StoreGate
    SG::WriteHandle<MuonSimDataCollection> sdoContainer(
        m_outputSDO_CollectionKey, ctx);
    ATH_CHECK(sdoContainer.record(std::make_unique<MuonSimDataCollection>()));
    ATH_MSG_DEBUG("TgcSDOCollection recorded in StoreGate.");

    // get the iterator pairs for this DetEl
    // iterate over hits and fill id-keyed drift time map
    IdContext tgcContext = m_idHelper->module_context();

    // Read needed conditions data
    const TgcDigitASDposData* ASDpos{};
    if (!m_readCondKey_ASDpos.empty()) {
        SG::ReadCondHandle<TgcDigitASDposData> readHandle_ASDpos{
            m_readCondKey_ASDpos, ctx};
        ASDpos = readHandle_ASDpos.cptr();
    } else {
        ATH_MSG_ERROR(
            "ASD Position parameters /TGC/DIGIT/ASDPOS must be available for "
            "TGC_Digitization. Check the configuration!");
    }
    const TgcDigitTimeOffsetData* TOffset{};
    if (!m_readCondKey_TimeOffset.empty()) {
        SG::ReadCondHandle<TgcDigitTimeOffsetData> readHandle_TimeOffset{
            m_readCondKey_TimeOffset, ctx};
        TOffset = readHandle_TimeOffset.cptr();
    } else {
        ATH_MSG_ERROR(
            "Timing Offset parameters /TGC/DIGIT/TOFFSET must be available for "
            "TGC_Digitization. Check the configuration!");
    }
    const TgcDigitCrosstalkData* Crosstalk{};
    if (!m_readCondKey_Crosstalk.empty()) {
        SG::ReadCondHandle<TgcDigitCrosstalkData> readHandle_Crosstalk{
            m_readCondKey_Crosstalk, ctx};
        Crosstalk = readHandle_Crosstalk.cptr();
    } else {
        ATH_MSG_WARNING(
            "/TGC/DIGIT/XTALK is not provided. Probabilities of TGC channel "
            "crosstalk will be zero.");
    }

    std::vector<std::unique_ptr<TgcDigitCollection> > collections;

    TimedHitCollection<TGCSimHit>::const_iterator i, e;
    while (m_thpcTGC->nextDetectorElement(i, e)) {
        ATH_MSG_DEBUG("TgcDigitizationTool::digitizeCore next element");

        // Loop over the hits:
        while (i != e) {
            TimedHitPtr<TGCSimHit> phit = *i++;
            const TGCSimHit& hit = *phit;
            double globalHitTime = hitTime(phit);
            double tof = phit->globalTime();
            TgcDigitCollection* digiHits = m_digitizer->executeDigi(
                &hit, globalHitTime, ASDpos, TOffset, Crosstalk, rndmEngine);

            if (!digiHits)
                continue;

            TgcDigitCollection::const_iterator it_digiHits;
            for (it_digiHits = digiHits->begin();
                 it_digiHits != digiHits->end(); ++it_digiHits) {

                /**
                   NOTE:
                   -----
                   Since not every hit might end up resulting in a
                   digit, this construction might take place after the hit loop
                   in a loop of its own!
                */

                Identifier newDigiId = (*it_digiHits)->identify();
                uint16_t newBcTag = (*it_digiHits)->bcTag();
                Identifier elemId = m_idHelper->elementID(newDigiId);

                TgcDigitCollection* digitCollection = nullptr;

                IdentifierHash coll_hash;
                if (m_idHelper->get_hash(elemId, coll_hash, &tgcContext)) {
                    ATH_MSG_WARNING(
                        "Unable to get TGC hash id from TGC Digit collection "
                        << "context begin_index = " << tgcContext.begin_index()
                        << " context end_index  = " << tgcContext.end_index()
                        << " the identifier is ");
                    elemId.show();
                }

                // make new TgcDigit and record it in StoreGate
                auto newDigit = std::make_unique<TgcDigit>(newDigiId, newBcTag);

                // record the digit container in StoreGate
                bool duplicate = false;
                if (coll_hash >= collections.size()) {
                    collections.resize(coll_hash + 1);
                }
                digitCollection = collections[coll_hash].get();
                if (nullptr == digitCollection) {
                    collections[coll_hash] =
                        std::make_unique<TgcDigitCollection>(elemId, coll_hash);
                    digitCollection = collections[coll_hash].get();
                    ATH_MSG_DEBUG("Digit Id(1st) = "
                                  << m_idHelper->show_to_string(newDigiId)
                                  << " BC tag = " << newBcTag
                                  << " Coll. key = " << coll_hash);
                    digitCollection->push_back(std::move(newDigit));
                } else {
                    // to avoid to store digits with identical id
                    TgcDigitCollection::const_iterator it_tgcDigit;
                    for (it_tgcDigit = digitCollection->begin();
                         it_tgcDigit != digitCollection->end(); ++it_tgcDigit) {
                        if (newDigiId == (*it_tgcDigit)->identify() &&
                            newBcTag == (*it_tgcDigit)->bcTag()) {
                            duplicate = true;
                            IdContext context = m_idHelper->channel_context();
                            ATH_MSG_DEBUG("Duplicate digit(removed) = "
                                          << m_idHelper->show_to_string(
                                                 newDigiId, &context)
                                          << " BC tag = " << newBcTag);
                            newDigit.reset();
                            break;
                        }
                    }
                    if (!duplicate) {
                        digitCollection->push_back(std::move(newDigit));
                        ATH_MSG_DEBUG("Digit Id= "
                                      << m_idHelper->show_to_string(newDigiId)
                                      << " BC tag = " << newBcTag);
                    }
                }

                if (!duplicate) {
                    static const double invalid_pos = -99999.;
                    Amg::Vector3D gpos(invalid_pos, invalid_pos, invalid_pos);
                    const MuonGM::TgcReadoutElement* tgcChamber =
                        m_mdManager->getTgcReadoutElement(newDigiId);
                    if (tgcChamber) {
                        gpos = tgcChamber->localToGlobalCoords(
                            hit.localPosition(), newDigiId);
                    }

                    // fill the SDO collection in StoreGate if not pile-up
                    if (!m_includePileUpTruth &&
                        HepMC::ignoreTruthLink(phit->particleLink(),
                                               m_vetoPileUpTruthLinks)) {
                        continue;
                    }

                    // link to MC info
                    // const HepMcParticleLink & particleLink =
                    // hit.particleLink();
                    // create here deposit for MuonSimData, link and tof
                    const EBC_EVCOLL evColl = EBC_MAINEVCOLL;
                    const HepMcParticleLink::PositionFlag idxFlag =
                        (phit.eventId() == 0) ? HepMcParticleLink::IS_POSITION
                                              : HepMcParticleLink::IS_INDEX;
                    std::vector<MuonSimData::Deposit> deposits;
                    deposits.emplace_back(
                        HepMcParticleLink(phit->trackNumber(), phit.eventId(),
                                          evColl, idxFlag),
                        MuonMCData(tof, 0));
                    MuonSimData simData(deposits, 0);
                    simData.setPosition(gpos);
                    simData.setTime(hitTime(phit));
                    sdoContainer->insert(std::make_pair(newDigiId, simData));
                }
            }
            delete digiHits;
            digiHits = nullptr;
        }  // while(i != e)
    }      // while(m_thpcTGC->nextDetectorElement(i, e))

    for (size_t coll_hash = 0; coll_hash < collections.size(); ++coll_hash) {
        if (collections[coll_hash]) {
            ATH_CHECK(digitContainer->addCollection(
                collections[coll_hash].release(), coll_hash));
        }
    }

    return StatusCode::SUCCESS;
}
