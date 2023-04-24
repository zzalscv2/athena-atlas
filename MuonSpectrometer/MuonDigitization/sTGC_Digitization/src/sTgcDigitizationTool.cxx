/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////////////////
//
// sTgcDigitizationTool
// ------------
// Authors:  Nectarios Benekos  <nectarios.benekos@cern.ch>
//           Jiaming Yu  <jiaming.yu@cern.ch>
////////////////////////////////////////////////////////////////////////////////

#include "MuonSimData/MuonSimDataCollection.h"
#include "MuonSimData/MuonSimData.h"

//Outputs
#include "MuonDigitContainer/sTgcDigitContainer.h"

//sTGC digitization includes
#include "sTGC_Digitization/sTgcDigitizationTool.h"
#include "sTGC_Digitization/sTgcSimDigitData.h"

//Geometry
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonSimEvent/sTgcHitIdHelper.h"
#include "MuonSimEvent/sTgcSimIdToOfflineId.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "TrkSurfaces/Surface.h"
#include "CLHEP/Random/RandGaussZiggurat.h"

//Truth
#include "GeneratorObjects/HepMcParticleLink.h"
#include "AtlasHepMC/GenParticle.h"

#include "AthenaKernel/RNGWrapper.h"

#include <sstream>
#include <iostream>
#include <fstream>

#include <memory>


using namespace MuonGM;

inline bool sort_digitsEarlyToLate(const sTgcSimDigitData &a, const sTgcSimDigitData &b){
  return a.getSTGCDigit().time() < b.getSTGCDigit().time();
}

/*******************************************************************************/
sTgcDigitizationTool::sTgcDigitizationTool(const std::string& type, const std::string& name, const IInterface* parent) :
    PileUpToolBase(type, name, parent) {}

/*******************************************************************************/
// member function implementation
//--------------------------------------------
StatusCode sTgcDigitizationTool::initialize() {

  ATH_MSG_INFO (" sTgcDigitizationTool  retrieved");
  ATH_MSG_INFO ( "Configuration  sTgcDigitizationTool" );
  ATH_MSG_INFO ( "doSmearing             "<< m_doSmearing);
  ATH_MSG_INFO ( "RndmSvc                " << m_rndmSvc             );
  ATH_MSG_INFO ( "RndmEngine             " << m_rndmEngineName      );
  ATH_MSG_INFO ( "InputObjectName        " << m_hitsContainerKey.key());
  ATH_MSG_INFO ( "OutputObjectName       " << m_outputDigitCollectionKey.key());
  ATH_MSG_INFO ( "OutputSDOName          " << m_outputSDO_CollectionKey.key());

  if (m_hitsContainerKey.key().empty()) {
    ATH_MSG_FATAL("Property InputObjectName not set !");
    return StatusCode::FAILURE;
  }

  if (m_onlyUseContainerName) m_inputObjectName = m_hitsContainerKey.key();
  ATH_MSG_DEBUG("Input objects in container: '" << m_inputObjectName << "'");

  // Pile-up merge service
  if (m_onlyUseContainerName) {
    ATH_CHECK(m_mergeSvc.retrieve());
  }

  // retrieve MuonDetctorManager from DetectorStore
  ATH_CHECK(detStore()->retrieve(m_mdManager));
  ATH_CHECK(m_idHelperSvc.retrieve());

  // sTgcHitIdHelper
  m_hitIdHelper = sTgcHitIdHelper::GetHelper();

  // calibration tool
  ATH_CHECK(m_calibTool.retrieve());

  // initialize ReadCondHandleKey
  ATH_CHECK(m_condThrshldsKey.initialize());

  // Initialize ReadHandleKey
  ATH_CHECK(m_hitsContainerKey.initialize(true));

  //initialize the output WriteHandleKeys
  ATH_CHECK(m_outputDigitCollectionKey.initialize());
  ATH_CHECK(m_outputSDO_CollectionKey.initialize());

  // initialize sTgcDigitMaker class to digitize hits
  // meanGasGain is the mean value of the polya gas gain function describing the
  // avalanche of electrons caused by the electric field
  // Parameterization is obtained from ATL-MUON-PUB-2014-001 and the corrected
  // fit to data to parameterize gain vs HV in kV
  // m_runVoltage MUST BE in kV!
  if (m_runVoltage < 2.3 || m_runVoltage > 3.2){
    ATH_MSG_ERROR("STGC run voltage must be in kV and within fit domain of 2.3 kV to 3.2 kV");
    return StatusCode::FAILURE;
  }
  double meanGasGain = 2.15 * 1E-4 * exp(6.88*m_runVoltage);
  m_digitizer = std::make_unique<sTgcDigitMaker>(m_hitIdHelper, m_mdManager, m_doEfficiencyCorrection, meanGasGain, m_doPadSharing);
  m_digitizer->setLevel(static_cast<MSG::Level>(msgLevel()));
  ATH_CHECK(m_digitizer->initialize(m_doChannelTypes));

  ATH_CHECK(m_rndmSvc.retrieve());
  // getting our random numbers stream
  ATH_MSG_DEBUG("Getting random number engine : <" << m_rndmEngineName << ">");

  return StatusCode::SUCCESS;
}
/*******************************************************************************/
StatusCode sTgcDigitizationTool::prepareEvent(const EventContext& /*ctx*/, unsigned int nInputEvents) {

  ATH_MSG_DEBUG("sTgcDigitizationTool::prepareEvent() called for " << nInputEvents << " input events" );
  m_STGCHitCollList.clear();

  return StatusCode::SUCCESS;
}
/*******************************************************************************/

