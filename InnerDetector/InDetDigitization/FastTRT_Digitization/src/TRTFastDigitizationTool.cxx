/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TRTFastDigitizationTool.cxx
//
//   Implementation file for class TRTFastDigitizationTool
//
///////////////////////////////////////////////////////////////////

#include "FastTRT_Digitization/TRTFastDigitizationTool.h"

#include "HitManagement/TimedHitCollection.h"
#include "InDetSimEvent/TRTUncompressedHitCollection.h"
#include "InDetSimEvent/TRTHitIdHelper.h"
#include "InDetSimData/InDetSimData.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "GeneratorObjects/HepMcParticleLink.h"

#include "GaudiKernel/SystemOfUnits.h"

// Det descr includes:
#include "InDetIdentifier/TRT_ID.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"

// Other includes
#include "PileUpTools/PileUpMergeSvc.h"

#include "TrkDetElementBase/TrkDetElementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "GeoPrimitives/GeoPrimitives.h"

// InDet stuff
#include "InDetPrepRawData/TRT_DriftCircle.h"

// CLHEP
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussZiggurat.h"

// Conditions data
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"

#include <memory>

// select the High threshold bits of TRT RDO words
static const unsigned int maskHT=0x04020100;

TRTFastDigitizationTool::TRTFastDigitizationTool( const std::string &type,
                                                  const std::string &name,
                                                  const IInterface *parent )
  : PileUpToolBase( type, name, parent )
{
}


StatusCode TRTFastDigitizationTool::initialize()
{
  ATH_MSG_DEBUG( "TRTFastDigitizationTool::initialize()" );

  // Get Random Service
  CHECK( m_rndmSvc.retrieve() );

  // Get the TRT Detector Manager
  CHECK( detStore()->retrieve( m_trt_manager, "TRT" ) );
  ATH_MSG_DEBUG( "Retrieved TRT_DetectorManager with version "  << m_trt_manager->getVersion().majorNum() );

  CHECK( detStore()->retrieve( m_trt_id, "TRT_ID" ) );

  // PileUp Merge Service
  CHECK( m_mergeSvc.retrieve() );

  // Argon / Xenon
  CHECK( m_trtStrawStatusSummaryTool.retrieve() );
  ATH_MSG_DEBUG( "Retrieved TRT_StrawStatusSummaryTool " << m_trtStrawStatusSummaryTool );

  // Check data object name
  if ( m_trtHitCollectionKey == "" ) {
    ATH_MSG_FATAL( "Property trtHitCollectionName not set!" );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG( "Input hits: " << m_trtHitCollectionKey );
  }

  ATH_CHECK(m_trtPrdTruthKey.initialize());
  ATH_CHECK(m_trtDriftCircleContainerKey.initialize());

  CHECK( m_trtDriftFunctionTool.retrieve() );

  if ( m_useTrtElectronPidTool ) {
    CHECK( m_trtElectronPidTool.retrieve() );
  }

  ATH_CHECK( m_EventInfoKey.initialize (m_useEventInfo) );

  StatusCode sc = initializeNumericalConstants();

  return sc;
}


StatusCode TRTFastDigitizationTool::prepareEvent(const EventContext& /*ctx*/, unsigned int )
{
  m_trtHitCollList.clear();
  m_thpctrt = new TimedHitCollection< TRTUncompressedHit >();
  m_HardScatterSplittingSkipper = false;

  StatusCode sc = initializeNumericalConstants();

  return sc;

}


StatusCode TRTFastDigitizationTool::processBunchXing( int bunchXing,
                                                      SubEventIterator bSubEvents,
                                                      SubEventIterator eSubEvents ) {

  // decide if this event will be processed depending on HardScatterSplittingMode & bunchXing
  if ( m_HardScatterSplittingMode == 2 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; return StatusCode::SUCCESS; }
  if ( m_HardScatterSplittingMode == 1 && m_HardScatterSplittingSkipper )  { return StatusCode::SUCCESS; }
  if ( m_HardScatterSplittingMode == 1 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; }

  using TimedHitCollList = PileUpMergeSvc::TimedList<TRTUncompressedHitCollection>::type;
  TimedHitCollList hitCollList;

  if (!(m_mergeSvc->retrieveSubSetEvtData(m_trtHitCollectionKey.value(), hitCollList, bunchXing,
                                          bSubEvents, eSubEvents).isSuccess()) &&
      hitCollList.empty()) {
    ATH_MSG_ERROR("Could not fill TimedHitCollList");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_VERBOSE(hitCollList.size() << " TRTUncompressedHitCollections with key " <<
                    m_trtHitCollectionKey << " found");
  }

  TimedHitCollList::iterator iColl(hitCollList.begin());
  TimedHitCollList::iterator endColl(hitCollList.end());

  for( ; iColl != endColl; ++iColl) {
    TRTUncompressedHitCollection *hitCollPtr = new TRTUncompressedHitCollection(*iColl->second);
    PileUpTimeEventIndex timeIndex(iColl->first);
    ATH_MSG_DEBUG("TRTUncompressedHitCollection found with " << hitCollPtr->size() <<
                  " hits");
    ATH_MSG_VERBOSE("time index info. time: " << timeIndex.time()
                    << " index: " << timeIndex.index()
                    << " type: " << timeIndex.type());
    m_thpctrt->insert(timeIndex, hitCollPtr);
    m_trtHitCollList.push_back(hitCollPtr);
  }

  return StatusCode::SUCCESS;
}


// initialize constants which are the same for all events
StatusCode TRTFastDigitizationTool::initializeNumericalConstants() {

  m_trtTailFraction = 4.7e-4;               // part of the fraction in Tails
  m_trtSigmaDriftRadiusTail = 4./sqrt(12.);   //  sigma of one TRT straw in R (Tail) [mm]

  return StatusCode::SUCCESS;
}

