/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "AFP_Digitization/AFP_PileUpTool.h"
#include "AFP_SimEv/AFP_TDSimHit.h"
#include "AFP_SimEv/AFP_SIDSimHit.h"
#include "xAODForward/AFPSiHit.h"
#include "xAODForward/AFPToFHit.h"
#include "xAODForward/AFPToFHitAuxContainer.h"
#include "xAODForward/AFPSiHitAuxContainer.h"
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussQ.h"
#include "CLHEP/Random/RandPoissonQ.h"

AFP_PileUpTool::AFP_PileUpTool(const std::string& type,
                               const std::string& name,
                               const IInterface* parent) :
  PileUpToolBase(type, name, parent),
  m_totToChargeTransformation ("totToChargeTransformation", "1909 + x*363 + x*x*141"),
  m_ArrSize(645120)
{
}


StatusCode AFP_PileUpTool::initialize()
{
  ATH_MSG_INFO("AFP_PileUpTool::initialize() called");
  ATH_MSG_INFO("    CollectionEff: " << m_CollectionEff);
  
  // Setting up quantum efficincy of PMT (in 5 nm steps)
  setupQuantumEff();

  m_deposited_charge.resize(m_ArrSize, 0.f); // = 4 x 6 x 336 x 80
  m_deposited_energy.resize(m_ArrSize, 0.f);
  
  // Offsets for TOF TDC (to have the average TDC consistent between all trains and bars)
  for(int i=0; i<4; ++i){
    for(int j=0; j<4; ++j){
      for(int k=0; k<4; ++k){
      m_TDC_offsets[i][j][k] = 0.0;
      }
    }
  }
  setupTDCOffsets();
  
  m_totToChargeTransformation=TF1(m_totToChargeTransfName.toString().c_str(), m_totToChargeTransfExpr.toString().c_str());
  for(int i=0; i<16; ++i)
    m_ChargeVsTot_LUT[i] = m_totToChargeTransformation.Eval(i); 

  m_deposited_charge = std::vector<float>(m_ArrSize);
  m_deposited_energy = std::vector<float>(m_ArrSize);
    
  m_SignalVect = std::vector<double>( 2*static_cast<unsigned int>( m_TofSignalTimeRangeLength ) );
  for(unsigned int i=0; i<m_SignalVect.size(); ++i)
  {
    m_SignalVect[i] = SignalFun( i+0.5, m_RiseTime, m_FallTime);
  }
    
  for( int i=0; i<4; i++) {
    for( int j=0; j<4; j++) {
      for( int k=0; k<4; k++) {
        // NOTE the signal histograms and the methods processing those signals assume that 1 bin = 1 ps
        m_SignalHist[i][j][k] = TH1F(Form("m_SignalHist%d%d%d", i, j, k), "", m_TofSignalTimeRangeLength, 0, m_TofSignalTimeRangeLength);
      }
    }
  }

  ATH_CHECK(m_randomSvc.retrieve());
  ATH_MSG_DEBUG("Retrieved RandomNumber Service");

  if (m_onlyUseContainerName) {
    ATH_CHECK(m_mergeSvc.retrieve());
    ATH_MSG_DEBUG("Retrieved PileUpMergeSvc");
  }


  m_mergedTDSimHitList = AFP_TDSimHitCollection("mergedTDSimHitList");
  m_mergedSIDSimHitList = AFP_SIDSimHitCollection("mergedSIDSimHitList");

  // check the input object names
  if (m_TDSimHitCollectionKey.key().empty()) {
    ATH_MSG_FATAL("Property TDSimHitCollectionName not set !");
    return StatusCode::FAILURE;
  }
  if (m_SIDSimHitCollectionKey.key().empty()) {
    ATH_MSG_FATAL("Property SIDSimHitCollectionName not set !");
    return StatusCode::FAILURE;
  }

  // Initialize ReadHandleKeys
  ATH_CHECK(m_TDSimHitCollectionKey.initialize());
  ATH_CHECK(m_SIDSimHitCollectionKey.initialize());
  
  ATH_MSG_INFO("m_onlyUseContainerName = " <<m_onlyUseContainerName);
  if(m_onlyUseContainerName)
  {
    m_TDSimHitCollectionName = m_TDSimHitCollectionKey.key();
    ATH_MSG_INFO("Input TD SimHits in container : '" <<m_TDSimHitCollectionName  << "'");
    m_SIDSimHitCollectionName = m_SIDSimHitCollectionKey.key();
    ATH_MSG_INFO("Input SID SimHits in container : '" <<m_SIDSimHitCollectionName  << "'");
  }
  else
  {
    ATH_MSG_INFO("TD SimHits container key: " <<m_TDSimHitCollectionKey);
    ATH_MSG_INFO("SID SimHits container key: " <<m_SIDSimHitCollectionKey);
  }
  
  ATH_CHECK(m_TDDigiCollectionKey.initialize());
  ATH_CHECK(m_SiDigiCollectionKey.initialize());
  
  ATH_CHECK(m_AFPSiHitsContainerName.initialize());
  ATH_CHECK(m_AFPHitsContainerNameToF.initialize());
  
  return StatusCode::SUCCESS;
}


StatusCode AFP_PileUpTool::recoAll(const EventContext& ctx, std::unique_ptr<AFP_TDDigiCollection>& digitCollection, std::unique_ptr<AFP_SiDigiCollection>& siDigiCollection) const
{  
  ATH_CHECK( recoSiHits(ctx, siDigiCollection) );
  ATH_CHECK( recoToFHits(ctx, digitCollection) );
  
  return StatusCode::SUCCESS;
}