StatusCode sTgcDigitizationTool::processBunchXing(int bunchXing,
              SubEventIterator bSubEvents,
              SubEventIterator eSubEvents) {
  ATH_MSG_DEBUG ( "sTgcDigitizationTool::in processBunchXing()" );
  if (m_thpcsTGC == nullptr) {
    m_thpcsTGC = std::make_unique<TimedHitCollection<sTGCSimHit>>();
  }
  using TimedHitCollList = PileUpMergeSvc::TimedList<sTGCSimHitCollection>::type;
  TimedHitCollList hitCollList;

  if (!(m_mergeSvc->retrieveSubSetEvtData(m_inputObjectName, hitCollList, bunchXing,
                                          bSubEvents, eSubEvents).isSuccess()) &&
      hitCollList.empty()) {
    ATH_MSG_ERROR("Could not fill TimedHitCollList");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_VERBOSE(hitCollList.size() << " sTGCSimHitCollection with key " <<
                    m_inputObjectName << " found");
  }

  TimedHitCollList::iterator iColl(hitCollList.begin());
  TimedHitCollList::iterator endColl(hitCollList.end());

  // Iterating over the list of collections
  for( ; iColl != endColl; ++iColl){

    auto hitCollPtr = std::make_unique<sTGCSimHitCollection>(*iColl->second);
    PileUpTimeEventIndex timeIndex(iColl->first);

    ATH_MSG_DEBUG("sTGCSimHitCollection found with " << hitCollPtr->size() <<
                  " hits");
    ATH_MSG_VERBOSE("time index info. time: " << timeIndex.time()
                    << " index: " << timeIndex.index()
                    << " type: " << timeIndex.type());

    m_thpcsTGC->insert(timeIndex, hitCollPtr.get());
    m_STGCHitCollList.push_back(std::move(hitCollPtr));
  }
  return StatusCode::SUCCESS;
}
/*******************************************************************************/
StatusCode sTgcDigitizationTool::getNextEvent(const EventContext& ctx) {

  ATH_MSG_DEBUG ( "sTgcDigitizationTool::getNextEvent()" );

  //  get the container(s)
  using TimedHitCollList = PileUpMergeSvc::TimedList<sTGCSimHitCollection>::type;

  // In case of single hits container just load the collection using read handles
  if (!m_onlyUseContainerName) {
    SG::ReadHandle<sTGCSimHitCollection> hitCollection(m_hitsContainerKey, ctx);
    if (!hitCollection.isValid()) {
      ATH_MSG_ERROR("Could not get sTGCSimHitCollection container " << hitCollection.name() << " from store " << hitCollection.store());
      return StatusCode::FAILURE;
    }

    // create a new hits collection
    m_thpcsTGC = std::make_unique<TimedHitCollection<sTGCSimHit>>(1);
    m_thpcsTGC->insert(0, hitCollection.cptr());
    ATH_MSG_DEBUG("sTGCSimHitCollection found with " << hitCollection->size() << " hits");
    return StatusCode::SUCCESS;
  }

  //this is a list<info<time_t, DataLink<sTGCSimHitCollection> > >
  TimedHitCollList hitCollList;

  if (!(m_mergeSvc->retrieveSubEvtsData(m_inputObjectName, hitCollList).isSuccess()) ) {
    ATH_MSG_ERROR ( "Could not fill TimedHitCollList" );
    return StatusCode::FAILURE;
  }
  if (hitCollList.empty()) {
    ATH_MSG_ERROR ( "TimedHitCollList has size 0" );
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG ( hitCollList.size() << " sTGC SimHitCollections with key " << m_inputObjectName << " found" );
  }

  //Perform null check on m_thpcsTGC. If pointer is not null throw error
  if (m_thpcsTGC == nullptr) {
    m_thpcsTGC = std::make_unique<TimedHitCollection<sTGCSimHit>>();
  }else{
  ATH_MSG_ERROR ( "m_thpcsTGC is not null" );
  return StatusCode::FAILURE;
  }

  //now merge all collections into one
  TimedHitCollList::iterator iColl(hitCollList.begin());
  TimedHitCollList::iterator endColl(hitCollList.end());
  while (iColl != endColl) {
    const sTGCSimHitCollection* p_collection(iColl->second);
    m_thpcsTGC->insert(iColl->first, p_collection);
    ATH_MSG_DEBUG ( "sTGC SimHitCollection found with " << p_collection->size() << " hits"  );
    ++iColl;
  }

  return StatusCode::SUCCESS;
}
/*******************************************************************************/
StatusCode sTgcDigitizationTool::mergeEvent(const EventContext& ctx) {

  StatusCode status = StatusCode::SUCCESS;

  ATH_MSG_DEBUG ( "sTgcDigitizationTool::in mergeEvent()" );

  status = doDigitization(ctx);
  if (status.isFailure())  {
    ATH_MSG_ERROR ( "doDigitization Failed" );
  }

  // reset the pointer
  m_thpcsTGC.reset();

  m_STGCHitCollList.clear();

  return status;
}
/*******************************************************************************/
StatusCode sTgcDigitizationTool::digitize(const EventContext& ctx) {
  return this->processAllSubEvents(ctx);
}
/*******************************************************************************/
StatusCode sTgcDigitizationTool::processAllSubEvents(const EventContext& ctx) {
  StatusCode status = StatusCode::SUCCESS;
  ATH_MSG_DEBUG (" sTgcDigitizationTool::processAllSubEvents()" );

  //merging of the hit collection in getNextEvent method
  if (m_thpcsTGC == nullptr) {
    status = getNextEvent(ctx);
    if (StatusCode::FAILURE == status) {
      ATH_MSG_INFO ( "There are no sTGC hits in this event" );
      return status;
    }
  }
  status = doDigitization(ctx);
  if (status.isFailure())  {
    ATH_MSG_ERROR ( "doDigitization() Failed" );
  }

  // reset the pointer
  m_thpcsTGC.reset();

  return status;
}

