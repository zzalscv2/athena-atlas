/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TileTBPulseMonitorAlgorithm.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileConditions/TileInfo.h"
#include "TileIdentifier/TileHWID.h"

#include "StoreGate/ReadHandle.h"

#include <algorithm>

StatusCode TileTBPulseMonitorAlgorithm::initialize() {

  ATH_MSG_INFO("in initialize()");
  ATH_CHECK( AthMonitorAlgorithm::initialize() );

  ATH_CHECK( m_rawChannelContainerKey.initialize() );
  ATH_CHECK( m_digitsContainerKey.initialize() );
  ATH_CHECK( m_cablingSvc.retrieve() );
  ATH_CHECK( detStore()->retrieve(m_tileHWID) );

  ATH_CHECK( detStore()->retrieve(m_tileInfo, m_tileInfoName) );
  m_t0SamplePosition = (int) m_tileInfo->ItrigSample();

  std::vector<std::string> modules;
  for (int fragID : m_fragIDs) {
    int ros = fragID >> 8;
    int drawer = fragID & 0x3F;
    modules.push_back(TileCalibUtils::getDrawerString(ros, drawer));
  }

  using namespace Monitored;
  m_pulseGroups = buildToolMap<int>(m_tools, "TilePulseShape", modules);
  m_pulseProfileGroups = buildToolMap<int>(m_tools, "TilePulseShapeProfile", modules);

  std::ostringstream os;
  if ( m_fragIDs.size() != 0) {
    std::sort(m_fragIDs.begin(), m_fragIDs.end());
    for (int fragID : m_fragIDs) {
      unsigned int ros    = fragID >> 8;
      unsigned int drawer = fragID & 0xFF;
      std::string module = TileCalibUtils::getDrawerString(ros, drawer);
      os << " " << module << "/0x" << std::hex << fragID << std::dec;
    }
  } else {
    os << "NONE";
  }

  ATH_MSG_INFO("Monitored modules/frag ID:" << os.str());

  return StatusCode::SUCCESS;
}


StatusCode TileTBPulseMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  // In case you want to measure the execution time
  auto timer = Monitored::Timer("TIME_execute");

  using Tile = TileCalibUtils;
  float amplitude[Tile::MAX_CHAN][Tile::MAX_GAIN];
  float time[Tile::MAX_CHAN][Tile::MAX_GAIN];
  float pedestal[Tile::MAX_CHAN][Tile::MAX_GAIN];

  SG::ReadHandle<TileRawChannelContainer> rawChannelContainer(m_rawChannelContainerKey, ctx);
  ATH_CHECK( rawChannelContainer.isValid() );

  SG::ReadHandle<TileDigitsContainer> digitsContainer(m_digitsContainerKey, ctx);
  ATH_CHECK( digitsContainer.isValid() );

  const TileFragHash& rchHashFunc = rawChannelContainer->hashFunc();
  const TileFragHash& digitsHashFunc = digitsContainer->hashFunc();

  std::vector<float> times;
  std::vector<float> amplitudes;

  for (int fragID : m_fragIDs) {

    IdentifierHash hash = static_cast<IdentifierHash>(rchHashFunc(fragID));
    const TileRawChannelCollection* rawChannelCollection = rawChannelContainer->indexFindPtr(hash);
    if (!rawChannelCollection || rawChannelCollection->empty()) continue;


    hash = static_cast<IdentifierHash>(digitsHashFunc(fragID));
    const TileDigitsCollection* digitsCollection = digitsContainer->indexFindPtr(hash);
    if (!digitsCollection || digitsCollection->empty()) continue;

    memset(amplitude, 0, sizeof(amplitude));
    memset(time, 0, sizeof(time));
    memset(pedestal, 0, sizeof(pedestal));

    for (const TileRawChannel* rch : *rawChannelCollection) {

      HWIdentifier adc_id = rch->adc_HWID();
      int channel = m_tileHWID->channel(adc_id);
      int adc = m_tileHWID->adc(adc_id);

      amplitude[channel][adc] = rch->amplitude();
      time[channel][adc] = rch->time();

      float ped = rch->pedestal();
      ped  = ped - int( (ped + 500 ) * 1e-4) * 10000; // Remove encoded problems into pedestal
      pedestal[channel][adc] = ped;

    }

    HWIdentifier adc_id = digitsCollection->front()->adc_HWID();
    int ros = m_tileHWID->ros(adc_id);
    int drawer = m_tileHWID->drawer(adc_id);
    std::string module = TileCalibUtils::getDrawerString(ros, drawer);

    for (const TileDigits* tile_digits : *digitsCollection) {

      adc_id = tile_digits->adc_HWID();
      int channel = m_tileHWID->channel(adc_id);
      int adc = m_tileHWID->adc(adc_id);

      std::vector<float> digits = tile_digits->samples();
      ATH_MSG_VERBOSE("Ros: " << ros
                      << " drawer: " << drawer
                      << " channel: " << channel
                      << " gain: " << adc
                      << " amp: " << amplitude[channel][adc]
                      << " time: " << time[channel][adc] );


      std::string channelGainSuffix = "_" + std::to_string(channel) + "_" + std::to_string(adc);

      times.clear();
      auto monTimes = Monitored::Collection("time" + channelGainSuffix, times);

      amplitudes.clear();
      auto monAmplitude = Monitored::Collection("amplitude" + channelGainSuffix, amplitudes);

      int sampleIdx = 0;
      if ( time[channel][adc] != 0 && amplitude[channel][adc] > 0.) {
        for (double digit : digits) {
          times.push_back( (sampleIdx - m_t0SamplePosition) * 25. - time[channel][adc] );
          amplitudes.push_back( (digit - pedestal[channel][adc] ) / amplitude[channel][adc] );
          ++sampleIdx;
        }
      }

      fill(m_tools[m_pulseGroups.at(module)], monTimes, monAmplitude);
      fill(m_tools[m_pulseProfileGroups.at(module)], monTimes, monAmplitude);
    }

  }

  fill("TileTBPulseMonExecuteTime", timer);

  return StatusCode::SUCCESS;
}
