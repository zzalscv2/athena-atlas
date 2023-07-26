/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file IChargeCalibrationParser.h
 * @author Shaun Roe
 * @date July, 2023
 * @brief Interface to parsers which accept a string or json object and return a ChargeCalibrationBundle
 */
#ifndef IChargeCalibrationParser_h
#define IChargeCalibrationParser_h

#include "PixelConditionsData/ChargeCalibrationBundle.h" 
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include <nlohmann/json.hpp>
#include "Identifier/IdentifierHash.h"
#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"

#include <string>
#include <utility> //for std::pair
#include <functional> //std::function
#include <type_traits>


class PixelModuleData;


namespace PixelChargeCalib{
  class IChargeCalibrationParser{
  public:
    IChargeCalibrationParser(const PixelModuleData * pModData, 
      const InDetDD::SiDetectorElementCollection * pElements, const PixelID * pId): m_configData(pModData), 
        m_elements(pElements), m_pixelID(pId){
      
    }
    
    //the interface dispatches to one of the implementations in derived classes
    template<class T> 
    ChargeCalibrationBundle parse(unsigned int hash, const T & data){ 
      return parseImpl(hash, data); 
    }
    virtual ~IChargeCalibrationParser() = default;
    
  protected:
    const PixelModuleData * m_configData{};
    const InDetDD::SiDetectorElementCollection * m_elements{};
    const PixelID* m_pixelID{};
    ///If the calculated charge exceeds this limit, a linear extrapolation is used at this point
    static constexpr float m_chargeLimit = 1e5; 
    //
    std::pair<int, int>
    getBecAndLayer(const IdentifierHash& hash) const{
      const Identifier & waferId = m_pixelID->wafer_id(hash);
      return  {m_pixelID->barrel_ec(waferId), m_pixelID->layer_disk(waferId)};
    }
    ///Return function converting a string to number type T at index i of data vector
    template<typename T, typename b = std::is_integral<T>>
    std::function<T(size_t)>
    getFunc(const std::vector<std::string> & data){
      auto f = [&data](size_t i)->T {
        return b::value ? std::stoi(data[i]) : std::stof(data[i]);
      };
      return f;
    } 
    
    ///Return function converting an item to number type T at index i of json  data
    template<typename T>
    std::function<T(size_t)>
    getFunc(const nlohmann::json & data){
      auto f = [&data](size_t i)->T {
        return data[i].get<T>();
      };
      return f;
    }  
    
    
  private:
    virtual ChargeCalibrationBundle
    parseImpl(unsigned int  /*hash*/, const nlohmann::json & /*data*/) = 0;
    virtual ChargeCalibrationBundle
    parseImpl(unsigned int  /*hash*/, const std::string & /*data*/) = 0;
 };
}

#endif
