/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////////////////
//
// MdtDigitizationTool:
//            Athena algorithm which produces MDTDigits out of MHits.
// ------------
// Authors:
//            Daniela Rebuzzi (daniela.rebuzzi@pv.infn.it)
//            Ketevi A. Assamagan (ketevi@lbn.gov)
//             Modified by:
//             2004-03-02 Niels Van Eldik (nveldik@nikhef.nl)
//             Modified by:
//             2004-04-05 Daniel Levin (dslevin@umich.edu)
//             generate drift time deltas based on wire sag.
//             first pass: assume wires are geomtrically centered-
//             but have top/bottom RT functions that would obtain
//             if the wire was off  axis by an amount determined
//             by the tube length, orientation, hit location and track angle
//             Modified by:
//             2004-02-08 Daniel Levin (dslevin@umich.edu)
//             generate new impact parameters based on physical wire sag.
//             2012-7-5 Shannon Walch (srwalch@umich.edu)
//             implement bug fixes in RT wiresag calculations
//             Modified by:
//             Dinos Bachas (konstantinos.bachas@cern.ch)
//
////////////////////////////////////////////////////////////////////////////////

// MDT digitization includes
#include "MDT_Digitization/MdtDigitizationTool.h"

#include "MDT_Digitization/MdtDigiToolInput.h"
#include "MDT_Digitization/chargeCalculator.h"
#include "MDT_Digitization/particleGamma.h"

// Gaudi - Core
#include "PathResolver/PathResolver.h"

// Geometry
#include "EventPrimitives/EventPrimitives.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonSimEvent/MdtHitIdHelper.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
// Pile-up

// Truth
#include "AtlasHepMC/GenParticle.h"
#include "GeneratorObjects/HepMcParticleLink.h"

// Random Numbers
#include "AthenaKernel/RNGWrapper.h"

// Calibration Service
#include "MdtCalibData/MdtFullCalibData.h"
#include "MdtCalibData/MdtTubeCalibContainer.h"
namespace {
    // what about this? does this also need to be 1/m_signalSpeed ?
    constexpr double s_inv_c_light(1. / Gaudi::Units::c_light);
}  // namespace
MdtDigitizationTool::MdtDigitizationTool(const std::string& type, const std::string& name, const IInterface* pIID) :
    PileUpToolBase(type, name, pIID) {}

StatusCode MdtDigitizationTool::initialize() {
    ATH_MSG_INFO("Configuration  MdtDigitizationTool");
    ATH_MSG_INFO("RndmSvc                " << m_rndmSvc);
    ATH_MSG_INFO("DigitizationTool       " << m_digiTool);
    ATH_MSG_INFO("MdtCalibrationDbTool    " << m_calibrationDbTool);
    ATH_MSG_INFO("UseDeadChamberSvc      " << m_UseDeadChamberSvc);
    if (!m_UseDeadChamberSvc) ATH_MSG_INFO("MaskedStations         " << m_maskedStations);
    ATH_MSG_INFO("OffsetTDC              " << m_offsetTDC);
    ATH_MSG_INFO("ns2TDCAMT              " << m_ns2TDCAMT);
    ATH_MSG_INFO("ns2TDCHPTDC            " << m_ns2TDCHPTDC);
    ATH_MSG_INFO("ResolutionTDC          " << m_resTDC);
    ATH_MSG_INFO("SignalSpeed            " << m_signalSpeed);
    ATH_MSG_INFO("InputObjectName        " << m_inputObjectName);
    ATH_MSG_INFO("OutputObjectName       " << m_outputObjectKey.key());
    ATH_MSG_INFO("OutputSDOName          " << m_outputSDOKey.key());
    ATH_MSG_INFO("UseAttenuation         " << m_useAttenuation);
    ATH_MSG_INFO("UseTof                 " << m_useTof);
    ATH_MSG_INFO("UseProp                " << m_useProp);
    ATH_MSG_INFO("UseWireSagGeom         " << m_useWireSagGeom);
    ATH_MSG_INFO("UseWireSagRT           " << m_useWireSagRT);
    ATH_MSG_INFO("UseDeformations        " << m_useDeformations);
    ATH_MSG_INFO("UseTimeWindow          " << m_useTimeWindow);
    ATH_MSG_INFO("BunchCountOffset       " << m_bunchCountOffset);
    ATH_MSG_INFO("MatchingWindow         " << m_matchingWindow);
    ATH_MSG_INFO("MaskWindow             " << m_maskWindow);
    ATH_MSG_INFO("DeadTime               " << m_deadTime);
    ATH_MSG_INFO("DiscardEarlyHits       " << m_DiscardEarlyHits);
    ATH_MSG_INFO("CheckSimHits           " << m_checkMDTSimHits);
    ATH_MSG_INFO("UseTwin                " << m_useTwin);
    ATH_MSG_INFO("UseAllBOLTwin          " << m_useAllBOLTwin);
    ATH_MSG_INFO("ResolutionTwinTube     " << m_resTwin);
    ATH_MSG_INFO("DoQballCharge          " << m_DoQballCharge);
    if (!m_useTof) {
        ATH_MSG_INFO("UseCosmicsOffSet1      " << m_useOffSet1);
        ATH_MSG_INFO("UseCosmicsOffSet2      " << m_useOffSet2);
    }
    ATH_MSG_INFO("IncludePileUpTruth     " << m_includePileUpTruth);
    ATH_MSG_INFO("ParticleBarcodeVet     " << m_vetoThisBarcode);

    // initialize transient detector store and MuonGeoModel OR MuonDetDescrManager
    if (detStore()->contains<MuonGM::MuonDetectorManager>("Muon")) {
        ATH_CHECK(detStore()->retrieve(m_MuonGeoMgr));
        ATH_MSG_DEBUG("Retrieved MuonGeoModelDetectorManager from StoreGate");
    }

    // initialize MuonIdHelperSvc
    ATH_CHECK(m_idHelperSvc.retrieve());

    if (m_onlyUseContainerName) { ATH_CHECK(m_mergeSvc.retrieve()); }

    // check the input object name
    if (m_hitsContainerKey.key().empty()) {
        ATH_MSG_FATAL("Property InputObjectName not set !");
        return StatusCode::FAILURE;
    }
    if (m_onlyUseContainerName) m_inputObjectName = m_hitsContainerKey.key();
    ATH_MSG_DEBUG("Input objects in container : '" << m_inputObjectName << "'");

    // Initialize ReadHandleKey
    ATH_CHECK(m_hitsContainerKey.initialize(true));

    // initialize the output WriteHandleKeys
    ATH_CHECK(m_outputObjectKey.initialize());
    ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputObjectKey);
    ATH_CHECK(m_outputSDOKey.initialize());
    ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputSDOKey);
    ATH_MSG_DEBUG("Output Digits: '" << m_outputObjectKey.key() << "'");

    // simulation identifier helper
    m_muonHelper = MdtHitIdHelper::GetHelper(m_idHelperSvc->mdtIdHelper().tubeMax());

    // get the r->t conversion tool
    ATH_CHECK(m_digiTool.retrieve());
    ATH_MSG_DEBUG("Retrieved digitization tool!" << m_digiTool);

    ATH_CHECK(m_rndmSvc.retrieve());

    ATH_CHECK(m_calibrationDbTool.retrieve());

    // Gather masked stations
    m_vMaskedStations.reserve(m_maskedStations.size());
    for (unsigned int i = 0; i < m_maskedStations.size(); ++i) {
        std::string_view mask(m_maskedStations[i]);
        std::string_view maskedName = mask.substr(0, mask.find(':'));
        std::string_view temps = mask.substr(maskedName.size() + 1, std::string::npos);
        auto col = temps.find(':');
        std::string_view maskedEta = (col !=  std::string_view::npos) ? temps.substr(0, col) : temps.substr(0);
        std::string_view maskedPhi = temps.substr(maskedEta.size() + 1, std::string::npos);
        m_vMaskedStations.emplace_back(maskedName, maskedEta, maskedPhi);
        if (!m_UseDeadChamberSvc)
            ATH_MSG_DEBUG("mask = " << mask << "  maskedName = " << maskedName << "  temps = " << temps << "  maskedEta = " << maskedEta
                                    << "  maskedPhi = " << maskedPhi);
    }

    // Retrieve the Conditions service
    if (m_UseDeadChamberSvc) {
        ATH_CHECK(m_readKey.initialize());
    } else {
        ATH_MSG_DEBUG("Using JobOptions for masking dead/missing chambers");
    }

    return StatusCode::SUCCESS;
}

