/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**************************************************************************
 Filename      : TileInfo.cxx 
 Author        : F. Merritt 
 Created       : May, 2002
 Last Modified : April 2006 - F Spano' (add covariance matrix for coherent 
 noise)
 Version 00-01-22
 
 :DESCRIPTION: see header file.
****************************************************************************/
//Gaudi includes
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "StoreGate/StoreGateSvc.h"

#include "TileConditions/TileInfo.h"
#include "TileConditions/TileCablingService.h"
#include "TileConditions/TilePulseShapes.h"
#include "TileConditions/TileWienerFilterWeights.h"

#include "TileCalibBlobObjs/TileCalibDrawerFlt.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

#include "CaloIdentifier/TileID.h"
#include "CaloIdentifier/TileTBID.h"
#include "TileIdentifier/TileHWID.h"
#include "TileIdentifier/TileFragHash.h"
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/IClassIDSvc.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>


//
//_____________________________________________________________________________
TileInfo::TileInfo()
  : m_tileID(nullptr)
  , m_tileHWID(nullptr)
  , m_tileTBID(nullptr)
  , m_cabling(nullptr)
  , m_channel_context(nullptr)
  , m_drawer_context(nullptr)
  , m_ADCmax(0)
  , m_ADCmaskValue(0)
  , m_nSamples(0)
  , m_iTrigSample(0)
  , m_tileNoise(0)
  , m_tileCoherNoise(0)
  , m_tileZeroSuppress(0)
  , m_thresholdRawChannel(0)
  , m_thresholdDigits(0)
  , m_ttL1Calib(0)
  , m_ttL1NoiseSigma(0)
  , m_ttL1Thresh(0)
  , m_ttL1Ped(0)
  , m_ttL1Max(0)
  , m_MuRcvCalib(0)
  , m_MuRcvNoiseSigma(0)
  , m_MuRcvThresh(0)
  , m_MuRcvPed(0)
  , m_MuRcvMax(0)
  , m_muL1Calib(0)
  , m_muL1NoiseSigma(0)
  , m_muL1Thresh(0)
  , m_muL1Ped(0)
  , m_muL1Max(0)
  , m_mbtsL1Calib(0)
  , m_mbtsL1NoiseSigma(0)
  , m_mbtsL1Thresh(0)
  , m_mbtsL1Ped(0)
  , m_mbtsL1Max(0)
  , m_ttL1CosmicsGain(0)
  , m_ttL1CosmicsThresh(0)
  , m_digitsNBinsHi(0)
  , m_digitsNBinsLo(0)
  , m_digitsTime0BinHi(0)
  , m_digitsTime0BinLo(0)
  , m_digitsBinsPerXHi(0)
  , m_digitsBinsPerXLo(0)
  , m_TTL1NBins(0)
  , m_TTL1Time0Bin(0)
  , m_TTL1BinsPerX(0)
  , m_MuRcvNBins(0)
  , m_MuRcvTime0Bin(0)
  , m_MuRcvBinsPerX(0)
  , m_MuL1NBins(0)
  , m_MuL1Time0Bin(0)
  , m_MuL1BinsPerX(0)
  , m_pulseShapes(nullptr)
  , m_WienerFilterWeights(nullptr)
  , m_tileCablingSvc("TileCablingSvc","TileInfo")
{
  //=== initialize TestBeam defaults
  for (int i=0; i<32; ++i)
    m_emscaleTB[i] = 1.0;
  
  for (int i=0; i<32; ++i)
    m_mev2adcTB[i] = 1.0;

  // scale factors from ADC HF noise to Opt filter noise calculated by Luca Fiorini
  m_noiseScaleFactor[0] = 1.0;  // no scale at all
  m_noiseScaleFactor[1] = 1.14; // opt filter without iterations
  m_noiseScaleFactor[2] = 1.28; // opt filter with iterations
  m_noiseScaleFactor[3] = 1.27; // fit method

  m_noiseScaleIndex = 1; // by default noise for opt filter without iterations
}


//
//_____________________________________________________________________________
TileInfo::~TileInfo() 
{
  if (m_channel_context) delete m_channel_context;
  if (m_drawer_context) delete m_drawer_context;

  int sizepart=m_decoCovaria.size();
  for (int i=0; i<sizepart; ++i){
    int sizemodu=(m_decoCovaria[i]).size();
    for (int j=0; j<sizemodu; ++j){
      int sizegain=(m_decoCovaria[i][j]).size();
      for (int k=0; k<sizegain; ++k){
        if (m_decoCovaria[i][j][k]) delete (TMatrixD*)(m_decoCovaria[i][j][k]);
      }
      m_decoCovaria[i][j].clear();
    }
    m_decoCovaria[i].clear();
  }
  m_decoCovaria.clear();

}


