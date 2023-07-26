/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file LUTChargeCalibParser.h
 * @author Shaun Roe
 * @date July, 2023
 * @brief Parses a database run4 Look-Up-Table format string to a ChargeCalibrationBundle
 */
#ifndef LUTChargeCalibParser_h
#define LUTChargeCalibParser_h
#include "IChargeCalibrationParser.h"
#include "PixelConditionsData/ChargeCalibrationBundle.h" 
#include <nlohmann/json.hpp>

#include <string>

class PixelModuleData;
class PixelID;
 
namespace PixelChargeCalib{
  class LUTChargeCalibParser : public IChargeCalibrationParser{
  public:
    LUTChargeCalibParser(const PixelModuleData * pModData, 
     const InDetDD::SiDetectorElementCollection * pElements, 
     const PixelID * pId):IChargeCalibrationParser(pModData,pElements, pId){
      //nop
    }
    
  private:
    virtual ChargeCalibrationBundle 
    parseImpl(unsigned int moduleHash, const nlohmann::json & data) override final;
    
    virtual ChargeCalibrationBundle
    parseImpl(unsigned int /*moduleHash*/, const std::string & /*data*/) override final {
      PixelChargeCalib::ChargeCalibrationBundle b(0);
      b.isValid=false;
      return b;
    }
  };

}
#endif