StatusCode MdtDigitizationTool::prepareEvent(const EventContext& /*ctx*/, unsigned int nInputEvents) {
    ATH_MSG_DEBUG("MdtDigitizationTool::prepareEvent() called for " << nInputEvents << " input events");

    m_MDTHitCollList.clear();
    m_thpcMDT = std::make_unique<TimedHitCollection<MDTSimHit>>();

    return StatusCode::SUCCESS;
}

StatusCode MdtDigitizationTool::processBunchXing(int bunchXing, SubEventIterator bSubEvents, SubEventIterator eSubEvents) {
    ATH_MSG_DEBUG("MdtDigitizationTool::processBunchXing() " << bunchXing);

    typedef PileUpMergeSvc::TimedList<MDTSimHitCollection>::type TimedHitCollList;
    TimedHitCollList hitCollList;

    if (!(m_mergeSvc->retrieveSubSetEvtData(m_inputObjectName, hitCollList, bunchXing, bSubEvents, eSubEvents).isSuccess()) &&
        hitCollList.empty()) {
        ATH_MSG_ERROR("Could not fill TimedHitCollList");
        return StatusCode::FAILURE;
    } else {
        ATH_MSG_VERBOSE(hitCollList.size() << " MDTSimHitCollection with key " << m_inputObjectName << " found");
    }

    TimedHitCollList::iterator iColl(hitCollList.begin());
    TimedHitCollList::iterator endColl(hitCollList.end());

    // Iterating over the list of collections
    for (; iColl != endColl; ++iColl) {
        MDTSimHitCollection* hitCollPtr = new MDTSimHitCollection(*iColl->second);
        PileUpTimeEventIndex timeIndex(iColl->first);

        ATH_MSG_DEBUG("MDTSimHitCollection found with " << hitCollPtr->size() << " hits");
        ATH_MSG_VERBOSE("time index info. time: " << timeIndex.time() << " index: " << timeIndex.index() << " type: " << timeIndex.type());

        m_thpcMDT->insert(timeIndex, hitCollPtr);
        m_MDTHitCollList.push_back(hitCollPtr);
    }

    return StatusCode::SUCCESS;
}

StatusCode MdtDigitizationTool::getNextEvent(const EventContext& ctx) {
    ATH_MSG_DEBUG("MdtDigitizationTool::getNextEvent()");

    //  get the container(s)
    using TimedHitCollList = PileUpMergeSvc::TimedList<MDTSimHitCollection>::type;

    // In case of single hits container just load the collection using read handles
    if (!m_onlyUseContainerName) {
        SG::ReadHandle<MDTSimHitCollection> hitCollection(m_hitsContainerKey, ctx);
        if (!hitCollection.isValid()) {
            ATH_MSG_ERROR("Could not get MDTSimHitCollection container " << hitCollection.name() << " from store "
                                                                         << hitCollection.store());
            return StatusCode::FAILURE;
        }

        // create a new hits collection
        m_thpcMDT = std::make_unique<TimedHitCollection<MDTSimHit>>(1);
        m_thpcMDT->insert(0, hitCollection.cptr());
        ATH_MSG_DEBUG("MDTSimHitCollection found with " << hitCollection->size() << " hits");

        return StatusCode::SUCCESS;
    }

    // this is a list<info<time_t, DataLink<MDTSimHitCollection> > >
    TimedHitCollList hitCollList;

    if (!(m_mergeSvc->retrieveSubEvtsData(m_inputObjectName, hitCollList).isSuccess())) {
        ATH_MSG_ERROR("Could not fill TimedHitCollList");
        return StatusCode::FAILURE;
    }
    if (hitCollList.empty()) {
        ATH_MSG_ERROR("TimedHitCollList has size 0");
        return StatusCode::FAILURE;
    } else {
        ATH_MSG_DEBUG(hitCollList.size() << " MDTSimHitCollections with key " << m_inputObjectName << " found");
    }

    // create a new hits collection
    m_thpcMDT = std::make_unique<TimedHitCollection<MDTSimHit>>();

    // now merge all collections into one
    TimedHitCollList::iterator iColl(hitCollList.begin());
    TimedHitCollList::iterator endColl(hitCollList.end());
    while (iColl != endColl) {
        const MDTSimHitCollection* p_collection(iColl->second);
        m_thpcMDT->insert(iColl->first, p_collection);
        ATH_MSG_DEBUG("MDTSimHitCollection found with " << p_collection->size() << " hits");
        ++iColl;
    }
    return StatusCode::SUCCESS;
}

