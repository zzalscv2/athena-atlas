/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "LUTChargeCalibParser.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "PixelConditionsData/ChargeCalibrationBundle.h" 
#include "PixelConditionsData/PixelModuleData.h"

namespace{
  constexpr int halfModuleThreshold{8};
  constexpr size_t FEStringSize{20};
} // namespace


namespace PixelChargeCalib{

  ChargeCalibrationBundle
  LUTChargeCalibParser::parseImpl(unsigned int moduleHash, const nlohmann::json & data){
    IdentifierHash wafer_hash = IdentifierHash(moduleHash);
    const InDetDD::SiDetectorElement *element = m_elements->getDetectorElement(wafer_hash);
    const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign*>(&element->design());
    // in some cases numberOfCircuits returns FEs per half-module
    unsigned int numFE = p_design->numberOfCircuits() < halfModuleThreshold ? p_design->numberOfCircuits() : 2 * p_design->numberOfCircuits();
    //
    ChargeCalibrationBundle b(numFE);
    //
    for (unsigned int j{}; j < numFE; j++) {
      const auto &calibArray = data.at(j);
      if (!calibArray.empty()) {
        // new charge calibration for RUN-3
        if ((p_design->getReadoutTechnology() == InDetDD::PixelReadoutTechnology::FEI4 && !(element->isDBM())) || (b.useLUT && p_design->getReadoutTechnology() == InDetDD::PixelReadoutTechnology::RD53)) {
          if (calibArray.size() != FEStringSize) {
            std::cout<<"Parameter size is not consistent(" << FEStringSize << ") " << calibArray.size() << " at (i,j)=(" << moduleHash << "," << j << ")\n";
            b.isValid = false;
            return b;
          }
          auto getInt = getFunc<int>(calibArray);
          auto getFloat = getFunc<float>(calibArray);
          auto &charges = b.tot2Charges.emplace_back(PixelChargeCalibCondData::IBLCalibration());
          for (size_t k{}; k < PixelChargeCalibCondData::IBLCalibrationSize; k++) {
            charges[k] = getFloat(k + 4ul);
          }
          b.calibrationType = PixelChargeCalibCondData::CalibrationStrategy::LUTFEI4;
          b.threshold.emplace_back(getInt(0), 0.f, getInt(1), 0.f);
          b.thresholdLong.emplace_back(getInt(2), 0.f, getInt(3), 0.f);
          b.thresholdGanged.emplace_back(getInt(2), 0.f, getInt(3), 0.f);
          b.params.emplace_back(0.f, 0.f, 0.f);
          b.paramsGanged.emplace_back(0.f, 0.f, 0.f);
          b.totRes.emplace_back(0.f, 0.f);
        } else { //normal CalibrationStrategy
          if (calibArray.size() != FEStringSize) {
            std::cout<<"Parameter size is not consistent(" << FEStringSize << ") " << calibArray.size() << " at (i,j)=(" << moduleHash << "," << j << ")\n";
            b.isValid = false;
            return b;
          }
          auto getInt = getFunc<int>(calibArray);
          auto getFloat = getFunc<float>(calibArray);
          b.threshold.emplace_back(getInt(0), getInt(1), getInt(2), getInt(3));
          b.thresholdLong.emplace_back(getInt(4), getInt(5), getInt(6), getInt(7));
          b.thresholdGanged.emplace_back(getInt(8), getInt(9), getInt(10), getInt(11));
          b.params.emplace_back(getFloat(12), getFloat(13), getFloat(14));
          b.paramsGanged.emplace_back(getFloat(15), getFloat(16), getFloat(17));
          b.totRes.emplace_back(getFloat(18), getFloat(19));

          // Linear extrapolation above large charge
          if (m_configData->getPIXLinearExtrapolation()) {
            b.calibrationType = PixelChargeCalibCondData::CalibrationStrategy::RUN3PIX;
            const auto & [barrel_ec, layer] = getBecAndLayer(wafer_hash);
            // search for ToT when charge exceeds threshold
            if (!(element->isDBM())) {
              //charge threshold beyond which linear fit will be used
              const int totIdxStart = m_configData->getToTThreshold(barrel_ec,layer);
              const int totIdxEnd = m_configData->getFEI3Latency(barrel_ec,layer);
              // Normal pixels
              int totlimit = b.idxAtChargeLimit(m_chargeLimit, InDetDD::PixelDiodeType::NORMAL, totIdxStart, totIdxEnd);
              b.insertLinearParams( InDetDD::PixelDiodeType::NORMAL, totlimit);
              // Ganged pixels
              totlimit = b.idxAtChargeLimit(m_chargeLimit, InDetDD::PixelDiodeType::GANGED, totIdxStart, totIdxEnd);
              b.insertLinearParams( InDetDD::PixelDiodeType::GANGED, totlimit);
            } else {    // DBM
              b.lin.emplace_back(0.f, 0.f);
              b.linGanged.emplace_back(0.f, 0.f);
            }
          } else {
            b.calibrationType = PixelChargeCalibCondData::CalibrationStrategy::RUN1PIX;
            b.lin.emplace_back(0.f, 0.f);
            b.linGanged.emplace_back(0.f, 0.f);
          }
        }
      } else {
        std::cout<<"Array size is zero in " << calibArray << " at (i,j)=(" << moduleHash << "," << j << ")\n";
        b.isValid = false;
        return b;
      }
    }
    return b;
  }
}