StatusCode AFP_PileUpTool::recoSiHits(const EventContext& ctx, std::unique_ptr<AFP_SiDigiCollection>& siDigiCollection) const
{
  auto afpSiHits=std::make_unique<xAOD::AFPSiHitContainer>();
  auto afpSiHitsAux=std::make_unique<xAOD::AFPSiHitAuxContainer>();
  afpSiHits->setStore(afpSiHitsAux.get());
  
  newXAODHitSi(afpSiHits, siDigiCollection);
    
  ATH_MSG_DEBUG("AFP_PileUpTool: after newXAODHitSi, simulated digi container size = "<<siDigiCollection->size()<<", got afpSiHits with size "<<afpSiHits->size());
  
  SG::WriteHandle<xAOD::AFPSiHitContainer> siHitContainer{m_AFPSiHitsContainerName, ctx};
  ATH_CHECK( siHitContainer.record(std::move(afpSiHits), std::move(afpSiHitsAux)) );
  
  return StatusCode::SUCCESS;
}


int AFP_PileUpTool::charge2tot(int ch) const
{
  int i = 0;
  do{
    if( ch < m_ChargeVsTot_LUT[i] ) break;
    else ++i;
  } while( i<16 );
  return i;
}


inline int AFP_PileUpTool::tot2charge(int tot) const
{
  return tot>0 ? static_cast<int>( m_totToChargeTransformation.Eval(tot) ) : 0;
}


void  AFP_PileUpTool::newXAODHitSi (std::unique_ptr<xAOD::AFPSiHitContainer>& siHitContainer, std::unique_ptr<AFP_SiDigiCollection>& container) const
{
  AFP_SiDigiConstIter it    = container->begin();
  AFP_SiDigiConstIter itend = container->end();
  
  for (; it != itend; ++it) {
    auto * xAODSiHit = siHitContainer->push_back(std::make_unique<xAOD::AFPSiHit>());
    
    xAODSiHit->setStationID(it->m_nStationID);
    xAODSiHit->setPixelLayerID(it->m_nDetectorID);
    xAODSiHit->setPixelColIDChip(80-it->m_nPixelRow); // Chip is rotated by 90 degree Row-->Col
    xAODSiHit->setPixelRowIDChip(336-it->m_nPixelCol); // Chip	is rotated by 90 degree	Col-->Row
    xAODSiHit->setDepositedCharge( it->m_fADC );
    int tot = charge2tot( it->m_fADC );
    tot = tot<16 ? tot : 16;
    xAODSiHit->setTimeOverThreshold( tot );
  }
}


StatusCode AFP_PileUpTool::recoToFHits(const EventContext& ctx, std::unique_ptr<AFP_TDDigiCollection>& digitCollection) const
{
  auto afpToFHits=std::make_unique<xAOD::AFPToFHitContainer>();
  auto afpToFHitsAux=std::make_unique<xAOD::AFPToFHitAuxContainer>();
  afpToFHits->setStore(afpToFHitsAux.get());
  
  newXAODHitToF(afpToFHits, digitCollection);
  
  ATH_MSG_DEBUG("AFP_PileUpTool: after recoToFHits, simulated TD digi container size = "<<digitCollection->size()<<", got afpToFHits with size "<<afpToFHits->size());
  
  SG::WriteHandle<xAOD::AFPToFHitContainer> ToFHitsContainer{m_AFPHitsContainerNameToF, ctx};
  ATH_CHECK(ToFHitsContainer.record(std::move(afpToFHits),std::move(afpToFHitsAux)));
  
  return StatusCode::SUCCESS;
}


void  AFP_PileUpTool::newXAODHitToF (std::unique_ptr<xAOD::AFPToFHitContainer>& tofHitContainer, std::unique_ptr<AFP_TDDigiCollection>& container) const
{
  AFP_TDDigiConstIter it    = container->begin();
  AFP_TDDigiConstIter itend = container->end();

  for (; it != itend; ++it) {
    auto * xAODToFHit = tofHitContainer->push_back(std::make_unique<xAOD::AFPToFHit>());
    xAODToFHit->setStationID(it->m_nStationID);
    xAODToFHit->setHptdcChannel(-1);
    xAODToFHit->setBarInTrainID(it->m_nDetectorID%10-1);
    xAODToFHit->setTrainID(it->m_nDetectorID/10-1);
    xAODToFHit->setHptdcID(-1);
    xAODToFHit->setPulseLength(it->m_fADC);
    xAODToFHit->setTime(it->m_fTDC);
  }
}