CLHEP::HepRandomEngine* MdtDigitizationTool::getRandomEngine(const std::string& streamName, const EventContext& ctx) const {
    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, streamName);
    std::string rngName = name() + streamName;
    rngWrapper->setSeed(rngName, ctx);
    return rngWrapper->getEngine(ctx);
}

StatusCode MdtDigitizationTool::mergeEvent(const EventContext& ctx) {
    ATH_MSG_DEBUG("MdtDigitizationTool::in mergeEvent()");

    // create and record the Digit container in StoreGate
    SG::WriteHandle<MdtDigitContainer> digitContainer(m_outputObjectKey, ctx);
    ATH_CHECK(digitContainer.record(std::make_unique<MdtDigitContainer>(m_idHelperSvc->mdtIdHelper().module_hash_max())));
    ATH_MSG_DEBUG("Recorded MdtDigitContainer called " << digitContainer.name() << " in store " << digitContainer.store());

    // create and record the SDO container in StoreGate
    SG::WriteHandle<MuonSimDataCollection> sdoContainer(m_outputSDOKey, ctx);
    ATH_CHECK(sdoContainer.record(std::make_unique<MuonSimDataCollection>()));
    ATH_MSG_DEBUG("Recorded MuonSimDataCollection called " << sdoContainer.name() << " in store " << sdoContainer.store());

    Collections_t collections;
    StatusCode status = doDigitization(ctx, collections, sdoContainer.ptr());
    if (status.isFailure()) { ATH_MSG_ERROR("doDigitization Failed"); }

    for (size_t coll_hash = 0; coll_hash < collections.size(); ++coll_hash) {
      if (collections[coll_hash]) {
        ATH_CHECK( digitContainer->addCollection (collections[coll_hash].release(), coll_hash) );
      }
    }

    // Clean-up
    std::vector<MDTSimHitCollection*>::iterator MDTHitColl = m_MDTHitCollList.begin();
    std::vector<MDTSimHitCollection*>::iterator MDTHitCollEnd = m_MDTHitCollList.end();
    while (MDTHitColl != MDTHitCollEnd) {
        delete (*MDTHitColl);
        ++MDTHitColl;
    }
    m_MDTHitCollList.clear();

    return status;
}

StatusCode MdtDigitizationTool::processAllSubEvents(const EventContext& ctx) {
    ATH_MSG_DEBUG("MdtDigitizationTool::processAllSubEvents()");

    // create and record the Digit container in StoreGate
    SG::WriteHandle<MdtDigitContainer> digitContainer(m_outputObjectKey, ctx);
    ATH_CHECK(digitContainer.record(std::make_unique<MdtDigitContainer>(m_idHelperSvc->mdtIdHelper().module_hash_max())));
    ATH_MSG_DEBUG("Recorded MdtDigitContainer called " << digitContainer.name() << " in store " << digitContainer.store());

    // create and record the SDO container in StoreGate
    SG::WriteHandle<MuonSimDataCollection> sdoContainer(m_outputSDOKey, ctx);
    ATH_CHECK(sdoContainer.record(std::make_unique<MuonSimDataCollection>()));
    ATH_MSG_DEBUG("Recorded MuonSimDataCollection called " << sdoContainer.name() << " in store " << sdoContainer.store());

    StatusCode status = StatusCode::SUCCESS;
    if (!m_thpcMDT) {
        status = getNextEvent(ctx);
        if (StatusCode::FAILURE == status) {
            ATH_MSG_INFO("There are no MDT hits in this event");
            return status;
        }
    }

    Collections_t collections;
    ATH_CHECK(doDigitization(ctx, collections, sdoContainer.ptr()));

    for (size_t coll_hash = 0; coll_hash < collections.size(); ++coll_hash) {
      if (collections[coll_hash]) {
        ATH_CHECK( digitContainer->addCollection (collections[coll_hash].release(), coll_hash) );
      }
    }

    return status;
}

StatusCode MdtDigitizationTool::doDigitization(const EventContext& ctx, Collections_t& collections,
                                               MuonSimDataCollection* sdoContainer) {
    // Set the RNGs to use for this event.
    CLHEP::HepRandomEngine* rndmEngine = getRandomEngine("", ctx);
    CLHEP::HepRandomEngine* twinRndmEngine = getRandomEngine("Twin", ctx);
    CLHEP::HepRandomEngine* toolRndmEngine = getRandomEngine(m_digiTool->name(), ctx);

    // Get the list of dead/missing chambers and cache it
    if (m_UseDeadChamberSvc) {
        SG::ReadCondHandle<MdtCondDbData> readHandle{m_readKey, ctx};
        const MdtCondDbData* readCdo{*readHandle};
        if (!readHandle.isValid() || !readCdo) {
            ATH_MSG_WARNING(readHandle.fullKey() << " is not available.");
            return StatusCode::FAILURE;
        }
        m_IdentifiersToMask = readCdo->getDeadStationsId();       
        ATH_MSG_DEBUG("Number of dead/missing stations retrieved from CondService= " << readCdo->getDeadStationsId().size());
    }

    // get the iterator infos for this DetEl
    // iterate over hits and fill id-keyed drift time map
    TimedHitCollection<MDTSimHit>::const_iterator i, e;

    // Perform null check on m_thpcMDT
    if (!m_thpcMDT) {
        ATH_MSG_ERROR("m_thpcMDT is null");
        return StatusCode::FAILURE;
    }

    while (m_thpcMDT->nextDetectorElement(i, e)) {
        // Loop over the hits:
        while (i != e) {
            handleMDTSimhit(*i, twinRndmEngine, toolRndmEngine);
            ++i;
        }
    }

    // loop over drift time map entries, convert to tdc value and construct/store the digit
    createDigits(collections, sdoContainer, rndmEngine);

    // reset hits
    m_hits.clear();

    // reset the pointer if it is not null
    m_thpcMDT.reset();

    return StatusCode::SUCCESS;
}

