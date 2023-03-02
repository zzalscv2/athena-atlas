/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileRawChannelFlxMonitorAlgorithm.h"
#include "TileIdentifier/TileHWID.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

// Athena includes
#include "StoreGate/ReadHandle.h"

#include <cstring>


StatusCode TileRawChannelFlxMonitorAlgorithm::initialize() {

  ATH_CHECK( AthMonitorAlgorithm::initialize() );

  ATH_MSG_INFO("in initialize()");

  ATH_CHECK( detStore()->retrieve(m_tileHWID) );
  ATH_CHECK( m_rawChannelContainerKeyLegacy.initialize() );
  ATH_CHECK( m_rawChannelContainerKeyFlx.initialize() );

  std::ostringstream os;

  if ( m_fragIDsToCompare.size() != 0) {
    std::sort(m_fragIDsToCompare.begin(), m_fragIDsToCompare.end());
    for (int fragID : m_fragIDsToCompare) {
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


StatusCode TileRawChannelFlxMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  // In case you want to measure the execution time
  auto timer = Monitored::Timer("TIME_execute");

  SG::ReadHandle<TileRawChannelContainer> rawChannelContainerLegacy(m_rawChannelContainerKeyLegacy, ctx);
  ATH_CHECK( rawChannelContainerLegacy.isValid() );

  SG::ReadHandle<TileRawChannelContainer> rawChannelContainerFlx(m_rawChannelContainerKeyFlx, ctx);
  ATH_CHECK( rawChannelContainerFlx.isValid() );

  std::array<std::string, 2> gainName{"_LG", "_HG"};

  float amplitude[TileCalibUtils::MAX_CHAN][TileCalibUtils::MAX_GAIN];
  float amplitudeFlx[TileCalibUtils::MAX_CHAN][TileCalibUtils::MAX_GAIN];
  int found[TileCalibUtils::MAX_CHAN][TileCalibUtils::MAX_GAIN];

  const TileFragHash& hashFunc = rawChannelContainerFlx->hashFunc();

  for (int fragID : m_fragIDsToCompare) {

    memset(found,0,sizeof(found));

    IdentifierHash hash = static_cast<IdentifierHash>(hashFunc(fragID));
    unsigned int drawer = (fragID & 0x3F);
    unsigned int ros = fragID >> 8;
    std::string module = TileCalibUtils::getDrawerString(ros, drawer);
    std::array<std::string, 2> moduleName{module+gainName[0], module+gainName[1]};

    // Legacy amplitudes
    const TileRawChannelCollection* rawChannelCollectionLegacy = rawChannelContainerLegacy->indexFindPtr(hash);
    for (const TileRawChannel* rawChannel : *rawChannelCollectionLegacy) {

      HWIdentifier adcId = rawChannel->adc_HWID();
      int channel = m_tileHWID->channel(adcId);
      int gain = m_tileHWID->adc(adcId);

      found[channel][gain] |= 1;
      amplitude[channel][gain] = rawChannel->amplitude();

      std::string channelName = moduleName[gain] + "_channel";
      auto monitoredChannel = Monitored::Scalar<float>(channelName, channel);

      std::string summaryName = moduleName[gain] + "_Summary_Legacy";
      auto summaryRawChannel = Monitored::Scalar<float>(summaryName, rawChannel->amplitude());
      fill("TileRawChannelLegacySummary", monitoredChannel, summaryRawChannel);
    }

    // FELIX amplitudes
    const TileRawChannelCollection* rawChannelCollectionFlx = rawChannelContainerFlx->indexFindPtr(hash);
    for (const TileRawChannel* rawChannel : *rawChannelCollectionFlx) {

      HWIdentifier adcId = rawChannel->adc_HWID();
      int channel = m_tileHWID->channel(adcId);
      int gain = m_tileHWID->adc(adcId);

      found[channel][gain] |= 2;
      amplitudeFlx[channel][gain] = rawChannel->amplitude();

      std::string channelName = moduleName[gain] + "_channel";
      auto monitoredChannel = Monitored::Scalar<float>(channelName, channel);

      std::string summaryName = moduleName[gain] + "_Summary_Felix";
      auto summaryRawChannel = Monitored::Scalar<float>(summaryName, rawChannel->amplitude());
      fill("TileRawChannelFlxSummary", monitoredChannel, summaryRawChannel);
    }

    // Compare amplitude and amplitudeFlx arrays and put results into histograms
    for(unsigned int gain = 0; gain<TileCalibUtils::MAX_GAIN; ++gain) {

      std::string channelName = moduleName[gain] + "_channel";
      std::string diffName    = moduleName[gain] + "_Diff";
      std::string legacyName  = moduleName[gain] + "_Legacy";

      auto monitoredChannel = Monitored::Scalar<float>(channelName, 0.0F);
      auto rawChannelDiff = Monitored::Scalar<float>(diffName, 0.0F);
      auto rawChannelLegacyAmp = Monitored::Scalar<float>(legacyName, 0.0F);

      for(unsigned int channel = 0; channel<TileCalibUtils::MAX_CHAN; ++channel) {

        if (found[channel][gain] == 3) {

          monitoredChannel = channel;
          rawChannelLegacyAmp = amplitude[channel][gain];
          rawChannelDiff = amplitudeFlx[channel][gain] - amplitude[channel][gain]*m_felixScale;

          fill("TileRawChannelDiffLegacyFlx", monitoredChannel, rawChannelDiff);
          fill("TileRawChannelDiffLegacyFlx_Legacy", rawChannelLegacyAmp, rawChannelDiff);
        }
      }
    }
  }

  fill("TileRawChannelFlxMonExecuteTime", timer);

  return StatusCode::SUCCESS;
}