/*******************************************************************************/
StatusCode sTgcDigitizationTool::doDigitization(const EventContext& ctx) {

  ATH_MSG_DEBUG ("sTgcDigitizationTool::doDigitization()" );

  CLHEP::HepRandomEngine* rndmEngine = getRandomEngine(m_rndmEngineName, ctx);

  // create and record the Digit container in StoreGate
  SG::WriteHandle<sTgcDigitContainer> digitContainer(m_outputDigitCollectionKey, ctx);
  ATH_CHECK(digitContainer.record(std::make_unique<sTgcDigitContainer>(m_idHelperSvc->stgcIdHelper().module_hash_max())));
  ATH_MSG_DEBUG ( "sTgcDigitContainer recorded in StoreGate." );

  // Create and record the SDO container in StoreGate
  SG::WriteHandle<MuonSimDataCollection> sdoContainer(m_outputSDO_CollectionKey, ctx);
  ATH_CHECK(sdoContainer.record(std::make_unique<MuonSimDataCollection>()));
  ATH_MSG_DEBUG ( "sTgcSDOCollection recorded in StoreGate." );


  TimedHitCollection<sTGCSimHit>::const_iterator i, e;

  // Collections of digits by digit type associated with a detector element
  std::map< Identifier, std::map< Identifier, std::vector<sTgcSimDigitData> > > unmergedPadDigits;
  std::map< Identifier, std::map< Identifier, std::vector<sTgcSimDigitData> > > unmergedStripDigits;
  std::map< Identifier, std::map< Identifier, std::vector<sTgcSimDigitData> > > unmergedWireDigits;

  std::map< Identifier, std::map< Identifier, std::vector<sTgcDigit> > > outputDigits;

  ATH_MSG_DEBUG("create Digit container of size " << m_idHelperSvc->stgcIdHelper().module_hash_max());

  IdContext tgcContext = m_idHelperSvc->stgcIdHelper().module_context();

  float earliestEventTime = 9999;

  // --nextDetectorElement>sets an iterator range with the hits of current detector element , returns a bool when done
  while(m_thpcsTGC->nextDetectorElement(i, e)) {
    int nhits = 0;
    ATH_MSG_VERBOSE("Next Detector Element");
    while(i != e){ //loop through the hits on this Detector Element
      ATH_MSG_VERBOSE("Looping over hit " << nhits+1 << " on this Detector Element." );

      ++nhits;
      TimedHitPtr<sTGCSimHit> phit = *i++;
      const sTGCSimHit& hit = *phit;
      ATH_MSG_VERBOSE("Hit Particle ID : " << hit.particleEncoding() );
      float eventTime = phit.eventTime();
      if(eventTime < earliestEventTime) earliestEventTime = eventTime;
      // Cut on energy deposit of the particle
      if(hit.depositEnergy() < m_energyDepositThreshold) {
        ATH_MSG_VERBOSE("Hit with Energy Deposit of " << hit.depositEnergy()
                        << " less than " << m_energyDepositThreshold << ". Skip this hit." );
        continue;
      }

      // Old HITS format doesn't have kinetic energy (i.e it is set to -1).
      float hit_kineticEnergy = hit.kineticEnergy();

      // Skip digitizing some problematic hits, if processing compatible HITS format
      if (hit_kineticEnergy > 0.) {
        // Skip electron with low kinetic energy, since electrons are mainly secondary particles.
        if ((std::abs(hit.particleEncoding()) == 11) && (hit_kineticEnergy < m_limitElectronKineticEnergy)) {
          ATH_MSG_DEBUG("Skip electron hit with kinetic energy " << hit_kineticEnergy
                      << ", which is less than the lower limit of " << m_limitElectronKineticEnergy);
          continue;
        }

       // No support for particles with direction perpendicular to the beam line, since such particles
       // can deposit energy on a lot of strips and pads of the gas gap. So a good model of charge
       // spreading should be implemented. Also, these particles are rare, and most of them are
       // secondary particles suh as electrons.
       if (std::abs(hit.globalPosition().z() - hit.globalPrePosition().z()) < 0.00001) {
         ATH_MSG_VERBOSE("Skip hit with a direction perpendicular to the beam line, ie z-component is less than 0.00001 mm.");
         continue;
       }
      }

      if(eventTime != 0){
         msg(MSG::DEBUG) << "Updated hit global time to include off set of " << eventTime << " ns from OOT bunch." << endmsg;
      }
      else {
          msg(MSG::DEBUG) << "This hit came from the in time bunch." << endmsg;
      }
      sTgcSimIdToOfflineId simToOffline(&m_idHelperSvc->stgcIdHelper());
      const int idHit = hit.sTGCId();
      ATH_MSG_VERBOSE("Hit ID " << idHit );
      Identifier layid = simToOffline.convert(idHit);
      ATH_MSG_VERBOSE("Layer ID[" << layid.getString() << "]");
      int eventId = phit.eventId();
      std::string stationName= m_idHelperSvc->stgcIdHelper().stationNameString(m_idHelperSvc->stgcIdHelper().stationName(layid));
      int isSmall = stationName[2] == 'S';
      int gasGap = m_idHelperSvc->stgcIdHelper().gasGap(layid);
      ATH_MSG_VERBOSE("Gas Gap " << gasGap );

      /// apply the smearing tool to decide if the hit has to be digitized or not
      /// based on layer efficiency
      if ( m_doSmearing ) {
        bool acceptHit = true;
        ATH_CHECK(m_smearingTool->isAccepted(layid,acceptHit,rndmEngine));
        if ( !acceptHit ) {
          ATH_MSG_DEBUG("Dropping the hit - smearing tool");
          continue;
        }
      }

      const MuonGM::sTgcReadoutElement* detEL = m_mdManager->getsTgcReadoutElement(layid);  //retreiving the sTGC this hit is located in
      if( !detEL ){
        ATH_MSG_WARNING("Failed to retrieve detector element for: isSmall " << isSmall
                         << " eta " << m_idHelperSvc->stgcIdHelper().stationEta(layid)
                         << " phi " << m_idHelperSvc->stgcIdHelper().stationPhi(layid)
                         << " ml " << m_idHelperSvc->stgcIdHelper().multilayer(layid));
        continue;
      }

      // project the hit position to wire surface (along the incident angle)
      ATH_MSG_VERBOSE("Projecting hit to Wire Surface" );
      const Amg::Vector3D& HPOS{hit.globalPosition()};  //Global position of the hit
      const Amg::Vector3D GLOBAL_ORIG(0., 0., 0.);
      const Amg::Vector3D GLOBAL_Z(0., 0., 1.);
      const Amg::Vector3D& GLODIRE{hit.globalDirection()};
      Amg::Vector3D global_preStepPos{hit.globalPrePosition()};

      ATH_MSG_VERBOSE("Global Z " << GLOBAL_Z );
      ATH_MSG_VERBOSE("Global Direction " << GLODIRE );
      ATH_MSG_VERBOSE("Global Position " << HPOS );

      int surfHash_wire =  detEL->surfaceHash(gasGap, 2);
      ATH_MSG_VERBOSE("Surface Hash for wire plane" << surfHash_wire );
      const Trk::PlaneSurface&  SURF_WIRE = detEL->surface(surfHash_wire);  //Plane of the wire surface in this gasGap
      ATH_MSG_VERBOSE("Wire Surface Defined " << SURF_WIRE.center() );

      Amg::Vector3D LOCAL_Z = SURF_WIRE.transform().inverse()*GLOBAL_Z - SURF_WIRE.transform().inverse()*GLOBAL_ORIG;
      Amg::Vector3D LOCDIRE = SURF_WIRE.transform().inverse()*GLODIRE - SURF_WIRE.transform().inverse()*GLOBAL_ORIG;
      Amg::Vector3D LPOS = SURF_WIRE.transform().inverse() * HPOS;  //Position of the hit on the wire plane in local coordinates

      ATH_MSG_VERBOSE("Local Z: (" << LOCAL_Z.x() << ", " << LOCAL_Z.y() << ", " << LOCAL_Z.z() <<")" );
      ATH_MSG_VERBOSE("Local Direction: (" << LOCDIRE.x() << ", " << LOCDIRE.y() << ", " << LOCDIRE.z() << ")" );
      ATH_MSG_VERBOSE("Local Position: (" << LPOS.x() << ", " << LPOS.y() << ", " << LPOS.z() << ")" );

      /* Backward compatibility with old sTGCSimHitCollection persistent class
       *  Two parameters (kinetic energy, pre-step position) are added in
       *  sTGCSimHitCollection_p3, and the direction parameter is removed.
       *  To preserve backwards compatibility, the digitization should be able
       *  to process hits from the old persistent classes (_p1 and _p2).
       *  When reading the old persistent classes, the kinetic energy is
       *  initialized to a negative value, while the pre-step position is not
       *  defined. So the pre-step position has to be derived from the
       *  direction vector.
       */

      constexpr double e = 1e-5;

      bool X_1 = std::abs( std::abs(LOCAL_Z.x()) - 1. ) < e;
      bool Y_1 = std::abs( std::abs(LOCAL_Z.y()) - 1. ) < e;
      bool Z_1 = std::abs( std::abs(LOCAL_Z.z()) - 1. ) < e;
      bool X_s = std::abs(LOCAL_Z.x()) < e;
      bool Y_s = std::abs(LOCAL_Z.y()) < e;
      bool Z_s = std::abs(LOCAL_Z.z()) < e;

      double scale = 0.;
      if (X_1 && Y_s && Z_s && std::abs(LOCDIRE.x()) > e)
        scale = -LPOS.x() / LOCDIRE.x();
      else if (X_s && Y_1 && Z_s && std::abs(LOCDIRE.y()) > e)
        scale = -LPOS.y() / LOCDIRE.y();
      else if (X_s && Y_s && Z_1 && std::abs(LOCDIRE.z()) > e)
        scale = -LPOS.z() / LOCDIRE.z();
      else
        ATH_MSG_DEBUG(" Wrong scale! ");

      // Hit on the wire surface in local coordinates
      Amg::Vector3D hitOnSurf_wire = LPOS + scale * LOCDIRE;

      // Some hits are created inside the gas gap, thus they pass through only a portion of the gap.
      if (hit_kineticEnergy > 0.0) {
        // Processing HITS format with valid pre-step position
        double segment_length = (HPOS - global_preStepPos).mag();
        // If segment doesn't intersect the wire plane, then find the hit position on the segment
        // and project onto the wire plane.
        if (std::abs(scale) > segment_length) {
          Amg::Vector3D temp_hit_pos = LPOS + (scale / std::abs(scale)) * segment_length * LOCDIRE;
          hitOnSurf_wire = Amg::Vector3D(temp_hit_pos.x(), temp_hit_pos.y(), 0.0);
        }
      } else {
        // Processing an old HITS format, so assume the particle passes through the gas gap following a straight line.
        // However, skip electron hit with scale factor greater than 8.0mm. For half-gap thickness of 1.425mm,
        //   a scale factor of 8 corresponds to an incident angle of about 80deg and the final position on the wire plane
        //   is more than two strips away from the post-step position.
        // With how the extrapolation is done, electron hit with large scale factor might result in a ionization
        //   position far from where it should be if the electron is created inside the gas gap.
        //if ((std::abs(hit.particleEncoding()) == 11) && (std::abs(scale) > 8.0)) {
        //  continue;
        //}
        Amg::Vector3D local_preStepPos = LPOS + 2 * scale * LOCDIRE;
        global_preStepPos = SURF_WIRE.transform() * local_preStepPos;
      }

      // In the local frame of the wire plane, the z-component of the hit on the wire surface is zero
      if (hitOnSurf_wire.z() > e) {
        ATH_MSG_DEBUG("Local Hit position on Wire surface (" << hitOnSurf_wire.x() << ", " << hitOnSurf_wire.y() << ", " << hitOnSurf_wire.z() << ") has non-zero z-component.");
      }

      //The hit on the wire in Global coordinates
      Amg::Vector3D glob_hitOnSurf_wire = SURF_WIRE.transform() * hitOnSurf_wire;

      ATH_MSG_VERBOSE("Local Hit on Wire Surface: (" << hitOnSurf_wire.x() << ", " << hitOnSurf_wire.y() << ", " << hitOnSurf_wire.z() << ")"  );
      ATH_MSG_VERBOSE("Global Hit on Wire Surface: (" << glob_hitOnSurf_wire.x() << ", " << glob_hitOnSurf_wire.y() << ", " << glob_hitOnSurf_wire.z() << ")" );

      ATH_MSG_DEBUG("sTgcDigitizationTool::doDigitization hits mapped");

      const EBC_EVCOLL evColl = EBC_MAINEVCOLL;
      const HepMcParticleLink::PositionFlag idxFlag = (eventId==0) ? HepMcParticleLink::IS_POSITION: HepMcParticleLink::IS_INDEX;
      const int barcode = hit.particleLink().barcode();
      const HepMcParticleLink particleLink(barcode, eventId, evColl, idxFlag);
      const sTGCSimHit temp_hit(hit.sTGCId(), hit.globalTime(),
                                HPOS,
                                hit.particleEncoding(),
                                hit.globalDirection(),
                                hit.depositEnergy(),
                                particleLink,
                                hit_kineticEnergy,
                                global_preStepPos
                                );


      float globalHitTime = temp_hit.globalTime() + eventTime;
      float tof = temp_hit.globalPosition().mag()/CLHEP::c_light;
      float bunchTime = globalHitTime - tof;

      // Create all the digits for this particular Sim Hit
      std::unique_ptr<sTgcDigitCollection> digiHits = m_digitizer->executeDigi(&temp_hit, globalHitTime, rndmEngine);
      if (digiHits == nullptr) {
        continue;
      }

      sTgcDigitCollection::const_iterator it_digiHits;
      ATH_MSG_VERBOSE("Hit produced " << digiHits->size() << " digits." );
      for(it_digiHits=digiHits->begin(); it_digiHits!=digiHits->end(); ++it_digiHits) {
        /*
          NOTE:
          -----
          Since not every hit might end up resulting in a
          digit, this construction might take place after the hit loop
          in a loop of its own!
        */
        // make new sTgcDigit
        Identifier newDigitId = (*it_digiHits)->identify(); //This Identifier should be sufficient to determine which RE the digit is from
        float newTime = (*it_digiHits)->time();
        int newChannelType = m_idHelperSvc->stgcIdHelper().channelType((*it_digiHits)->identify());

        float timeJitterElectronicsStrip = CLHEP::RandGaussZiggurat::shoot(rndmEngine, 0, m_timeJitterElectronicsStrip);
        float timeJitterElectronicsPad = CLHEP::RandGaussZiggurat::shoot(rndmEngine, 0, m_timeJitterElectronicsPad);
        if(newChannelType==1)
          newTime += timeJitterElectronicsStrip;
        else
          newTime += timeJitterElectronicsPad;
        uint16_t newBcTag = bcTagging(newTime+bunchTime, newChannelType);

        if(m_doToFCorrection)
          newTime += bunchTime;
        else
          newTime += globalHitTime;

        float newCharge = (*it_digiHits)->charge();

        if(newChannelType!=0 && newChannelType!=1 && newChannelType!=2) {
          ATH_MSG_WARNING( "Wrong channelType " << newChannelType );
        }

        bool isDead = 0;
        bool isPileup = 0;
        ATH_MSG_VERBOSE("Hit is from the main signal subevent if eventId is zero, eventId = " << eventId << " newTime: " << newTime);
        if(eventId != 0)  //hit not from the main signal subevent
          isPileup = 1;

        // Create a new digit with updated time and BCTag
        sTgcDigit newDigit(newDigitId, newBcTag, newTime, newCharge, isDead, isPileup);
        ATH_MSG_VERBOSE("Unmerged Digit") ;
        ATH_MSG_VERBOSE(" BC tag = "    << newDigit.bcTag()) ;
        ATH_MSG_VERBOSE(" digitTime = " << newDigit.time()) ;
        ATH_MSG_VERBOSE(" charge = "    << newDigit.charge()) ;


        // Create a MuonSimData (SDO) corresponding to the digit
        MuonSimData::Deposit deposit(particleLink, MuonMCData(hit.depositEnergy(), tof));
        std::vector<MuonSimData::Deposit> deposits;
        deposits.push_back(deposit);
        MuonSimData simData(std::move(deposits), hit.particleEncoding());
        // The sTGC SDO should be placed at the center of the gap, on the wire plane.
        // We use the position from the hit on the wire surface which is by construction in the center of the gap
        // glob_hitOnSurf_wire projects the whole hit to the center of the gap
        simData.setPosition(glob_hitOnSurf_wire);
        simData.setTime(globalHitTime);

        Identifier layerID = m_idHelperSvc->stgcIdHelper().channelID(
          newDigitId,
          m_idHelperSvc->stgcIdHelper().multilayer(newDigitId),
          m_idHelperSvc->stgcIdHelper().gasGap(newDigitId),
          newChannelType, 1);

        if(newChannelType == sTgcIdHelper::sTgcChannelTypes::Pad){ //Pad Digit
          //Put the hit and digit in a vector associated with the RE
          unmergedPadDigits[layerID][newDigitId].emplace_back(simData, newDigit);
        }
        else if(newChannelType == sTgcIdHelper::sTgcChannelTypes::Strip){ //Strip Digit
          //Put the hit and digit in a vector associated with the RE
          unmergedStripDigits[layerID][newDigitId].emplace_back(simData, newDigit);
        }
        else if(newChannelType == sTgcIdHelper::sTgcChannelTypes::Wire){ //Wire Digit
          //Put the hit and digit in a vector associated with the RE
          unmergedWireDigits[layerID][newDigitId].emplace_back(simData, newDigit);
        }
      } // end of loop digiHits
    } // end of while(i != e)
  } //end of while(m_thpcsTGC->nextDetectorElement(i, e))

  /***************************
  * Retrieve conditions data *
  ***************************/

  // set up pointer to conditions object
  SG::ReadCondHandle<NswCalibDbThresholdData> readThresholds{m_condThrshldsKey, ctx};
  if(!readThresholds.isValid()){
    ATH_MSG_ERROR("Cannot find conditions data container for VMM thresholds!");
  }
  const NswCalibDbThresholdData* thresholdData = readThresholds.cptr();

  /*********************
  * Process Strip Digits *
  *********************/
  /* Comments from Alexandre Laurier, October 2022:
    Big update to VMM handling of digits to sTGC digitization
    For each channel type, the digits are processed on a layer-by-layer level
    This is done to improve the performance of strip neighborOn functionnality
    For wires, pads, and neighborOn=false strips, the digits on each channel
    are ordered by earlier to latest and processed in order.
    The digits are merged according to the VMM merging time window.
    Above threshold digits are saved to output unless a previous digit is found
    within the deadtime window.
    --- For neighborOn=true strips ---
    A strip above threshold forces the VMM readout of neighbor strips, even if
    neighbor strips are below threshold.
    We apply the logic as above, but for strips below threshold we search for one
    direct neighbor strip to be above VMM threshold which triggers the VMM
    to read the strip digit.
  */
  std::map<Identifier, std::vector<sTgcSimDigitData> > processedDigits;
  // Loop through strip digits on each layer
  // unmergedStripDigits is a map of [layer ID , map[channelID, vector<sTGCSimDigitData ]   ]
  for (auto& it_LAYER : unmergedStripDigits) { // layer ID : map[channelID, vector<sTGCSimDigitData>]
    processedDigits = processDigitsWithVMM(ctx, m_deadtimeStrip, it_LAYER.second, thresholdData, m_doNeighborOn);

    // processedDigits is a map of <Identifier, pair< vector<sTgcDigit>, vector<MuonSimData> >
    // Need to save to output digits with identifier of the detectorElement
    // and not layer due to logic of how outputDigits if further processed.
    const Identifier& elemId = m_idHelperSvc->stgcIdHelper().elementID(it_LAYER.first);
    for (const auto& strip : processedDigits){
      for (const auto& simDigit : strip.second){
        sdoContainer->insert( std::make_pair(strip.first, simDigit.getSimData()) );
        outputDigits[elemId][strip.first].push_back(simDigit.getSTGCDigit());
      }
    }

  } // end of strip digit processing

  /*********************
  * Process Pad Digits *
  *********************/
  // Loop through pads for each layer
  // unmergedPadDigits is a map of [layer ID , map[channelID, vector<sTGCSimDigitData ]   ]
  for (auto& it_LAYER : unmergedPadDigits) { // layer ID : map[channelID, vector<sTGCSimDigitData>]
    processedDigits = processDigitsWithVMM(ctx, m_deadtimePad, it_LAYER.second, thresholdData, false);

    const Identifier& elemId = m_idHelperSvc->stgcIdHelper().elementID(it_LAYER.first);
    for (const auto& pad : processedDigits){
      for (const auto& simDigit : pad.second){
        sdoContainer->insert( std::make_pair(pad.first, simDigit.getSimData()) );
        outputDigits[elemId][pad.first].push_back(simDigit.getSTGCDigit());
      }
    }

  } // end of pad digit processing

  /*********************
  * Process Wire Digits *
  *********************/
  // Loop through wire groups for each layer
  // unmergedWireDigits is a map of [layer ID , map[channelID, vector<sTGCSimDigitData ]   ]
  for (auto& it_LAYER : unmergedWireDigits) { // layer ID : map[channelID, vector<sTGCSimDigitData>]
    processedDigits = processDigitsWithVMM(ctx, m_deadtimeWire, it_LAYER.second, thresholdData, false);

    const Identifier& elemId = m_idHelperSvc->stgcIdHelper().elementID(it_LAYER.first);
    for (const auto& wire : processedDigits){
      for (const auto& simDigit : wire.second){
        sdoContainer->insert( std::make_pair(wire.first, simDigit.getSimData()) );
        outputDigits[elemId][wire.first].push_back(simDigit.getSTGCDigit());
      }
    }

  } // end of wire digit processing

  /*************************************************
   * Output the digits to the StoreGate collection *
  *************************************************/
  for(std::map< Identifier, std::map< Identifier, std::vector<sTgcDigit> > >::iterator it_coll = outputDigits.begin(); it_coll != outputDigits.end(); ++it_coll){
    Identifier elemId = it_coll->first;
    IdentifierHash coll_hash;
    m_idHelperSvc->stgcIdHelper().get_module_hash(elemId, coll_hash);
    msg(MSG::VERBOSE) << "coll = "<< elemId << endmsg;
    auto digitCollection = std::make_unique<sTgcDigitCollection>(elemId, coll_hash);

    for(std::map< Identifier, std::vector<sTgcDigit> >::iterator it_REID = it_coll->second.begin(); it_REID != it_coll->second.end(); ++it_REID){
      for(std::vector< sTgcDigit >::iterator it_digit = it_REID->second.begin(); it_digit != it_REID->second.end(); ++it_digit){

  // apply the smearing before adding the digit
  bool acceptDigit = true;
  float chargeAfterSmearing(it_digit->charge());

  if ( m_doSmearing ) {
    ATH_CHECK(m_smearingTool->smearCharge(it_digit->identify(), chargeAfterSmearing, acceptDigit, rndmEngine) );
  }

  if ( acceptDigit ) {

    if ( m_idHelperSvc->stgcIdHelper().channelType(it_digit->identify()) == 1 ) {
      // Select strips with charge > 0.001 pC to avoid having zero ADC count when converting
      // charge [pC] to PDO [ADC count]
      if (chargeAfterSmearing < 0.001) continue;
    }

    std::unique_ptr<sTgcDigit> finalDigit = std::make_unique<sTgcDigit>(it_digit->identify(),
									      it_digit->bcTag(),
									      it_digit->time(),
									      chargeAfterSmearing,
									      it_digit->isDead(),
									      it_digit->isPileup());

	  ATH_MSG_VERBOSE("Final Digit") ;
	  ATH_MSG_VERBOSE(" BC tag = "    << finalDigit->bcTag()) ;
	  ATH_MSG_VERBOSE(" digitTime = " << finalDigit->time()) ;
	  ATH_MSG_VERBOSE(" charge = "    << finalDigit->charge()) ;
    digitCollection->push_back(std::move(finalDigit));
  }

      } // end of loop for all the digit object of the same ReadoutElementID

    } // end of loop for all the ReadoutElementID

    if (!digitCollection->empty()){
      const IdentifierHash hash = digitCollection->identifierHash();
      ATH_MSG_VERBOSE("Adding collection to m_digitcontainer : HashId = " << hash << " of size " << digitCollection->size());

      for(const sTgcDigit *digit : *digitCollection) {
        ATH_MSG_VERBOSE(" BC tag = "       << digit->bcTag()) ;
        ATH_MSG_VERBOSE(" digitTime = "    << digit->time()) ;
        ATH_MSG_VERBOSE(" charge_6bit = "  << digit->charge_6bit()) ;
        ATH_MSG_VERBOSE(" charge_10bit = " << digit->charge_10bit()) ;
      }

      if(digitContainer->addCollection(digitCollection.release(), hash).isFailure()) {
        ATH_MSG_WARNING("Failed to add collection with hash " << hash);
      }
    }
  }

  return StatusCode::SUCCESS;
}