bool MdtDigitizationTool::handleMDTSimhit(const TimedHitPtr<MDTSimHit>& phit, CLHEP::HepRandomEngine* twinRndmEngine,
                                          CLHEP::HepRandomEngine* toolRndmEngine) {
    const MDTSimHit& hit(*phit);
    MDTSimHit newSimhit(*phit);  // hit can be modified later

    double globalHitTime(hitTime(phit));

    // Important checks for hits (global time, position along tube, masked chambers etc..) DO NOT SET THIS CHECK TO FALSE IF YOU DON'T KNOW
    // WHAT YOU'RE DOING !
    if (m_checkMDTSimHits) {
        if (!checkMDTSimHit(hit)) return false;
    }

    const int id = hit.MDTid();
    double driftRadius = hit.driftRadius();
    ATH_MSG_DEBUG("Hit bunch time  " << globalHitTime - hit.globalTime() << " tot " << globalHitTime << " tof " << hit.globalTime()
                                     << " driftRadius " << driftRadius);

    std::string stationName = m_muonHelper->GetStationName(id);
    int stationEta = m_muonHelper->GetZSector(id);
    int stationPhi = m_muonHelper->GetPhiSector(id);
    int multilayer = m_muonHelper->GetMultiLayer(id);
    int layer = m_muonHelper->GetLayer(id);
    int tube = m_muonHelper->GetTube(id);

    // construct Atlas identifier from components
    Identifier DigitId = m_idHelperSvc->mdtIdHelper().channelID(stationName, stationEta, stationPhi, multilayer, layer, tube);

    // get distance to readout
    double distRO(0.);

    // find the detector element associated to the hit
    const MuonGM::MdtReadoutElement* element = m_MuonGeoMgr->getMdtReadoutElement(DigitId);

    if (!element) {
        ATH_MSG_ERROR("MuonGeoManager does not return valid element for given id!");
        return false;
    } else {
        distRO = element->tubeFrame_localROPos(DigitId).z();
    }

    if (m_useDeformations) {
        newSimhit = applyDeformations(hit, element, DigitId);
        driftRadius = newSimhit.driftRadius();
    }

    // store local hit position + sign
    GeoCorOut result = correctGeometricalWireSag(hit, DigitId, element);
    if (m_useDeformations) { result = correctGeometricalWireSag(newSimhit, DigitId, element); }
    double saggingSign = result.sagSign;
    double trackingSign = result.trackingSign;
    Amg::Vector3D lpos = result.localPosition;
    double localSag = result.localSag;

    // set segment (radius + distance to readout)
    if (m_useWireSagGeom) {
        driftRadius = lpos.perp();

        double projectiveSag = hit.driftRadius() - std::abs(driftRadius);
        if (m_useDeformations) projectiveSag = newSimhit.driftRadius() - std::abs(driftRadius);

        ATH_MSG_DEBUG(" Geometrical WIRESAGINFO "
                      << stationName << " " << stationEta << " " << stationPhi << " " << multilayer << " " << layer << " " << tube << " "
                      << Amg::toString(hit.localPosition(), 3) << " "
                      << hit.driftRadius() << " " << element->tubeLength(DigitId) << " " << " " << localSag
                      << " " << projectiveSag << " " << driftRadius << " " << saggingSign);
    }

    // correctly set sign of drift radius
    driftRadius *= trackingSign;

    //+Implementation for RT_Relation_DB_Tool
    MdtDigiToolInput digiInput(std::abs(driftRadius), distRO, 0., 0., 0., 0.);
    double qcharge = 1.;
    double qgamma = -9999.;

    if (m_DoQballCharge == true) {
        // chargeCalculator returns the value of electric charge for Qball particle.
        // particleGamma returns the value of gamma for Qball particle.
        qgamma = particleGamma(hit, phit.eventId());
        qcharge = chargeCalculator(hit, phit.eventId());

        MdtDigiToolInput digiInput1(std::abs(driftRadius), distRO, 0., 0., qcharge, qgamma);
        digiInput = digiInput1;

        if (m_digiTool.name() == "RT_Relation_DB_DigiTool") {
            MdtDigiToolInput digiInput2(std::abs(driftRadius), distRO, 0., 0., qcharge, qgamma, DigitId);
            digiInput = digiInput2;
        }
    } else {
        MdtDigiToolInput digiInput1(std::abs(driftRadius), distRO, 0., 0., 0., 0.);
        digiInput = digiInput1;

        if (m_digiTool.name() == "RT_Relation_DB_DigiTool") {
            MdtDigiToolInput digiInput2(std::abs(driftRadius), distRO, 0., 0., 0., 0., DigitId);
            digiInput = digiInput2;
        }
    }

    // digitize input
    MdtDigiToolOutput digiOutput(m_digiTool->digitize(digiInput, toolRndmEngine));
    //-Implementation for RT_Relation_DB_Tool

    // simulate tube response, check if tube fired
    if (digiOutput.wasEfficient()) {
        double driftTime = digiOutput.driftTime();
        double adc = digiOutput.adc();

        ATH_MSG_VERBOSE("Tube efficient: driftTime  " << driftTime << " adc value " << adc);

        // compute RT effect
        if (m_useWireSagRT && !element->barrel() && stationName != "EOS" && stationName != "EOL") {
            Amg::Vector3D gpos = element->localToGlobalTransf(DigitId)*lpos;

            // fit parameters for drift time difference vs impact radius for a wire 500 microns off axis
            // garfield calculation. details on http://dslevin.home.cern.ch/atlas/wiresag.ppt
            // Line below: old code
            // double param[4] = {-0.3025,0.58303,0.012177,0.0065818};
            // New code
            static constexpr std::array<double, 6> param{-4.47741E-3, 1.75541E-2, -1.32913E-2, 2.57938E-3, -4.55015E-5, -1.70821E-7};

            // get delta T, change in drift time for reference sag (default=100 microns) and scale by projective sag
            double deltaT{0.};
            double dR = std::abs(driftRadius);
            for (int i = 0; i < 6; ++i) { deltaT += param[i] * std::pow(dR, i); }

            // reference sag now set to 0.1 mm
            double referenceSag = 0.1;

            // Calculate angle at which track cross plane of sag.
            // Note that this assumes the track is coming from the center of the detector.
            double cosTheta = std::abs(gpos.z()) / gpos.mag();

            // This calculates the sag seen by a track; if a particle passes parallel to the sag,
            // the shift in drift circle location will have no affect.
            double projectiveSag = localSag * cosTheta;

            deltaT *= (projectiveSag / referenceSag);

            // saggingSign is calculated by the correctGeometricalWireSag function of this class
            // It is +/- 1 depending on whether or not the track passed above or below the wire.
            deltaT = -1 * saggingSign * deltaT;

            double driftTimeOriginal = driftTime;
            driftTime += deltaT;  // update drift time

            ATH_MSG_DEBUG(" RT WIRESAGINFO " << stationName << " " << stationEta << " " << stationPhi << " " << multilayer << " " << layer
                                             << " " << tube << " " << Amg::toString(hit.localPosition(), 3) 
                                             << " " << driftRadius << " " << element->tubeLength(DigitId) / 1000.
                                             << " " << cosTheta << " " << localSag << " " << projectiveSag << " " << deltaT << "   "
                                             << driftTimeOriginal << "    " << driftTime);
        }  // m_useWireSagRT

        if (m_useProp) {
            double position_along_wire = hit.localPosition().z();
            if (m_useDeformations) { position_along_wire = newSimhit.localPosition().z(); }

            // prop delay calculated with respect to the center of the tube
            double sign(-1.);
            if (distRO < 0.) sign = 1.;
            double propagation_delay = sign * (1. / m_signalSpeed) * position_along_wire;
            //------------------------------------------------------------
            // calculate propagation delay, as readout side the side with
            // negative local
            // position along the wire is taken

            driftTime += propagation_delay;  // add prop time
            ATH_MSG_VERBOSE("Position along wire:  " << position_along_wire << " propagation delay:  " << propagation_delay
                                                     << " new driftTime " << driftTime);
        }

        // add tof + bunch time
        if (m_useTof) {
            driftTime += globalHitTime;
            ATH_MSG_VERBOSE("Time off Flight + bunch offset:  " << globalHitTime << " new driftTime " << driftTime);
        }
        ATH_MSG_DEBUG(m_idHelperSvc->mdtIdHelper().show_to_string(DigitId)
                      << "  Drift time computation " << driftTime << " radius " << driftRadius << " adc " << adc);

        // add hit to hit collection
        m_hits.insert(mdt_hit_info(DigitId, driftTime, adc, driftRadius, &phit));
        ATH_MSG_VERBOSE(" handleMDTSimHit() phit-" << &phit << "  hit.localPosition().z() = " << hit.localPosition().z()
                                                   << " driftRadius = " << driftRadius);

        // + TWIN TUBES  (A. Koutsman)
        if (m_useTwin) {
            // two chambers in ATLAS are installed with Twin Tubes; in detector coordinates BOL4A13 & BOL4C13; only INNER multilayer(=1) is
            // with twin tubes
            bool BOL4X13 = false;
            // find these two chambers in identifier scheme coordinates as in MdtIdHelper
            if (stationName == "BOL" && std::abs(stationEta) == 4 && stationPhi == 7 && multilayer == 1) { BOL4X13 = true; }

            // implement twin tubes digitizing either for all BOL (m_useAllBOLTwin = true) _OR_ only for two chambers really installed
            if ((m_useAllBOLTwin && stationName == "BOL") || BOL4X13) {
                int twin_tube = 0;
                Identifier twin_DigitId{0};
                double twin_sign_driftTime = 0.;
                // twinpair is connected via a HV-jumper with a delay of ~6ns
                constexpr double HV_delay = 6.;
                double twin_tubeLength{0.}, twin_geo_pos_along_wire{0.},
                       twin_sign_pos_along_wire{0.}, twin_sign{-1.};

                // twinpair is interconnected with one tube in between, so modulo 4 they are identical
                if (tube % 4 == 1 || tube % 4 == 2)
                    twin_tube = tube + 2;
                else if (tube % 4 == 0 || tube % 4 == 3)
                    twin_tube = tube - 2;
                // construct Atlas identifier from components for the twin
                twin_DigitId = m_idHelperSvc->mdtIdHelper().channelID(stationName, stationEta, stationPhi, multilayer, layer, twin_tube);
                // get twin tube length for propagation delay
                twin_tubeLength = element->tubeLength(twin_DigitId);

                // prop delay calculated with respect to the center of the tube
                if (distRO < 0.) twin_sign = 1.;
                twin_geo_pos_along_wire = hit.localPosition().z();
                if (m_useDeformations) { twin_geo_pos_along_wire = newSimhit.localPosition().z(); }
                twin_sign_pos_along_wire = twin_sign * twin_geo_pos_along_wire;
                double twin_propagation_delay = twin_sign * (1. / m_signalSpeed) * twin_geo_pos_along_wire;

                // calculate drift-time for twin from prompt driftTime + propagation delay + length of tube + hv-delay
                // ( -2* for propagation_delay, cause prop_delay already in driftTime)
                twin_sign_driftTime = driftTime + twin_tubeLength / m_signalSpeed - 2 * twin_propagation_delay + HV_delay;

                // smear the twin time by a gaussian with a stDev given by m_resTwin
                double rand = CLHEP::RandGaussZiggurat::shoot(twinRndmEngine, twin_sign_driftTime, m_resTwin);
                twin_sign_driftTime = rand;

                ATH_MSG_DEBUG(" TWIN TUBE stname " << stationName << " steta " << stationEta << " stphi " << stationPhi << " mLayer "
                                                   << multilayer << " layer " << layer << " tube " << tube
                                                   << " signed position along wire = " << twin_sign_pos_along_wire
                                                   << " propagation delay = " << twin_propagation_delay << " drifttime = " << driftTime
                                                   << "    twin_driftTime = " << twin_sign_driftTime
                                                   << "  TWIN time-difference = " << (twin_sign_driftTime - driftTime));

                // add twin-hit to hit collection
                m_hits.insert(mdt_hit_info(twin_DigitId, twin_sign_driftTime, adc, driftRadius, &phit));

            }  // end select all BOLs or installed chambers
        }      // end if(m_useTwin){
               // - TWIN TUBES   (A. Koutsman)
    } else {
        ATH_MSG_DEBUG(m_idHelperSvc->mdtIdHelper().show_to_string(DigitId) << "  Tube not efficient "
                                                                           << " radius " << driftRadius);
    }

    return true;
}