// set (pileup-dependent) numerical constants
StatusCode TRTFastDigitizationTool::setNumericalConstants() {

  // Efficiency and resolution dependence on  pileup
  // Resolution is parametrized with a double gaussian so there are two parameters (res1 = core, res2= tail)

  static const float eff_corr_pileup_dependence = -0.0005;   // variation of efficiency with the number of Xing
  static const float res1_corr_pileup_dependence = 0.005;   // variation of core resolution (fractional) with the number of Xing
  static const float res2_corr_pileup_dependence = 0.015;   // variation of tail resolution (fractional) with the number of Xing
  // scale factors relative to the value for mu=20
  float effcorr = 1+eff_corr_pileup_dependence*(m_NCollPerEvent-20);
  float res1corr =  1+res1_corr_pileup_dependence*(m_NCollPerEvent-20);
 float res2corr =  1+res2_corr_pileup_dependence*(m_NCollPerEvent-20);

  // Now the numerical parameters for efficiency and resolution
  static const float tailRes = 3.600;  // scale factor for tail resolution
  static const float coreFracEndcap_Xe = 0.40;  // fraction of events in resolution core (Xe)
  static const float coreFracEndcap_Ar = 0.40;  // fraction of events in resolution core (Ar)
  static const float coreFracBarrel_Xe = 0.250;  // fraction of events in resolution core (Xe)
  static const float coreFracBarrel_Ar = 0.250;  // fraction of events in resolution core (Ar)


  static const float eff_BarrelA_Xe = 0.840;     // efficiency scale factor
  static const float eff_EndcapA_Xe = 0.875;
  static const float eff_BarrelC_Xe = 0.833;
  static const float eff_EndcapC_Xe = 0.894;
  static const float eff_BarrelA_Ar = 0.933;
  static const float eff_EndcapA_Ar = 0.949;
  static const float eff_BarrelC_Ar = 0.937;
  static const float eff_EndcapC_Ar = 0.977;
  static const float err_Barrel_Xe = 0.997;     // scale factor for the error as returned by the drift function tool
  static const float err_Endcap_Xe = 1.065;
  static const float err_Barrel_Ar = 1.020;
  static const float err_Endcap_Ar = 1.040;

  static const float coreRes_Barrel_Xe = 0.4;  // scale factor for core resolution
  static const float coreRes_Endcap_Xe = 0.5;
  static const float coreRes_Barrel_Ar = 0.4;
  static const float coreRes_Endcap_Ar = 0.5;

  m_cFit[ 0 ][ 0 ] = effcorr*eff_BarrelA_Xe;   // Barrel A-side Xenon
  m_cFit[ 0 ][ 1 ] = err_Barrel_Xe;
  m_cFit[ 0 ][ 2 ] = coreFracBarrel_Xe;
  m_cFit[ 0 ][ 3 ] = res1corr*coreRes_Barrel_Xe;
  m_cFit[ 0 ][ 4 ] = res2corr*tailRes;
  m_cFit[ 1 ][ 0 ] = effcorr*eff_EndcapA_Xe;   // Endcap A-side Xenon
 m_cFit[ 1 ][ 1 ] = err_Endcap_Xe;
  m_cFit[ 1 ][ 2 ] = coreFracEndcap_Xe;
  m_cFit[ 1 ][ 3 ] = res1corr*coreRes_Endcap_Xe;
  m_cFit[ 1 ][ 4 ] = res2corr*tailRes;
  m_cFit[ 2 ][ 0 ] = effcorr*eff_BarrelC_Xe;   // Barrel C-side Xenon
  m_cFit[ 2 ][ 1 ] = err_Barrel_Xe;
  m_cFit[ 2 ][ 2 ] = coreFracBarrel_Xe;
  m_cFit[ 2 ][ 3 ] = res1corr*coreRes_Barrel_Xe;
  m_cFit[ 2 ][ 4 ] = res2corr*tailRes;
  m_cFit[ 3 ][ 0 ] = effcorr*eff_EndcapC_Xe;   // Endcap C-side Xenon
  m_cFit[ 3 ][ 1 ] = err_Endcap_Xe;
  m_cFit[ 3 ][ 2 ] = coreFracEndcap_Xe;
  m_cFit[ 3 ][ 3 ] = res1corr*coreRes_Endcap_Xe;
  m_cFit[ 3 ][ 4 ] = res2corr*tailRes;
  m_cFit[ 4 ][ 0 ] = effcorr*eff_BarrelA_Ar;   // Barrel A-side Argon
  m_cFit[ 4 ][ 1 ] = err_Barrel_Ar;
  m_cFit[ 4 ][ 2 ] = coreFracBarrel_Ar;
  m_cFit[ 4 ][ 3 ] = res1corr*coreRes_Barrel_Ar;
  m_cFit[ 4 ][ 4 ] = res2corr*tailRes;
  m_cFit[ 5 ][ 0 ] = effcorr*eff_EndcapA_Ar;   // Endcap A-side Argon
  m_cFit[ 5 ][ 1 ] = err_Endcap_Ar;
  m_cFit[ 5 ][ 2 ] = coreFracEndcap_Ar;
  m_cFit[ 5 ][ 3 ] = res1corr*coreRes_Endcap_Ar;
  m_cFit[ 5 ][ 4 ] = res2corr*tailRes;
  m_cFit[ 6 ][ 0 ] = effcorr*eff_BarrelC_Ar;   // Barrel C-side Argon
  m_cFit[ 6 ][ 1 ] = err_Barrel_Ar;
  m_cFit[ 6 ][ 2 ] = coreFracBarrel_Ar;
  m_cFit[ 6 ][ 3 ] = res1corr*coreRes_Barrel_Ar;
  m_cFit[ 6 ][ 4 ] = res2corr*tailRes;
  m_cFit[ 7 ][ 0 ] = effcorr*eff_EndcapC_Ar;   // Endcap C-side Argon
  m_cFit[ 7 ][ 1 ] = err_Endcap_Ar;
  m_cFit[ 7 ][ 2 ] = coreFracEndcap_Ar;
  m_cFit[ 7 ][ 3 ] = res1corr*coreRes_Endcap_Ar;
  m_cFit[ 7 ][ 4 ] = res2corr*tailRes;

   return StatusCode::SUCCESS;
}