/*******************************************************************************/
uint16_t sTgcDigitizationTool::bcTagging(const float digitTime, const int channelType) const {

  uint16_t bctag = 0;

  //double offset, window;
  if(channelType == 0) { //pads
    ATH_MSG_VERBOSE("Determining BC tag for pad channel");
  }
  else if(channelType == 1) { //strips
    ATH_MSG_VERBOSE("Determining BC tag for strip channel");
  }
  else if (channelType == 2) { // wire groups
    ATH_MSG_VERBOSE("Determining BC tag for wiregroup channel");
  }

   int bunchInteger;  //Define the absolute distance from t0 in units of BX
   if(digitTime > 0) bunchInteger = (int)(abs(digitTime/25.0));  //absolute bunch for future bunches
   else bunchInteger = (int)(abs(digitTime/25.0)) + 1; //The absolute bunch for negative time needs to be shifted by 1 as there is no negative zero bunch
   bctag = (bctag | bunchInteger);  //Store bitwise the abs(BX).  This should be equivalent to regular variable assignment
   if(digitTime < 0) bctag = ~bctag;  //If from a PREVIOUS BX, apply bitwise negation

  return bctag;
}

float sTgcDigitizationTool::getChannelThreshold(const EventContext& ctx, const Identifier& channelID, const NswCalibDbThresholdData* thresholdData) const {

  float threshold = m_chargeThreshold;
  float elecThrsld = 0;

  if(!thresholdData->getThreshold(channelID, elecThrsld))
    ATH_MSG_ERROR("Cannot find retrieve VMM threshold from conditions data base!");
  if(!m_calibTool->pdoToCharge(ctx, true, elecThrsld, channelID, threshold))
    ATH_MSG_ERROR("Cannot convert VMM charge threshold via conditions data!");

  return threshold;
}


