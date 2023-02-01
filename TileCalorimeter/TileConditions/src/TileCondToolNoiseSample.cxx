/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileConditions/TileCondToolNoiseSample.h"

// Athena includes
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadCondHandle.h"

//
//____________________________________________________________________
static const InterfaceID IID_TileCondToolNoiseSample("TileCondToolNoiseSample", 1, 0);
const InterfaceID& TileCondToolNoiseSample::interfaceID() {
  return IID_TileCondToolNoiseSample;
}

//
//____________________________________________________________________
TileCondToolNoiseSample::TileCondToolNoiseSample(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type, name, parent)
{
  declareInterface<ITileCondToolNoise>(this);
  declareInterface<TileCondToolNoiseSample>(this);
}

//
//____________________________________________________________________
TileCondToolNoiseSample::~TileCondToolNoiseSample() {
}

//
//____________________________________________________________________
StatusCode TileCondToolNoiseSample::initialize() {

  ATH_MSG_DEBUG( "In initialize()" );

  //=== retrieve proxy
  ATH_CHECK( m_sampleNoiseKey.initialize() );

  ATH_CHECK( m_onlineSampleNoiseKey.initialize (SG::AllowEmpty));

  ATH_CHECK( m_emScaleKey.initialize() );

  return StatusCode::SUCCESS;
}

//
//____________________________________________________________________
StatusCode TileCondToolNoiseSample::finalize() {
  ATH_MSG_DEBUG( "finalize called" );

  return StatusCode::SUCCESS;
}

//
//____________________________________________________________________
float TileCondToolNoiseSample::getPed(unsigned int drawerIdx, unsigned int channel, unsigned int adc,
                                      TileRawChannelUnit::UNIT unit, const EventContext &ctx) const {

  SG::ReadCondHandle<TileSampleNoise> sampleNoise(m_sampleNoiseKey, ctx);
  float ped = sampleNoise->getPed(drawerIdx, channel, adc);

  if (unit > TileRawChannelUnit::ADCcounts) {
    SG::ReadCondHandle<TileEMScale> emScale(m_emScaleKey, ctx);
    ped = emScale->calibrateChannel(drawerIdx, channel, adc, ped, TileRawChannelUnit::ADCcounts, unit);
  }

  return ped;

}

//
//____________________________________________________________________
float TileCondToolNoiseSample::getHfn(unsigned int drawerIdx, unsigned int channel, unsigned int adc,
                                      TileRawChannelUnit::UNIT unit, const EventContext &ctx) const {

  SG::ReadCondHandle<TileSampleNoise> sampleNoise(m_sampleNoiseKey, ctx);
  float hfn = sampleNoise->getHfn(drawerIdx, channel, adc);

  if (unit > TileRawChannelUnit::ADCcounts) {
    SG::ReadCondHandle<TileEMScale> emScale(m_emScaleKey, ctx);
    hfn = emScale->calibrateChannel(drawerIdx, channel, adc, hfn, TileRawChannelUnit::ADCcounts, unit);
  }

  return hfn;

}

//
//____________________________________________________________________
float TileCondToolNoiseSample::getLfn(unsigned int drawerIdx, unsigned int channel, unsigned int adc,
                                      TileRawChannelUnit::UNIT unit, const EventContext &ctx) const {

  SG::ReadCondHandle<TileSampleNoise> sampleNoise(m_sampleNoiseKey, ctx);
  float lfn = sampleNoise->getLfn(drawerIdx, channel, adc);

  if (unit > TileRawChannelUnit::ADCcounts) {
    SG::ReadCondHandle<TileEMScale> emScale(m_emScaleKey, ctx);
    lfn = emScale->calibrateChannel(drawerIdx, channel, adc, lfn, TileRawChannelUnit::ADCcounts, unit);
  }

  return lfn;
}

//
//____________________________________________________________________
float TileCondToolNoiseSample::getHfn1(unsigned int drawerIdx, unsigned int channel, unsigned int adc,
                                       const EventContext &ctx) const {

  SG::ReadCondHandle<TileSampleNoise> sampleNoise(m_sampleNoiseKey, ctx);
  float hfn1 = sampleNoise->getHfn1(drawerIdx, channel, adc);

  return hfn1;

}

//
//____________________________________________________________________
float TileCondToolNoiseSample::getHfn2(unsigned int drawerIdx, unsigned int channel, unsigned int adc,
                                       const EventContext &ctx) const {

  SG::ReadCondHandle<TileSampleNoise> sampleNoise(m_sampleNoiseKey, ctx);
  float hfn2 = sampleNoise->getHfn2(drawerIdx, channel, adc);

  return hfn2;
}

//
//____________________________________________________________________
float TileCondToolNoiseSample::getHfnNorm(unsigned int drawerIdx, unsigned int channel, unsigned int adc,
                                          const EventContext &ctx) const {

  SG::ReadCondHandle<TileSampleNoise> sampleNoise(m_sampleNoiseKey, ctx);
  float hfnNorm = sampleNoise->getHfnNorm(drawerIdx, channel, adc);

  return hfnNorm;
}


float TileCondToolNoiseSample::getOnlinePedestalDifference(unsigned int drawerIdx, unsigned int channel,
                                                           unsigned int adc, TileRawChannelUnit::UNIT onlineUnit,
                                                           const EventContext &ctx) const {

  float pedestalDifference(0.0);

  if (!m_onlineSampleNoiseKey.empty()) {
    SG::ReadCondHandle<TileSampleNoise> sampleNoise(m_sampleNoiseKey, ctx);
    SG::ReadCondHandle<TileSampleNoise> onlineSampleNoise(m_onlineSampleNoiseKey, ctx);

    float pedestal = sampleNoise->getPed(drawerIdx, channel, adc);
    float onlinePedestal = onlineSampleNoise->getPed(drawerIdx, channel, adc);

    SG::ReadCondHandle<TileEMScale> emScale(m_emScaleKey, ctx);
    pedestalDifference = emScale->calibrateOnlineChannel(drawerIdx, channel, adc, (onlinePedestal - pedestal), onlineUnit);
  }

  return pedestalDifference;
}