StatusCode TRTFastDigitizationTool::produceDriftCircles(const EventContext& ctx,
                                                        CLHEP::HepRandomEngine* rndmEngine,
                                                        TimedHitCollection< TRTUncompressedHit >& thpctrt)
{
  // Create OUTPUT PRD_MultiTruthCollection for TRT measurements
  SG::WriteHandle< PRD_MultiTruthCollection > trtPrdTruth(m_trtPrdTruthKey, ctx);
  trtPrdTruth = std::make_unique< PRD_MultiTruthCollection >();
  if ( !trtPrdTruth.isValid() ) {
    ATH_MSG_FATAL( "Could not record collection " << trtPrdTruth.name() );
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG( "PRD_MultiTruthCollection " << trtPrdTruth.name() << " registered in StoreGate" );

  if(m_useEventInfo){

     SG::ReadHandle<xAOD::EventInfo> eventInfoContainer(m_EventInfoKey, ctx);
     if(eventInfoContainer.isValid()){
       m_NCollPerEvent = (float) eventInfoContainer->averageInteractionsPerCrossing();
     }
     else{
      ATH_MSG_INFO("Cannot retrieve event info");
     }
  }

  StatusCode sc = setNumericalConstants();
  if(sc != StatusCode::SUCCESS) return sc;

  m_driftCircleMap.clear();

  TimedHitCollection< TRTUncompressedHit >::const_iterator itr1, itr2;
  while ( thpctrt.nextDetectorElement( itr1, itr2 ) ) {

    for ( ; itr1 != itr2; ++itr1 ) {

      const TimedHitPtr< TRTUncompressedHit > &hit = *itr1;

      // Get hitID
      int hitID = hit->GetHitID();

      if ( hitID & 0xc0000000 ) {
        ATH_MSG_ERROR( "Hit ID not Valid (" << MSG::hex << hitID << ")" << MSG::dec );
        continue;
      }

      // Convert hitID to Identifier
      IdentifierHash hash;
      Identifier layer_id;
      bool status;
      Identifier straw_id = getIdentifier( hitID, hash, layer_id, status );
      if ( !status ) {
        ATH_MSG_ERROR( "Ignoring simhits with suspicious identifier (1)" );
        continue;
      }

      const InDetDD::TRT_BaseElement *trtBaseElement = m_trt_manager->getElement( hash );
      Identifier hit_id = trtBaseElement->identify();

      int BEC = m_trt_id->barrel_ec( hit_id );
      bool isArgon = isArgonStraw( straw_id );
      int idx = ( BEC > 0 ? BEC : 2 - BEC ) + 4 * isArgon - 1;

      // Get 'track-to-wire' distance
      double driftRadiusLoc = getDriftRadiusFromXYZ( hit );

      double efficiency = strawEfficiency( driftRadiusLoc, abs( BEC ) );
      efficiency *= m_cFit[ idx ][ 0 ];

      // Decide wether to throw away this cluster or not
      if ( CLHEP::RandFlat::shoot( rndmEngine ) < ( 1. - efficiency ) ) continue;

      // Decide core/tail fraction
      bool isTail = ( CLHEP::RandFlat::shoot( rndmEngine ) < m_trtTailFraction );

      double sigmaTrt = m_trtSigmaDriftRadiusTail;
      if ( !isTail ) {
        double driftTime = m_trtDriftFunctionTool->approxDriftTime( std::abs( driftRadiusLoc ) );
        sigmaTrt = m_trtDriftFunctionTool->errorOfDriftRadius( driftTime, hit_id, m_NCollPerEvent );
      }

      // driftRadiusLoc smearing procedure
      double dR = 0;
      int ii = 0;
      do {
        double tailSmearing = CLHEP::RandFlat::shoot( rndmEngine );
        dR = CLHEP::RandGaussZiggurat::shoot( rndmEngine, 0., ( tailSmearing < m_cFit[ idx ][ 2 ] ? m_cFit[ idx ][ 3 ] : m_cFit[ idx ][ 4 ] ) ) * sigmaTrt;
        ++ii;
        if ( ii > 50 ) {  // should not appear in simulation
          dR = 2. - driftRadiusLoc;
          break;
        }
      }
      while ( driftRadiusLoc + dR > 2. || driftRadiusLoc + dR < 0. );
      double smearedRadius = driftRadiusLoc + dR;

      Amg::Vector2D hitLocalPosition( smearedRadius, 0. );
      std::vector< Identifier > rdoList = { straw_id };

      auto hitErrorMatrix = Amg::MatrixX(1, 1);
      (hitErrorMatrix)(Trk::driftRadius, Trk::driftRadius) =
        sigmaTrt * sigmaTrt * m_cFit[idx][1] * m_cFit[idx][1];

      // the TRT word simulate only TR information for the moment
      // consult TRTElectronicsProcessing::EncodeDigit() in TRT_Digitization/src/TRTElectronicsProcessing.cxx
      unsigned int word = 0x00007c00;  // set to a standard low threshold hit: word = 0; for ( unsigned int j = 10; j < 15; ++j ) word += 1 << ( 25 - j - j / 8 );

      // set High Threshold bit
      HepGeom::Point3D< double > hitGlobalPosition = getGlobalPosition( hit );
      int particleEncoding = hit->GetParticleEncoding();
      float kineticEnergy = hit->GetKineticEnergy();

      if ( m_useTrtElectronPidTool ) {

        double position = ( std::abs(BEC) == 1 ? hitGlobalPosition.z() : hitGlobalPosition.perp() );

        double probability;
        if ( abs( particleEncoding ) == 11 && kineticEnergy > 5000. ) {  // electron
           probability = m_trtHighProbabilityBoostEle*getProbHT( particleEncoding, kineticEnergy, straw_id, smearedRadius, position);
        }
        else{
           probability = m_trtHighProbabilityBoostBkg*getProbHT( particleEncoding, kineticEnergy, straw_id, smearedRadius, position);
        }

        if ( CLHEP::RandFlat::shoot( rndmEngine ) < probability ) word |= maskHT;
      }
      else {

        double eta = hitGlobalPosition.pseudoRapidity();
        // double mass = particleMass( particle );
        // double p = sqrt( kineticEnergy * kineticEnergy + 2. * kineticEnergy * mass );
        float p = kineticEnergy;  // like in TRT_Digitization ( previously we also use zero mass due to bug in particleMass routine )

        if ( abs( particleEncoding ) == 11 && p > 5000. ) {  // electron
          double probability = ( p < 20000. ? HTProbabilityElectron_low_pt( eta ) : HTProbabilityElectron_high_pt( eta ) );
          if ( CLHEP::RandFlat::shoot( rndmEngine ) < probability ) word |= maskHT;
        }
        else if ( abs( particleEncoding ) == 13 || abs( particleEncoding ) > 100 ) {  // muon or other particle
          double probability = ( p < 20000. ? HTProbabilityMuon_5_20( eta ) : HTProbabilityMuon_60( eta ) );
          if ( CLHEP::RandFlat::shoot( rndmEngine ) < probability ) word |= maskHT;
        }

      }

      InDet::TRT_DriftCircle* trtDriftCircle =
        new InDet::TRT_DriftCircle(straw_id,
                                   hitLocalPosition,
                                   std::move(rdoList),
                                   std::move(hitErrorMatrix),
                                   trtBaseElement,
                                   word);
      if (!trtDriftCircle)
        continue;

      m_driftCircleMap.insert( std::multimap< Identifier, InDet::TRT_DriftCircle * >::value_type( straw_id, trtDriftCircle ) );

      if ( hit->particleLink().isValid() ) {
        if (!HepMC::ignoreTruthLink(hit->particleLink(), m_vetoPileUpTruthLinks)) {
          trtPrdTruth->insert( std::make_pair( trtDriftCircle->identify(), hit->particleLink() ) );
          ATH_MSG_DEBUG( "Truth map filled with cluster " << trtDriftCircle << " and link = " << hit->particleLink() );
        }
      }
      else {
        ATH_MSG_DEBUG( "Particle link NOT valid!! Truth map NOT filled with cluster " << trtDriftCircle << " and link = " << hit->particleLink() );
      }

    }
  }

  return StatusCode::SUCCESS;
}


StatusCode TRTFastDigitizationTool::processAllSubEvents(const EventContext& ctx) {

  ATH_MSG_DEBUG( "TRTFastDigitizationTool::processAllSubEvents()" );

  using HitCollectionTimedList = PileUpMergeSvc::TimedList<TRTUncompressedHitCollection>::type;

  HitCollectionTimedList hitCollectionTimedList;
  unsigned int numberOfSimHits = 0;
  if ( m_mergeSvc->retrieveSubEvtsData( m_trtHitCollectionKey.value(), hitCollectionTimedList, numberOfSimHits ).isFailure() && hitCollectionTimedList.empty() ) {
    ATH_MSG_ERROR( "Could not fill HitCollectionTimedList" );
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG( hitCollectionTimedList.size() << " TRTUncompressedHitCollections with key " << m_trtHitCollectionKey << " found" );
  }

  m_HardScatterSplittingSkipper = false;
  TimedHitCollection< TRTUncompressedHit > timedHitCollection( numberOfSimHits );
  for (auto & itr : hitCollectionTimedList) {
    // decide if this event will be processed depending on HardScatterSplittingMode & bunchXing
    if ( m_HardScatterSplittingMode == 2 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; continue; }
    if ( m_HardScatterSplittingMode == 1 && m_HardScatterSplittingSkipper )  { continue; }
    if ( m_HardScatterSplittingMode == 1 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; }
    timedHitCollection.insert( itr.first, static_cast< const TRTUncompressedHitCollection * >( itr.second ) );
  }

  // Set the RNG to use for this event.
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_randomEngineName);
  const std::string rngName = name()+m_randomEngineName;
  rngWrapper->setSeed( rngName, ctx );
  CLHEP::HepRandomEngine *rndmEngine = rngWrapper->getEngine(ctx);

  // Process the Hits straw by straw: get the iterator pairs for given straw
  CHECK( this->produceDriftCircles(ctx, rndmEngine, timedHitCollection ) );

  CHECK( this->createAndStoreRIOs(ctx, rndmEngine) );
  ATH_MSG_DEBUG ( "createAndStoreRIOs() succeeded" );

  return StatusCode::SUCCESS;
}