CLHEP::HepRandomEngine* sTgcDigitizationTool::getRandomEngine(const std::string& streamName, const EventContext& ctx) const
{
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, streamName);
  std::string rngName = name()+streamName;
  rngWrapper->setSeed( rngName, ctx );
  CLHEP::HepRandomEngine* engine = rngWrapper->getEngine(ctx);
  ATH_MSG_VERBOSE(streamName<<" rngName "<<rngName<<" "<<engine);
  return engine;
}

std::map<Identifier, std::vector<sTgcSimDigitData> > sTgcDigitizationTool::processDigitsWithVMM(const EventContext& ctx, const float vmmDeadTime, const std::map<Identifier, std::vector<sTgcSimDigitData> >& inputLayerDigits, const NswCalibDbThresholdData* thresholdData, const bool isNeighborOn) const {

  std::map<Identifier, std::vector<sTgcSimDigitData> > inputChannels = inputLayerDigits;
  std::map<Identifier, std::vector<sTgcSimDigitData> > savedDigits;

  // Sort digits on every channel by earliest to latest time
  // Also do hit merging to help with neighborOn logic
  float threshold = m_chargeThreshold;
  for (auto& channel : inputChannels){
    std::stable_sort(channel.second.begin(), channel.second.end(), sort_digitsEarlyToLate);

    if(m_useCondThresholds)
      threshold = getChannelThreshold(ctx, channel.first, thresholdData);

    // merge digits in time. Do weighed average to find time of
    // digits originally below threshold. Follows what we expect from real VMM.
    unsigned int nDigits = channel.second.size();
    for (unsigned int i=0; i<nDigits; i++){
      sTgcDigit digit1 = channel.second[i].getSTGCDigit();
      float deltaT = 0.;
      float totalCharge = digit1.charge();
      float weightedTime = digit1.time();
      // loop through subsequent digits that are close in time.
      unsigned int j = i+1;
      while (j < nDigits && deltaT < m_hitTimeMergeThreshold){
        sTgcDigit digit2 = channel.second[j].getSTGCDigit();
        deltaT = digit2.time() - digit1.time();
        // If future digits are within window, digit1 absorbs its charge
        if (deltaT < m_hitTimeMergeThreshold){
          // If digit1 is not above threshold prior to merging, the new time is
          // a weighted average. Do it for every merging pair.
          weightedTime = (totalCharge >= threshold) ? weightedTime
            : (weightedTime * totalCharge + digit2.time() * digit2.charge())
            / (totalCharge + digit2.charge());
          totalCharge += digit2.charge();
        }
        j++;
      }
      digit1.set_charge(totalCharge);
      digit1.set_time(weightedTime);
      channel.second[i].setSTGCDigit(digit1);
    } // End of loops over the i digits

  } // end of time-ordering and hit merging loop

  for (const auto& channel : inputChannels){
    const Identifier& channelID = channel.first;
    int ChannelType = m_idHelperSvc->stgcIdHelper().channelType(channelID);

    // channel.second is a vector of time-order digits
    for (auto& simDigit : channel.second ) {
      sTgcDigit digit1 = simDigit.getSTGCDigit();

      // If the digit is within deadTimeWindow of last saved digit, skip digit
      auto it = savedDigits.find(channelID);
      if (it != savedDigits.end()){ // Have digits saved to VMM
        // By construction of the method, saved digits are time ordered.
        if (digit1.time() < it->second.back().getSTGCDigit().time()) ATH_MSG_WARNING("sTGC Digits are not time ordered like expected!");
        if (digit1.time() - it->second.back().getSTGCDigit().time() <= vmmDeadTime) continue;
      }

      float threshold = m_chargeThreshold;
      if(m_useCondThresholds)
        threshold = getChannelThreshold(ctx, channelID, thresholdData);

      if (digit1.charge() >= threshold)
        savedDigits[channelID].push_back(simDigit);

      else if (isNeighborOn && ChannelType == sTgcIdHelper::sTgcChannelTypes::Strip){

        int stripNumber = m_idHelperSvc->stgcIdHelper().channel(channelID);
        int maxStripNumber = m_mdManager->getsTgcReadoutElement(channelID)->numberOfStrips(channelID);
        float neighbor_threshold = m_chargeThreshold;

        int multiplet = m_idHelperSvc->stgcIdHelper().multilayer(channelID);
        int layer = m_idHelperSvc->stgcIdHelper().gasGap(channelID);

        bool neighborAboveThreshold = false;
        // Look at above strip if stripNumber != max
        if (stripNumber >= 1 && stripNumber < maxStripNumber){
          const Identifier& neighborID = m_idHelperSvc->stgcIdHelper().channelID(
            channelID, multiplet, layer, sTgcIdHelper::sTgcChannelTypes::Strip, stripNumber+1);

          // Neighbor must be above its own charge threshold
          if(m_useCondThresholds)
            neighbor_threshold = getChannelThreshold(ctx, neighborID, thresholdData);

          // If the neighbor has a strip digit within time window, trigger VMM to save
          neighborAboveThreshold = neighborStripAboveThreshold(digit1.time(), neighborID, neighbor_threshold, savedDigits, inputLayerDigits);
        }

        // Look at below strip if stripNumber != 1
        // Also no point in repeating if above strip is already trigging the VMM
        if (!neighborAboveThreshold && stripNumber > 1 && stripNumber <= maxStripNumber){ // Below strip neighbor
          // Get neighbor strip identifier
          const Identifier& neighborID = m_idHelperSvc->stgcIdHelper().channelID(
            channelID, multiplet, layer, sTgcIdHelper::sTgcChannelTypes::Strip, stripNumber-1);

          if(m_useCondThresholds)
            neighbor_threshold = getChannelThreshold(ctx, neighborID, thresholdData);

          // If the neighbor has a strip digit within time window, trigger VMM to save
          neighborAboveThreshold = neighborStripAboveThreshold(digit1.time(), neighborID, neighbor_threshold, savedDigits, inputLayerDigits);
        }

        if (neighborAboveThreshold)
          savedDigits[channelID].push_back(simDigit);
      }

    } // Looping through every digit on channel

  } // Looping through every channel on Layer

  return savedDigits;
}

