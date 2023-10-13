/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>

#include "BCM_DigitizationTool.h"

#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/RNGWrapper.h"

// CLHEP includes
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandGaussZiggurat.h"

#include "GeneratorObjects/HepMcParticleLink.h"
#include "InDetBCM_RawData/BCM_RawData.h"
#include "InDetBCM_RawData/BCM_RDO_Collection.h"
#include "xAODEventInfo/EventInfo.h"             // NEW EDM
#include "xAODEventInfo/EventAuxInfo.h"          // NEW EDM

//----------------------------------------------------------------------
// Constructor with parameters:
//----------------------------------------------------------------------
BCM_DigitizationTool::BCM_DigitizationTool(const std::string &type, const std::string &name, const IInterface *parent) :
  PileUpToolBase(type,name,parent)
{
  declareProperty("ModNoise", m_modNoise, "RMS noise averaged over modules");
  declareProperty("ModSignal", m_modSignal, "Average MIP signal in modules");
  declareProperty("NinoThr", m_ninoThr, "NINO threshold voltage");
}

//----------------------------------------------------------------------
// Initialize method:
//----------------------------------------------------------------------
StatusCode BCM_DigitizationTool::initialize()
{
  ATH_MSG_VERBOSE ( "initialize()");

  if (m_onlyUseContainerName) {
    ATH_CHECK(m_mergeSvc.retrieve());
  }

  // get random service
  ATH_CHECK(m_rndmGenSvc.retrieve());

  // check the input object name
  if (m_hitsContainerKey.key().empty()) {
    ATH_MSG_FATAL("Property InputObjectName not set !");
    return StatusCode::FAILURE;
  }
  if(m_onlyUseContainerName) m_inputObjectName = m_hitsContainerKey.key();
  ATH_MSG_DEBUG("Input objects in container : '" << m_inputObjectName << "'");

  // Initialize ReadHandleKey
  ATH_CHECK(m_hitsContainerKey.initialize(true));

  // Write handle keys
  ATH_CHECK( m_outputKey.initialize() );
  ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputKey);
  ATH_CHECK( m_outputSDOKey.initialize() );
  ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputSDOKey);

  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------