bool MdtDigitizationTool::checkMDTSimHit(const MDTSimHit& hit) const {
    // get the hit Identifier and info
    const int id = hit.MDTid();
    std::string stationName = m_muonHelper->GetStationName(id);
    int stationEta = m_muonHelper->GetZSector(id);
    int stationPhi = m_muonHelper->GetPhiSector(id);
    int multilayer = m_muonHelper->GetMultiLayer(id);
    int layer = m_muonHelper->GetLayer(id);
    int tube = m_muonHelper->GetTube(id);

    Identifier DigitId = m_idHelperSvc->mdtIdHelper().channelID(stationName, stationEta, stationPhi, multilayer, layer, tube);
    ATH_MSG_DEBUG("Working on hit: " << m_idHelperSvc->mdtIdHelper().show_to_string(DigitId) << "  "
                                     << m_idHelperSvc->mdtIdHelper().stationNameString(m_idHelperSvc->mdtIdHelper().stationName(DigitId))
                                     << " " << stationEta << " " << stationPhi);

    //+MASKING OF DEAD/MISSING CHAMBERS
    if (m_UseDeadChamberSvc) {
        for (const Identifier& Id : m_IdentifiersToMask) {
         
            if ((stationName == m_idHelperSvc->mdtIdHelper().stationNameString(m_idHelperSvc->mdtIdHelper().stationName(Id))) &&
                (stationEta == m_idHelperSvc->mdtIdHelper().stationEta(Id)) &&
                (stationPhi == m_idHelperSvc->mdtIdHelper().stationPhi(Id))) {
                ATH_MSG_DEBUG("Hit is located in chamber that is dead/missing: Masking the hit!");
                return false;
            }
        }
    } else {
        bool nameMasked = false;
        bool etaMasked = false;
        bool phiMasked = false;
        bool masked = false;

        for (unsigned int i = 0; i < m_maskedStations.size(); ++i) {
            bool temp = true;

            for (unsigned int k = 0; k < 3; ++k) {
                char c = m_vMaskedStations[i].maskedName[k];
                char cc = stationName[k];
                if (c == '*') continue;
                if (c == cc) continue;
                temp = false;
            }
            if (!temp) continue;
            nameMasked = temp;

            etaMasked = m_vMaskedStations[i].maskedEta == "*" || m_vMaskedStations[i].imaskedEta == stationEta;
            phiMasked = m_vMaskedStations[i].maskedPhi == "*" || m_vMaskedStations[i].imaskedPhi == stationPhi;
            masked = nameMasked && etaMasked && phiMasked;
            if (masked) { return false; }
        }
    }
    //-MASKING OF DEAD/MISSING CHAMBERS

    double tubeL{0.}, tubeR{0.};

    const MuonGM::MdtReadoutElement* element = m_MuonGeoMgr->getMdtReadoutElement(DigitId);

    if (nullptr == element) {
        ATH_MSG_ERROR("MuonGeoManager does not return valid element for given id!");
    } else {
        tubeL = element->tubeLength(DigitId);
        tubeR = element->innerTubeRadius();
    }

    bool ok(true);

    if (std::abs(hit.driftRadius()) > tubeR) {
        ok = false;
        ATH_MSG_DEBUG("MDTSimHit has invalid radius: " << hit.driftRadius() << "   tubeRadius " << tubeR);
    }

    if (std::abs(hit.localPosition().z()) > 0.5 * tubeL) {
        ok = false;
        ATH_MSG_DEBUG("MDTSimHit has invalid position along tube: " << hit.localPosition().z() << "   tubeLength " << tubeL);
    }

    if (m_useTof) {
        double minTof = minimumTof(DigitId, m_MuonGeoMgr);
        if ((hit.globalTime() < 0 || hit.globalTime() > 10 * minTof) && m_DiscardEarlyHits) {
            ok = false;
            ATH_MSG_DEBUG("MDTSimHit has invalid global time: " << hit.globalTime() << "   minimum Tof " << minTof);
        }
    } else {
        ATH_MSG_DEBUG("MDTSimHit global time: " << hit.globalTime() << " accepted anyway as UseTof is false");
    }

    return ok;
}

