/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file Run2ChargeCalibParser.h
 * @author Shaun Roe
 * @date July, 2023
 * @brief Parses a database run3 format string to a ChargeCalibrationBundle
 */
#ifndef Run2ChargeCalibParser_h
#define Run2ChargeCalibParser_h
#include "IChargeCalibrationParser.h"
#include "PixelConditionsData/ChargeCalibrationBundle.h" 

#include <string>

class PixelModuleData;
class PixelID;
 
namespace PixelChargeCalib{
  class Run2ChargeCalibParser : public IChargeCalibrationParser{
  public:
    Run2ChargeCalibParser(const PixelModuleData * pModData, 
     const InDetDD::SiDetectorElementCollection * pElements, 
     const PixelID * pId):IChargeCalibrationParser(pModData,pElements, pId){
      //nop
    }
    
  private:    
    virtual ChargeCalibrationBundle 
    parseImpl(unsigned int hash, const std::string & data) override final;
    
    virtual ChargeCalibrationBundle
    parseImpl(unsigned int /*hash*/, const nlohmann::json & /*data*/) override final {
    PixelChargeCalib::ChargeCalibrationBundle b(0);
    b.isValid=false;
    return b;
  }
  };
  

}
#endif