// createOutpuContainers method:
//----------------------------------------------------------------------
StatusCode BCM_DigitizationTool::createOutputContainers(const EventContext& ctx)
{
  // Creating output RDO container
  SG::WriteHandle<BCM_RDO_Container> outputContainer(m_outputKey, ctx);
  ATH_CHECK(outputContainer.record(std::make_unique<BCM_RDO_Container>()));
  if (!outputContainer.isValid()) {
    ATH_MSG_ERROR("Could not record output BCM RDO container " << outputContainer.name() << " to store " << outputContainer.store());
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Recorded output BCM RDO container " << outputContainer.name() << " in store " << outputContainer.store());
  m_rdoContainer = outputContainer.ptr();

  // Creating output SDO collection container
  SG::WriteHandle<InDetSimDataCollection> outputSDOContainer(m_outputSDOKey, ctx);
  ATH_CHECK(outputSDOContainer.record(std::make_unique<InDetSimDataCollection>()));
  if (!outputSDOContainer.isValid()) {
    ATH_MSG_ERROR("Could not record output BCM SDO container " << outputSDOContainer.name() << " to store " << outputSDOContainer.store());
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Recorded output BCM SDO container " << outputSDOContainer.name() << " in store " << outputSDOContainer.store());
  m_simDataCollMap = outputSDOContainer.ptr();


  // Clear G4 hit info vectors
  for (unsigned int iMod=0; iMod<8; ++iMod) {
    m_enerVect[iMod].clear();
    m_timeVect[iMod].clear();
    m_depositVect[iMod].clear();
  }

  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------
// ProcessSiHit method:
//----------------------------------------------------------------------
void BCM_DigitizationTool::processSiHit(const SiHit &currentHit, double eventTime, unsigned int evtIndex, const EventContext& ctx)
{
  const int moduleNo = currentHit.getLayerDisk();
  const float enerDep = computeEnergy(currentHit.energyLoss(), currentHit.localStartPosition(), currentHit.localEndPosition());
  const float hitTime = currentHit.meanTime() + eventTime + m_timeDelay;
  // Fill vectors with hit info
  m_enerVect[moduleNo].push_back(enerDep);
  m_timeVect[moduleNo].push_back(hitTime);
  // Create new deposit and add to vector
  const EBC_EVCOLL evColl = EBC_MAINEVCOLL;
  const HepMcParticleLink::PositionFlag idxFlag = (evtIndex==0) ? HepMcParticleLink::IS_POSITION: HepMcParticleLink::IS_INDEX;
  const HepMcParticleLink particleLink{HepMcParticleLink(currentHit.trackNumber(), evtIndex, evColl, idxFlag, ctx)};
  const int barcode = particleLink.barcode();
  if (barcode == 0 || barcode == 10001){
    return;
  }
  m_depositVect[moduleNo].emplace_back(particleLink,currentHit.energyLoss());
}

//----------------------------------------------------------------------
// createRDOs method:
//----------------------------------------------------------------------
void BCM_DigitizationTool::createRDOsAndSDOs(const EventContext& ctx)
{
  // Set the RNG to use for this event.
  ATHRNG::RNGWrapper* rngWrapper = m_rndmGenSvc->getEngine(this);
  rngWrapper->setSeed( name(), ctx );

  // Digitize hit info and create RDO for each module
  for (int iMod=0; iMod<8; ++iMod) {
    if (!m_depositVect[iMod].empty()) m_simDataCollMap->emplace(Identifier(iMod), m_depositVect[iMod]);
    std::vector<float> analog = createAnalog(iMod,m_enerVect[iMod],m_timeVect[iMod]);
    addNoise(iMod,analog, rngWrapper->getEngine(ctx));
    for (int iGain=0; iGain<2; ++iGain) {
      std::bitset<64> digital = applyThreshold(iGain*8+iMod,analog);
      int p1x,p1w,p2x,p2w;
      applyFilter(digital);
      findPulses(digital,p1x,p1w,p2x,p2w);
      fillRDO(iGain*8+iMod,p1x,p1w,p2x,p2w);
    }
  }
  }

//----------------------------------------------------------------------
// PrepareEvent method:
//----------------------------------------------------------------------
StatusCode BCM_DigitizationTool::prepareEvent(const EventContext& ctx, unsigned int nInputEvents)
{
  ATH_MSG_DEBUG ( "prepareEvent() called for " << nInputEvents << " input events" );

  CHECK(createOutputContainers(ctx));

  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------//
// Digitize method:                                                     //
//----------------------------------------------------------------------//
StatusCode BCM_DigitizationTool::processAllSubEvents(const EventContext& ctx)
{
  ATH_MSG_DEBUG ( "processAllSubEvents()" );

  ATH_CHECK(createOutputContainers(ctx));

  // Fetch SiHitCollections for this bunch crossing
  if (!m_onlyUseContainerName) {
    SG::ReadHandle<SiHitCollection> hitCollection(m_hitsContainerKey, ctx);
    if (!hitCollection.isValid()) {
      ATH_MSG_ERROR("Could not get BCM SiHitCollection container " << hitCollection.name() <<
                    " from store " << hitCollection.store());
      return StatusCode::FAILURE;
    }
    const unsigned int evtIndex = 0;
    const double time = 0.0;
    ATH_MSG_DEBUG ( "SiHitCollection found with " << hitCollection->size() << " hits" );
    // Read hits from this collection
    for (const auto& siHit : *hitCollection) {
      processSiHit(siHit, time, evtIndex, ctx);
    }

  }
  else {
    using TimedHitCollList = PileUpMergeSvc::TimedList<SiHitCollection>::type;
    TimedHitCollList hitCollList;
    if (!(m_mergeSvc->retrieveSubEvtsData(m_inputObjectName, hitCollList).isSuccess()) && hitCollList.empty()) {
      ATH_MSG_ERROR ( "Could not fill TimedHitCollList" );
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_DEBUG ( hitCollList.size() << " SiHitCollections with key " << m_inputObjectName << " found" );
    }

    // Store hit info in vectors and fill SDO map
    TimedHitCollList::iterator iColl(hitCollList.begin());
    TimedHitCollList::iterator endColl(hitCollList.end());
    for (; iColl != endColl; ++iColl) {
      const SiHitCollection* tmpColl(iColl->second);
      const unsigned int evtIndex = (iColl->first).index();
      const double time = (iColl->first).time();
      ATH_MSG_DEBUG ( "SiHitCollection found with " << tmpColl->size() << " hits" );
      // Read hits from this collection
      for (const auto& siHit : *tmpColl) {
        processSiHit(siHit, time, evtIndex, ctx);
      }
    }
  }

  createRDOsAndSDOs(ctx);

  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------
// ProcessBunchXing method:
//----------------------------------------------------------------------
StatusCode BCM_DigitizationTool::processBunchXing(int bunchXing,
                                                  SubEventIterator bSubEvents,
                                                  SubEventIterator eSubEvents)
{
  const EventContext &ctx = Gaudi::Hive::currentContext();

  ATH_MSG_DEBUG ( "processBunchXing() " << bunchXing );

  SubEventIterator iEvt = bSubEvents;
  for (; iEvt!=eSubEvents; ++iEvt) {
    StoreGateSvc& seStore = *iEvt->ptr()->evtStore();
    ATH_MSG_VERBOSE ( "SubEvt StoreGate " << seStore.name() << " :"
                      << " bunch crossing : " << bunchXing
                      << " time offset : " << iEvt->time()
                      << " event number : " << iEvt->ptr()->eventNumber()
                      << " run number : " << iEvt->ptr()->runNumber()
                      );
    const SiHitCollection* seHitColl = nullptr;
    CHECK(seStore.retrieve(seHitColl,m_inputObjectName));
    ATH_MSG_DEBUG ( "SiHitCollection found with " << seHitColl->size() << " hits" );
    SiHitCollection::const_iterator i = seHitColl->begin();
    SiHitCollection::const_iterator e = seHitColl->end();
    // Read hits from this collection
    for (; i!=e; ++i) {
      processSiHit(*i, iEvt->time(), iEvt->index(), ctx);
    }
  }

  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------
// MergeEvent method:
//----------------------------------------------------------------------
StatusCode BCM_DigitizationTool::mergeEvent(const EventContext& ctx)
{
  ATH_MSG_DEBUG ( "mergeEvent()" );

  createRDOsAndSDOs(ctx);

  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------
// ComputeEnergy method:
//----------------------------------------------------------------------
float BCM_DigitizationTool::computeEnergy(float simEner, const HepGeom::Point3D<double>& startPos, const HepGeom::Point3D<double>& endPos)
{
  // Initialize output energy
  float calcEner = 0;
  // Break hit up into 10 discrete energy deposits
  // For each deposit, weight energy by charge collection efficiency based on position on diamond surface
  float xStep, yStep, rStep, r0Step, effStep;
  for (int iStep=0; iStep<10; ++iStep) {
    xStep = startPos.x()+iStep*(endPos.x()-startPos.x())/9;
    yStep = startPos.y()+iStep*(endPos.y()-startPos.y())/9;
    if (xStep==0 && yStep==0) effStep = 1.;
    else {
      rStep = sqrt(pow(xStep,2)+pow(yStep,2));
      r0Step = std::abs(yStep)>std::abs(xStep) ? rStep*m_effPrmDistance/std::abs(yStep) : rStep*m_effPrmDistance/std::abs(xStep);
      effStep = 1/(1+std::exp((rStep-r0Step)/m_effPrmSharpness));
    }
    calcEner+= 0.1*simEner*effStep;
  }
  return calcEner;
}

//----------------------------------------------------------------------
// CreateAnalog method:
//----------------------------------------------------------------------
std::vector<float> BCM_DigitizationTool::createAnalog(int iMod, std::vector<float> enerVect, std::vector<float> timeVect)
{
  std::vector<float> analog(64,0);
  for (unsigned int iHit=0; iHit<enerVect.size(); ++iHit) {
    float enerDep = enerVect.at(iHit);
    float hitTime = timeVect.at(iHit);
    int startBin = (int)(hitTime*64/25);
    float signalMax = enerDep*m_modSignal[iMod]/m_mipDeposit;
    float slopeup = signalMax/5; // pulse rise in 2ns ~ 5 bins @ 390ps/bin
    float slopedown = signalMax/10; // pulse fall in 3.8ns ~ 10 bins @ 390ps/bin
    int iBin = startBin-1;
    float signal = 0;
    if (iBin>=0 && startBin<64) {
      while (signal>=0 && iBin<64) {
        analog[iBin] += signal;
        signal += slopeup;
        if (iBin > startBin+4) signal -= slopeup+slopedown;
        iBin++;
      }
    }
  }
  return analog;
}

//----------------------------------------------------------------------
// AddNoise method:
//----------------------------------------------------------------------
void BCM_DigitizationTool::addNoise(int iMod, std::vector<float> &analog, CLHEP::HepRandomEngine* randomEngine)
{
  for (float & iBin : analog) iBin+=CLHEP::RandGaussZiggurat::shoot(randomEngine,0.,m_modNoise[iMod]);
}

//----------------------------------------------------------------------
// ApplyThreshold method:
//----------------------------------------------------------------------
std::bitset<64> BCM_DigitizationTool::applyThreshold(int iChan, const std::vector<float>& analog)
{
  std::bitset<64> digital;
  digital.reset();
  float factor = iChan<8 ? 1./13 : 12./13;
  for (int iBin=0; iBin<64; ++iBin)
    if (analog[iBin]*factor>m_ninoThr[iChan%8]) digital.set(iBin);
  return digital;
}

//----------------------------------------------------------------------
// ApplyFilter method:
//----------------------------------------------------------------------
void BCM_DigitizationTool::applyFilter(std::bitset<64> &digital)
{
  // 1101, 1011 -> 1111
  for (int iSamp=2; iSamp<63; iSamp++) {
    if (digital[iSamp-2] && digital[iSamp-1] && !digital[iSamp] && digital[iSamp+1]) digital[iSamp] = 1;
    if (digital[iSamp-2] && !digital[iSamp-1] && digital[iSamp] && digital[iSamp+1]) digital[iSamp-1] = 1;
  }
  // 010 -> 000
  if (digital[0] && !digital[1]) digital[0] = 0;
  for (int iSamp=1; iSamp<63; iSamp++) {
    if (!digital[iSamp-1] && digital[iSamp] && !digital[iSamp+1]) digital[iSamp] = 0;
  }
  if (!digital[62] && digital[63]) digital[63] = 0;
}

//----------------------------------------------------------------------
// FindPulses method:
//----------------------------------------------------------------------
void BCM_DigitizationTool::findPulses(const std::bitset<64>& digital, int &p1x, int &p1w, int &p2x, int &p2w)
{
  p1x = 0; p1w = 0; p2x = 0; p2w = 0;

  if (!digital.count()) return; // skip if waveform is empty

  bool p1done = false, p2done = false; bool ignorepulse = false;
  if (digital[0] && digital[1]) ignorepulse = true;
  else if (!digital[0] && digital[1] && digital[2]) p1x = 1;
  for (int iBin=2; iBin<63; iBin++) {
    if (!digital[iBin-2] && !digital[iBin-1] && digital[iBin] && digital[iBin+1]) { // rising edge
      if (!p1done && !p2done) p1x = iBin;
      else if (p1done && !p2done) p2x = iBin;
    }
    else if (digital[iBin-2] && digital[iBin-1] && !digital[iBin] && !digital[iBin+1]) { // falling edge
      if (!ignorepulse) {
        if (!p1done && !p2done) {
          p1w = iBin - p1x;
          p1done = true;
          if (p1w >= 32) {
            p2x = p1x + 31;
            p2w = p1w - 31;
            p2done = true;
            p1w = 31;
          }
        }
        else if (p1done && !p2done) {
          p2w = iBin - p2x;
          p2done = true;
          if (p2w >= 32) p2w = 31;
        }
      }
      else ignorepulse = false;
    }
  }
  if (digital[61] && digital[62] && !digital[63]) {
    if (!ignorepulse) {
      if (!p1done && !p2done) {
        p1w = 63 - p1x;
        p1done = true;
        if (p1w >= 32) {
          p2x = p1x + 31;
          p2w = p1w - 31;
          p2done = true;
          p1w = 31;
        }
      }
      else if (p1done && !p2done) {
        p2w = 63 - p2x;
        p2done = true;
        if (p2w >= 32) p2w = 31;
      }
    }
    else ignorepulse = false;
  }
  else if (digital[62] && digital[63]) {
    if (!ignorepulse) {
      if (!p1done && !p2done) {
        p1w = 1;
        p1done = true;
        if (64 - p1x >= 32) {
          p2x = p1x + 31;
          p2w = 1;
          p2done = true;
          p1w = 31;
        }
      }
      else if (p1done && !p2done) {
        p2w = 1;
        p2done = true;
        if (64 - p2x >= 32) p2w = 31;
      }
    }
    else ignorepulse = false;
  }
}

//----------------------------------------------------------------------
// FillRDO method:
//----------------------------------------------------------------------
void BCM_DigitizationTool::fillRDO(unsigned int chan, int p1x, int p1w, int p2x, int p2w)
{
  BCM_RDO_Collection *RDOColl = nullptr;
  // Check if collection for this channel already exists
  bool collExists = false;
  BCM_RDO_Container::iterator it_coll = m_rdoContainer->begin();
  BCM_RDO_Container::iterator it_collE= m_rdoContainer->end();
  for (; it_coll!=it_collE; ++it_coll) {
    if ((*it_coll)->getChannel()==chan) {
      collExists = true;
      RDOColl = *it_coll;
    }
  }
  // Create the RDO collection if needed
  if (!collExists) {
    RDOColl = new BCM_RDO_Collection(chan);
    // Add collection to container
    m_rdoContainer->push_back(RDOColl);
  }
  // Create RDO & add it to collection
  BCM_RawData *bcmrdo = new BCM_RawData(chan,p1x,p1w,p2x,p2w,0,0,0);
  if (bcmrdo) RDOColl->push_back(bcmrdo);
}
