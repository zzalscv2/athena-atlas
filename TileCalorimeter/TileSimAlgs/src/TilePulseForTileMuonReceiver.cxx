/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************************
//
//  Filename : TilePulseForTileMuonReceiver.cxx
//  Author   : Joao Gentil Mendes Saraiva (jmendes@cern.ch)
//  Created  : October 2013
//
//  DESCRIPTION:
//
//     The algorithm will do:
//     Create a digitized pulse with 7 samples based in a measured pulse.
//     The pulse is reconstructed using the matched filter alogtrithm.
//     Digits and reconstructed pulse parameters (e,t,qf) are saved in contaienrs in TES.
//
//     This is part of the tile-d project (2015).
//
//  HISTORY:
//
//     10.dec.2013 ready for submission in svn
//     16.jan.2014 fixed coverity errors
//
//     BUGS:
//          * scaleAmplitude for TileRawChannel objects is not recognized (line 523 is commented)
//          * output is still in adc counts
//
//****************************************************************************************

#include "GaudiKernel/ISvcLocator.h"
#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/IAtRndmGenSvc.h"

// Calo includes

#include "CaloIdentifier/TileID.h"

// Tile includes

#include "TileIdentifier/TileHWID.h"
#include "TileConditions/TileInfo.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileConditions/TileCablingService.h"
#include "TileConditions/TileCondToolEmscale.h"
#include "TileConditions/TileCondToolNoiseSample.h"
#include "TileConditions/TilePulseShapes.h"
#include "TileConditions/TileCondToolPulseShape.h"
#include "TileConditions/ITileBadChanTool.h"
#include "TileEvent/TileHitContainer.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileRawChannel.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileRecUtils/TileRawChannelBuilder.h"
#include "TileRecUtils/TileRawChannelBuilderMF.h"

#include "TileRecUtils/TileBeamInfoProvider.h"

#include "TileSimAlgs/TilePulseForTileMuonReceiver.h"

// external
#include "cmath"
#include <CLHEP/Random/Randomize.h>
#include <CLHEP/Units/SystemOfUnits.h>

using CLHEP::RandGaussQ;
using CLHEP::RandFlat;
using CLHEP::MeV;

