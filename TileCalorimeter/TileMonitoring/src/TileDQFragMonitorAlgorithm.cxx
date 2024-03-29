/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TileDQFragMonitorAlgorithm.h"
#include "TileIdentifier/TileHWID.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileRecUtils/TileRawChannelBuilder.h"
#include "TileConditions/TileInfo.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandle.h"


StatusCode TileDQFragMonitorAlgorithm::initialize() {

  ATH_CHECK( AthMonitorAlgorithm::initialize() );

  ATH_MSG_INFO("in initialize()");

  ATH_CHECK( detStore()->retrieve(m_tileID) );
  ATH_CHECK( detStore()->retrieve(m_tileHWID) );

  ATH_CHECK( m_badChannelsKey.initialize() );

  ATH_CHECK( m_cablingSvc.retrieve() );
  m_cabling = m_cablingSvc->cablingService();

  ATH_CHECK( m_DQstatusKey.initialize() );
  ATH_CHECK( m_DCSStateKey.initialize(m_checkDCS) );
  ATH_CHECK( m_emScaleKey.initialize() );
  ATH_CHECK( m_digitsContainerKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_rawChannelContainerKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_eventInfoTileStatusKey.initialize() );

  using Tile = TileCalibUtils;

  m_errorsGroups = Monitored::buildToolMap<std::vector<int>>(m_tools, "TileDigiErrors",
                                                             Tile::MAX_ROS - 1, Tile::MAX_DRAWER);

  m_errorsVsLBGroups = Monitored::buildToolMap<std::vector<int>>(m_tools, "FracTileDigiErrors",
                                                                 Tile::MAX_ROS - 1, Tile::MAX_DRAWER);

  m_badChannelJumpGroups = Monitored::buildToolMap<int>(m_tools, "TileBadChannelsJumpMap", Tile::MAX_ROS - 1);
  m_badChannelJumpNotMaskGroups = Monitored::buildToolMap<int>(m_tools, "TileBadChannelsJumpNotMaskMap", Tile::MAX_ROS - 1);

  m_badChannelNegGroups = Monitored::buildToolMap<int>(m_tools, "TileBadChannelsNegMap", Tile::MAX_ROS - 1);
  m_badChannelNegNotMaskGroups = Monitored::buildToolMap<int>(m_tools, "TileBadChannelsNegNotMaskMap", Tile::MAX_ROS - 1);

  m_badPulseQualityGroups = Monitored::buildToolMap<int>(m_tools, "TileBadPulseQualityMap", Tile::MAX_ROS - 1);
  m_negativeEnergyGroups = Monitored::buildToolMap<int>(m_tools, "TileNegativeEnergyMap", Tile::MAX_ROS - 1);

  ATH_CHECK( detStore()->retrieve(m_tileInfo, m_infoName) );
  m_ADCmaxMinusEps = m_tileInfo->ADCmax() - 0.01;
  m_ADCmaskValueMinusEps = m_tileInfo->ADCmaskValue() - 0.01;  // indicates channels which were masked in background dataset

  return StatusCode::SUCCESS;
}


