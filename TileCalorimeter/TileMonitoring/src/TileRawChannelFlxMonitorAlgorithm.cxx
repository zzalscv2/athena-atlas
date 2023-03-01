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
  ATH_CHECK( m_rawChannelContainerKeyFlx.initialize() );
  ATH_CHECK( m_rawChannelContainerKeyLegacy.initialize() );

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

  std::array<std::string, 2> gainName{"LG", "HG"};

  float amplitude[TileCalibUtils::MAX_CHAN][TileCalibUtils::MAX_GAIN];
  float amplitudeFlx[TileCalibUtils::MAX_CHAN][TileCalibUtils::MAX_GAIN];
  int ampFound[TileCalibUtils::MAX_CHAN][TileCalibUtils::MAX_GAIN];
  memset(ampFound,0,sizeof(ampFound));

  const TileFragHash& hashFunc = rawChannelContainerFlx->hashFunc();
 
  for (int fragID : m_fragIDsToCompare) {

    IdentifierHash hash = static_cast<IdentifierHash>(hashFunc(fragID));
    unsigned int drawer = (fragID & 0x3F);
    unsigned int ros = fragID >> 8;
    std::string module = TileCalibUtils::getDrawerString(ros, drawer) + "_";

    // Legacy amplitudes
    const TileRawChannelCollection* rawChannelCollectionLegacy = rawChannelContainerLegacy->indexFindPtr(hash);
    for (const TileRawChannel* rawChannel : *rawChannelCollectionLegacy) {
  
      HWIdentifier adcId = rawChannel->adc_HWID();
      int channel = m_tileHWID->channel(adcId);
      int gain = m_tileHWID->adc(adcId);

      ampFound[channel][gain] |= 1;
      amplitude[channel][gain] = rawChannel->amplitude();

      std::string channelName = module + gainName[gain] + "_channel";
      auto monitoredChannel = Monitored::Scalar<float>(channelName, channel);
  
      std::string summaryName = module + gainName[gain] + "_Summary_Legacy";
      auto summaryRawChannel = Monitored::Scalar<float>(summaryName, rawChannel->amplitude());
      fill("TileRawChannelLegacySummary", monitoredChannel, summaryRawChannel);
    }
 
    // FELIX amplitudes
    const TileRawChannelCollection* rawChannelCollectionFlx = rawChannelContainerFlx->indexFindPtr(hash);
    for (const TileRawChannel* rawChannel : *rawChannelCollectionFlx) {
  
      HWIdentifier adcId = rawChannel->adc_HWID();
      int channel = m_tileHWID->channel(adcId);
      int gain = m_tileHWID->adc(adcId);
  
      ampFound[channel][gain] |= 2;
      amplitudeFlx[channel][gain] = rawChannel->amplitude();

      std::string channelName = module + gainName[gain] + "_channel";
      auto monitoredChannel = Monitored::Scalar<float>(channelName, channel);

      std::string summaryName = module + gainName[gain] + "_Summary_Felix";
      auto summaryRawChannel = Monitored::Scalar<float>(summaryName, rawChannel->amplitude());
      fill("TileRawChannelFlxSummary", monitoredChannel, summaryRawChannel);
    }

    // Compare amplitude and amplitudeFlx arrays and put results into histograms
    for(int gain = 0; gain<2; ++gain){

      std::string channelName = module + gainName[gain] + "_channel";
      std::string diffName    = module + gainName[gain] + "_Diff";
      std::string legacyName  = module + gainName[gain] + "_Legacy";

      for(int channel = 0; channel<48; ++channel){

        if (ampFound[channel][gain] == 3) {

          auto monitoredChannel = Monitored::Scalar<float>(channelName, channel);

          float diff = amplitudeFlx[channel][gain] - amplitude[channel][gain]*m_felixScale;

          auto rawChannelDiff = Monitored::Scalar<float>(diffName, diff);
          fill("TileRawChannelDiffLegacyFlx", monitoredChannel, rawChannelDiff);

          auto rawChannelLegacyAmp = Monitored::Scalar<float>(legacyName, amplitude[channel][gain]);
          fill("TileRawChannelDiffLegacyFlx_Legacy", rawChannelLegacyAmp, rawChannelDiff);
        }
      }
    }
  }

  fill("TileRawChannelFlxMonExecuteTime", timer);

  return StatusCode::SUCCESS;
}