//
//_____________________________________________________________________________
StatusCode
TileInfo::initialize()
{
  MsgStream log(msgSvc(),"TileInfo");
  MSG::Level logLevel = log.level();
  bool debug = (logLevel <= MSG::DEBUG);

  if(debug) log<<MSG::DEBUG<<"In TileInfo::initialize..."<<endmsg;

    //=== get TileCablingSvc
  StatusCode sc = m_tileCablingSvc.retrieve();
  if(sc.isFailure()){
    log << MSG::ERROR
        << "Unable to retrieve " << m_tileCablingSvc << endmsg;
    return StatusCode::FAILURE;
  }
  //=== cache pointers to cabling helpers
  m_cabling  = m_tileCablingSvc->cablingService();
  if(!m_cabling){
    log << MSG::ERROR
        << "Pointer to TileCablingService is zero: " << m_cabling << endmsg;
    return StatusCode::FAILURE;
    
  }
  m_tileID   = m_cabling->getTileID();
  m_tileTBID = m_cabling->getTileTBID();
  m_tileHWID = m_cabling->getTileHWID();


  //=== initialize channel context
  m_channel_context = new IdContext(m_tileHWID->channel_context());
  //=== initialize drawer context
  m_drawer_context = new IdContext(m_tileHWID->drawer_context());

  //=== Initialize TilePulseShapes
  if (m_pulseShapes)
    m_pulseShapes->load(log);
  
  //=== Read WienerFilter Weights in TileInfoLoader.cxx
  if (m_WienerFilterWeights) {
    m_WienerFilterWeights->loadWeights(log);

    if (!m_WienerFilterWeights->loaded()) {
      log << MSG::ERROR
        << "Unable to load WienerFilter weights"<<endmsg;
      return StatusCode::FAILURE;
    }
  }

  if(debug) log << MSG::DEBUG << " TileInfo initialization completed. " << endmsg;  
  return StatusCode::SUCCESS;
}

//
//_____________________________________________________________________________
IMessageSvc* 
TileInfo::msgSvc() const 
{
  return Athena::getMessageSvc();
}

//_____________________________________________________________________________
const TMatrixD*
TileInfo::DecoCovariance(int ros, int drawer, int hilo) const {

  int part(0);
  switch (ros) {
    case TileHWID::EXTBAR_POS: part=0; break; // EBA
    case TileHWID::BARREL_POS: part=1; break; // LBA
    case TileHWID::BARREL_NEG: part=2; break; // LBC
    case TileHWID::EXTBAR_NEG: part=3; break; // EBC
  }

  int gain = (hilo) ? 0 : 1;

  return m_decoCovaria[part][drawer][gain];
}


//////////////////////////////////////////////////////////////////////////////////
// Return shape vector of a TTL1 signal, to multiply by (summed) TTL1 Amplitude
// to construct a TTL1 object. phase is a phase shift, default=0.
//
//_____________________________________________________________________________
void TileInfo::ttl1Shape(const int nsamp, const int itrig, const double phase,std::vector<double> &ttl1shape) const {

    //ttl1shape.resize(nsamp, 0.); // assume that resize already done in calling function
    for (int i=0; i<nsamp; ++i) {
      int j = m_TTL1Time0Bin + (i-itrig)*m_TTL1BinsPerX+(int)(phase*(m_TTL1BinsPerX*(1./25.0)));
      if(j<0) j = 0;
      if(j>=m_TTL1NBins) j = m_TTL1NBins-1;
      ttl1shape[i] = m_TTL1FullShape[j];
    }
#ifndef NDEBUG
    MsgStream log(msgSvc(),"TileInfo");
    if (log.level() <= MSG::DEBUG){
      log << MSG::DEBUG << " Shaping profile at beam crossings:   nsamp = " << nsamp << ", itrig = " << itrig << endmsg;
      int jc = 0;
      for (int i=0; i<nsamp; i++) {
	if(jc==0) log << MSG::DEBUG << "      bin = " << i << "   Shape factor =";
	log << MSG::DEBUG << std::setw(8) << std::setprecision(4) << ttl1shape[i] << "  ";
	if(++jc==5) {
	  log << MSG::DEBUG << endmsg;
	  jc=0;
	}
      }
      log << MSG::DEBUG << endmsg; 
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the shape vector of the Muon Receiver Board input signal, to multiply by the channel amplitude
// to construct a Muon Receiver Board object. The phase is a phase shift (default=0).
//
//_____________________________________________________________________________

void TileInfo::muRcvShape(const int nsamp, const int itrig, const double phase,std::vector<double> &murcvshape) const {

    for (int i=0; i<nsamp; ++i) {
      int j = m_MuRcvTime0Bin + (i-itrig)*m_MuRcvBinsPerX+(int)(phase*(m_MuRcvBinsPerX*(1./25.0)));
      if(j<0) j = 0;
      if(j>=m_MuRcvNBins) j = m_MuRcvNBins-1;
      murcvshape[i] = m_MuRcvFullShape[j];
    }

}