StatusCode TileDQFragMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  using Tile = TileCalibUtils;

  // In case you want to measure the execution time
  auto timer = Monitored::Timer("TIME_execute");

  const xAOD::EventInfo* eventInfo = GetEventInfo(ctx).get();

  const TileDQstatus* dqStatus = SG::makeHandle (m_DQstatusKey, ctx).get();
  const TileDCSState* dcsState = m_checkDCS ? SG::ReadCondHandle(m_DCSStateKey, ctx).cptr() : nullptr;
  SG::ReadCondHandle<TileEMScale> emScale(m_emScaleKey, ctx);
  ATH_CHECK( emScale.isValid() );

  SG::ReadCondHandle<TileBadChannels> badChannels(m_badChannelsKey, ctx);
  ATH_CHECK( badChannels.isValid() );

  auto lumiBlock = Monitored::Scalar<int>("lumiBlock", eventInfo->lumiBlock());

  auto monitoredROS = Monitored::Scalar<int>("ROS", -1);
  auto monitoredModule = Monitored::Scalar<int>("module", -1);
  auto monitoredChannel = Monitored::Scalar<int>("channel", -1);

  std::vector<int> dmus;
  auto drawerDMUs = Monitored::Collection("DMU", dmus);

  std::vector<int> errors;
  auto errorsInDMUs = Monitored::Collection("Error", errors);

  const int Trig_b7(7);

  unsigned int tileEventInfoFlag = eventInfo->eventFlags(xAOD::EventInfo::Tile);
  bool isTileErrorState = (eventInfo->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error);
  int nBadConsecutiveModules = isTileErrorState ? MAX_DMU : ((tileEventInfoFlag >> MAX_DMU) & 0xF);

  auto monitoredConsecutiveBad = Monitored::Scalar<int>("TileConsecutiveBad", nBadConsecutiveModules);

  if (isTileErrorState) {
    fill("TileEventsWithErrEventInfoLB", lumiBlock);
  }

  fill("TileConsecutiveBadModules", monitoredConsecutiveBad);
  fill("TileConsecutiveBadModulesLB", monitoredConsecutiveBad, lumiBlock);

  const TileDigitsContainer* digitsContainer{nullptr};
  if (!m_digitsContainerKey.empty()) {
    digitsContainer = SG::makeHandle(m_digitsContainerKey, ctx).get();
  }

  uint32_t l1TriggerType(eventInfo->level1TriggerType());

  bool physicRun = (l1TriggerType == 0) || (((l1TriggerType >> Trig_b7) & 1) == 1);
  if (physicRun && !m_rawChannelContainerKey.empty()) {

    const TileRawChannelContainer* rawChannelContainer = SG::makeHandle(m_rawChannelContainerKey, ctx).get();

    TileRawChannelUnit::UNIT rawChannelUnit = rawChannelContainer->get_unit();
    if (rawChannelUnit != TileRawChannelUnit::ADCcounts
        && rawChannelUnit != TileRawChannelUnit::OnlineADCcounts ) {

      ATH_MSG_WARNING( "Tile raw channel units are not ADC counts => will not check neagative amplitudes!" );
    } else {

      for (const TileRawChannelCollection* rawChannelCollection : *rawChannelContainer) {

        int fragId = rawChannelCollection->identify();
        int drawer = (fragId & 0x3F); // range 0-63
        int ros = fragId >> 8;  // range 1-4
        unsigned int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);

        monitoredModule = drawer;
        clearDigiError(dmus, errors);
        for (const TileRawChannel* rawChannel : *rawChannelCollection) {

          HWIdentifier adcId = rawChannel->adc_HWID();
          int channel = m_tileHWID->channel(adcId);
          int gain = m_tileHWID->adc(adcId);

          // By convetion errors are saved in pedestal as 100000 + 10000*error
          float pedestal = rawChannel->pedestal();
          float quality = std::abs(rawChannel->quality());
          float amplitude = rawChannel->amplitude();
          float time = rawChannel->uncorrTime(); // take uncorrected time (if available)

          monitoredChannel = channel;

          if (dqStatus->isChanDQgood(ros, drawer, channel)
               && !(badChannels->getAdcStatus(adcId).isBad()
                    || (m_checkDCS && dcsState->isStatusBad(ros, drawer, channel)))) {

            if (pedestal > 80000. || quality > m_qualityCut) {
              fill(m_tools[m_badPulseQualityGroups[ros - 1]], monitoredModule, monitoredChannel);
            }

            int pmt;
            int index;
            Identifier cell_id = m_cabling->h2s_cell_id_index (ros, drawer, channel, index, pmt);
            if (index >= 0) { // connected channel, exluding MBTS and E4'

              if (quality > m_qualityCut) {
                bool overflow = (pedestal > 10000. + m_ADCmaskValueMinusEps);
                if (!(overflow && gain == TileID::LOWGAIN && amplitude > 0.
                      && time > m_timeMinThresh && time < m_timeMaxThresh)) { // overflow in low gain is not masked
                  int dmu = channel / 3;
                  setDigiError(dmus, errors, dmu, MAX_DIGI_ERROR + BAD_QUALITY);
                }
              }

              float minEnergy = (m_tileID->sample(cell_id) == TileID::SAMP_E) ? m_minGapEnergy : m_minChannelEnergy;
              float energy = emScale->calibrateChannel(drawerIdx, channel, gain, amplitude, rawChannelUnit, TileRawChannelUnit::MegaElectronVolts);
              if (energy < minEnergy) {
                int dmu = channel / 3;
                setDigiError(dmus, errors, dmu, MAX_DIGI_ERROR + BIG_NEGATIVE_AMPLITUDE);

                fill(m_tools[m_negativeEnergyGroups[ros - 1]], monitoredModule, monitoredChannel);
              }
            }
          }

          if (amplitude < ((gain) ? m_negativeAmplitudeCutHG : m_negativeAmplitudeCutLG)) {

            // Channel number divided by 2
            int channelDividedBy2 = channel >> 1;

            if (!(m_skipGapCells && ros > 2  // Choose extended barrel and channels: 0,1, 12,13, 18,19
                  && (channelDividedBy2 == 0 || channelDividedBy2 == 6 || channelDividedBy2 == 9))
                && !(m_skipMasked && badChannels->getAdcStatus(adcId).isBad())) {

              fill(m_tools[m_badChannelNegGroups[ros - 1]], monitoredModule, monitoredChannel);

              if (!(m_checkDCS && !dcsState->isStatusBad(ros, drawer))
                  && dqStatus->isChanDQgood(ros, drawer, channel)) {

                fill(m_tools[m_badChannelNegNotMaskGroups[ros - 1]], monitoredModule, monitoredChannel);

                if (pedestal > 100000. && digitsContainer) {

                  IdentifierHash hash = static_cast<IdentifierHash>(rawChannelContainer->hashFunc()(fragId));
                  const TileDigitsCollection* digitsCollection = digitsContainer->indexFindPtr(hash);

                  if (digitsCollection) {

                    for (const TileDigits* tile_digits : *digitsCollection) {

                      if (m_tileHWID->channel(tile_digits->adc_HWID()) == channel) {

                        msg(MSG::INFO) << "LB " << eventInfo->lumiBlock()
                                       << " Evt " << eventInfo->eventNumber()
                                       << " " << Tile::getDrawerString(ros, drawer)
                                       << std::setfill(' ') << std::setw(3) << channel
                                       << ((gain) ? " HG" : " LG") << " negative amplitude: "
                                       << std::setprecision(2) << std::fixed << std::setw(7)
                                       << amplitude << "   Samples: " << std::setprecision(0);

                        std::vector<float> samples = tile_digits->samples();
                        for (float sample : samples) {
                          msg(MSG::INFO) << sample << " ";
                        }

                        msg(MSG::INFO) << "  error = " << TileRawChannelBuilder::BadPatternName(pedestal) << endmsg;
                      }
                    }
                  }
                }
              }
            }
          }
        }
        if (!errors.empty()) {
          fill(m_tools[m_errorsGroups[ros - 1][drawer]], drawerDMUs, errorsInDMUs);
        }
      }
    }
  }


  if (digitsContainer) {


    for (const TileDigitsCollection* digitsCollection : *digitsContainer) {

      int fragId = digitsCollection->identify();
      int drawer = (fragId & 0x3F); // range 0-63
      int ros = fragId >> 8;  // range 1-4

      monitoredROS = ros;
      monitoredModule = drawer;

      if (l1TriggerType != digitsCollection->getLvl1Type()) {
        fill("TileMismatchedL1TriggerType", monitoredModule, monitoredROS);
      }

      if (l1TriggerType == 0x82) {

        unsigned int nBadOrDisconnectedChannels(0);
        for (unsigned int channel = 0; channel < Tile::MAX_CHAN; ++channel) {
          HWIdentifier channel_id = m_tileHWID->channel_id(ros, drawer, channel);
          if (badChannels->getChannelStatus(channel_id).isBad()
              || (m_cabling->isDisconnected(ros, drawer, channel)) ) {
            ++nBadOrDisconnectedChannels;
          }
        }

        unsigned int nRequiredChannels(Tile::MAX_CHAN - nBadOrDisconnectedChannels);
        if (digitsCollection->size() < nRequiredChannels) {
          fill("TileNoAllDigits", monitoredModule, monitoredROS);
          ATH_MSG_VERBOSE("No All channels with digits (Trigger Type: 0x82) in module "
                          << Tile::getDrawerString(ros, drawer)
                          << ", present channels: " << digitsCollection->size()
                          << ", required channels: " << nRequiredChannels);
        }

      }

      int error;
      float minSample;
      float maxSample;

      for (const TileDigits* tile_digits : *digitsCollection) {

        clearDigiError(dmus, errors);
        uint16_t corruptedData[MAX_CORRUPTED_ERROR] = {0u};

        HWIdentifier adcId = tile_digits->adc_HWID();
        int channel = m_tileHWID->channel(adcId);
        int gain = m_tileHWID->adc(adcId);

        monitoredChannel = channel;

        error = TileRawChannelBuilder::CorruptedData(ros, drawer, channel, gain, tile_digits->samples(), minSample, maxSample, m_ADCmaxMinusEps, m_ADCmaskValueMinusEps);

        if ( (error > 0) &&
             !(m_cabling->isDisconnected(ros, drawer, channel) || badChannels->getAdcStatus(adcId).isBad()) ) {

          if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << "LB " << eventInfo->lumiBlock()
                            << " Evt " << eventInfo->eventNumber()
                            << " " << Tile::getDrawerString(ros, drawer)
                            << std::setfill(' ') << std::setw(3) << channel
                            << ((gain) ? " HG" : " LG")
                            << "   Samples: ";

            std::vector<float> samples = tile_digits->samples();
            for (float sample : samples) {
              msg(MSG::DEBUG) << sample << " ";
            }
            msg(MSG::DEBUG) << "  error = " << TileRawChannelBuilder::BadPatternName(100000. + error * 10000) << endmsg;
          }

          fill(m_tools[m_badChannelJumpGroups[ros - 1]], monitoredModule, monitoredChannel);

          if (!(m_checkDCS && dcsState->isStatusBad(ros, drawer)) && dqStatus->isChanDQgood(ros, drawer, channel)) {

            fill(m_tools[m_badChannelJumpNotMaskGroups[ros - 1]], monitoredModule, monitoredChannel);

            if (error <= MAX_CORRUPTED_ERROR) {
              corruptedData[error - 1] |= 1u << (unsigned int) (channel / 3);
            }
          }
        }

        bool fillCorruptedData(false);
        for (int error = 0; error < MAX_CORRUPTED_ERROR; ++error) {
          if (corruptedData[error] > 0u) {
            fillCorruptedData = true;
            for (unsigned int dmu = 0u; dmu < 16u; ++dmu) {
              if (corruptedData[error] & (1u << dmu)) {
                setDigiError(dmus, errors, dmu, MAX_DIGI_ERROR + error);
              }
            }
          }
        }

        if (fillCorruptedData) {
          fill(m_tools[m_errorsGroups[ros - 1][drawer]], drawerDMUs, errorsInDMUs);
        }
      }
    }
  }

  std::vector<int> rosWithGloblaCRC;
  std::vector<int> drawerWithGlobalCRC;
  auto fractionOfBadDMUs = Monitored::Scalar<float>("fractionOfBadDMUs", 0.0);

  for (unsigned int ros = 1; ros < Tile::MAX_ROS; ++ros) {
    for (unsigned int drawer = 0; drawer < Tile::MAX_DRAWER; ++drawer) {

      clearDigiError(dmus, errors);
      bool isGoodModuleDCS(true);

      if (m_checkDCS && dcsState->isStatusBad(ros, drawer)) {

        fractionOfBadDMUs = -1.0; // By convention
        isGoodModuleDCS = false;

      }

      int status = dqStatus->checkGlobalErr(ros, drawer, 0);
      TileDigiErrors error(OK);
      if (status & (TileFragStatus::ALL_FF | TileFragStatus::ALL_00)) {
        error = DUMMY_FRAG;
      } else if (status & (TileFragStatus::NO_FRAG | TileFragStatus::NO_ROB)) {
        error = NO_RECO_FRAG;
      }

      float nBadNotMaskedDMUs = 0;

      for (int dmu = 0; dmu < MAX_DMU; ++dmu) { // loop over dmus
        int channel = 3 * dmu;

        bool isMaskedDMU = m_ignoreNoRecoFragError ? (error == NO_RECO_FRAG) : false;

        TileBchStatus channelStatus0 = badChannels->getChannelStatus( m_tileHWID->channel_id(ros, drawer, channel) );
        TileBchStatus channelStatus1 = badChannels->getChannelStatus( m_tileHWID->channel_id(ros, drawer, channel + 1) );
        TileBchStatus channelStatus2 = badChannels->getChannelStatus( m_tileHWID->channel_id(ros, drawer, channel + 2) );

        bool specialEB; // special treatment of EBA15, EBC18

        if ((ros == 3 && drawer == 14) || (ros == 4 && drawer == 17)) {
          specialEB = true; // EBA15, EBC18
        } else {
          specialEB = false;
        }

        if ((channelStatus0.isBad() && channelStatus1.isBad() && channelStatus2.isBad())
            // Check disconnected channels for EBs
            || ((ros > 2 && ((channel == 18 && !specialEB) || channel == 33)) && channelStatus2.isBad())
            // Check disconnected channels for LBs
            || ((ros < 3 && channel == 30) && channelStatus2.isBad())
            // Check disconnected channels for LBs
            || ((ros < 3 && channel == 42) && channelStatus0.isBad() && channelStatus2.isBad())
            // Check void DMUs for EBs
            || (ros > 2 && (channel == 24 || channel == 27 || channel == 42 || channel == 45))
            || (specialEB && channel == 0) // Check void DMU 0 for EBA15, EBC18
            // Check disconnected PMT of DMU 1 for EBA15, EBC18
            || ((specialEB && channel == 3) && channelStatus1.isBad() && channelStatus2.isBad())) {

          setDigiError(dmus, errors, dmu, MASKED);
          isMaskedDMU = true;
        }

        if (isGoodModuleDCS) {

          if (m_checkDCS
              && ((dcsState->isStatusBad(ros, drawer, channel)
                   && !channelStatus0.contains(TileBchPrbs::NoHV) && !channelStatus0.contains(TileBchPrbs::WrongHV))
                  || (dcsState->isStatusBad(ros, drawer, channel + 1)
                      && !channelStatus1.contains(TileBchPrbs::NoHV) && !channelStatus1.contains(TileBchPrbs::WrongHV))
                  || (dcsState->isStatusBad(ros, drawer, channel + 2)
                      && !channelStatus2.contains(TileBchPrbs::NoHV) && !channelStatus2.contains(TileBchPrbs::WrongHV)))) {

            setDigiError(dmus, errors, dmu, ANY_CH_BAD_HV);
          }

          if (dqStatus->isChanDQgood(ros, drawer, channel)) {
            setDigiError(dmus, errors, dmu, OK);
          } else {

            if (!(isMaskedDMU
                  || (ros > 2 && (dmu == 8 || dmu == 9 || dmu == 14 || dmu == 15))
                  || (specialEB && dmu == 0))) {

              ++nBadNotMaskedDMUs;
            }

            if (error != OK) {
              setDigiError(dmus, errors, dmu, error);
            } else if (dqStatus->checkHeaderFormatErr(ros, drawer, dmu, 0) != 0) {
              // In case of format errors, we only fill this one
              setDigiError(dmus, errors, dmu, HEADER_FORM);
            } else {
              if (dqStatus->checkHeaderParityErr(ros, drawer, dmu, 0) != 0) {
                setDigiError(dmus, errors, dmu, HEADER_PAR);
              }
              if (dqStatus->checkMemoryParityErr(ros, drawer, dmu, 0) != 0) {
                setDigiError(dmus, errors, dmu, MEMO_PAR);
              }
              if (dqStatus->checkFE_CRCErr(ros, drawer, dmu, 0) != 0) {
                setDigiError(dmus, errors, dmu, FE_CRC);
              }
              if (dqStatus->checkROD_CRCErr(ros, drawer, dmu, 0) != 0) {
                setDigiError(dmus, errors, dmu, ROD_CRC);
              }
              if (dqStatus->checkBCIDErr(ros, drawer, dmu, 0) != 0) {
                setDigiError(dmus, errors, dmu, BCID);
              }
              if (dqStatus->checkSampleFormatErr(ros, drawer, dmu, 0) != 0) {
                setDigiError(dmus, errors, dmu, SAMPLE_FORM);
              }
              if (dqStatus->checkSampleParityErr(ros, drawer, dmu, 0) != 0) {
                setDigiError(dmus, errors, dmu, SAMPLE_PAR);
              }
              if (dqStatus->checkDoubleStrobeErr(ros, drawer, dmu, 0) != 0) {
                setDigiError(dmus, errors, dmu, DOUBLE_STB);
              }
              if (dqStatus->checkSingleStrobeErr(ros, drawer, dmu, 0) != 0) {
                setDigiError(dmus, errors, dmu, SINGLE_STB);
              }
              if (dqStatus->checkGlobalCRCErr(ros, drawer, 0) != 0) {
                setDigiError(dmus, errors, dmu, GLOBAL_CRC);
              }
            }
          }
        } else {
          setDigiError(dmus, errors, dmu, ALL_M_BAD_DCS);
        }
      } // loop over DMUs

      if (isGoodModuleDCS) {
        fractionOfBadDMUs = nBadNotMaskedDMUs / MAX_DMU;
      }

      // Keep the number of processed events in underflow
      // in histograms with DMU header errors
      setDigiError(dmus, errors, -1, OK);
      fill(m_tools[m_errorsGroups[ros - 1][drawer]], drawerDMUs, errorsInDMUs);
      fill(m_tools[m_errorsVsLBGroups[ros - 1][drawer]], lumiBlock, fractionOfBadDMUs);

      if (dqStatus->checkGlobalCRCErr(ros, drawer, 0) != 0) {
        rosWithGloblaCRC.push_back(ros);
        drawerWithGlobalCRC.push_back(drawer);
      }
    }
  }

  if (!rosWithGloblaCRC.empty()) {
    auto monModule = Monitored::Collection("module", drawerWithGlobalCRC);
    auto monROS = Monitored::Collection("ROS", rosWithGloblaCRC);
    fill("TileBadGlobalCRC", monModule, monROS);
  }

  fill("TileDQFragMonExecuteTime", timer);

  return StatusCode::SUCCESS;
}


void TileDQFragMonitorAlgorithm::setDigiError(std::vector<int>& dmus, std::vector<int>& errors,
                                              int dmu, int error) const {
  dmus.push_back(dmu);
  errors.push_back(error);
}


void TileDQFragMonitorAlgorithm::clearDigiError(std::vector<int>& dmus, std::vector<int>& errors) const {
  dmus.clear();
  errors.clear();
}