bool MdtDigitizationTool::createDigits(Collections_t& collections, MuonSimDataCollection* sdoContainer,
                                       CLHEP::HepRandomEngine* rndmEngine) {
    Identifier currentDigitId{0}, currentElementId{0};

    double currentDeadTime = 0.;
    MdtDigitCollection* digitCollection = nullptr;
    // loop over sorted hits
    m_hits.sort();
    HitIt it = m_hits.begin();

    // +For Cosmics add
    double timeOffsetEvent = 0.0;
    double timeOffsetTotal = 0.0;

    // this offset emulates the timing spead of cosmics: +/- 1 BC
    if (m_useTof == false && m_useOffSet2 == true) {
        int inum = CLHEP::RandFlat::shootInt(rndmEngine, 0, 10);
        if (inum == 8) {
            timeOffsetEvent = -25.0;
        } else if (inum == 9) {
            timeOffsetEvent = 25.0;
        }
        ATH_MSG_DEBUG("Emulating timing spead of cosmics: +/- 1 BC. Adding  " << timeOffsetEvent << " ns to time");
    }
    //-ForCosmics

    for (; it != m_hits.end(); ++it) {
        Identifier idDigit = it->id;
        Identifier elementId = m_idHelperSvc->mdtIdHelper().elementID(idDigit);
        const MuonGM::MdtReadoutElement* geo = m_MuonGeoMgr->getMdtReadoutElement(idDigit);

        // Check if we are in a new chamber, if so get the DigitCollection
        if (elementId != currentElementId) {
            currentElementId = elementId;
            digitCollection = getDigitCollection(elementId, collections);

            //+ForCosmics
            // this offset emulates the time jitter of cosmic ray muons w.r.t LVL1 accept
            if (m_useTof == false && m_useOffSet1 == true) {
                timeOffsetTotal = timeOffsetEvent + CLHEP::RandFlat::shoot(rndmEngine, -12.5, 12.5);
                ATH_MSG_DEBUG("Emulating time jitter of cosmic ray muons w.r.t LVL1 accept. Adding  " << timeOffsetTotal << " ns to time");
            }
            //-ForCosmics
        }
        if (!digitCollection) {
            ATH_MSG_ERROR("Trying to use nullptr digitCollection");
            return false;
        }

        float driftRadius = it->radius;
        double driftTime = it->time;
        double charge = it->adc;

        ATH_MSG_VERBOSE("New hit : driftTime " << driftTime << " adc  " << charge);

        // check if we are in a new tube
        if (idDigit != currentDigitId) {
            currentDigitId = idDigit;
            // set the deadTime
            currentDeadTime = driftTime + charge + m_deadTime;
            ATH_MSG_VERBOSE("New tube, setting dead time:  " << currentDeadTime << "  driftTime " << driftTime);
        } else {
            // check if tube is dead
            if (driftTime > currentDeadTime) {
                // tube produces a second hit, set the new deadtime
                currentDeadTime = driftTime + charge + m_deadTime;
                ATH_MSG_VERBOSE("Additional hit, setting dead time:  " << currentDeadTime << "  driftTime " << driftTime);
            } else {
                // tube is dead go to next hit
                ATH_MSG_VERBOSE("Hit within dead time:  " << currentDeadTime << "  driftTime " << driftTime);
                continue;
            }
        }

        const TimedHitPtr<MDTSimHit>& phit = *(it->simhit);
        const MDTSimHit& hit(*phit);

        // check if the hits lies within the TDC time window
        // subtrack the minimum Tof (= globalPosition().mag()/c) from the tof of the hit
        double relativeTime = driftTime - minimumTof(idDigit, m_MuonGeoMgr);
        bool insideMatch = insideMatchingWindow(relativeTime);
        bool insideMask = insideMaskWindow(relativeTime);
        if (insideMask && insideMatch) {
            ATH_MSG_WARNING(" Digit in matching AND masking window, please check window definition: relative time " << relativeTime);
            insideMask = false;
        }
        if (insideMatch || insideMask) {
            // get calibration constants from DbTool
            double t0 = m_offsetTDC;
            const MuonCalib::MdtFullCalibData data = m_calibrationDbTool->getCalibration(geo->identifyHash(), geo->detectorElementHash());
            if (data.tubeCalib) {
                int ml = m_idHelperSvc->mdtIdHelper().multilayer(idDigit) - 1;
                int layer = m_idHelperSvc->mdtIdHelper().tubeLayer(idDigit) - 1;
                int tube = m_idHelperSvc->mdtIdHelper().tube(idDigit) - 1;
                if (ml >= 0 && layer >= 0 && tube >= 0) {
                    // extract calibration constants for single tube
                    const MuonCalib::MdtTubeCalibContainer::SingleTubeCalib* singleTubeData = data.tubeCalib->getCalib(ml, layer, tube);
                    if (singleTubeData) { 
                        ATH_MSG_DEBUG("Extracted the following calibration constant for "<<m_idHelperSvc->toString(idDigit)<<" "<<singleTubeData->t0);
                        t0 = singleTubeData->t0 + m_t0ShiftTuning; 
                    } else  ATH_MSG_WARNING("No calibration data found, using t0=" << m_offsetTDC<<" "<<m_idHelperSvc->toString(idDigit));
                }
            } else {
                ATH_MSG_WARNING("No calibration data found, using t0=" << m_offsetTDC<<" for "<<m_idHelperSvc->toString(idDigit));
            }
            bool isHPTDC = m_idHelperSvc->hasHPTDC(idDigit);
            int tdc = digitizeTime(driftTime + t0 + timeOffsetTotal, isHPTDC, rndmEngine);
            int adc = digitizeTime(it->adc, isHPTDC, rndmEngine);
            ATH_MSG_DEBUG(" >> Digit Id = " << m_idHelperSvc->mdtIdHelper().show_to_string(idDigit) << " driftTime " << driftTime
                                            << " driftRadius " << driftRadius << " TDC " << tdc << " ADC " << adc << " mask bit "
                                            << insideMask);

            MdtDigit* newDigit = new MdtDigit(idDigit, tdc, adc, insideMask);
            digitCollection->push_back(newDigit);

            ATH_MSG_VERBOSE(" createDigits() phit-" << &phit << " hit-" << hit.print() << "    localZPos = " << hit.localPosition().z());

            // Do not store pile-up truth information
            if (!m_includePileUpTruth && ((phit->trackNumber() == 0) || (phit->trackNumber() == m_vetoThisBarcode))) { continue; }

            // Create the Deposit for MuonSimData
            const EBC_EVCOLL evColl = EBC_MAINEVCOLL;
            const HepMcParticleLink::PositionFlag idxFlag =
                (phit.eventId() == 0) ? HepMcParticleLink::IS_POSITION : HepMcParticleLink::IS_INDEX;
            MuonSimData::Deposit deposit(HepMcParticleLink(phit->trackNumber(), phit.eventId(), evColl, idxFlag),
                                         MuonMCData(driftRadius, hit.localPosition().z()));

            // Record the SDO collection in StoreGate
            std::vector<MuonSimData::Deposit> deposits;
            deposits.push_back(deposit);
            MuonSimData tempSDO(deposits, 0);
            const Amg::Vector3D& tempLocPos = (*(it->simhit))->localPosition();
            Amg::Vector3D p = geo->localToGlobalTransf(idDigit)*tempLocPos;
            tempSDO.setPosition(p);
            tempSDO.setTime(hitTime(phit));
            sdoContainer->insert(std::make_pair(idDigit, tempSDO));

        } else {
            ATH_MSG_DEBUG("  >> OUTSIDE TIME WINDOWS << "
                          << " Digit Id = " << m_idHelperSvc->toString(idDigit) << " driftTime " << driftTime
                          << " --> hit ignored");
        }
    }  // for (; it != hits.end(); ++it)

    return true;
}

