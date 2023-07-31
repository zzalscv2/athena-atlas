/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELCHARGECALIBCONDDATA_H
#define PIXELCHARGECALIBCONDDATA_H

#include <AthenaKernel/CLASS_DEF.h>
#include <PixelReadoutDefinitions/PixelReadoutDefinitions.h>

#include <map>
#include <vector>
#include <array>
namespace PixelChargeCalib{
  struct ChargeCalibrationBundle;
  struct LegacyFitParameters;
  struct LinearFitParameters;
  struct Thresholds;
  struct Resolutions;
}

class PixelModuleData;
class PixelID;

class PixelChargeCalibCondData
{
  public:
    PixelChargeCalibCondData() = default;
    PixelChargeCalibCondData(std::size_t max_module_hash);

    static constexpr size_t IBLCalibrationSize{16};
    using IBLCalibration = std::array<float, IBLCalibrationSize>;
    using IBLModule = std::vector<IBLCalibration>;

    enum class CalibrationStrategy
    {
      RUN1PIX,
      RUN3PIX,
      LUTFEI4,
      RD53
    };

    void 
    setAllFromBundle(unsigned int moduleHash, const PixelChargeCalib::ChargeCalibrationBundle& b);
    
    void 
    setAllFromConfigData(unsigned int moduleHash, const PixelModuleData * configData, const std::pair<int, int> &becLayer, unsigned int numFE);
    
    void 
    setThresholds(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::Thresholds> & thresholds);
    //
    void 
    setAnalogThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value);
    //
    void 
    setLegacyFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::LegacyFitParameters> &parameters);
    //
    void 
    setLinearFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::LinearFitParameters> &parameters);
    //
    void 
    setTotResolutions(unsigned int moduleHash, const std::vector<PixelChargeCalib::Resolutions> &value);
    //
    PixelChargeCalib::Thresholds 
    getThresholds(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    //
    PixelChargeCalib::LegacyFitParameters 
    getLegacyFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    //
    PixelChargeCalib::LinearFitParameters 
    getLinearFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    //
    float 
    getQ2TotF(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    //
    float 
    getQ2TotG(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    //
    float 
    getTotRes(unsigned int moduleHash, unsigned int FE, float Q) const;
    //
    float 
    getToT(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE, float Q) const;
    //
    float 
    getCharge(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE, float ToT) const;
    //
    // new IBL calibration
    void 
    setCalibrationStrategy(unsigned int moduleHash, CalibrationStrategy strategy);
    //
    void 
    setTot2Charges(unsigned int moduleHash, IBLModule charges);
    //
    const IBLCalibration &
    getTot2Charges(unsigned int moduleHash, unsigned int FE) const;
    //
    CalibrationStrategy 
    getCalibrationStrategy(unsigned int moduleHash) const;
    //
    float 
    getChargeLUTFEI4(unsigned int moduleHash, unsigned int FE, unsigned int ToT) const;
    //
    float 
    getToTLUTFEI4(unsigned int moduleHash, unsigned int FE, float Q) const;

  private:
    std::size_t m_maxModuleHash = 0;

    constexpr static std::size_t s_NPixelDiodes = enum2uint(InDetDD::PixelDiodeType::N_DIODETYPES);

   template <typename T>
    static void resize(std::size_t idx, std::size_t max_size, T &container)  { if (idx >= container.size()) { container.resize(max_size); } }
    template <typename T, typename T_Value>
    static void setValue(std::size_t max_size, T &container, unsigned int moduleHash, T_Value &&value)  {
       resize( moduleHash,max_size, container);
       container.at(moduleHash) = std::move(value);
    }

    template <typename T, typename T_Value>
    static void setValue(std::size_t max_size, T &container, InDetDD::PixelDiodeType type, unsigned int moduleHash, T_Value &&value)  {
       resize( moduleHash,max_size, container.at(enum2uint(type)));
       container.at(enum2uint(type)).at(moduleHash) = std::move(value);
    }

    using chipThreshold = std::vector<std::vector<int>>;
    using chipCharge = std::vector<std::vector<float>>;
    using chipThresholdMap = std::array<chipThreshold, s_NPixelDiodes>;
    using chipChargeMap = std::array<chipCharge, s_NPixelDiodes>;
        
    chipThresholdMap m_analogThreshold;
    chipThresholdMap m_analogThresholdSigma;
    chipThresholdMap m_analogThresholdNoise;
    chipThresholdMap m_inTimeThreshold;

    chipChargeMap m_totA;
    chipChargeMap m_totE;
    chipChargeMap m_totC;
    chipChargeMap m_totF;
    chipChargeMap m_totG;

    chipCharge m_totRes1;
    chipCharge m_totRes2;

    // new IBL calibration
    std::map<int, CalibrationStrategy> m_calibrationStrategy;
    std::map<int, IBLModule> m_tot2Charges;
    //
    void setQ2TotA(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value);
    void setQ2TotE(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value);
    void setQ2TotC(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value);
    //
    float getQ2TotA(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    float getQ2TotE(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    float getQ2TotC(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    //
    void setQ2TotF(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value);
    void setQ2TotG(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value);
    //
    void setTotRes1(unsigned int moduleHash, std::vector<float> &&value);
    void setTotRes2(unsigned int moduleHash, std::vector<float> &&value);
    //
    void setAnalogThresholdSigma(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value);
    void setAnalogThresholdNoise(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value);
    void setInTimeThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value);
    //
    int getAnalogThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    int getAnalogThresholdSigma(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    int getAnalogThresholdNoise(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    int getInTimeThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;

};

CLASS_DEF( PixelChargeCalibCondData , 345532779 , 1 )

#include "AthenaKernel/CondCont.h"
CONDCONT_DEF( PixelChargeCalibCondData, 578786399 );

#endif