StatusCode AFP_PileUpTool::processAllSubEvents(const EventContext& ctx)
{
  ATH_MSG_DEBUG ( "AFP_PileUpTool::processAllSubEvents()" );

  using TimedTDSimHitCollList = PileUpMergeSvc::TimedList<AFP_TDSimHitCollection>::type;
  using TimedSIDSimHitCollList = PileUpMergeSvc::TimedList<AFP_SIDSimHitCollection>::type;

  TimedHitCollection<AFP_TDSimHit> thpcAFP_TDPmt;
  TimedHitCollection<AFP_SIDSimHit> thpcAFP_SiPmt;

  if (!m_onlyUseContainerName) {
    SG::ReadHandle<AFP_TDSimHitCollection> hitCollection(m_TDSimHitCollectionKey, ctx);
    if (!hitCollection.isValid()) {
      ATH_MSG_ERROR("Could not get AFP_TDSimHitCollection container " << hitCollection.name() << " from store " << hitCollection.store());
      return StatusCode::FAILURE;
    }

    // create a new hits collection
    thpcAFP_TDPmt = TimedHitCollection<AFP_TDSimHit>(1);
    thpcAFP_TDPmt.insert(0, hitCollection.cptr());
    ATH_MSG_DEBUG("AFP_TDSimHitCollection found with " << hitCollection->size() << " hits");
  }
  else {
    TimedTDSimHitCollList TDSimHitCollList;
    unsigned int numberOfTDSimHits{0};
    if (not (m_mergeSvc->retrieveSubEvtsData(m_TDSimHitCollectionName, TDSimHitCollList, numberOfTDSimHits).isSuccess()) and TDSimHitCollList.empty()) {
      ATH_MSG_FATAL ( "Could not fill TimedTDSimHitCollList" );
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG ( " PileUp: Merge " << TDSimHitCollList.size() << " AFP_TDSimHitCollections with key " << m_TDSimHitCollectionName << " found." );

    TimedTDSimHitCollList::iterator iColl  (TDSimHitCollList.begin());
    TimedTDSimHitCollList::iterator endColl(TDSimHitCollList.end());

    while (iColl != endColl) {
      const AFP_TDSimHitCollection* tmpColl(iColl->second);
      thpcAFP_TDPmt.insert(iColl->first, tmpColl);
      ATH_MSG_DEBUG ( " AFP_TDSimHitCollection found with " << tmpColl->size() << " hits " << iColl->first );
      ++iColl;
    }
  }

  if (!m_onlyUseContainerName) {
    SG::ReadHandle<AFP_SIDSimHitCollection> hitCollection(m_SIDSimHitCollectionKey, ctx);
    if (!hitCollection.isValid()) {
      ATH_MSG_ERROR("Could not get AFP_SIDSimHitCollection container " << hitCollection.name() << " from store " << hitCollection.store());
      return StatusCode::FAILURE;
    }

    // create a new hits collection
    thpcAFP_SiPmt = TimedHitCollection<AFP_SIDSimHit>(1);
    thpcAFP_SiPmt.insert(0, hitCollection.cptr());
    ATH_MSG_DEBUG("AFP_SIDSimHitCollection found with " << hitCollection->size() << " hits");
  }
  else {
    TimedSIDSimHitCollList SIDSimHitCollList;
    unsigned int numberOfSIDSimHits{0};
    if (not (m_mergeSvc->retrieveSubEvtsData(m_SIDSimHitCollectionName, SIDSimHitCollList, numberOfSIDSimHits).isSuccess()) and SIDSimHitCollList.empty()) {
      ATH_MSG_FATAL ( "Could not fill TimedSIDSimHitCollList" );
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG ( " PileUp: Merge " << SIDSimHitCollList.size() << " AFP_SIDSimHitCollections with key " << m_SIDSimHitCollectionName << " found." );

    TimedSIDSimHitCollList::iterator iSiColl  (SIDSimHitCollList.begin());
    TimedSIDSimHitCollList::iterator endSiColl(SIDSimHitCollList.end());

    while (iSiColl != endSiColl) {
      const AFP_SIDSimHitCollection* tmpSiColl(iSiColl->second);
      thpcAFP_SiPmt.insert(iSiColl->first, tmpSiColl);
      ATH_MSG_DEBUG ( " AFP_SIDSimHitCollection found with " << tmpSiColl->size() << " hits " << iSiColl->first );
      ++iSiColl;
    }
  }

  ATHRNG::RNGWrapper* rngWrapper = m_randomSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName, ctx );
  CLHEP::HepRandomEngine* rngEngine = rngWrapper->getEngine(ctx);
  
  std::unique_ptr<AFP_TDDigiCollection> digitCollection = std::make_unique<AFP_TDDigiCollection>();
  std::unique_ptr<AFP_SiDigiCollection> siDigiCollection = std::make_unique<AFP_SiDigiCollection>();
  
  ATH_CHECK(fillTDDigiCollection(thpcAFP_TDPmt, rngEngine, ctx, digitCollection));
  ATH_CHECK(fillSiDigiCollection(thpcAFP_SiPmt, ctx, siDigiCollection));

  ATH_CHECK( recoAll(ctx, digitCollection, siDigiCollection) );
  
  SG::WriteHandle<AFP_TDDigiCollection> digitWriteHandle{m_TDDigiCollectionKey, ctx};
  ATH_CHECK( digitWriteHandle.record(std::move(digitCollection)) );
  
  SG::WriteHandle<AFP_SiDigiCollection> siDigiWriteHandle{m_SiDigiCollectionKey, ctx};
  ATH_CHECK( siDigiWriteHandle.record(std::move(siDigiCollection)) );
  
  return StatusCode::SUCCESS; 
}


StatusCode AFP_PileUpTool::prepareEvent(const EventContext& /*ctx*/, const unsigned int nInputEvents)
{
  ATH_MSG_DEBUG ( "AFP_PileUpTool::prepareEvent() called for " << nInputEvents << " input events" );

  m_digitCollection = std::make_unique<AFP_TDDigiCollection>();
  m_mergedTDSimHitList.clear();

  m_SiDigiCollection = std::make_unique<AFP_SiDigiCollection>();
  m_mergedSIDSimHitList.clear();
  
  return StatusCode::SUCCESS;
}


StatusCode AFP_PileUpTool::processBunchXing(int bunchXing, SubEventIterator bSubEvents, SubEventIterator eSubEvents)
{
  ATH_MSG_DEBUG ( "AFP_PileUpTool::processBunchXing() " << bunchXing );
  SubEventIterator iEvt = bSubEvents;
  for (; iEvt!=eSubEvents; ++iEvt) {
    StoreGateSvc& seStore = *iEvt->ptr()->evtStore();
    ATH_MSG_VERBOSE ("SubEvt StoreGate " << seStore.name() << " :"
                     << " bunch crossing : " << bunchXing
                     << " time offset : " << iEvt->time()
                     << " event number : " << iEvt->ptr()->eventNumber()
                     << " run number : " << iEvt->ptr()->runNumber()
                     );

    const AFP_TDSimHitCollection* tmpColl = nullptr;

    if (!seStore.retrieve(tmpColl, m_TDSimHitCollectionName).isSuccess()) {
      ATH_MSG_ERROR ( "SubEvent AFP_TDSimHitCollection not found in StoreGate " << seStore.name() );
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG ( "AFP_TDSimHitCollection found with " << tmpColl->size() << " hits" );

    AFP_TDSimHitCollection::const_iterator iPmt = tmpColl->begin();
    AFP_TDSimHitCollection::const_iterator ePmt = tmpColl->end();

    for (; iPmt!=ePmt; ++iPmt) m_mergedTDSimHitList.push_back((*iPmt));

    const AFP_SIDSimHitCollection* tmpSiColl = nullptr;

    if (!seStore.retrieve(tmpSiColl, m_SIDSimHitCollectionName).isSuccess()) {
      ATH_MSG_ERROR ( "SubEvent AFP_SIDSimHitCollection not found in StoreGate " << seStore.name() );
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG ( "AFP_TDSimHitCollection found with " << tmpSiColl->size() << " hits" );

    AFP_SIDSimHitCollection::const_iterator iSiPmt = tmpSiColl->begin();
    AFP_SIDSimHitCollection::const_iterator eSiPmt = tmpSiColl->end();

    for (; iSiPmt!=eSiPmt; ++iSiPmt) m_mergedSIDSimHitList.push_back((*iSiPmt));
  }

  return StatusCode::SUCCESS;
}


StatusCode AFP_PileUpTool::mergeEvent(const EventContext& ctx)
{
  ATHRNG::RNGWrapper* rngWrapper = m_randomSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName, ctx );
  CLHEP::HepRandomEngine* rngEngine = rngWrapper->getEngine(ctx);
    
  ATH_CHECK(fillTDDigiCollection(m_mergedTDSimHitList, rngEngine, ctx));
  ATH_CHECK(fillSiDigiCollection(m_mergedSIDSimHitList, ctx));
  
  SG::WriteHandle<AFP_TDDigiCollection> digitWriteHandle{m_TDDigiCollectionKey, ctx};
  ATH_CHECK( digitWriteHandle.record(std::move(m_digitCollection)) );
  
  SG::WriteHandle<AFP_SiDigiCollection> siDigiWriteHandle{m_SiDigiCollectionKey, ctx};
  ATH_CHECK( siDigiWriteHandle.record(std::move(m_SiDigiCollection)) );
  
  return StatusCode::SUCCESS;
}


StatusCode AFP_PileUpTool::finalize()
{ 
  return StatusCode::SUCCESS;
}


void AFP_PileUpTool::resetSignalHistograms()
{
  for( int i=0; i<4; i++) {
    for( int j=0; j<4; j++) {
      for( int k=0; k<4; k++) {
        m_SignalHist[i][j][k].Reset();
      }
    }
  }
}


StatusCode AFP_PileUpTool::fillTDDigiCollection(TimedHitCollection<AFP_TDSimHit>& thpcAFP, CLHEP::HepRandomEngine* rndEngine, const EventContext& ctx, std::unique_ptr<AFP_TDDigiCollection>& digitCollection)
{
  resetSignalHistograms();
    
  TimedHitCollection<AFP_TDSimHit> thpc = thpcAFP;
  TimedHitCollection<AFP_TDSimHit>::const_iterator i, e, it;
    
  while (thpc.nextDetectorElement(i, e)) {
    for (it = i; it != e; ++it) {
      int Station     = (*it)->m_nStationID;
      int Detector    = (*it)->m_nDetectorID;
      int SensitiveElement = (*it)->m_nSensitiveElementID;
      float GlobalTime     =  (*it)->m_fGlobalTime;
      float WaveLength = (*it)->m_fWaveLength;

      if(SensitiveElement%2 == 1) createTDDigi(Station, Detector, SensitiveElement, GlobalTime, WaveLength, rndEngine);
    }
  }

  ATH_CHECK(StoreTDDigi(ctx, digitCollection));
  return StatusCode::SUCCESS;
}


StatusCode AFP_PileUpTool::fillTDDigiCollection(AFP_TDSimHitCollection& AFP_TDSimHitColl, CLHEP::HepRandomEngine* rndEngine, const EventContext& ctx)
{
  resetSignalHistograms();

  AFP_TDSimHitConstIter it    = AFP_TDSimHitColl.begin();
  AFP_TDSimHitConstIter itend = AFP_TDSimHitColl.end();

  for (; it != itend; ++it) {
    int Station     = it->m_nStationID;
    int Detector = it->m_nDetectorID;
    int SensitiveElement = it->m_nSensitiveElementID;
    float GlobalTime     =  it->m_fGlobalTime;
    float WaveLength = it->m_fWaveLength;

    if(SensitiveElement%2 == 1) createTDDigi(Station, Detector, SensitiveElement, GlobalTime, WaveLength, rndEngine);
  }
  
  ATH_CHECK(StoreTDDigi(ctx, m_digitCollection));

  return StatusCode::SUCCESS;
}


StatusCode AFP_PileUpTool::fillSiDigiCollection(TimedHitCollection<AFP_SIDSimHit>& thpcAFP, const EventContext& ctx, std::unique_ptr<AFP_SiDigiCollection>& siDigiCollection)
{
  TimedHitCollection<AFP_SIDSimHit> thpc = thpcAFP;
  TimedHitCollection<AFP_SIDSimHit>::const_iterator i, e, it;

  m_deposited_charge.assign(m_ArrSize, 0.f);  // here just 6 layers per detector are considered

  while (thpc.nextDetectorElement(i, e)) {
    for (it = i; it != e; ++it) {
      int Station     = (*it)->m_nStationID;
      int Detector    = (*it)->m_nDetectorID;
      int PixelRow    = (*it)->m_nPixelRow;
      int PixelCol    = (*it)->m_nPixelCol;
      float PreStepX  = (*it)->m_fPreStepX;
      float PreStepY  = (*it)->m_fPreStepY;
      float PreStepZ  = (*it)->m_fPreStepZ;
      float PostStepX = (*it)->m_fPostStepX;
      float PostStepY = (*it)->m_fPostStepY;
      float PostStepZ = (*it)->m_fPostStepZ;
      float DepEnergy = (*it)->m_fEnergyDeposit;

      createSiDigi(ctx, Station, Detector, PixelRow, PixelCol, PreStepX, PreStepY, PreStepZ, PostStepX, PostStepY, PostStepZ,  DepEnergy);
    }
  }
  ATH_CHECK(StoreSiDigi(ctx, siDigiCollection));
  return StatusCode::SUCCESS;
}


StatusCode AFP_PileUpTool::fillSiDigiCollection(AFP_SIDSimHitCollection& AFP_SIDSimHitColl, const EventContext& ctx)
{
  AFP_SIDSimHitConstIter it    = AFP_SIDSimHitColl.begin();
  AFP_SIDSimHitConstIter itend = AFP_SIDSimHitColl.end();

  m_deposited_charge.assign(m_ArrSize, 0.f);  // here just 6 layers per detector are considered

  for (; it != itend; ++it) {
    int Station     = it->m_nStationID;
    int Detector    = it->m_nDetectorID;
    int PixelRow    = it->m_nPixelRow;
    int PixelCol    = it->m_nPixelCol;
    float PreStepX  = it->m_fPreStepX;
    float PreStepY  = it->m_fPreStepY;
    float PreStepZ  = it->m_fPreStepZ;
    float PostStepX = it->m_fPostStepX;
    float PostStepY = it->m_fPostStepY;
    float PostStepZ = it->m_fPostStepZ;
    float DepEnergy = it->m_fEnergyDeposit;

  	createSiDigi(ctx, Station, Detector, PixelRow, PixelCol, PreStepX, PreStepY, PreStepZ, PostStepX, PostStepY, PostStepZ, DepEnergy);
  }
  ATH_CHECK(StoreSiDigi(ctx, m_SiDigiCollection));
  return StatusCode::SUCCESS;
}


bool AFP_PileUpTool::isPhotoelectronInduced(double lambda, CLHEP::HepRandomEngine* rndEngine) const
{
  double qEff = getQE( lambda );
  return CLHEP::RandFlat::shoot(rndEngine, 0.0, 1.0) < qEff*m_CollectionEff;
}


void AFP_PileUpTool::addPhotoconvTimeSmear(double & t, CLHEP::HepRandomEngine* rndEngine) const
{
  t += CLHEP::RandGaussQ::shoot(rndEngine, 5.* m_ConversionSpr, m_ConversionSpr);
}


void AFP_PileUpTool::createTDDigi(int Station, int Detector, int SensitiveElement, float GlobalTime, float WaveLength, CLHEP::HepRandomEngine* rndEngine)
{
  ATH_MSG_DEBUG ( " iterating Pmt " << Station << "  " << Detector << "  " << SensitiveElement << " " << GlobalTime << " " << WaveLength ); 

  if ( Station >3 || Station < 0 || Detector >49 || Detector < 10 || (SensitiveElement-1)/2>1 || (SensitiveElement-1)/2<0) {
    ATH_MSG_ERROR ( "Wrong station, detector or sensitive detector id" );
    return;
  }
    
  if( isPhotoelectronInduced(WaveLength, rndEngine) )
  {
    const int train = Detector/10 - 1;
    const int bar = Detector%10-1;
    //check index value against array extent
    if (train<0 or train >3 or bar<0 or bar>3){
      ATH_MSG_ERROR ( "Wrong train or bar; allowed values are 0-3, actual values "<<train<<", "<<bar);
      return;
    }
        
    double photoelectronTime = GlobalTime - m_TimeOffset - m_TDC_offsets[Station][train][bar];
    addPhotoconvTimeSmear( photoelectronTime, rndEngine );
    addSignalFunc( m_SignalHist[Station][train][bar], photoelectronTime );
  }
}


double AFP_PileUpTool::getTDC(const TH1F & hSignal) const
{
  const int nBinsDelay = static_cast<int>(m_CfSignalDelay); // assuming that 1 bin = 1 ps
  TH1F hSignal_delayed(hSignal);
  for(int l = hSignal.GetNbinsX(); l>0; l-- ){
    double val = l > nBinsDelay ? hSignal.GetBinContent(l-nBinsDelay) : 0;
    hSignal_delayed.SetBinContent(l, val);
  }
  TH1F hSignal_forTDC(hSignal);
  hSignal_forTDC.Add(&hSignal, &hSignal_delayed, -m_CfdThr, 1);
  
  const int bin = hSignal_forTDC.FindFirstBinAbove(0);
  double TDC = hSignal_forTDC.GetBinCenter( bin );
  if( bin-1 <= nBinsDelay ) // very erly signals 
    TDC = 0;
  return TDC;
}


double AFP_PileUpTool::getADC(const TH1F & hSignal, const double threshold) const
{
  int first = hSignal.FindFirstBinAbove(threshold);
  int last = first;
  while( hSignal.GetBinContent(++last) > threshold && last < hSignal.GetNbinsX() );
  double ADC = last-first;
  return ADC;
}


StatusCode AFP_PileUpTool::StoreTDDigi(const EventContext& /*ctx*/, std::unique_ptr<AFP_TDDigiCollection>& digitCollection) const
{  
  for( int i=0; i<4; i++) {
    for( int j=0; j<4; j++) {
      for( int k=0; k<4; k++){
                
        const TH1F & hSignal = m_SignalHist[i][j][k];
        const double peakVal = hSignal.GetBinContent( hSignal.GetMaximumBin() );
                
        if( peakVal > 2 /*signal from more than two photoel.*/ ){

          const double TDC = getTDC( hSignal );
          const double ADC = getADC( hSignal, 0.5*peakVal );
          
          AFP_TDDigi tddigi;
          tddigi.m_nStationID=i;
          tddigi.m_nDetectorID=10*(j+1)+k+1; // restoring original numeration of bars and trains
          tddigi.m_nSensitiveElementID=-1; // this variable is currently redundant
          tddigi.m_fADC=ADC;
          tddigi.m_fTDC=TDC;
          
          digitCollection->push_back(tddigi);
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}


double AFP_PileUpTool::generateSiCCE(CLHEP::HepRandomEngine* rndEngine) const
{
  double eff = CLHEP::RandGaussQ::shoot(rndEngine, m_SiT_ChargeCollEff, m_SiT_ChargeCollEffSigma);
  eff = eff>1?1:eff;
  eff = eff<0?0:eff;
  return eff;
}


void AFP_PileUpTool::createSiDigi(const EventContext& ctx, int Station, int Detector, int PixelRow, int PixelCol, float PreStepX, float PreStepY, float PreStepZ, float PostStepX, float PostStepY, float PostStepZ, float DepEnergy)
{
  ATH_MSG_DEBUG ( " iterating Pmt, station " << Station << ", detector " << Detector << ", pixel_col " << PixelCol << ", pixel_row " << PixelRow << ", dep_energy" << DepEnergy << " (x,y,z)_pre (" << PreStepX <<"," << PreStepY <<"," << PreStepZ <<"), (x,y,z)_post (" << PostStepX <<"," << PostStepY <<"," << PostStepZ <<")" ); 
    
  ATHRNG::RNGWrapper* rngWrapper = m_randomSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName, ctx );
  CLHEP::HepRandomEngine* rngEngine = rngWrapper->getEngine(ctx);
    
  double cce = generateSiCCE(rngEngine);
  double depositedCharge = DepEnergy * cce * m_SiT_Energy2ChargeFactor;
    
  ATH_MSG_DEBUG ( "deposted charge for given hit " << depositedCharge );
    
  if ((PixelCol>=336) || (PixelRow >= 80) || (Station >= 4) || (Detector >= 6 ) )
  {
    if (Detector == 11) 
    {
      ATH_MSG_DEBUG ( "Hit in the vacuum layer in front of the station " << Station );
    }
    else
    {
      ATH_MSG_WARNING ( "WRONG NUMBER of PIXEL coordinates or station or detector !!!:" );
      ATH_MSG_WARNING ( "station [max 4] " << Station << ", detector [max 6]" << Detector << ", pixel_col [max 336] " << PixelCol << ", pixel_row [max 80] " << PixelRow );
    }
    return;
  }   
    
  m_deposited_charge[6*336*80*Station + 80*336*Detector + 80*PixelCol + PixelRow] += depositedCharge;
  m_deposited_energy[6*336*80*Station + 80*336*Detector + 80*PixelCol + PixelRow] += DepEnergy;
}


inline double AFP_PileUpTool::generateSiNoise(CLHEP::HepRandomEngine* rndEngine) const
{
  return CLHEP::RandGaussQ::shoot(rndEngine, m_SiT_NoiseMu, m_SiT_NoiseSigma);
}


StatusCode AFP_PileUpTool::StoreSiDigi(const EventContext& ctx, std::unique_ptr<AFP_SiDigiCollection>& siDigiCollection)
{   
  ATHRNG::RNGWrapper* rngWrapper = m_randomSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName, ctx );
  CLHEP::HepRandomEngine* rngEngine = rngWrapper->getEngine(ctx);
  
  long index = 0;
  while ( index != m_ArrSize ) // here just 6 layers per detector are considered
  {
    // adding random noise
    m_deposited_charge[index] += generateSiNoise(rngEngine);
    
    int tot = charge2tot( m_deposited_charge[index] );

    if (tot >= m_SiT_ToTThresholdForHit )
    {
      ATH_MSG_DEBUG ( " total # of pairs from dep_energy (with all effects included) " << m_deposited_charge[index]);
      ATH_MSG_DEBUG ( " total # of pairs from dep_energy (true value)" << m_SiT_Energy2ChargeFactor*m_deposited_energy[index]);
      
      int station  = static_cast<int>(index/(80*336*6)); 
      int detector = static_cast<int>((index-station*80*336*6)/(80*336)); 
      int column   = static_cast<int>((index-station*80*336*6-detector*80*336)/80);
      int row    = static_cast<int>(index-station*80*336*6-detector*80*336-column*80);
      
      ATH_MSG_DEBUG ( " reversed mapping, station " << station << ", detector " << detector << ", pixel_col " << column << ", pixel_row " << row ); 
      
      AFP_SiDigi sidigi;

      sidigi.m_nStationID = station;
      sidigi.m_nDetectorID = detector;
      sidigi.m_nPixelCol = column;
      sidigi.m_nPixelRow = row;
      sidigi.m_fADC = tot2charge(tot);
      sidigi.m_fTDC = 0.;
      
      siDigiCollection->push_back(sidigi);
    }
    
    index++;
  }
  
  return StatusCode::SUCCESS;
}


void AFP_PileUpTool::addSignalFunc(TH1F & h, double x) const
{
  int xDiscrete = static_cast<int>(x);
  int iMin = xDiscrete;
  int iMax = h.GetNbinsX();
  if(xDiscrete<0)
  {
    iMin = 0;
    iMax = h.GetNbinsX() + xDiscrete - 1; 
  }
  for(int i = iMin; i<=iMax; ++i){
    h.SetBinContent(i, h.GetBinContent(i) + m_SignalVect[i-xDiscrete]);
  }
}


double AFP_PileUpTool::getQE(double lambda) const
{
  int id = (static_cast<int>(lambda)-200)/5;
  if(id > 81 || id < 0) return 0;
  return m_QuantumEff_PMT[id];
}


double AFP_PileUpTool::SignalFun(double Time, double RiseTime, double FallTime, double offset) const
{
  double f=0;
  Time -= offset;
  if ( Time < 0) return f;
  double p = (FallTime-RiseTime*TMath::Log(1.+FallTime/RiseTime))/TMath::Log(10.);
  f = TMath::Power(Time/p,RiseTime/p)*TMath::Exp(-(Time/p));
  f /= (TMath::Power(RiseTime/p,RiseTime/p)*TMath::Exp(-(RiseTime/p)));
  return f;
}


void AFP_PileUpTool::setupTDCOffsets()
{
  m_TDC_offsets[0][0][0] = -65.125366;
  m_TDC_offsets[0][0][1] = -78.942017;
  m_TDC_offsets[0][0][2] = -92.000610;
  m_TDC_offsets[0][0][3] = -87.115967;
  m_TDC_offsets[0][1][0] = 21.883667;
  m_TDC_offsets[0][1][1] = 10.356201;
  m_TDC_offsets[0][1][2] = 2.336792;
  m_TDC_offsets[0][1][3] = -9.625732;
  m_TDC_offsets[0][2][0] = 46.980957;
  m_TDC_offsets[0][2][1] = 45.204224;
  m_TDC_offsets[0][2][2] = 53.081421;
  m_TDC_offsets[0][2][3] = 43.045776;
  m_TDC_offsets[0][3][0] = 90.227905;
  m_TDC_offsets[0][3][1] = 84.472900;
  m_TDC_offsets[0][3][2] = 81.739990;
  m_TDC_offsets[0][3][3] = 74.882812;
  m_TDC_offsets[3][0][0] = -23.122681;
  m_TDC_offsets[3][0][1] = -16.655273;
  m_TDC_offsets[3][0][2] = -35.254150;
  m_TDC_offsets[3][0][3] = -27.525635;
  m_TDC_offsets[3][1][0] = 45.818359;
  m_TDC_offsets[3][1][1] = 46.052856;
  m_TDC_offsets[3][1][2] = 58.809570;
  m_TDC_offsets[3][1][3] = 49.068848;
  m_TDC_offsets[3][2][0] = 67.897339;
  m_TDC_offsets[3][2][1] = 78.327393;
  m_TDC_offsets[3][2][2] = 72.782471;
  m_TDC_offsets[3][2][3] = 69.975464;
  m_TDC_offsets[3][3][0] = 96.650635;
  m_TDC_offsets[3][3][1] = 97.994019;
  m_TDC_offsets[3][3][2] = 88.561279;
  m_TDC_offsets[3][3][3] = 79.530396;
}


void AFP_PileUpTool::setupQuantumEff()
{
  switch(m_QEffVer){
    case QE1:
    {
      // QE ver. 1
      m_QuantumEff_PMT[0]=0.035;
      m_QuantumEff_PMT[1]=0.047;
      m_QuantumEff_PMT[3]=0.064;
      m_QuantumEff_PMT[4]=0.084;
      m_QuantumEff_PMT[5]=0.101;
      m_QuantumEff_PMT[6]=0.114;
      m_QuantumEff_PMT[7]=0.122;
      m_QuantumEff_PMT[8]=0.128;
      m_QuantumEff_PMT[9]=0.132;
      m_QuantumEff_PMT[10]=0.134;
      m_QuantumEff_PMT[11]=0.135;
      m_QuantumEff_PMT[12]=0.138;
      m_QuantumEff_PMT[13]=0.142;
      m_QuantumEff_PMT[14]=0.146;
      m_QuantumEff_PMT[15]=0.151;
      m_QuantumEff_PMT[16]=0.158;
      m_QuantumEff_PMT[17]=0.164;
      m_QuantumEff_PMT[18]=0.171;
      m_QuantumEff_PMT[19]=0.178;
      m_QuantumEff_PMT[20]=0.185;
      m_QuantumEff_PMT[21]=0.194;
      m_QuantumEff_PMT[22]=0.203;
      m_QuantumEff_PMT[23]=0.211;
      m_QuantumEff_PMT[24]=0.217;
      m_QuantumEff_PMT[25]=0.224;
      m_QuantumEff_PMT[26]=0.229;
      m_QuantumEff_PMT[27]=0.232;
      m_QuantumEff_PMT[28]=0.235;
      m_QuantumEff_PMT[29]=0.237;
      m_QuantumEff_PMT[30]=0.240;
      m_QuantumEff_PMT[31]=0.242;
      m_QuantumEff_PMT[32]=0.244;
      m_QuantumEff_PMT[33]=0.248;
      m_QuantumEff_PMT[34]=0.250;
      m_QuantumEff_PMT[35]=0.253;
      m_QuantumEff_PMT[36]=0.256;
      m_QuantumEff_PMT[37]=0.258;
      m_QuantumEff_PMT[38]=0.260;
      m_QuantumEff_PMT[39]=0.262;
      m_QuantumEff_PMT[40]=0.262;
      m_QuantumEff_PMT[41]=0.261;
      m_QuantumEff_PMT[42]=0.257;
      m_QuantumEff_PMT[43]=0.253;
      m_QuantumEff_PMT[44]=0.250;
      m_QuantumEff_PMT[45]=0.245;
      m_QuantumEff_PMT[46]=0.240;
      m_QuantumEff_PMT[47]=0.235;
      m_QuantumEff_PMT[48]=0.230;
      m_QuantumEff_PMT[49]=0.225;
      m_QuantumEff_PMT[50]=0.221;
      m_QuantumEff_PMT[51]=0.213;
      m_QuantumEff_PMT[52]=0.203;
      m_QuantumEff_PMT[53]=0.191;
      m_QuantumEff_PMT[54]=0.179;
      m_QuantumEff_PMT[55]=0.169;
      m_QuantumEff_PMT[56]=0.161;
      m_QuantumEff_PMT[57]=0.154;
      m_QuantumEff_PMT[58]=0.147;
      m_QuantumEff_PMT[59]=0.141;
      m_QuantumEff_PMT[60]=0.138;
      m_QuantumEff_PMT[61]=0.134;
      m_QuantumEff_PMT[62]=0.129;
      m_QuantumEff_PMT[63]=0.119;
      m_QuantumEff_PMT[64]=0.103;
      m_QuantumEff_PMT[65]=0.086;
      m_QuantumEff_PMT[66]=0.072;
      m_QuantumEff_PMT[67]=0.062;
      m_QuantumEff_PMT[68]=0.055;
      m_QuantumEff_PMT[69]=0.049;
      m_QuantumEff_PMT[70]=0.045;
      m_QuantumEff_PMT[71]=0.041;
      m_QuantumEff_PMT[72]=0.037;
      m_QuantumEff_PMT[73]=0.034;
      m_QuantumEff_PMT[74]=0.031;
      m_QuantumEff_PMT[75]=0.028;
      m_QuantumEff_PMT[76]=0.025;
      m_QuantumEff_PMT[77]=0.022;
      m_QuantumEff_PMT[78]=0.020;
      m_QuantumEff_PMT[79]=0.017;
      m_QuantumEff_PMT[80]=0.015;
      m_QuantumEff_PMT[81]=0.013;
      break;
    }
    case QE2:
    {
      // QE ver. 2 
      m_QuantumEff_PMT[0]=0.042;
      m_QuantumEff_PMT[1]=0.056;
      m_QuantumEff_PMT[3]=0.075;
      m_QuantumEff_PMT[4]=0.095;
      m_QuantumEff_PMT[5]=0.110;
      m_QuantumEff_PMT[6]=0.119;
      m_QuantumEff_PMT[7]=0.123;
      m_QuantumEff_PMT[8]=0.125;
      m_QuantumEff_PMT[9]=0.126;
      m_QuantumEff_PMT[10]=0.125;
      m_QuantumEff_PMT[11]=0.125;
      m_QuantumEff_PMT[12]=0.126;
      m_QuantumEff_PMT[13]=0.130;
      m_QuantumEff_PMT[14]=0.133;
      m_QuantumEff_PMT[15]=0.137;
      m_QuantumEff_PMT[16]=0.143;
      m_QuantumEff_PMT[17]=0.148;
      m_QuantumEff_PMT[18]=0.153;
      m_QuantumEff_PMT[19]=0.159;
      m_QuantumEff_PMT[20]=0.163;
      m_QuantumEff_PMT[21]=0.170;
      m_QuantumEff_PMT[22]=0.179;
      m_QuantumEff_PMT[23]=0.187;
      m_QuantumEff_PMT[24]=0.196;
      m_QuantumEff_PMT[25]=0.204;
      m_QuantumEff_PMT[26]=0.210;
      m_QuantumEff_PMT[27]=0.215;
      m_QuantumEff_PMT[28]=0.220;
      m_QuantumEff_PMT[29]=0.225;
      m_QuantumEff_PMT[30]=0.229;
      m_QuantumEff_PMT[31]=0.232;
      m_QuantumEff_PMT[32]=0.235;
      m_QuantumEff_PMT[33]=0.239;
      m_QuantumEff_PMT[34]=0.243;
      m_QuantumEff_PMT[35]=0.247;
      m_QuantumEff_PMT[36]=0.251;
      m_QuantumEff_PMT[37]=0.252;
      m_QuantumEff_PMT[38]=0.255;
      m_QuantumEff_PMT[39]=0.257;
      m_QuantumEff_PMT[40]=0.259;
      m_QuantumEff_PMT[41]=0.260;
      m_QuantumEff_PMT[42]=0.260;
      m_QuantumEff_PMT[43]=0.261;
      m_QuantumEff_PMT[44]=0.261;
      m_QuantumEff_PMT[45]=0.260;
      m_QuantumEff_PMT[46]=0.258;
      m_QuantumEff_PMT[47]=0.256;
      m_QuantumEff_PMT[48]=0.252;
      m_QuantumEff_PMT[49]=0.249;
      m_QuantumEff_PMT[50]=0.245;
      m_QuantumEff_PMT[51]=0.241;
      m_QuantumEff_PMT[52]=0.236;
      m_QuantumEff_PMT[53]=0.229;
      m_QuantumEff_PMT[54]=0.222;
      m_QuantumEff_PMT[55]=0.213;
      m_QuantumEff_PMT[56]=0.206;
      m_QuantumEff_PMT[57]=0.199;
      m_QuantumEff_PMT[58]=0.193;
      m_QuantumEff_PMT[59]=0.186;
      m_QuantumEff_PMT[60]=0.181;
      m_QuantumEff_PMT[61]=0.177;
      m_QuantumEff_PMT[62]=0.173;
      m_QuantumEff_PMT[63]=0.165;
      m_QuantumEff_PMT[64]=0.150;
      m_QuantumEff_PMT[65]=0.129;
      m_QuantumEff_PMT[66]=0.108;
      m_QuantumEff_PMT[67]=0.092;
      m_QuantumEff_PMT[68]=0.081;
      m_QuantumEff_PMT[69]=0.073;
      m_QuantumEff_PMT[70]=0.066;
      m_QuantumEff_PMT[71]=0.060;
      m_QuantumEff_PMT[72]=0.055;
      m_QuantumEff_PMT[73]=0.050;
      m_QuantumEff_PMT[74]=0.046;
      m_QuantumEff_PMT[75]=0.041;
      m_QuantumEff_PMT[76]=0.037;
      m_QuantumEff_PMT[77]=0.033;
      m_QuantumEff_PMT[78]=0.029;
      m_QuantumEff_PMT[79]=0.025;
      m_QuantumEff_PMT[80]=0.022;
      m_QuantumEff_PMT[81]=0.019;
      break;
    }
    default: break;
  }
}