// constructor
//
TilePulseForTileMuonReceiver::TilePulseForTileMuonReceiver(std::string name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
  , m_tileID(0)
  , m_tileHWID(0)
  , m_tileInfo(0)
  , m_cablingService(0)
  , nSamp(0)
  , iTrig(0)
  , adcMax(0)
  , tileThresh(0.0)
  , nShape(0)
  , nBinsPerX(0)
  , binTime0(0)
  , timeStep(0.0)
  , m_pHRengine(0)
  , m_rndmSvc("AtRndmGenSvc", name)
  , m_tileToolEmscale("TileCondToolEmscale")
  , m_tileToolNoiseSample("TileCondToolNoiseSample")
  , m_tileToolPulseShape("TileCondToolPulseShape")
  , m_tileBadChanTool("TileBadChanTool")
  , m_beamInfo("TileBeamInfoProvider/TileBeamInfoProvider") {
  // declare properties...

  declareProperty("TileHitContainer"               , m_hitContainer         = "TileHitCnt");
  declareProperty("MuonReceiverDigitsContainer"    , m_MuRcvDigitsContainer = "MuRcvDigitsCnt");
  declareProperty("MuonReceiverRawChannelContainer", m_MuRcvRawChContainer  = "MuRcvRawChCnt");
  declareProperty("TileInfoName"           , m_infoName             = "TileInfo");
  declareProperty("IntegerDigits"          , m_integerDigits        = false, "Round digits (default=false)");
  declareProperty("TileBadChanTool"        , m_tileBadChanTool);
  declareProperty("TileCondToolPulseShape" , m_tileToolPulseShape);
  declareProperty("MaskBadChannels"        , m_maskBadChannels      = false, "Remove channels tagged bad (default=false)");
  declareProperty("UseCoolPulseShapes"     , m_useCoolPulseShapes   = false, "Pulse shapes from database (default=false)");
  declareProperty("TileCondToolNoiseSample", m_tileToolNoiseSample);
  declareProperty("UseCoolNoise"           , m_tileNoise            = false, "Noise from database (default=false)");
  declareProperty("UseCoolPedestal"        , m_tilePedestal         = false, "Pedestal from database (default=false)");
  declareProperty("RndmSvc"                , m_rndmSvc, "Random Number Service used in TilePulseForTileMuonReceiver");
  declareProperty("TileCondToolEmscale"    , m_tileToolEmscale, "Service to calibrate all channels");
  declareProperty("TileRawChannelBuilderMF", m_MuRcvBuildTool, "The tool by default is the Matched Filter");
}

// destructor
//
TilePulseForTileMuonReceiver::~TilePulseForTileMuonReceiver() {
}

// initialize
//
StatusCode TilePulseForTileMuonReceiver::initialize() {

  //  ATH_MSG_INFO( "Initializing TilePulseForTileMuonReceiver" );
  //  Check cabling RUN>=RUN2 OK
  //
  m_cablingService = TileCablingService::getInstance();

  if (m_cablingService->getCablingType() != TileCablingService::RUN2Cabling) {
    ATH_MSG_INFO("TilePulseForTileMuonReceiver should not be used for RUN1 simulations");
    return StatusCode::SUCCESS;
  } else {
    ATH_MSG_INFO("Initializing TilePulseForTileMuonReceiver");
  }

  //=== retrieve TileID helper and TileInfo from det store
  CHECK(detStore()->retrieve(m_tileID));
  CHECK(detStore()->retrieve(m_tileHWID));
  CHECK(detStore()->retrieve(m_tileInfo, m_infoName));
  //=== get TileCondToolEmscale
  CHECK(m_tileToolEmscale.retrieve());
  //=== get TileCondToolNoiseSample
  CHECK(m_tileToolNoiseSample.retrieve());

  CHECK(m_MuRcvBuildTool.retrieve());

  nSamp = m_tileInfo->NdigitSamples();    // number of time slices for each chan
  iTrig = m_tileInfo->ItrigSample();      // index of the triggering time slice
  adcMax = m_tileInfo->ADCmax();           // adc saturation value
  tileThresh = m_tileInfo->ThresholdDigits(TileID::LOWGAIN);

  ATH_MSG_VERBOSE("Cabling Services: " << m_cablingService
                   << " Number of Samples: " << nSamp
                   << " Triggering tile slice: " << iTrig
                   << " ADC saturation value: " << adcMax
                   << " TileCal Threshold LOW GAIN: " << tileThresh);

  m_pHRengine = m_rndmSvc->GetEngine("Tile_PulseForTileMuonReceiver");

  nShape    = m_tileInfo->MuRcvNBins();
  nBinsPerX = m_tileInfo->MuRcvBinsPerX();
  binTime0  = m_tileInfo->MuRcvTime0Bin();
  timeStep  = 25.0 / nBinsPerX;

  ATH_MSG_INFO( "Pulse info : "
		<< "shape "<< nShape
                <<" nbins "<< nBinsPerX
                <<" time "<< binTime0
                <<" time step "<< timeStep
                <<" Triggering tile sample "<< iTrig);

  // Decrease by 1, now they are indexes of last element in a vector
  --nShape;

  if (m_useCoolPulseShapes) {
    ATH_MSG_INFO( "Using pulse from database.");

    CHECK(m_tileToolPulseShape.retrieve());
  } else {
    ATH_MSG_INFO( "Using pulse from TileInfo.");

    m_shapeMuonReceiver = m_tileInfo->MuRcvFullShape();
    m_shapeMuonReceiver.push_back(0.0);
  }

  if (m_maskBadChannels) CHECK(m_tileBadChanTool.retrieve());

  ATH_MSG_VERBOSE("TilePulseForTileMuonReceiver initialization completed");
  return StatusCode::SUCCESS;
}

// execute
//

StatusCode TilePulseForTileMuonReceiver::execute() {

  if (m_cablingService->getCablingType() != TileCablingService::RUN2Cabling) {
    ATH_MSG_VERBOSE( "ATT: RUN1 settings TilePulseForTileMuonReceiver will end now" );
    return StatusCode::SUCCESS;
  } else {
    ATH_MSG_VERBOSE( "ATT: RUN2 settings TilePulseForTileMuonReceiver will run now" );
    ATH_MSG_DEBUG( "Executing TilePulseForTileMuonReceiver" );
  }

  int Dchan[4];
  int jch = 0;
  int ind = 0;

  // PULSE

  // Random generators output arrays
  //
  double Rndm[16];
  double Rndm_dG[1];

  // Set of variables for management when fetching the pulse from COOL
  //
  int ishift;
  int n_hits;
  int k;
  float phase;
  float y;
  float dy;
  double shape;

  // Noise and pedestal from db
  //
  double pedSim = 0.;
  double sigma_Hfn1 = 0.;
  double sigma_Hfn2 = 0.;
  double sigma_Norm = 0.;
  double sigmaSim(0.0);

  // Measured parameters: noise, pedestal and calibration
  //
  double muRcv_NoiseSigma;
  double muRcv_Ped;
  double muRcv_Calib;
  double muRcv_Max;
  // double muRcv_Thresh;

  // Tile_Base_ID::pmt_id          ( int section, int side,
  //                              int module,   int tower,
  //                              int sample,   int pmt )       const

  for (int tower = 10; tower < 13; tower += 2) {
    //  for (int tower = 0; tower<16; tower+=2) {
    for (int pm = 0; pm < 2; pm++) {
      Identifier pmt_id = m_tileID->pmt_id(TileID::EXTBAR, 1, 0, tower, TileID::SAMP_D, pm);
      HWIdentifier ch_id = m_cablingService->s2h_channel_id(pmt_id);
      Dchan[ind] = m_tileHWID->channel(ch_id);
      ATH_MSG_VERBOSE( "Used channels " << ind
                       << " " << m_tileID->to_string(pmt_id)
                       << " " << m_tileHWID->to_string(ch_id)
                       << " " << Dchan[ind]);
      ind++;
    }
  }

  // Get hit container from TES
  //
  const TileHitContainer* hitCont;
  CHECK(evtStore()->retrieve(hitCont, m_hitContainer));

  // Set up buffers for handling information in a single collection.
  //
  IdentifierHash idhash;
  IdContext drawer_context = m_tileHWID->drawer_context();

  // Get a container for the digits
  //
  TileDigitsContainer* MuonReceiverDigitsContainer = new TileDigitsContainer(true);

  // Get a container for the raw channels
  //
  TileRawChannelContainer* MuonReceiverRawChannelContainer = new TileRawChannelContainer(true, TileFragHash::MF, TileRawChannelUnit::MegaElectronVolts, SG::VIEW_ELEMENTS);

  // Vector of digits to set into the container
  //
  std::vector<float> digitsBuffer(nSamp);

  // (a) iterate over all collections in the HIT container : access 'ros' and 'drawer'
  //
  TileHitContainer::const_iterator collItr = hitCont->begin();
  TileHitContainer::const_iterator lastColl = hitCont->end();

  for (; collItr != lastColl; ++collItr) {

    ATH_MSG_VERBOSE("(A.01)   Looping over all collections in the HIT container");

    // Get array of HWID's for this drawer (stored locally).
    //
    HWIdentifier drawer_id = m_tileHWID->drawer_id((*collItr)->identify());
    int ros = m_tileHWID->ros(drawer_id);
    int drawer = m_tileHWID->drawer(drawer_id);
    int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);

    // In here a first selection of the events to follow can be made by selection of the interesting ros
    //
    if (ros == 1 || ros == 2) continue;

    double pDigitSamplesArray[4][7];
    memset(pDigitSamplesArray, 0, sizeof(pDigitSamplesArray));

    ATH_MSG_VERBOSE("Going through collection ROS/DRAWER : "<< ros <<"/"<< drawer);

    if (m_cablingService->connected(ros, drawer)) {
      ATH_MSG_VERBOSE("(A.02)   ROS: "<< ros << " drawer: " << drawer << " is connected");
    } else {
      ATH_MSG_VERBOSE("(A.03)   ROS: "<< ros << " drawer: " << drawer << " is not connected");
      continue;
    }

    // Get drawer idhash for later access to the database to get ped and noi
    //
    m_tileHWID->get_hash(drawer_id, idhash, &drawer_context);

    // (a.1) Iterate over all hits in a collection : access 'channel'
    //
    TileHitCollection::const_iterator hitItr = (*collItr)->begin();
    TileHitCollection::const_iterator lastHit = (*collItr)->end();

    for (; hitItr != lastHit; ++hitItr) {

      n_hits = 0;

      ATH_MSG_VERBOSE("(B.00)   Iterate over all the hits in the ExtBar collection");

      // Get the pmt ID
      Identifier pmt_id = (*hitItr)->pmt_ID();

      if (m_tileID->section(pmt_id) != TileID::EXTBAR || m_tileID->sample(pmt_id) != TileID::SAMP_D)
        continue;

      int index = m_tileID->pmt(pmt_id) + m_tileID->tower(pmt_id) - 10;

      jch = Dchan[index];

      ATH_MSG_VERBOSE("   Test01: Channel " << jch << " has index " << index);

      if (ros == 3) {
        jch = Dchan[index];
      } else if (ros == 4) {
        //index = index + std::pow(-1, index);
        int sgn = (index&1) ? -1 : 1;
        index += sgn;
        jch = Dchan[index];
      }

      HWIdentifier adc_id = m_tileHWID->adc_id(drawer_id, jch, TileID::LOWGAIN);

      ATH_MSG_VERBOSE( "   Test02: Correct pmt being transported in Dchan[]: " << index << " " << Dchan[index]
                       << "=?" << m_tileHWID->channel(m_cablingService->s2h_channel_id(pmt_id))
                       << " then Ok. For reference get adc_id: " << m_tileHWID->to_string(adc_id, -1));

      double* pDigitSamples = pDigitSamplesArray[index];

      // Prepare to construct your pulse
      //
      ATH_MSG_DEBUG("   New hit in ROS/DRAWER/PMT " << ros << "/" << drawer << "/" << jch << " pmt_id " << m_tileID->to_string(pmt_id) <<" adc_id "<< m_tileHWID->to_string(adc_id,-1));

      // Scintillator Energy -> Cell Energy (uses sampling fraction)
      //
      double hit_calib = m_tileInfo->HitCalib(pmt_id);

      ATH_MSG_VERBOSE("(B.01)   Sampling fraction: " << hit_calib);

      // (a.2) Loop over the subhits of this channel
      //       Calibrations are applied per subhit and energy added per subhit of a channel
      //
      n_hits = (*hitItr)->size();

      ATH_MSG_VERBOSE("(B.02)   Number of sub-hits in channel: " << n_hits);

      for (int ihit = 0; ihit < n_hits; ++ihit) {

        ATH_MSG_VERBOSE("(C.00)   Iterating over the subhits of the channel : Sub-hit #" << ihit);

        double e_hit = (*hitItr)->energy(ihit); // [MeV] energy deposited in scintillator
        double e_pmt = e_hit * hit_calib;         // [MeV] true cell energy

        ATH_MSG_VERBOSE("(C.01)   Energy in scintillator [MeV]: " << e_hit
                         << " true cell energy [MeV]: " << e_pmt);

        // Need to pass the negative of t_hit, this is because the trigger returns the amplitude
        // at a given phase whereas the t_hit returns it from t=0 when the hit took place
        //

        double t_hit = (*hitItr)->time(ihit);
        ATH_MSG_VERBOSE("(C.02.01)   phase " << t_hit);

        // Load pulse
        //
        ishift = 0;
        k = 0;
        phase = 0;
        y = 0;
        dy = 0;
        shape = 0;

        ishift = (int) (t_hit / timeStep + 0.5);
        ATH_MSG_VERBOSE( "(C.02.02)   ishift :" << t_hit << "/" << timeStep << "+0.5 = " << ishift);

        if (m_useCoolPulseShapes) {

          for (int js = 0; js < nSamp; ++js) {
            k = binTime0 + (js - iTrig) * nBinsPerX - ishift;
            if (k < 0) k = 0;
            else if (k > nShape) k = nShape;
            ATH_MSG_VERBOSE( "(C.02.03)   k : " << binTime0 << "+(" << js << "-" << iTrig << ")*" << nBinsPerX <<  "-" << ishift << " = " << k);

            phase = (k - binTime0) * timeStep;
            ATH_MSG_VERBOSE( "(C.02.04)   phase : " << k << "-" << binTime0 << "*" << timeStep << " = " << phase);

            m_tileToolPulseShape->getPulseShapeYDY(drawerIdx, jch, TileID::LOWGAIN, phase, y, dy);
            shape = (double) y;
            pDigitSamples[js] += e_pmt * shape; // MeV
            ATH_MSG_VERBOSE( "(C.03.0) Sample no.= " << js
                            << " idx= " << k
                            << " Shape wt. = " << shape
                            << " Amp = " << pDigitSamples[js]
                            << "[MeV]  Energy: " << e_pmt << "[MeV] LOGAIN from COOL");

          } //END loop over samples

        } else {
          for (int js = 0; js < nSamp; ++js) {
            k = binTime0 + (js - iTrig) * nBinsPerX - ishift;
            if (k < 0) k = 0;
            else if (k > nShape) k = nShape;
            ATH_MSG_VERBOSE( "(C.02.03)   k : " << binTime0 << "+(" << js << "-" << iTrig << ")*" << nBinsPerX <<  "-" << ishift << " = " << k);

            pDigitSamples[js] += e_pmt * m_shapeMuonReceiver[k]; // MeV
            ATH_MSG_VERBOSE( "(C.03.0) Sample no.= " << js
                            << " idx= " << k
                            << " Shape wt. = " << m_shapeMuonReceiver[k]
                            << " Amp = " << pDigitSamples[js]
                            << "[MeV]  Energy: " << e_pmt << " LOGAIN from TileInfo");

          } //END loop over samples
        } // END if (m_useCoolPulseShapes)
      } // END loop over sub-HITS

      ATH_MSG_VERBOSE("(C.04)   ENDED Loop over sub hits");
      ATH_MSG_DEBUG("   Number of hits " << n_hits
                    << " channel " << m_tileHWID->to_string(drawer_id,-1) << "/" << Dchan[index]
                    << " digitized pulse [MeV] "<< pDigitSamples[0]
                    << "/" << pDigitSamples[1]
                    << "/" << pDigitSamples[2]
                    << "/" << pDigitSamples[3]
                    << "/" << pDigitSamples[4]
                    << "/" << pDigitSamples[5]
                    << "/" << pDigitSamples[6]);

    } // END loop over a HIT collection

    ATH_MSG_VERBOSE("(B.04)   ENDED Loop over a hit collection");

    // (a.3) The pulse has a shape and a amplitude in MeV now we convert it to ADC counts and add NOISE and PEDESTAL
    //       PEDESTAL [ADC counts] and NOISE [ADC counts] as stored in Tile Conditions (for NOW are fixed values)
    //       Keep containers for each module (each partition) the same size between events
    //

    for (int index = 0; index < 4; index++) {

      jch = Dchan[index];
      double* pDigitSamples = pDigitSamplesArray[index];

      HWIdentifier adc_id = m_tileHWID->adc_id(drawer_id, jch, TileID::LOWGAIN);

      if (index == 0)
        ATH_MSG_VERBOSE( "(D.00)   Add noise and pedestal for interesting channels (4 per module)"
                        << " in ROS (3/4): " << ros
                        << " drawer: " << drawer
                        << " drawer idx: " << drawerIdx
                        << " drawer_id: " << m_tileHWID->to_string(adc_id,-1));

      ATH_MSG_DEBUG("   Add noise and pedestal for : " << m_tileHWID->to_string(adc_id,-1));

      // different for each channel_id might be the case in the future (now a const. in TileInfoLoader.cxx)
      //
      muRcv_NoiseSigma = m_tileInfo->MuRcvNoiseSigma(adc_id); // [adc]
      // muRcv_Thresh = m_tileInfo->MuRcvThresh(adc_id);     // [adc] ... not used
      muRcv_Ped = m_tileInfo->MuRcvPed(adc_id);        // [adc]
      muRcv_Calib = m_tileInfo->MuRcvCalib(adc_id);      // pCb->[adc]
      muRcv_Max = m_tileInfo->MuRcvMax(adc_id);        // [adc]

      ATH_MSG_VERBOSE( "(D.01)   Tile Muon Receiver parameters:"
                      << " sig " << muRcv_NoiseSigma
                      << " noi " << muRcv_NoiseSigma
                      << " ped " << muRcv_Ped
                      << " cal " << muRcv_Calib
                      << " max " << muRcv_Max);

      // adc/pCb / MeV/pCb = adc/MeV
      //
      double mev2ADC_factor = muRcv_Calib / m_tileToolEmscale->channelCalib(drawerIdx, jch, TileID::LOWGAIN, 1.
                                                                            , TileRawChannelUnit::PicoCoulombs
                                                                            , TileRawChannelUnit::MegaElectronVolts);

      // Generate an array to randomize the noise for each digit
      //
      RandGaussQ::shootArray(m_pHRengine, nSamp, Rndm, 0.0, 1.0);

      ATH_MSG_VERBOSE( "(D.02)   Pulse digits [MeV]:"
                       << " " << pDigitSamples[0]
                       << " " << pDigitSamples[1]
                       << " " << pDigitSamples[2]
                       << " " << pDigitSamples[3]
                       << " " << pDigitSamples[4]
                       << " " << pDigitSamples[5]
                       << " " << pDigitSamples[6]
                       << " [All ZERO if there is no hit in channel.] ");

      ATH_MSG_VERBOSE( "(D.02.00)   Channel: " << ros << '/' << drawer << '/' << jch
                      << " adc/pCb: " << muRcv_Calib
                      << " Mev/pCb: " << m_tileToolEmscale->channelCalib(drawerIdx , jch , TileID::LOWGAIN , 1., TileRawChannelUnit::PicoCoulombs, TileRawChannelUnit::MegaElectronVolts)
                      << " final calibration factor adc/MeV: " << mev2ADC_factor);

      // Collecting pedestal from the database
      if (m_tilePedestal) {
        pedSim = m_tileToolNoiseSample->getPed(idhash, jch, TileID::LOWGAIN);
        // As in TileDigitsMaker bug fix for wrong ped value in DB
        if (pedSim == 0.0) pedSim = 30.;
      } else {
        pedSim = muRcv_Ped;
      }

      // Collecting noise from the database
      if (m_tileNoise) {
        RandFlat::shootArray(m_pHRengine, 1, Rndm_dG, 0.0, 1.0);

        sigma_Hfn1 = m_tileToolNoiseSample->getHfn1(idhash, jch, TileID::LOWGAIN);
        sigma_Hfn2 = m_tileToolNoiseSample->getHfn2(idhash, jch, TileID::LOWGAIN);

        if (sigma_Hfn1 > 0 || sigma_Hfn2) {
          sigma_Norm = sigma_Hfn1 / (sigma_Hfn1 + sigma_Hfn2 * m_tileToolNoiseSample->getHfnNorm(idhash, jch, TileID::LOWGAIN));
        } else {
          sigma_Hfn1 = m_tileToolNoiseSample->getHfn(idhash, jch, TileID::LOWGAIN);
          sigma_Norm = 1.;
        }

        if (Rndm_dG[0] < sigma_Norm) sigmaSim = sigma_Hfn1;
        else sigmaSim = sigma_Hfn2;

      } else {
        sigmaSim = muRcv_NoiseSigma;
      }

      // Loop over samples and either use noise and ped from db or user location (TileInfoLoader.cxx)
      for (int js = 0; js < nSamp; ++js) {

        ATH_MSG_VERBOSE("(D.02.0" << js << ")   sample " << js
                        << " E [MeV]: " << pDigitSamples[js]);

        digitsBuffer[js] = pDigitSamples[js] * mev2ADC_factor;

        ATH_MSG_VERBOSE( "(D.02.0" << js << ")   sample " << js
                         << " calibration Mev->adc "<< mev2ADC_factor
                         << "-> E [adc]: " << digitsBuffer[js]);
        // Pedestal (amp)
        digitsBuffer[js] += pedSim;
        ATH_MSG_VERBOSE("(D.02.0" << js << ")   sample " << js
                        << " adding pedestal " << pedSim
                        << "-> E [adc]: "<< digitsBuffer[js]);

        // Noise (rms)
        digitsBuffer[js] += sigmaSim * Rndm[js];
        ATH_MSG_VERBOSE( "(D.02.0" << js << ")   sample " << js
                        << " adding noise " << sigmaSim * Rndm[js]
                        << "-> E [adc]: " << digitsBuffer[js]);

        // simulated pulse above allowed maximum
        //
        if (digitsBuffer[js] > muRcv_Max) digitsBuffer[js] = muRcv_Max;
        // rounding the ADC counts
        //
        if (m_integerDigits) digitsBuffer[js] = round(digitsBuffer[js]);
      }

      // If channel is good, create TileDigits object and store in container.
      //
      bool chanIsBad = false;

      if (m_maskBadChannels) {
        TileBchStatus status = m_tileBadChanTool->getAdcStatus(drawerIdx, jch, TileID::LOWGAIN);
        chanIsBad = status.isBad();
      }

      if (chanIsBad) {
        for (int js = 0; js < nSamp; ++js) {
          digitsBuffer[js] = 2047;
        }
        ATH_MSG_VERBOSE( "(D.03)   Masking Channel: " << ros << '/' << drawer << '/' << jch << " LowGain");
      } else {
        ATH_MSG_VERBOSE( "(D.03)   Good Channel: " << ros << '/' << drawer << '/' << jch << " LowGain");
      }

      ATH_MSG_VERBOSE("   Create a TileDigits object and set it into a container for adc_id : " << m_tileHWID->to_string(adc_id,-1));

      TileDigits* MuonReceiverDigits = new TileDigits(adc_id, digitsBuffer);
      MuonReceiverDigitsContainer->push_back(MuonReceiverDigits);

      ATH_MSG_VERBOSE("(D.04.1)   Create a TileRawChannelObject object and set it into a container ");
      ATH_MSG_VERBOSE("(D.04.2)   Create a TileRawChannelObject object ");

      TileRawChannel* MuRcvRawChannel = m_MuRcvBuildTool->rawChannel(MuonReceiverDigits);

      ATH_MSG_VERBOSE( "(D.04.3)     Set amplitude units back to MeV ");

      MuRcvRawChannel->scaleAmplitude((float) 1./mev2ADC_factor);

      ATH_MSG_VERBOSE("(D.04.3)   Put it in a container ");

      MuonReceiverRawChannelContainer->push_back(MuRcvRawChannel);

      ATH_MSG_DEBUG( "   Raw channel reconstruction Ch: " << m_tileHWID->to_string(adc_id,-1)
                     << " E [MeV]: " << MuRcvRawChannel->amplitude() / mev2ADC_factor
                     << " Time [ns]: " << MuRcvRawChannel->time()
                     << " Qf: " << MuRcvRawChannel->quality());
    }
  }	// END loop over all HIT collections in container


  if (msgLvl(MSG::DEBUG)) MuonReceiverDigitsContainer->print();

  ATH_MSG_VERBOSE( "(A.03)   ENDED Loop over a hit collection " );

  // (b) Register the digits container in the TES
  //
  ATH_MSG_VERBOSE ("(A.04)   Send to event store all collected TileDigits objects for this event " );
  CHECK(evtStore()->record(MuonReceiverDigitsContainer, m_MuRcvDigitsContainer, false));

  ATH_MSG_VERBOSE( "(A.05)   Send to event store all collected TileRawChannel objects for this event " );
  CHECK(evtStore()->record(MuonReceiverRawChannelContainer, m_MuRcvRawChContainer, false));

  ATH_MSG_DEBUG( "TilePulseForTileMuonReceiver execution completed" );
  return StatusCode::SUCCESS;
}

// finalize
//
StatusCode TilePulseForTileMuonReceiver::finalize() {
  ATH_MSG_VERBOSE("Finalizing TilePulseForTileMuonReceiver");

  ATH_MSG_INFO("TilePulseForTileMuonReceiver finalized successfully");
  return StatusCode::SUCCESS;
}