MdtDigitCollection* MdtDigitizationTool::getDigitCollection(Identifier elementId, Collections_t& collections) {
    IdContext mdtContext = m_idHelperSvc->mdtIdHelper().module_context();
    IdentifierHash coll_hash;
    if (m_idHelperSvc->mdtIdHelper().get_hash(elementId, coll_hash, &mdtContext)) {
        ATH_MSG_ERROR("Unable to get MDT hash id from MDT Digit collection "
                      << "context begin_index = " << mdtContext.begin_index() << " context end_index  = " << mdtContext.end_index()
                      << " the identifier is ");
        elementId.show();
    }

    if (coll_hash >= collections.size()) {
      collections.resize (coll_hash+1);
    }

    auto& coll = collections[coll_hash];
    if (!coll) {
      coll = std::make_unique<MdtDigitCollection>(elementId, coll_hash);
    }
    return coll.get();
}

int MdtDigitizationTool::digitizeTime(double time, bool isHPTDC, CLHEP::HepRandomEngine* rndmEngine) const {
    int tdcCount{0};
    double tmpCount = isHPTDC ? time / m_ns2TDCHPTDC : time / m_ns2TDCAMT;
    tdcCount = CLHEP::RandGaussZiggurat::shoot(rndmEngine, tmpCount, m_resTDC);
    if (tdcCount < 0 || tdcCount > 4096) { ATH_MSG_DEBUG(" Count outside TDC window: " << tdcCount); }
    return tdcCount;
}

