/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelChargeCalibCondAlg.h"
#include "Identifier/IdentifierHash.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "GaudiKernel/EventIDRange.h"
#include <memory>
#include <sstream>

#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>

namespace
{

constexpr int halfModuleThreshold{8};
constexpr size_t FEStringSize{21};

} // namespace


PixelChargeCalibCondAlg::PixelChargeCalibCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode PixelChargeCalibCondAlg::initialize() {
  ATH_MSG_DEBUG("PixelChargeCalibCondAlg::initialize()");

  ATH_CHECK(detStore()->retrieve(m_pixelID,"PixelID"));
  ATH_CHECK(m_pixelDetEleCollKey.initialize());
  ATH_CHECK(m_configKey.initialize());
  ATH_CHECK(m_readKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PixelChargeCalibCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("PixelChargeCalibCondAlg::execute()");

  SG::WriteCondHandle<PixelChargeCalibCondData> writeHandle(m_writeKey, ctx);
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }

  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey, ctx);
  const InDetDD::SiDetectorElementCollection* elements(*pixelDetEleHandle);
  if (not pixelDetEleHandle.isValid() or elements==nullptr) {
    ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
    return StatusCode::FAILURE;
  }

  const std::array<InDetDD::PixelDiodeType, 4> diodeTypes
    = {InDetDD::PixelDiodeType::NORMAL, InDetDD::PixelDiodeType::LONG, InDetDD::PixelDiodeType::GANGED, InDetDD::PixelDiodeType::LARGE};

  SG::ReadCondHandle<PixelModuleData> configDataHandle(m_configKey, ctx);
  const PixelModuleData *configData = *configDataHandle;

  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelChargeCalibCondData> writeCdo(std::make_unique<PixelChargeCalibCondData>(m_pixelID->wafer_hash_max()));

  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 0,                       0,                       EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};

  EventIDRange rangeW{start, stop};
  unsigned int channel_warnings=0;
  unsigned int max_channel_warnings=10;
  unsigned int min_invalid_channel=std::numeric_limits<unsigned int>::max();
  unsigned int max_invalid_channel=0;
  if (!m_readKey.empty()) {
    SG::ReadCondHandle<CondAttrListCollection> readHandle(m_readKey, ctx);
    const CondAttrListCollection* readCdo = *readHandle;
    if (readCdo==nullptr) {
      ATH_MSG_FATAL("Null pointer to the read conditions object");
      return StatusCode::FAILURE;
    }
    // Get the validitiy range
    if (not readHandle.range(rangeW)) {
      ATH_MSG_FATAL("Failed to retrieve validity range for " << readHandle.key());
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_INFO("Range of input is " << rangeW);

    for (const auto & attrList : *readCdo) {
      const CondAttrListCollection::ChanNum &channelNumber = attrList.first;
      const CondAttrListCollection::AttributeList &payload = attrList.second;

      // RUN-3 format
      if (payload.exists("data_array") and not payload["data_array"].isNull()) {
        const nlohmann::json jsonData = nlohmann::json::parse(payload["data_array"].data<std::string>());

        for (const auto &[hash, data] : jsonData.items()) {
          const unsigned int moduleHash = std::stoul(hash);
          IdentifierHash wafer_hash = IdentifierHash(moduleHash);
          const InDetDD::SiDetectorElement *element = elements->getDetectorElement(wafer_hash);
          const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign*>(&element->design());
          // in some cases numberOfCircuits returns FEs per half-module
          unsigned int numFE = p_design->numberOfCircuits() < halfModuleThreshold ? p_design->numberOfCircuits() : 2 * p_design->numberOfCircuits();

          std::vector<int> analogThreshold;
          std::vector<int> analogThresholdSigma;
          std::vector<int> analogThresholdNoise;
          std::vector<int> inTimeThreshold;

          std::vector<int> analogThresholdLong;
          std::vector<int> analogThresholdSigmaLong;
          std::vector<int> analogThresholdNoiseLong;
          std::vector<int> inTimeThresholdLong;

          std::vector<int> analogThresholdGanged;
          std::vector<int> analogThresholdSigmaGanged;
          std::vector<int> analogThresholdNoiseGanged;
          std::vector<int> inTimeThresholdGanged;

          std::vector<float> totA;
          std::vector<float> totE;
          std::vector<float> totC;

          std::vector<float> totAGanged;
          std::vector<float> totEGanged;
          std::vector<float> totCGanged;

          std::vector<float> totF;
          std::vector<float> totG;

          std::vector<float> totFGanged;
          std::vector<float> totGGanged;

          std::vector<float> totRes1;
          std::vector<float> totRes2;

          analogThreshold.reserve(numFE);
          analogThresholdSigma.reserve(numFE);
          analogThresholdNoise.reserve(numFE);
          inTimeThreshold.reserve(numFE);

          analogThresholdLong.reserve(numFE);
          analogThresholdSigmaLong.reserve(numFE);
          analogThresholdNoiseLong.reserve(numFE);
          inTimeThresholdLong.reserve(numFE);

          analogThresholdGanged.reserve(numFE);
          analogThresholdSigmaGanged.reserve(numFE);
          analogThresholdNoiseGanged.reserve(numFE);
          inTimeThresholdGanged.reserve(numFE);

          totA.reserve(numFE);
          totE.reserve(numFE);
          totC.reserve(numFE);

          totAGanged.reserve(numFE);
          totEGanged.reserve(numFE);
          totCGanged.reserve(numFE);

          totF.reserve(numFE);
          totG.reserve(numFE);

          totFGanged.reserve(numFE);
          totGGanged.reserve(numFE);

          totRes1.reserve(numFE);
          totRes2.reserve(numFE);

          for (unsigned int j{}; j < numFE; j++) {
            const auto &calibArray = data.at(j);
            if (!calibArray.empty()) {
              // conventional calibration
              if (calibArray.size() != FEStringSize-1) {
                ATH_MSG_FATAL("Parameter size is not consistent(" << FEStringSize << ") " << calibArray.size() << " at (i,j)=(" << moduleHash << "," << j << ")");
                return StatusCode::FAILURE;
              }

              analogThreshold.push_back(calibArray[0].get<int>());
              analogThresholdSigma.push_back(calibArray[1].get<int>());
              analogThresholdNoise.push_back(calibArray[2].get<int>());
              inTimeThreshold.push_back(calibArray[3].get<int>());

              analogThresholdLong.push_back(calibArray[4].get<int>());
              analogThresholdSigmaLong.push_back(calibArray[5].get<int>());
              analogThresholdNoiseLong.push_back(calibArray[6].get<int>());
              inTimeThresholdLong.push_back(calibArray[7].get<int>());

              analogThresholdGanged.push_back(calibArray[8].get<int>());
              analogThresholdSigmaGanged.push_back(calibArray[9].get<int>());
              analogThresholdNoiseGanged.push_back(calibArray[10].get<int>());
              inTimeThresholdGanged.push_back(calibArray[11].get<int>());

              float paramA = calibArray[12].get<float>();
              float paramE = calibArray[13].get<float>();
              float paramC = calibArray[14].get<float>();
              totA.push_back(paramA);
              totE.push_back(paramE);
              totC.push_back(paramC);

              float paramAGanged = calibArray[15].get<float>();
              float paramEGanged = calibArray[16].get<float>();
              float paramCGanged = calibArray[17].get<float>();
              totAGanged.push_back(paramAGanged);
              totEGanged.push_back(paramEGanged);
              totCGanged.push_back(paramCGanged);

              totRes1.push_back(calibArray[18].get<float>());
              totRes2.push_back(calibArray[19].get<float>());

              // Linear extrapolation above large charge
              if (configData->getPIXLinearExtrapolation() && p_design->getReadoutTechnology()==InDetDD::PixelReadoutTechnology::FEI3) {
                writeCdo -> setCalibrationStrategy(moduleHash, PixelChargeCalibCondData::CalibrationStrategy::RUN3PIX);

                Identifier wafer_id = m_pixelID->wafer_id(IdentifierHash(moduleHash));
                int barrel_ec = m_pixelID->barrel_ec(wafer_id);
                int layer     = m_pixelID->layer_disk(wafer_id);

                // search for ToT when charge exceeds threshold
                if (!(element->isDBM())) {
                  int totlimit = -1;
                  float exth = 1e5;   // The calibration function is analytically connected at the threshold exth.

                  // Normal pixels
                  for (int itot=configData->getToTThreshold(barrel_ec,layer); itot<configData->getFEI3Latency(barrel_ec,layer); itot++) {
                    float tmpcharge = (paramC*itot/paramA-paramE)/(1.0-itot/paramA);
                    if (tmpcharge>exth) { totlimit=itot; break; }
                  }
                  if (totlimit>0) {
                    float x1 = totlimit;
                    float x2 = totlimit-5;
                    float y1 = (paramC*x1/paramA-paramE)/(1.0-x1/paramA);
                    float y2 = (paramC*x2/paramA-paramE)/(1.0-x2/paramA);
                    totF.push_back((y1-y2)/(x1-x2));
                    totG.push_back((y2*x1-y1*x2)/(x1-x2));
                  }
                  else {
                    totF.push_back(0.0);
                    totG.push_back(0.0);
                  }

                  // Ganged pixels
                  totlimit = -1;
                  for (int itot=configData->getToTThreshold(barrel_ec,layer); itot<configData->getFEI3Latency(barrel_ec,layer); itot++) {
                    float tmpcharge = (paramCGanged*itot/paramAGanged-paramEGanged)/(1.0-itot/paramAGanged);
                    if (tmpcharge>exth) { totlimit=itot; break; }
                  }
                  if (totlimit>0) {
                    float x1 = totlimit;
                    float x2 = totlimit-5;
                    float y1 = (paramCGanged*x1/paramAGanged-paramEGanged)/(1.0-x1/paramAGanged);
                    float y2 = (paramCGanged*x2/paramAGanged-paramEGanged)/(1.0-x2/paramAGanged);
                    totFGanged.push_back((y1-y2)/(x1-x2));
                    totGGanged.push_back((y2*x1-y1*x2)/(x1-x2));
                  }
                  else {
                    totFGanged.push_back(0.0);
                    totGGanged.push_back(0.0);
                  }
                }
                else {    // DBM
                  totF.push_back(0.0);
                  totG.push_back(0.0);
                  totFGanged.push_back(0.0);
                  totGGanged.push_back(0.0);
                }
              }
              else {
                writeCdo -> setCalibrationStrategy(moduleHash, PixelChargeCalibCondData::CalibrationStrategy::RUN1PIX);
                totF.push_back(0.0);
                totG.push_back(0.0);
                totFGanged.push_back(0.0);
                totGGanged.push_back(0.0);
              }
            }
            else {
              ATH_MSG_FATAL("Array size is zero in " << calibArray << " at (i,j)=(" << moduleHash << "," << j << ")");
              return StatusCode::FAILURE;
            }
          }

          // Normal pixel
          writeCdo -> setAnalogThreshold(InDetDD::PixelDiodeType::NORMAL, moduleHash, std::move(analogThreshold));
          writeCdo -> setAnalogThresholdSigma(InDetDD::PixelDiodeType::NORMAL, moduleHash, std::move(analogThresholdSigma));
          writeCdo -> setAnalogThresholdNoise(InDetDD::PixelDiodeType::NORMAL, moduleHash, std::move(analogThresholdNoise));
          writeCdo -> setInTimeThreshold(InDetDD::PixelDiodeType::NORMAL, moduleHash, std::move(inTimeThreshold));

          writeCdo -> setQ2TotA(InDetDD::PixelDiodeType::NORMAL, moduleHash, std::vector<float> (totA)); // can not move as shared
          writeCdo -> setQ2TotE(InDetDD::PixelDiodeType::NORMAL, moduleHash, std::vector<float> (totE)); // can not move as shared
          writeCdo -> setQ2TotC(InDetDD::PixelDiodeType::NORMAL, moduleHash, std::vector<float> (totC)); // can not move as shared

          writeCdo -> setQ2TotF(InDetDD::PixelDiodeType::NORMAL, moduleHash, std::vector<float> (totF)); // can not move as shared
          writeCdo -> setQ2TotG(InDetDD::PixelDiodeType::NORMAL, moduleHash, std::vector<float> (totG)); // can not move as shared

          writeCdo -> setTotRes1(moduleHash, std::move(totRes1));
          writeCdo -> setTotRes2(moduleHash, std::move(totRes2));

          // Long pixel
          writeCdo -> setAnalogThreshold(InDetDD::PixelDiodeType::LONG, moduleHash, std::move(analogThresholdLong));
          writeCdo -> setAnalogThresholdSigma(InDetDD::PixelDiodeType::LONG, moduleHash, std::move(analogThresholdSigmaLong));
          writeCdo -> setAnalogThresholdNoise(InDetDD::PixelDiodeType::LONG, moduleHash, std::move(analogThresholdNoiseLong));
          writeCdo -> setInTimeThreshold(InDetDD::PixelDiodeType::LONG, moduleHash, std::move(inTimeThresholdLong));

          writeCdo -> setQ2TotA(InDetDD::PixelDiodeType::LONG, moduleHash, std::move(totA) ); // can move now
          writeCdo -> setQ2TotE(InDetDD::PixelDiodeType::LONG, moduleHash, std::move(totE) ); // can move now
          writeCdo -> setQ2TotC(InDetDD::PixelDiodeType::LONG, moduleHash, std::move(totC) ); // can move now

          writeCdo -> setQ2TotF(InDetDD::PixelDiodeType::LONG, moduleHash, std::move(totF)); // can move now
          writeCdo -> setQ2TotG(InDetDD::PixelDiodeType::LONG, moduleHash, std::move(totG)); // can move now

          // Ganged/large pixel
          writeCdo -> setAnalogThreshold(InDetDD::PixelDiodeType::GANGED, moduleHash, std::move(analogThresholdGanged));
          writeCdo -> setAnalogThresholdSigma(InDetDD::PixelDiodeType::GANGED, moduleHash, std::move(analogThresholdSigmaGanged));
          writeCdo -> setAnalogThresholdNoise(InDetDD::PixelDiodeType::GANGED, moduleHash, std::move(analogThresholdNoiseGanged));
          writeCdo -> setInTimeThreshold(InDetDD::PixelDiodeType::GANGED, moduleHash, std::move(inTimeThresholdGanged));

          writeCdo -> setQ2TotA(InDetDD::PixelDiodeType::GANGED, moduleHash, std::move(totAGanged));
          writeCdo -> setQ2TotE(InDetDD::PixelDiodeType::GANGED, moduleHash, std::move(totEGanged));
          writeCdo -> setQ2TotC(InDetDD::PixelDiodeType::GANGED, moduleHash, std::move(totCGanged));

          writeCdo -> setQ2TotF(InDetDD::PixelDiodeType::GANGED, moduleHash, std::move(totFGanged));
          writeCdo -> setQ2TotG(InDetDD::PixelDiodeType::GANGED, moduleHash, std::move(totGGanged));
        }
      }
      // RUN-2 format
      else if (payload.exists("data") and not payload["data"].isNull()) {
        // ignore invalid channelNumbers
        // otherwise usage of e.g. CONDBR2-HLTP-2018-03 will lead to range errors.
        if (channelNumber >= m_pixelID->wafer_hash_max()) {
           min_invalid_channel = std::min(min_invalid_channel,channelNumber);
           max_invalid_channel = std::max(max_invalid_channel,channelNumber);
           if (channel_warnings++ < max_channel_warnings) {
              ATH_MSG_WARNING("Invalid module hash (COOL channel number: " << channelNumber << " !< " <<  m_pixelID->wafer_hash_max() << ")."
                              << (channel_warnings==max_channel_warnings ? " Further such warnings will not be reported." : ""));
           }
           continue;
        }
        const unsigned int moduleHash = channelNumber;
        const InDetDD::SiDetectorElement *element = elements->getDetectorElement(IdentifierHash(moduleHash));
        const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign*>(&element->design());

        std::string stringStatus = payload["data"].data<std::string>();

        std::stringstream ss(stringStatus);
        std::vector<std::string> component;
        std::string buffer;
        std::getline(ss, buffer, '\n'); // skip first line
        while (std::getline(ss, buffer, '\n')) { component.push_back(buffer); }

        std::vector<int> analogThreshold;
        std::vector<int> analogThresholdSigma;
        std::vector<int> analogThresholdNoise;
        std::vector<int> inTimeThreshold;

        std::vector<int> analogThresholdLong;
        std::vector<int> analogThresholdSigmaLong;
        std::vector<int> analogThresholdNoiseLong;
        std::vector<int> inTimeThresholdLong;

        std::vector<int> analogThresholdGanged;
        std::vector<int> analogThresholdSigmaGanged;
        std::vector<int> analogThresholdNoiseGanged;
        std::vector<int> inTimeThresholdGanged;

        std::vector<float> totA;
        std::vector<float> totE;
        std::vector<float> totC;

        std::vector<float> totAGanged;
        std::vector<float> totEGanged;
        std::vector<float> totCGanged;

        std::vector<float> totF;
        std::vector<float> totG;

        std::vector<float> totFGanged;
        std::vector<float> totGGanged;

        std::vector<float> totRes1;
        std::vector<float> totRes2;

        analogThreshold.reserve(component.size());
        analogThresholdSigma.reserve(component.size());
        analogThresholdNoise.reserve(component.size());
        inTimeThreshold.reserve(component.size());

        analogThresholdLong.reserve(component.size());
        analogThresholdSigmaLong.reserve(component.size());
        analogThresholdNoiseLong.reserve(component.size());
        inTimeThresholdLong.reserve(component.size());

        analogThresholdGanged.reserve(component.size());
        analogThresholdSigmaGanged.reserve(component.size());
        analogThresholdNoiseGanged.reserve(component.size());
        inTimeThresholdGanged.reserve(component.size());

        totA.reserve(component.size());
        totE.reserve(component.size());
        totC.reserve(component.size());

        totAGanged.reserve(component.size());
        totEGanged.reserve(component.size());
        totCGanged.reserve(component.size());

        totF.reserve(component.size());
        totG.reserve(component.size());

        totFGanged.reserve(component.size());
        totGGanged.reserve(component.size());

        totRes1.reserve(component.size());
        totRes2.reserve(component.size());

        // loop over FEs
        for (size_t i{}; i < component.size(); i++) {
          std::stringstream checkFE(component[i]);
          std::vector<std::string> FEString;
          while (std::getline(checkFE, buffer, ' ')) { FEString.push_back(buffer); }

          if (FEString.size() < FEStringSize) {
            ATH_MSG_ERROR("size of FEString is " << FEString.size() << " and is less than expected, " << FEStringSize << ".");
            ATH_MSG_ERROR("This is the problem in the contents in conditions DB. This should rather be fixed in DB-side.");
            return StatusCode::FAILURE;
          }

          analogThreshold.push_back(std::stoi(FEString[1]));
          analogThresholdSigma.push_back(std::stoi(FEString[2]));
          analogThresholdNoise.push_back(std::stoi(FEString[3]));
          inTimeThreshold.push_back(std::stoi(FEString[4]));

          analogThresholdLong.push_back(std::stoi(FEString[5]));
          analogThresholdSigmaLong.push_back(std::stoi(FEString[6]));
          analogThresholdNoiseLong.push_back(std::stoi(FEString[7]));
          inTimeThresholdLong.push_back(std::stoi(FEString[8]));

          analogThresholdGanged.push_back(std::stoi(FEString[9]));
          analogThresholdSigmaGanged.push_back(std::stoi(FEString[10]));
          analogThresholdNoiseGanged.push_back(std::stoi(FEString[11]));
          inTimeThresholdGanged.push_back(std::stoi(FEString[12]));

          float paramA = std::stof(FEString[13]);
          float paramE = std::stof(FEString[14]);
          float paramC = std::stof(FEString[15]);
          totA.push_back(paramA);
          totE.push_back(paramE);
          totC.push_back(paramC);

          float paramAGanged = std::stof(FEString[16]);
          float paramEGanged = std::stof(FEString[17]);
          float paramCGanged = std::stof(FEString[18]);
          totAGanged.push_back(paramAGanged);
          totEGanged.push_back(paramEGanged);
          totCGanged.push_back(paramCGanged);

          totRes1.push_back(std::stof(FEString[19]));
          totRes2.push_back(std::stof(FEString[20]));

          // Linear extrapolation above large charge
          if (configData->getPIXLinearExtrapolation() && p_design->getReadoutTechnology()==InDetDD::PixelReadoutTechnology::FEI3) {
            writeCdo -> setCalibrationStrategy(moduleHash, PixelChargeCalibCondData::CalibrationStrategy::RUN3PIX);

            Identifier wafer_id = m_pixelID->wafer_id(IdentifierHash(moduleHash));
            int barrel_ec = m_pixelID->barrel_ec(wafer_id);
            int layer     = m_pixelID->layer_disk(wafer_id);

            // search for ToT when charge exceeds threshold
            int totlimit = -1;
            float exth = 1e5;   // The calibration function is analytically connected at the threshold exth.

            // Normal pixels
            for (int itot=configData->getToTThreshold(barrel_ec,layer); itot<configData->getFEI3Latency(barrel_ec,layer); itot++) {
              float tmpcharge = (paramC*itot/paramA-paramE)/(1.0-itot/paramA);
              if (tmpcharge>exth) { totlimit=itot; break; }
            }
            if (totlimit>0) {
              float x1 = totlimit;
              float x2 = totlimit-5;
              float y1 = (paramC*x1/paramA-paramE)/(1.0-x1/paramA);
              float y2 = (paramC*x2/paramA-paramE)/(1.0-x2/paramA);
              totF.push_back((y1-y2)/(x1-x2));
              totG.push_back((y2*x1-y1*x2)/(x1-x2));
            }
            else {
              totF.push_back(0.0);
              totG.push_back(0.0);
            }

            // Ganged pixels
            totlimit = -1;
            for (int itot=configData->getToTThreshold(barrel_ec,layer); itot<configData->getFEI3Latency(barrel_ec,layer); itot++) {
              float tmpcharge = (paramCGanged*itot/paramAGanged-paramEGanged)/(1.0-itot/paramAGanged);
              if (tmpcharge>exth) { totlimit=itot; break; }
            }
            if (totlimit>0) {
              float x1 = totlimit;
              float x2 = totlimit-5;
              float y1 = (paramCGanged*x1/paramAGanged-paramEGanged)/(1.0-x1/paramAGanged);
              float y2 = (paramCGanged*x2/paramAGanged-paramEGanged)/(1.0-x2/paramAGanged);
              totFGanged.push_back((y1-y2)/(x1-x2));
              totGGanged.push_back((y2*x1-y1*x2)/(x1-x2));
            }
            else {
              totFGanged.push_back(0.0);
              totGGanged.push_back(0.0);
            }
          }
          else {
            writeCdo -> setCalibrationStrategy(moduleHash, PixelChargeCalibCondData::CalibrationStrategy::RUN1PIX);
            totF.push_back(0.0);
            totG.push_back(0.0);
            totFGanged.push_back(0.0);
            totGGanged.push_back(0.0);
          }
        }

        // Normal pixel
        writeCdo -> setAnalogThreshold(InDetDD::PixelDiodeType::NORMAL, channelNumber, std::move(analogThreshold));
        writeCdo -> setAnalogThresholdSigma(InDetDD::PixelDiodeType::NORMAL, channelNumber, std::move(analogThresholdSigma));
        writeCdo -> setAnalogThresholdNoise(InDetDD::PixelDiodeType::NORMAL, channelNumber, std::move(analogThresholdNoise));
        writeCdo -> setInTimeThreshold(InDetDD::PixelDiodeType::NORMAL, channelNumber, std::move(inTimeThreshold));

        writeCdo -> setQ2TotA(InDetDD::PixelDiodeType::NORMAL, channelNumber, std::vector<float> (totA)); // can not move as shared
        writeCdo -> setQ2TotE(InDetDD::PixelDiodeType::NORMAL, channelNumber, std::vector<float> (totE)); // can not move as shared
        writeCdo -> setQ2TotC(InDetDD::PixelDiodeType::NORMAL, channelNumber, std::vector<float> (totC)); // can not move as shared

        writeCdo -> setQ2TotF(InDetDD::PixelDiodeType::NORMAL, channelNumber, std::vector<float> (totF)); // can not move as shared
        writeCdo -> setQ2TotG(InDetDD::PixelDiodeType::NORMAL, channelNumber, std::vector<float> (totG)); // can not move as shared

        writeCdo -> setTotRes1(channelNumber, std::move(totRes1));
        writeCdo -> setTotRes2(channelNumber, std::move(totRes2));

        // Long pixel
        writeCdo -> setAnalogThreshold(InDetDD::PixelDiodeType::LONG, channelNumber, std::move(analogThresholdLong));
        writeCdo -> setAnalogThresholdSigma(InDetDD::PixelDiodeType::LONG, channelNumber, std::move(analogThresholdSigmaLong));
        writeCdo -> setAnalogThresholdNoise(InDetDD::PixelDiodeType::LONG, channelNumber, std::move(analogThresholdNoiseLong));
        writeCdo -> setInTimeThreshold(InDetDD::PixelDiodeType::LONG, channelNumber, std::move(inTimeThresholdLong));

        writeCdo -> setQ2TotA(InDetDD::PixelDiodeType::LONG, channelNumber, std::move(totA)); // can move now
        writeCdo -> setQ2TotE(InDetDD::PixelDiodeType::LONG, channelNumber, std::move(totE)); // can move now
        writeCdo -> setQ2TotC(InDetDD::PixelDiodeType::LONG, channelNumber, std::move(totC)); // can move now

        writeCdo -> setQ2TotF(InDetDD::PixelDiodeType::LONG, channelNumber, std::move(totF)); // can move now
        writeCdo -> setQ2TotG(InDetDD::PixelDiodeType::LONG, channelNumber, std::move(totG)); // can move now

        // Ganged pixel
        writeCdo -> setAnalogThreshold(InDetDD::PixelDiodeType::GANGED, channelNumber, std::move(analogThresholdGanged));
        writeCdo -> setAnalogThresholdSigma(InDetDD::PixelDiodeType::GANGED, channelNumber, std::move(analogThresholdSigmaGanged));
        writeCdo -> setAnalogThresholdNoise(InDetDD::PixelDiodeType::GANGED, channelNumber, std::move(analogThresholdNoiseGanged));
        writeCdo -> setInTimeThreshold(InDetDD::PixelDiodeType::GANGED, channelNumber, std::move(inTimeThresholdGanged));

        writeCdo -> setQ2TotA(InDetDD::PixelDiodeType::GANGED, channelNumber, std::move(totAGanged));
        writeCdo -> setQ2TotE(InDetDD::PixelDiodeType::GANGED, channelNumber, std::move(totEGanged));
        writeCdo -> setQ2TotC(InDetDD::PixelDiodeType::GANGED, channelNumber, std::move(totCGanged));
        writeCdo -> setQ2TotF(InDetDD::PixelDiodeType::GANGED, channelNumber, std::move(totFGanged));
        writeCdo -> setQ2TotG(InDetDD::PixelDiodeType::GANGED, channelNumber, std::move(totGGanged));
      } else {
        ATH_MSG_ERROR("payload[\"data\"] does not exist for ChanNum " << channelNumber);
        return StatusCode::FAILURE;
      }
    }
  }
  else {
    for (unsigned int moduleHash{}; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++) {
      IdentifierHash wafer_hash = IdentifierHash(moduleHash);
      Identifier wafer_id = m_pixelID->wafer_id(wafer_hash);
      int barrel_ec = m_pixelID->barrel_ec(wafer_id);
      int layer     = m_pixelID->layer_disk(wafer_id);
      const InDetDD::SiDetectorElement *element = elements->getDetectorElement(wafer_hash);
      const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign*>(&element->design());
      // in some cases numberOfCircuits returns FEs per half-module
      unsigned int numFE = p_design->numberOfCircuits() < halfModuleThreshold ? p_design->numberOfCircuits() : 2 * p_design->numberOfCircuits();

      for (InDetDD::PixelDiodeType type : diodeTypes) {
        writeCdo -> setAnalogThreshold(type, moduleHash, std::vector<int>(numFE, configData->getDefaultAnalogThreshold(barrel_ec, layer)));
        writeCdo -> setAnalogThresholdSigma(type, moduleHash, std::vector<int>(numFE, configData->getDefaultAnalogThresholdSigma(barrel_ec, layer)));
        writeCdo -> setAnalogThresholdNoise(type, moduleHash, std::vector<int>(numFE, configData->getDefaultAnalogThresholdNoise(barrel_ec, layer)));
        writeCdo -> setInTimeThreshold(type, moduleHash, std::vector<int>(numFE, configData->getDefaultInTimeThreshold(barrel_ec, layer)));

        writeCdo -> setQ2TotA(type, moduleHash, std::vector<float>(numFE, configData->getDefaultQ2TotA()));
        writeCdo -> setQ2TotE(type, moduleHash, std::vector<float>(numFE, configData->getDefaultQ2TotE()));
        writeCdo -> setQ2TotC(type, moduleHash, std::vector<float>(numFE, configData->getDefaultQ2TotC()));
        writeCdo -> setQ2TotF(type, moduleHash, std::vector<float>(numFE, 0.0));
        writeCdo -> setQ2TotG(type, moduleHash, std::vector<float>(numFE, 0.0));
      }

      writeCdo -> setTotRes1(moduleHash, std::vector<float>(numFE, 0.0));
      writeCdo -> setTotRes2(moduleHash, std::vector<float>(numFE, 0.0));
    }
  }
  if (channel_warnings>max_channel_warnings) {
     ATH_MSG_WARNING("Encountered " << channel_warnings << " invalid channel numbers (range " << min_invalid_channel << " .. "
                     << max_invalid_channel << " !< " <<  m_pixelID->wafer_hash_max() << ")");
  }


  // Scan over if the DB contents need to be overwritten.
  // This is useful for threshold study. So far only threshold value.
  for (unsigned int moduleHash{}; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++) {
    IdentifierHash wafer_hash = IdentifierHash(moduleHash);
    Identifier wafer_id = m_pixelID->wafer_id(wafer_hash);
    int barrel_ec = m_pixelID->barrel_ec(wafer_id);
    int layer     = m_pixelID->layer_disk(wafer_id);
    const InDetDD::SiDetectorElement *element = elements->getDetectorElement(wafer_hash);
    const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign*>(&element->design());
    // in some cases numberOfCircuits returns FEs per half-module
    unsigned int numFE = p_design->numberOfCircuits() < halfModuleThreshold ? p_design->numberOfCircuits() : 2 * p_design->numberOfCircuits();

    if (configData->getDefaultAnalogThreshold(barrel_ec, layer) > -0.1) {
      for (InDetDD::PixelDiodeType type : diodeTypes) {
        writeCdo -> setAnalogThreshold(type, moduleHash, std::vector<int>(numFE, configData->getDefaultAnalogThreshold(barrel_ec, layer)));
      }
    }
  }

  if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelChargeCalibCondData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
}