bool sTgcDigitizationTool::neighborStripAboveThreshold(const float digitTime, const Identifier& neighborID, const float neighbor_threshold, const std::map<Identifier, std::vector<sTgcSimDigitData> >& savedDigits, const std::map<Identifier, std::vector<sTgcSimDigitData> >& layerStripDigits) const {


  auto it = savedDigits.find(neighborID);
  // If neighbor strip has saved output digits,
  // check if any saved digit is within time and above threshold
  if (it != savedDigits.end()) {
    for (const sTgcSimDigitData& simDigit : it->second){
      if (simDigit.getSTGCDigit().charge() >= neighbor_threshold)
        if (std::abs(digitTime - simDigit.getSTGCDigit().time()) <= m_hitTimeMergeThreshold)
          return true;
    }
    // If theres a digit, it means this strip has been processed already
    // so dont look at the unprocessed strips
    return false;
  }

  it = layerStripDigits.find(neighborID);
  if (it  != layerStripDigits.end()){
    // Strip was either not processed or all digits below threshold
    // Check if neighborstrip has any unsaved digits above threshold
    // and make sure the strips are not dead with simple logic
    float timeOfPreviousLiveDigit = -1000.;
    for (const sTgcSimDigitData& simDigit : it->second){
      if (simDigit.getSTGCDigit().charge() >= neighbor_threshold &&
          simDigit.getSTGCDigit().time() - timeOfPreviousLiveDigit >= m_deadtimeStrip){
        // above threshold strip not in a deadtime window
          if ( std::abs(digitTime - simDigit.getSTGCDigit().time()) <= m_hitTimeMergeThreshold)
            return true;
          timeOfPreviousLiveDigit = simDigit.getSTGCDigit().time();
      }
    }
  }
  return false;
}
