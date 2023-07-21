/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ChargeCalibrationBundle.h
 * @author Shaun Roe
 * @date July, 2023
 * @brief Struct for holding vectors of charge calibration constants, with utility methods
 */
 
#ifndef ChargeCalibrationBundle_h
#define ChargeCalibrationBundle_h

#include "PixelReadoutDefinitions/PixelReadoutDefinitions.h" //diode types
#include "PixelConditionsData/ChargeCalibParameters.h" //Thresholds, LegacyFitParameters etc
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include <utility> // for std::pair
#include <vector>
#include <map>
 
namespace PixelChargeCalib{
  ///bundles of parameters used together in the PixelChargeCalibCondAlg
  struct ChargeCalibrationBundle{
    bool isValid=true;
    std::vector<PixelChargeCalib::Thresholds> threshold;
    std::vector<PixelChargeCalib::Thresholds> thresholdLong;
    std::vector<PixelChargeCalib::Thresholds> thresholdGanged;
    std::vector<PixelChargeCalib::LegacyFitParameters> params;
    std::vector<PixelChargeCalib::LegacyFitParameters> paramsGanged;
    std::vector<PixelChargeCalib::LinearFitParameters> lin;
    std::vector<PixelChargeCalib::LinearFitParameters> linGanged;
    std::vector<PixelChargeCalib::Resolutions> totRes;
    PixelChargeCalibCondData::CalibrationStrategy  calibrationType{PixelChargeCalibCondData::CalibrationStrategy::RUN1PIX};
    //
    ///constructor with reserve for the vectors, n = number of frontends
    ChargeCalibrationBundle(size_t n){
      threshold.reserve(n);
      thresholdLong.reserve(n);
      thresholdGanged.reserve(n);
      params.reserve(n);
      paramsGanged.reserve(n);
      lin.reserve(n);
      linGanged.reserve(n);
      totRes.reserve(n);
    }
    //return  the index, within range (start, end), where the charge threshold is exceeded
    //or -1 if it is not
    int 
    idxAtChargeLimit(float chargeLimit, InDetDD::PixelDiodeType type, int start, int end) const {
      const auto &relevantArray = (type == InDetDD::PixelDiodeType::NORMAL) ? params : paramsGanged;
      int idx=-1;
      for (int i = start;i!=end;++i){
        const float tmpcharge = relevantArray.back().Q(i);
        if (tmpcharge > chargeLimit){
          idx = i;
          break;
        }
      }
      return idx;
    }
    
    void
    insertLinearParams(InDetDD::PixelDiodeType type, int idxLimit){
      auto &targetArray = (type == InDetDD::PixelDiodeType::NORMAL) ? lin : linGanged;
      if (idxLimit <= 0){ //duplicates original logic
        targetArray.emplace_back(0.f,0.f);
        return;
      }
      //not entirely satisfactory, the fit derivatives are not continuous
      //and a difference of 5 seems a bit arbitrary
      const auto &readArray = (type == InDetDD::PixelDiodeType::NORMAL) ? params : paramsGanged;
      static constexpr int dx = 5;
      const float x1 = idxLimit;
      const float x2 = idxLimit-dx;
      const float y1 = readArray.back().Q(x1);
      const float y2 = readArray.back().Q(x2);
      targetArray.emplace_back((y1-y2)/dx, (y2*x1-y1*x2)/dx);
    }
  };
}

#endif