StatusCode TRTFastDigitizationTool::mergeEvent(const EventContext& ctx) {

  ATH_MSG_DEBUG( "TRTFastDigitizationTool::mergeEvent()" );

  // Set the RNG to use for this event.
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_randomEngineName);
  const std::string rngName = name()+m_randomEngineName;
  rngWrapper->setSeed( rngName, ctx );
  CLHEP::HepRandomEngine *rndmEngine = rngWrapper->getEngine(ctx);

  // Process the Hits straw by straw: get the iterator pairs for given straw
  if ( m_thpctrt != nullptr ) {
    CHECK( this->produceDriftCircles(ctx, rndmEngine, *m_thpctrt) );
  }

  // Clean up temporary containers
  delete m_thpctrt;
  for(TRTUncompressedHitCollection* ptr : m_trtHitCollList) delete ptr;
  m_trtHitCollList.clear();

  CHECK( this->createAndStoreRIOs(ctx, rndmEngine) );
  ATH_MSG_DEBUG ( "createAndStoreRIOs() succeeded" );

  return StatusCode::SUCCESS;
}


StatusCode TRTFastDigitizationTool::createAndStoreRIOs(const EventContext& ctx, CLHEP::HepRandomEngine* rndmEngine)
{
  // Create OUTPUT TRT_DriftCircleContainer and register it in StoreGate
  SG::WriteHandle<InDet::TRT_DriftCircleContainer > trtDriftCircleContainer(m_trtDriftCircleContainerKey, ctx);
  trtDriftCircleContainer = std::make_unique< InDet::TRT_DriftCircleContainer >( m_trt_id->straw_layer_hash_max() );
  if ( !trtDriftCircleContainer.isValid() ) {
    ATH_MSG_FATAL( "Could not create TRT_DriftCircleContainer" );
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG( "InDet::TRT_DriftCircleContainer " << trtDriftCircleContainer.name() << " registered in StoreGate" );

  using DriftCircleMapItr = std::multimap<Identifier, InDet::TRT_DriftCircle *>::iterator;
  using HashMapItr = std::multimap<IdentifierHash, InDet::TRT_DriftCircle *>::iterator;

  // empiric parameterization of the probability to merge to LT hits into a HT hit as a function of the number of collisions
  // TL - I first determined the value of highTRMergeProb which gives a good
  // agreeement with full simulation as being 0.04 for mu=10 and 0.12 for mu=60;
  // I decided to use the functional form a*pow(mu,b) to describe the mu
  // dependence; solving the 2x2 system gives a=0.01 and b=0.61.
  float highTRMergeProb = 0.012*pow(m_NCollPerEvent,0.61);

  std::multimap< IdentifierHash, InDet::TRT_DriftCircle * > idHashMap;

  for ( DriftCircleMapItr itr = m_driftCircleMap.begin() ; itr != m_driftCircleMap.end(); itr = m_driftCircleMap.upper_bound( itr->first ) ) {

    const Identifier trtid = itr->first;
    std::pair< DriftCircleMapItr, DriftCircleMapItr > hitsInOneStraw = m_driftCircleMap.equal_range( trtid );
    unsigned int numberOfHitsInOneStraw = m_driftCircleMap.count( itr->first );
    InDet::TRT_DriftCircle *trtDriftCircle = ( hitsInOneStraw.first )->second;
    IdentifierHash hash = trtDriftCircle->detectorElement()->identifyHash();

    // delete all driftCircles in TRT straw excert the first one, see ATLPHYSVAL-395
    bool isHT=false;
    for ( DriftCircleMapItr itr2 = ++( hitsInOneStraw.first ); itr2 != hitsInOneStraw.second; ++itr2 ) {
      InDet::TRT_DriftCircle *trtDriftCircle2 = itr2->second;
      if(trtDriftCircle2->getWord() & maskHT) isHT = true;
      delete trtDriftCircle2;
    }

  // set the word of the first hit to high threshold with some probability, unless any of the hits is HT already
    if( !(trtDriftCircle->getWord() & maskHT) && !isHT && numberOfHitsInOneStraw > 1) {
      unsigned int newword = 0;
      if(highTRMergeProb*(numberOfHitsInOneStraw-1) > CLHEP::RandFlat::shoot( rndmEngine )) newword += 1 << (26-9);
      const unsigned int newword2 = newword;
      const Amg::Vector2D locpos = trtDriftCircle->localPosition();
      const std::vector<Identifier> &rdolist = trtDriftCircle->rdoList();
      const InDetDD::TRT_BaseElement* detEl = trtDriftCircle->detectorElement();
      InDet::TRT_DriftCircle* trtDriftCircle2 = new InDet::TRT_DriftCircle(
        trtid,
        locpos,
        std::vector<Identifier>(rdolist),
        Amg::MatrixX(trtDriftCircle->localCovariance()),
        detEl,
        newword2);
      idHashMap.insert(
        std::multimap<IdentifierHash, InDet::TRT_DriftCircle*>::value_type(
          hash, trtDriftCircle2));
      delete trtDriftCircle;
    }
    else{
      idHashMap.insert( std::multimap<IdentifierHash, InDet::TRT_DriftCircle * >::value_type( hash, trtDriftCircle ) );
    }

  }

  for ( HashMapItr itr = idHashMap.begin(); itr != idHashMap.end(); itr = idHashMap.upper_bound( itr->first ) ) {

    std::pair< HashMapItr, HashMapItr > itrPair = idHashMap.equal_range( itr->first );

    const InDetDD::TRT_BaseElement *trtBaseElement = ( itrPair.first )->second->detectorElement();
    IdentifierHash hash = trtBaseElement->identifyHash();

    InDet::TRT_DriftCircleCollection *trtDriftCircleCollection = new InDet::TRT_DriftCircleCollection( hash );
    trtDriftCircleCollection->setIdentifier( trtBaseElement->identify() );

    for ( HashMapItr itr2 = itrPair.first; itr2 != itrPair.second; ++itr2 ) {
      InDet::TRT_DriftCircle *trtDriftCircle = itr2->second;
      trtDriftCircle->setHashAndIndex( trtDriftCircleCollection->identifyHash(), trtDriftCircleCollection->size() );
      trtDriftCircleCollection->push_back( trtDriftCircle );
    }

    if ( trtDriftCircleContainer->addCollection( trtDriftCircleCollection, hash ).isFailure() ) {
      ATH_MSG_WARNING( "Could not add collection to Identifyable container" );
    }
  }

  idHashMap.clear();
  m_driftCircleMap.clear();

  return StatusCode::SUCCESS;
}


double TRTFastDigitizationTool::getDriftRadiusFromXYZ( const TimedHitPtr< TRTUncompressedHit > &hit )
{
  HepGeom::Vector3D< double > vecEnter( hit->GetPreStepX(), hit->GetPreStepY(), hit->GetPreStepZ() );
  HepGeom::Vector3D< double > vecExit( hit->GetPostStepX(), hit->GetPostStepY(), hit->GetPostStepZ() );

  HepGeom::Vector3D< double > vecDir = vecExit - vecEnter;
  static const HepGeom::Vector3D< double > vecStraw( 0., 0., 1. );

  vecDir = vecDir.unit();

  double driftRadius = 0.;
  if ( std::abs( vecDir.x() ) < 1.0e-6 && std::abs( vecDir.y() ) < 1.0e-6 ) {
    driftRadius = vecEnter.perp();
  }
  else {
    double a = vecEnter.dot( vecStraw );
    double b = vecEnter.dot( vecDir );
    double c = vecDir.dot( vecStraw );

    double paramStraw = ( a - b*c ) / ( 1. - c*c );
    double paramTrack = -( b - a*c ) / ( 1. - c*c );

    HepGeom::Vector3D<double> vecClosestAppr = vecEnter + paramTrack * vecDir - paramStraw * vecStraw;
    driftRadius = vecClosestAppr.mag();
  }

  return driftRadius;
}


Identifier TRTFastDigitizationTool::getIdentifier( int hitID, IdentifierHash &hash, Identifier &layer_id, bool &status ) const
{
  status = true;

  Identifier straw_id;

  const int mask( 0x0000001F );
  const int word_shift( 5 );

  if ( hitID & 0x00200000 ) {  // endcap
    int strawID  = hitID & mask;
    int planeID  = ( hitID >> word_shift ) & mask;
    int sectorID = ( hitID >> 2 * word_shift ) & mask;
    int wheelID  = ( hitID >> 3 * word_shift ) & mask;
    int trtID    = ( hitID >> 4 * word_shift );

    // change trtID (which is 2/3 for endcaps) to use 0/1 in getEndcapElement
    trtID = ( trtID == 3 ? 0 : 1 );

    const InDetDD::TRT_EndcapElement *endcapElement = m_trt_manager->getEndcapElement( trtID, wheelID, planeID, sectorID );
    if ( endcapElement ) {
      hash = endcapElement->identifyHash();
      layer_id = endcapElement->identify();
      straw_id = m_trt_id->straw_id( layer_id, strawID );
    }
    else {
      ATH_MSG_ERROR( "Could not find detector element for endcap identifier with (ipos,iwheel,isector,iplane,istraw) = ("
                     << trtID << ", " << wheelID << ", " << sectorID << ", " << planeID << ", " << strawID << ")" << endmsg
                     << "If this happens very rarely, don't be alarmed (it is a Geant4 'feature')" << endmsg
                     << "If it happens a lot, you probably have misconfigured geometry in the sim. job." );
      status = false;
    }

  }
  else {  // barrel
    int strawID  = hitID & mask;
    int layerID  = ( hitID >> word_shift ) & mask;
    int moduleID = ( hitID >> 2 * word_shift ) & mask;
    int ringID   = ( hitID >> 3 * word_shift ) & mask;
    int trtID    = ( hitID >> 4 * word_shift );

    const InDetDD::TRT_BarrelElement *barrelElement = m_trt_manager->getBarrelElement( trtID, ringID, moduleID, layerID );
    if ( barrelElement ) {
      hash = barrelElement->identifyHash();
      layer_id = barrelElement->identify();
      straw_id = m_trt_id->straw_id( layer_id, strawID );
    } else {
      ATH_MSG_ERROR( "Could not find detector element for barrel identifier with (ipos,iring,imod,ilayer,istraw) = ("
                     << trtID << ", " << ringID << ", " << moduleID << ", " << layerID << ", " << strawID << ")" );
      status = false;
    }

  }

  return straw_id;
}


HepGeom::Point3D< double > TRTFastDigitizationTool::getGlobalPosition( const TimedHitPtr<TRTUncompressedHit> &hit )
{
  int hitID = hit->GetHitID();
  const HepGeom::Point3D< double > hitPreStep( hit->GetPreStepX(), hit->GetPreStepY(), hit->GetPreStepZ() );

  const int mask( 0x0000001F );
  const int word_shift( 5 );

  if ( hitID & 0x00200000 ) {  // endcap
    int strawID  = hitID & mask;
    int planeID  = ( hitID >> word_shift ) & mask;
    int sectorID = ( hitID >> 2 * word_shift ) & mask;
    int wheelID  = ( hitID >> 3 * word_shift ) & mask;
    int trtID    = ( hitID >> 4 * word_shift );

    // change trtID (which is 2/3 for endcaps) to use 0/1 in getEndcapElement
    trtID = ( trtID == 3 ? 0 : 1 );

    const InDetDD::TRT_EndcapElement *endcapElement = m_trt_manager->getEndcapElement( trtID, wheelID, planeID, sectorID );
    if ( endcapElement ) {
      return endcapElement->getAbsoluteTransform( strawID ) * hitPreStep;
    }

  }
  else {  // barrel
    int strawID  = hitID & mask;
    int layerID  = ( hitID >> word_shift ) & mask;
    int moduleID = ( hitID >> 2 * word_shift ) & mask;
    int ringID   = ( hitID >> 3 * word_shift ) & mask;
    int trtID    = ( hitID >> 4 * word_shift );

    const InDetDD::TRT_BarrelElement *barrelElement = m_trt_manager->getBarrelElement( trtID, ringID, moduleID, layerID );
    if ( barrelElement ) {
      return barrelElement->getAbsoluteTransform( strawID ) * hitPreStep;
    }

  }

  ATH_MSG_WARNING( "Could not find global coordinate of a straw - drifttime calculation will be inaccurate" );
  return { 0., 0., 0. };
}


bool TRTFastDigitizationTool::isArgonStraw( const Identifier &straw_id ) const
{
  // TRTCond::StrawStatus::Good == Xenon
  // return ( m_trtStrawStatusSummarySvc->getStatusHT( straw_id ) != TRTCond::StrawStatus::Good ? true : false );
  return ( gasType( straw_id ) == 1 );
}


int TRTFastDigitizationTool::gasType( const Identifier &straw_id ) const
{
  // getStatusHT returns enum EStatus { Undefined, Dead, Good, Xenon, Argon, Krypton } // from 20.7.1
  // see InnerDetector/InDetConditions/TRT_ConditionsData/TRT_ConditionsData/StrawStatus.h
  // TRT representation of gasType = Xenon: 0, Argon: 1, Krypton: 2

  int status = m_trtStrawStatusSummaryTool->getStatusHT( straw_id );

  if ( status == 2 || status == 3 )
    return 0;
  else if ( status == 1 || status == 4 )
    return 1;
  else if ( status == 5 )
    return 2;
  else {
    ATH_MSG_WARNING( "TRTFastDigitizationTool::gasType() getStatusHT = " << status << ", must be in [1..5] range" );
    return -1;
  }

}


double TRTFastDigitizationTool::getProbHT( int particleEncoding, float kineticEnergy, const Identifier &straw_id, double rTrkWire, double hitGlobalPosition ) const {

  Trk::ParticleHypothesis hypothesis = Trk::pion;

  switch( abs( particleEncoding ) ) {

    case 11:
    hypothesis = Trk::electron;
    break;

    case 13:
    hypothesis = Trk::muon;
    break;

    case 321:
    hypothesis = Trk::kaon;
    break;

    case 211:
    default:
    hypothesis = Trk::pion;
    break;

  } // end of switch

  float pTrk = sqrt( kineticEnergy * kineticEnergy + 2. * kineticEnergy * Trk::ParticleMasses::mass[ hypothesis ] );
  if ( pTrk < 250. || pTrk > 7000000. ) return 0.;

  int layerOrWheel = m_trt_id->layer_or_wheel( straw_id );
  int strawLayer = m_trt_id->straw_layer( straw_id );

  // trtPart = Barrel: 0, EndcapA: 1, EndcapB: 2
  int trtPart = 0;
  if ( abs( m_trt_id->barrel_ec( straw_id ) ) == 2 ) trtPart = ( ( layerOrWheel < 6 ) ? 1 : 2 );

  // strawLayer = Barrel: 0-72, Endcap A-side: 0-95 (16 layers in 6 modules), EndcapB: 0-63 (8 layers in 8 modules)
  if ( trtPart == 0 ) {       // Barrel
    if ( layerOrWheel ) strawLayer += 19 + ( layerOrWheel == 1 ? 0 : 24 );
  }
  else if ( trtPart == 1 ) {  // EndcapA
    strawLayer += 16 * layerOrWheel;
  }
  else {                      // EndcapB
    strawLayer += 8 * ( layerOrWheel - 6 );
  }

  const int strawLayerMax[] = { 72, 95, 63 };
  if ( strawLayer > strawLayerMax[ trtPart ] || strawLayer < 0 ) {
    ATH_MSG_WARNING( "strawLayer was outside allowed range: trtPart = " << trtPart << ", strawLayer = " << strawLayer );
    return 0.;
  }

  const double hitGlobalPositionMin[] = {   0.,  630.,  630. };
  const double hitGlobalPositionMax[] = { 720., 1030., 1030. };

  if ( std::abs(hitGlobalPosition) < hitGlobalPositionMin[ trtPart ] ) {
    ATH_MSG_WARNING( "hitGlobalPosition was below allowed range (will be adjusted): trtPart = " << trtPart << ", hitGlobalPosition = " << hitGlobalPosition );
    hitGlobalPosition = copysign(hitGlobalPositionMin[ trtPart ] + 0.001,hitGlobalPosition);
  }
  if ( std::abs(hitGlobalPosition) > hitGlobalPositionMax[ trtPart ] ) {
    ATH_MSG_WARNING( "hitGlobalPosition was above allowed range (will be adjusted): trtPart = " << trtPart << ", hitGlobalPosition = " << hitGlobalPosition );
    hitGlobalPosition = copysign(hitGlobalPositionMax[ trtPart ] - 0.001,hitGlobalPosition);
  }

  if ( rTrkWire > 2.2 ) rTrkWire = 2.175;

  double Occupancy = 0.11+0.014*m_NCollPerEvent;

  double probHT = m_trtElectronPidTool->probHTRun2( pTrk, hypothesis, trtPart, gasType( straw_id ), strawLayer, hitGlobalPosition, rTrkWire, Occupancy );
  if ( probHT == 0.5 || probHT == 1. ) probHT = 0.;
  if(hypothesis == Trk::electron) probHT *= 1.3;

  return probHT;
}


double TRTFastDigitizationTool::HTProbabilityElectron_high_pt( double eta )
{
  constexpr std::array< double, 14 > bins = { 0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.85, 1.05, 1.4, 1.5, 1.6, 1.82 };
  constexpr std::array< double, 15 > probability = { 0.210,             // [ 0.,   0.05 ]
                                                     0.218,             // [ 0.05, 0.15 ]
                                                     0.226,             // [ 0.15, 0.25 ]
                                                     0.234,             // [ 0.25, 0.35 ]
                                                     0.242,             // [ 0.35, 0.45 ]
                                                     0.250,             // [ 0.45, 0.55 ]
                                                     0.258,             // [ 0.55, 0.65 ]
                                                     0.266,             // [ 0.65, 0.75 ]
                                                     0.274,             // [ 0.75, 0.85 ]
                                                     0.280,             // [ 0.85, 1.05 ]
                                                     0.265,             // [ 1.05, 1.40 ]
                                                     0.275,             // [ 1.40, 1.50 ]
                                                     0.295,             // [ 1.50, 1.60 ]
                                                     0.330,             // [ 1.60, 1.82 ]
                                                     0.365              // > 1.82
                                                   };

  return probability[ std::distance( bins.begin(), std::lower_bound( bins.begin(), bins.end(), std::abs( eta ) ) ) ];
}


double TRTFastDigitizationTool::HTProbabilityElectron_low_pt( double eta )
{
  constexpr std::array< double, 14 > bins = { 0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.85, 1.05, 1.4, 1.5, 1.6, 1.82 };
  constexpr std::array< double, 15 > probability = { 0.210,             // [ 0.,   0.05 ]
                                                     0.218,             // [ 0.05, 0.15 ]
                                                     0.226,             // [ 0.15, 0.25 ]
                                                     0.234,             // [ 0.25, 0.35 ]
                                                     0.242,             // [ 0.35, 0.45 ]
                                                     0.250*0.88,        // [ 0.45, 0.55 ]
                                                     0.258*0.88,        // [ 0.55, 0.65 ]
                                                     0.266*0.88,        // [ 0.65, 0.75 ]
                                                     0.274*0.88,        // [ 0.75, 0.85 ]
                                                     0.280*0.88,        // [ 0.85, 1.05 ]
                                                     0.265*0.88,        // [ 1.05, 1.40 ]
                                                     0.275*0.88,        // [ 1.40, 1.50 ]
                                                     0.295*0.88,        // [ 1.50, 1.60 ]
                                                     0.330*0.88,        // [ 1.60, 1.82 ]
                                                     0.365              // > 1.82
                                                   };

  return probability[ std::distance( bins.begin(), std::lower_bound( bins.begin(), bins.end(), std::abs( eta ) ) ) ];
}


double TRTFastDigitizationTool::HTProbabilityMuon_5_20( double eta )
{
  constexpr std::array< double, 41 > bins = { -2.05, -1.95, -1.85, -1.75, -1.65, -1.55, -1.45, -1.35, -1.25, -1.15, -1.05, -0.95, -0.85,
                                              -0.75, -0.65, -0.55, -0.45, -0.35, -0.25, -0.15, -0.05, 0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65,
                                              0.75, 0.85, 0.95, 1.05, 1.15, 1.25, 1.35, 1.45, 1.55, 1.65, 1.75, 1.85, 1.95
                                            };

  constexpr std::array< double, 42 > probability = { 0.04466501,        // < -2.05
                                                     0.05624099,        // [ -2.05, -1.95 ]
                                                     0.05715916,        // [ -1.95, -1.85 ]
                                                     0.05758092,        // [ -1.85, -1.75 ]
                                                     0.05713670,        // [ -1.75, -1.65 ]
                                                     0.05320733,        // [ -1.65, -1.55 ]
                                                     0.05447418,        // [ -1.55, -1.45 ]
                                                     0.05460775,        // [ -1.45, -1.35 ]
                                                     0.05443998,        // [ -1.35, -1.25 ]
                                                     0.05671015,        // [ -1.25, -1.15 ]
                                                     0.06043842,        // [ -1.15, -1.05 ]
                                                     0.06098093,        // [ -1.05, -0.95 ]
                                                     0.06124813,        // [ -0.95, -0.85 ]
                                                     0.05757168,        // [ -0.85, -0.75 ]
                                                     0.05230566,        // [ -0.75, -0.65 ]
                                                     0.05136644,        // [ -0.65, -0.55 ]
                                                     0.05021782,        // [ -0.55, -0.45 ]
                                                     0.05046960,        // [ -0.45, -0.35 ]
                                                     0.04935652,        // [ -0.35, -0.25 ]
                                                     0.05074021,        // [ -0.25, -0.15 ]
                                                     0.04959613,        // [ -0.15, -0.05 ]
                                                     0.05090863,        // [ -0.05,  0.05 ]
                                                     0.05185448,        // [  0.05,  0.15 ]
                                                     0.05083610,        // [  0.15,  0.25 ]
                                                     0.05113032,        // [  0.25,  0.35 ]
                                                     0.05158703,        // [  0.35,  0.45 ]
                                                     0.05255587,        // [  0.45,  0.55 ]
                                                     0.05343067,        // [  0.55,  0.65 ]
                                                     0.05695859,        // [  0.65,  0.75 ]
                                                     0.06233243,        // [  0.75,  0.85 ]
                                                     0.06418306,        // [  0.85,  0.95 ]
                                                     0.06027916,        // [  0.95,  1.05 ]
                                                     0.05693816,        // [  1.05,  1.15 ]
                                                     0.05514142,        // [  1.15,  1.25 ]
                                                     0.05557067,        // [  1.25,  1.35 ]
                                                     0.05436613,        // [  1.35,  1.45 ]
                                                     0.05360627,        // [  1.45,  1.55 ]
                                                     0.05266918,        // [  1.55,  1.65 ]
                                                     0.05237728,        // [  1.65,  1.75 ]
                                                     0.05439599,        // [  1.75,  1.85 ]
                                                     0.05630533,        // [  1.85,  1.95 ]
                                                     0.06067052         // >  1.95
                                                   };

  return probability[ std::distance( bins.begin(), std::lower_bound( bins.begin(), bins.end(), eta ) ) ];
}


double TRTFastDigitizationTool::HTProbabilityMuon_60( double eta )
{
  constexpr std::array< double, 41 > bins = { -2.05, -1.95, -1.85, -1.75, -1.65, -1.55, -1.45, -1.35, -1.25, -1.15, -1.05, -0.95, -0.85,
                                              -0.75, -0.65, -0.55, -0.45, -0.35, -0.25, -0.15, -0.05, 0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65,
                                              0.75, 0.85, 0.95, 1.05, 1.15, 1.25, 1.35, 1.45, 1.55, 1.65, 1.75, 1.85, 1.95
                                            };

  constexpr std::array< double, 42 > probability = { 0.058,             // < -2.05
                                                     0.06316120,        // [ -2.05, -1.95 ]
                                                     0.06552401,        // [ -1.95, -1.85 ]
                                                     0.06462226,        // [ -1.85, -1.75 ]
                                                     0.06435722,        // [ -1.75, -1.65 ]
                                                     0.06404993,        // [ -1.65, -1.55 ]
                                                     0.06324595,        // [ -1.55, -1.45 ]
                                                     0.06318947,        // [ -1.45, -1.35 ]
                                                     0.06381197,        // [ -1.35, -1.25 ]
                                                     0.06366957,        // [ -1.25, -1.15 ]
                                                     0.06561866,        // [ -1.15, -1.05 ]
                                                     0.07307306,        // [ -1.05, -0.95 ]
                                                     0.07682944,        // [ -0.95, -0.85 ]
                                                     0.07430728,        // [ -0.85, -0.75 ]
                                                     0.06897150,        // [ -0.75, -0.65 ]
                                                     0.06393667,        // [ -0.65, -0.55 ]
                                                     0.06049334,        // [ -0.55, -0.45 ]
                                                     0.05774767,        // [ -0.45, -0.35 ]
                                                     0.05544898,        // [ -0.35, -0.25 ]
                                                     0.05456561,        // [ -0.25, -0.15 ]
                                                     0.05378204,        // [ -0.15, -0.05 ]
                                                     0.05196537,        // [ -0.05,  0.05 ]
                                                     0.05391259,        // [  0.05,  0.15 ]
                                                     0.05474811,        // [  0.15,  0.25 ]
                                                     0.05734638,        // [  0.25,  0.35 ]
                                                     0.05959219,        // [  0.35,  0.45 ]
                                                     0.06266565,        // [  0.45,  0.55 ]
                                                     0.06806432,        // [  0.55,  0.65 ]
                                                     0.07347304,        // [  0.65,  0.75 ]
                                                     0.07654586,        // [  0.75,  0.85 ]
                                                     0.07328693,        // [  0.85,  0.95 ]
                                                     0.06541597,        // [  0.95,  1.05 ]
                                                     0.06348016,        // [  1.05,  1.15 ]
                                                     0.06322222,        // [  1.15,  1.25 ]
                                                     0.06428555,        // [  1.25,  1.35 ]
                                                     0.06299531,        // [  1.35,  1.45 ]
                                                     0.06469499,        // [  1.45,  1.55 ]
                                                     0.06560785,        // [  1.55,  1.65 ]
                                                     0.06777871,        // [  1.65,  1.75 ]
                                                     0.06843851,        // [  1.75,  1.85 ]
                                                     0.06727150,        // [  1.85,  1.95 ]
                                                     0.05935051         // >  1.95
                                                   };

  return probability[ std::distance( bins.begin(), std::lower_bound( bins.begin(), bins.end(), eta ) ) ];
}


// parametrization of TRT efficiency as function of 'track-to-wire' distance
double TRTFastDigitizationTool::strawEfficiency( double driftRadius, int BEC )
{
  const double p[][ 5 ] = { { 0.478,    0.9386,  0.9325,  0.2509,   0.03232 }, // old parametrization
                            { 0.477001, 1.02865, 1.02910, 0.185082, 0. },      // Barrel
                            { 0.482528, 1.03601, 1.03693, 0.182581, 0. }       // EndCap
                          };

  const double &trtFitAmplitude = p[ BEC ][ 0 ];
  const double &trtFitMu = p[ BEC ][ 1 ];
  const double &trtFitR = p[ BEC ][ 2 ];
  const double &trtFitSigma = p[ BEC ][ 3 ];
  const double &trtFitConstant = p[ BEC ][ 4 ];

  double efficiency = trtFitAmplitude * ( erf( ( trtFitMu + trtFitR - driftRadius ) / ( std::sqrt( 2 ) * trtFitSigma ) )
                                                + erf( ( trtFitMu + trtFitR + driftRadius ) / ( std::sqrt( 2 ) * trtFitSigma ) )
                                                - erf( ( trtFitMu - trtFitR - driftRadius ) / ( std::sqrt( 2 ) * trtFitSigma ) )
                                                - erf( ( trtFitMu - trtFitR + driftRadius ) / ( std::sqrt( 2 ) * trtFitSigma ) )
                                            )
                      + trtFitConstant;

  return efficiency;

  /*
  static const std::vector< double > bins = { 0.05, 1.43, 1.48, 1.53, 1.58, 1.63, 1.68, 1.73, 1.78, 1.83, 1.88, 1.93, 1.99 };
  static const std::vector< double > efficiency = { 0.94,        // [ 0., 0.05 ]
                                                    0.96,        // [ 0.05, 1.43 ]
                                                    0.955,       // [ 1.43, 1.48 ]
                                                    0.95,        // [ 1.48, 1.53 ]
                                                    0.945,       // [ 1.53, 1.58 ]
                                                    0.94,        // [ 1.58, 1.63 ]
                                                    0.93,        // [ 1.63, 1.68 ]
                                                    0.92,        // [ 1.68, 1.73 ]
                                                    0.89,        // [ 1.73, 1.78 ]
                                                    0.86,        // [ 1.78, 1.83 ]
                                                    0.8,         // [ 1.83, 1.88 ]
                                                    0.74,        // [ 1.88, 1.93 ]
                                                    0.695,       // [ 1.93, 1.99 ]
                                                    0.65         // > 1.99
                                                  };

  return efficiency[ std::distance( bins.begin(), std::lower_bound( bins.begin(), bins.end(), driftRadius ) ) ];
  */
}


double TRTFastDigitizationTool::correctionHT( double momentum, Trk::ParticleHypothesis hypothesis )
{
  const double par[][ 6 ] = { { 5.96038,  0.797671,  1.28832, -2.02763, -2.24630, 21.6857  },  // pion
                              { 0.522755, 0.697029, -3.90787,  6.32952,  1.06347,  3.51847 }   // electron
                            };

  int j = ( hypothesis == Trk::electron ? 1 : 0 );
  double x0 = momentum * 1.e-3;
  double x1 = 1. / ( x0 + par[ j ][ 0 ] );
  double x2 = log( x0 ) * x1;
  double value = par[ j ][ 1 ] + par[ j ][ 2 ] * x1 + par[ j ][ 3 ] * x1 * x1 + par[ j ][ 4 ] * x2 + par[ j ][ 5 ] * x1 * x2;
  return ( momentum > 1500. ? value : 1. );
}


StatusCode TRTFastDigitizationTool::finalize()
{
  ATH_MSG_DEBUG( "TRTFastDigitizationTool::finalize()" );

  return StatusCode::SUCCESS;
}
