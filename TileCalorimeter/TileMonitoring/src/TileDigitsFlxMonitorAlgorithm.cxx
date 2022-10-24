/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileDigitsFlxMonitorAlgorithm.h"
#include "TileIdentifier/TileHWID.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

// Athena includes
#include "StoreGate/ReadHandle.h"

#include <cmath>
#include <algorithm>

StatusCode TileDigitsFlxMonitorAlgorithm::initialize() {

  ATH_CHECK( AthMonitorAlgorithm::initialize() );

  ATH_MSG_INFO("in initialize()");

  ATH_CHECK( detStore()->retrieve(m_tileHWID) );
  ATH_CHECK( m_digitsContainerKey.initialize() );

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
    os << "ALL";
  }

  ATH_MSG_INFO("Monitored modules/frag ID:" << os.str());

  return StatusCode::SUCCESS;
}


StatusCode TileDigitsFlxMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  // In case you want to measure the execution time
  auto timer = Monitored::Timer("TIME_execute");

  std::array<std::string, 2> gainName{"LG", "HG"};

  SG::ReadHandle<TileDigitsContainer> digitsContainer(m_digitsContainerKey, ctx);
  ATH_CHECK( digitsContainer.isValid() );

  for (const TileDigitsCollection* digitsCollection : *digitsContainer) {
 
    int fragId = digitsCollection->identify();
    unsigned int drawer = (fragId & 0x3F);
    unsigned int ros = fragId >> 8;

    for (const TileDigits* tile_digits : *digitsCollection) {

      if (!m_fragIDs.empty() && !std::binary_search(m_fragIDs.begin(), m_fragIDs.end(), fragId)) {
        continue;
      }

      ATH_MSG_VERBOSE((std::string) *tile_digits);

      HWIdentifier adcId = tile_digits->adc_HWID();
      int channel = m_tileHWID->channel(adcId);
      int gain = m_tileHWID->adc(adcId);         
      std::vector<float> digits = tile_digits->samples();

      std::string nameSample = TileCalibUtils::getDrawerString(ros, drawer)
        + "_ch_" + std::to_string(channel) + "_" + gainName[gain] +  "_samples";
      auto channelSample = Monitored::Scalar<float>(nameSample, 0.0F);

      std::string channelName = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_channel";
      auto monitoredChannel = Monitored::Scalar<float>(channelName, channel);

      std::vector<float> digits_monitored;
      if (digits.size() > 1) {

        unsigned int lastSample = std::min((unsigned int) m_lastSample, (unsigned int) digits.size());
        unsigned int firstSample = std::min((unsigned int) m_firstSample, (lastSample - 1));
        for(unsigned int i = firstSample; i < lastSample; ++i) {
          float sample = digits[i];
          channelSample = sample;
          fill("TileFlxMonSamples", channelSample);
          digits_monitored.push_back(sample);
        }

        unsigned int nSamples = digits_monitored.size();
        float sampleMean = std::accumulate( digits_monitored.begin(), digits_monitored.end(), 0.0) / nSamples;

        double sampleRMS = std::accumulate( digits_monitored.begin(), digits_monitored.end(), 0.0, [] (double acc, float sample) {
                                                                                                     return acc + sample * sample;
                                                                                                   });
        sampleRMS = sampleRMS / nSamples - sampleMean * sampleMean;
        sampleRMS = (sampleRMS > 0.0) ? sqrt(sampleRMS * nSamples / (nSamples - 1)) : 0.0;

        std::string monHFN = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_HFN";
        auto hfn = Monitored::Scalar<float>(monHFN, sampleRMS);
        fill("TileFlxMonHFN", monitoredChannel, hfn);

        std::string monPedestal = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_Pedestal";
        auto pedestal = Monitored::Scalar<float>(monPedestal, digits[firstSample]);
        fill("TileFlxMonPed", monitoredChannel, pedestal);
      }
    }
  }

  fill("TileDigitsFlxMonExecuteTime", timer);
    
  return StatusCode::SUCCESS;
}
