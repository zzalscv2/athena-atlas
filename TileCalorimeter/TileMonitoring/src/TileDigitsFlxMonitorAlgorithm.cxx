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
  ATH_CHECK( m_digitsContainerKeyLegacy.initialize() );
  ATH_CHECK( m_digitsContainerKeyFlx.initialize() );

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

  SG::ReadHandle<TileDigitsContainer> digitsContainerLegacy(m_digitsContainerKeyLegacy, ctx);
  ATH_CHECK( digitsContainerLegacy.isValid() );

  SG::ReadHandle<TileDigitsContainer> digitsContainerFlx(m_digitsContainerKeyFlx, ctx);
  ATH_CHECK( digitsContainerFlx.isValid() );

  std::vector<float>digitsFlx[TileCalibUtils::MAX_CHAN][TileCalibUtils::MAX_GAIN];
  std::vector<float>digitsLegacy[TileCalibUtils::MAX_CHAN][TileCalibUtils::MAX_GAIN];

  const TileFragHash& hashFunc = digitsContainerFlx->hashFunc();
  for (int fragID : m_fragIDsToCompare) {   
    IdentifierHash hash = static_cast<IdentifierHash>(hashFunc(fragID));
    unsigned int drawer = (fragID & 0x3F);
    unsigned int ros = fragID >> 8;
    unsigned int lastSampleLegacy;
    unsigned int firstSampleLegacy;
    unsigned int lastSampleFlx;
    unsigned int firstSampleFlx;

    //For Legacy Digits 
    const TileDigitsCollection* digitsCollectionLegacy = digitsContainerLegacy->indexFindPtr(hash);
    for (const TileDigits* tile_digits : *digitsCollectionLegacy) {
      if (!m_fragIDs.empty() && !std::binary_search(m_fragIDs.begin(), m_fragIDs.end(), fragID)) {
        continue;
      }
      ATH_MSG_VERBOSE((std::string) *tile_digits);
      HWIdentifier adcId = tile_digits->adc_HWID();
      int channel = m_tileHWID->channel(adcId);
      int gain = m_tileHWID->adc(adcId);
      std::vector<float> digits = tile_digits->samples();

      std::string nameSample = TileCalibUtils::getDrawerString(ros, drawer)
        + "_ch_" + std::to_string(channel) + "_" + gainName[gain] +  "_samples";


      std::string channelName = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_channel";
      auto monitoredChannel = Monitored::Scalar<float>(channelName, channel);
      auto channelSample = Monitored::Scalar<float>(nameSample, 0.0F);


      std::vector<float> digits_monitored;
      if (digits.size() > 1) {
         lastSampleLegacy = std::min(static_cast<unsigned int>(m_lastSample),static_cast<unsigned int>(digits.size()));
         firstSampleLegacy = std::min(static_cast<unsigned int>(m_firstSample), (lastSampleLegacy - 1));

         for(unsigned int i = firstSampleLegacy; i < lastSampleLegacy; ++i) {
            float  sample = digits[i];
            channelSample = sample;
            fill("TileLegacyMonSamples", channelSample);
            digitsLegacy[channel][gain] = digits;
            digits_monitored.push_back(sample);
            std::string Summary_name_Legacy = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_Summary_Legacy";


         }
         if (digits_monitored.size()>1){
            unsigned int nSamples = digits_monitored.size();
            float sampleMean = std::accumulate( digits_monitored.begin(), digits_monitored.end(), 0.0) / nSamples;

            double sampleRMS = std::accumulate( digits_monitored.begin(), digits_monitored.end(), 0.0, [] (double acc, float sample) {
                                                                                                     return acc + sample * sample;
                                                                                                   });
             sampleRMS = sampleRMS / nSamples - sampleMean * sampleMean;
             sampleRMS = (sampleRMS > 0.0) ? std::sqrt(sampleRMS * nSamples / (nSamples - 1)) : 0.0;

            std::string monHFN = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_HFN";
            auto hfn = Monitored::Scalar<float>(monHFN, sampleRMS);
            fill("TileLegacyMonHFN", monitoredChannel, hfn);

            std::string monPedestal = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_Pedestal";
            auto pedestal = Monitored::Scalar<float>(monPedestal, digits[0]);
            fill("TileLegacyMonPed", monitoredChannel, pedestal);
       }
      }
   }

  //For FELIX digits
  const TileDigitsCollection* digitsCollectionFlx = digitsContainerFlx->indexFindPtr(hash);

   for (const TileDigits* tile_digits : *digitsCollectionFlx) {

      if (!m_fragIDs.empty() && !std::binary_search(m_fragIDs.begin(), m_fragIDs.end(), fragID)) {
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
        lastSampleFlx = std::max(static_cast<unsigned int>(m_lastSample),static_cast<unsigned int>(digits.size()));
        firstSampleFlx = std::min(static_cast<unsigned int>(m_firstSample), (lastSampleFlx - 1));
        for(unsigned int i = firstSampleFlx; i < lastSampleFlx; ++i) { 
          float sample = digits[i];
          channelSample = sample;
          digitsFlx[channel][gain] = digits;
          std::string Summary_name_Flx = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_Summary_Flx";
          fill("TileFlxMonSamples", channelSample);
          digits_monitored.push_back(sample);

        }
        if (digits_monitored.size()>1){
        unsigned int nSamples = digits_monitored.size();
        float sampleMean = std::accumulate( digits_monitored.begin(), digits_monitored.end(), 0.0) / nSamples;

        double sampleRMS = std::accumulate( digits_monitored.begin(), digits_monitored.end(), 0.0, [] (double acc, float sample) {
                                                                                                     return acc + sample * sample;
                                                                                                   });
        sampleRMS = sampleRMS / nSamples - sampleMean * sampleMean;
        sampleRMS = (sampleRMS > 0.0) ? std::sqrt(sampleRMS * nSamples / (nSamples - 1)) : 0.0;

        std::string monHFN = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_HFN";
        auto hfn = Monitored::Scalar<float>(monHFN, sampleRMS);
        fill("TileFlxMonHFN", monitoredChannel, hfn);

        std::string monPedestal = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_Pedestal";
        auto pedestal = Monitored::Scalar<float>(monPedestal, digits[firstSampleFlx]);
        fill("TileFlxMonPed", monitoredChannel, pedestal);
      }
     }

    }

    //compare digits from Legacy and Felix

    for(int gain = 0; gain<2; gain++){
     float diff_digits;
      for(int channel = 0; channel<48; channel++){
        std::vector<float> digits_diff;
        std::string channelName = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_channel";
        auto monitoredChannel = Monitored::Scalar<float>(channelName, channel);

        for (auto it1=digitsLegacy[channel][gain].begin(), it2=digitsFlx[channel][gain].begin();it1!=digitsLegacy[channel][gain].end() && it2!=digitsFlx[channel][gain].end();++it1, ++it2){
           std::string channelName = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_channel";
           auto monitoredChannel = Monitored::Scalar<float>(channelName, channel);

           std::string nameSample = TileCalibUtils::getDrawerString(ros, drawer)
              + "_ch_" + std::to_string(channel) + "_" + gainName[gain] +  "_samples";
           auto channelSample_diff = Monitored::Scalar<float>(nameSample, 0.0F);
           diff_digits =(*it2)- ((*it1)*(m_felixScale));
           channelSample_diff = diff_digits;

           std::string Legacy_name = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_Legacy";

           digits_diff.push_back(diff_digits);

           fill("TileDigitsDiffLegacyFlx", channelSample_diff);


           std::string nameSampleModule = TileCalibUtils::getDrawerString(ros,drawer) + "_" + gainName[gain] +  "_samples";
           auto moduleSample_diff = Monitored::Scalar<float>(nameSampleModule, 0.0F);


           moduleSample_diff = diff_digits;
           fill("TileDigitsDiffModule",moduleSample_diff );
           std::string ModuleProf = TileCalibUtils::getDrawerString(ros, drawer) + "_" + gainName[gain] + "_Profile";
           auto profile = Monitored::Scalar<float>(ModuleProf, moduleSample_diff);
           fill("TileFlxMonProf", monitoredChannel, profile);
       }
      }
     }
    

  }
  fill("TileDigitsFlxMonExecuteTime", timer);
    
  return StatusCode::SUCCESS;
}