double MdtDigitizationTool::minimumTof(Identifier DigitId, const MuonGM::MuonDetectorManager* detMgr) const {
    if (!m_useTof) return 0.;

    // get distance to vertex for tof correction before applying the time window
    double distanceToVertex(0.);
    const MuonGM::MdtReadoutElement* element = detMgr->getMdtReadoutElement(DigitId);

    if (!element) {
        ATH_MSG_ERROR("MuonGeoManager does not return valid element for given id!");
    } else {
        distanceToVertex = element->tubePos(DigitId).mag();
    }
    // what about this? does this also need to be 1/m_signalSpeed ?
    ATH_MSG_DEBUG("minimumTof calculated " << distanceToVertex * s_inv_c_light);
    return distanceToVertex * s_inv_c_light;
}

bool MdtDigitizationTool::insideMatchingWindow(double time) const {
    if (m_useTimeWindow)
        if (time < m_bunchCountOffset || time > static_cast<double>(m_bunchCountOffset) + m_matchingWindow) {
            ATH_MSG_VERBOSE("hit outside MatchingWindow " << time);
            return false;
        }
    return true;
}

bool MdtDigitizationTool::insideMaskWindow(double time) const {
    if (m_useTimeWindow)
        if (time < static_cast<double>(m_bunchCountOffset) - m_maskWindow || time > m_bunchCountOffset) {
            ATH_MSG_VERBOSE("hit outside MaskWindow " << time);
            return false;
        }
    return true;
}

//+emulate deformations here
MDTSimHit MdtDigitizationTool::applyDeformations(const MDTSimHit& hit, const MuonGM::MdtReadoutElement* element,
                                                 const Identifier& DigitId) const {
    const int id = hit.MDTid();

    // make the deformation
    Amg::Vector3D hitAtGlobalFrame = element->nodeform_localToGlobalTransf(DigitId) * hit.localPosition();
    Amg::Vector3D hitDeformed = element->globalToLocalTransf(DigitId) * hitAtGlobalFrame;

    MDTSimHit simhit2{id, hit.globalTime(), hitDeformed.perp(), hitDeformed, hit.trackNumber()};

    return simhit2;
}

MdtDigitizationTool::GeoCorOut MdtDigitizationTool::correctGeometricalWireSag(const MDTSimHit& hit, const Identifier& id,
                                                                              const MuonGM::MdtReadoutElement* element) const {
    Amg::Vector3D lpos = hit.localPosition();
    Amg::Transform3D gToWireFrame = element->globalToLocalTransf(id);

    // local track direction in precision plane
    Amg::Vector3D ldir(-lpos.y(), lpos.x(), 0.);
    ldir.normalize();

    //  calculate the position of the hit sagged wire frame
    // transform to global coords
    const Amg::Transform3D& transf{element->localToGlobalTransf(id)};
    Amg::Vector3D gpos = transf*lpos;
    Amg::Vector3D gdir = transf* ldir;

    // get wire surface
    const Trk::SaggedLineSurface& surface = element->surface(id);

    // check whether direction is pointing away from IP
    double pointingCheck = gpos.dot(gdir) < 0 ? -1. : 1.;
    if (pointingCheck < 0) { gdir *= pointingCheck; }

    double trackingSign = 1.;
    double localSag = 0.0;
    if (m_useWireSagGeom) {
        // calculate local hit position in nominal wire frame
        Amg::Vector2D lp{Amg::Vector2D::Zero()};
        surface.globalToLocal(gpos, gpos, lp);

        // calculate sagged wire position
        std::unique_ptr<const Trk::StraightLineSurface> wireSurface {surface.correctedSurface(lp)};
        // calculate displacement of wire from nominal
        // To do this, note that the sagged surface is modeled as a straight line
        // through the point in the space that the bit of wire closest to the hit
        // sagged to and is parallel to the nominal wire. Find the center of the
        // sagged surface in global coordinates, transform this into the nominal
        // surface's local coordinates, and calculate that point's distance from
        // the origin.
        const Amg::Vector3D gSaggedSpot = wireSurface->center();
        Amg::Vector3D lSaggedSpot = gToWireFrame * gSaggedSpot;
        
        localSag = lSaggedSpot.perp();

        // global to local sagged wire frame transform
        gToWireFrame = wireSurface->transform().inverse();

        // local hit position in sagged wire frame
        lpos = gToWireFrame * gpos;
        ldir = gToWireFrame * gdir;
        ldir.normalize();

        // compute drift radius ( = impact parameter)
        double alpha = -1 * lpos.dot(ldir);
        lpos = lpos + alpha * ldir;

        // calculate global point of closest approach
        Amg::Vector3D saggedGPos = wireSurface->transform() * lpos;

        // recalculate tracking sign
        Amg::Vector2D lpsag{Amg::Vector2D::Zero()};
        wireSurface->globalToLocal(saggedGPos, gdir, lpsag);
        trackingSign = lpsag[Trk::locR] < 0 ? -1. : 1.;

    } else {
        // recalculate tracking sign
        Amg::Vector2D lpsag{Amg::Vector2D::Zero()};
        surface.globalToLocal(gpos, gdir, lpsag);
        trackingSign = lpsag[Trk::locR] < 0 ? -1. : 1.;
    }

    // local gravity vector
    Amg::Vector3D gravityDir =-1. * Amg::Vector3D::UnitY();
    Amg::Vector3D lgravDir = gToWireFrame * gravityDir;

    // Project gravity vector onto X-Y plane, so the z-components (along the wire)
    // don't contribute to the dot-product.
    lgravDir.z() = 0.;

    // calculate whether hit above or below wire (using gravity direction
    // 1 -> hit below wire (in same hemisphere as gravity vector)
    //-1 -> hit above wire (in opposite hemisphere as gravity vector)
    double sign = lpos.dot(lgravDir) < 0 ? -1. : 1.;

    return GeoCorOut(sign, trackingSign, lpos, localSag);
}
