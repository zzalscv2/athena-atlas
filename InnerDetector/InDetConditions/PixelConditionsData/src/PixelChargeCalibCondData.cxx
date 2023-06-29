/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include <cfloat>


PixelChargeCalibCondData::PixelChargeCalibCondData()
{
   static_assert(static_cast<std::size_t>(InDetDD::PixelDiodeType::NORMAL) < s_NPixelDiods);
   static_assert(static_cast<std::size_t>(InDetDD::PixelDiodeType::LONG)   < s_NPixelDiods);
   static_assert(static_cast<std::size_t>(InDetDD::PixelDiodeType::GANGED) < s_NPixelDiods);
   static_assert(static_cast<std::size_t>(InDetDD::PixelDiodeType::LARGE)  < s_NPixelDiods);
}

PixelChargeCalibCondData::PixelChargeCalibCondData(std::size_t max_module_hash) : m_maxModuleHash(max_module_hash)
{
   static_assert(static_cast<std::size_t>(InDetDD::PixelDiodeType::NORMAL) < s_NPixelDiods);
   static_assert(static_cast<std::size_t>(InDetDD::PixelDiodeType::LONG)   < s_NPixelDiods);
   static_assert(static_cast<std::size_t>(InDetDD::PixelDiodeType::GANGED) < s_NPixelDiods);
   static_assert(static_cast<std::size_t>(InDetDD::PixelDiodeType::LARGE)  < s_NPixelDiods);
}

void PixelChargeCalibCondData::setAnalogThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value)
{
   setValue(m_maxModuleHash, m_analogThreshold, type, moduleHash, std::move(value));
}

void PixelChargeCalibCondData::setAnalogThresholdSigma(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value)
{
   setValue(m_maxModuleHash, m_analogThresholdSigma, type, moduleHash, std::move(value));
}

void PixelChargeCalibCondData::setAnalogThresholdNoise(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value)
{
   setValue(m_maxModuleHash, m_analogThresholdNoise, type, moduleHash, std::move(value));
}

void PixelChargeCalibCondData::setInTimeThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value)
{
   setValue(m_maxModuleHash, m_inTimeThreshold,type, moduleHash, std::move(value));
}

void PixelChargeCalibCondData::setQ2TotA(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value)
{
   setValue(m_maxModuleHash, m_totA,type,moduleHash,std::move(value));
}

void PixelChargeCalibCondData::setQ2TotE(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value)
{
   setValue(m_maxModuleHash, m_totE, type, moduleHash,  std::move(value));
}

void PixelChargeCalibCondData::setQ2TotC(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value)
{
   setValue(m_maxModuleHash, m_totC,type,moduleHash,  std::move(value));
}

void PixelChargeCalibCondData::setQ2TotF(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value)
{
   setValue(m_maxModuleHash, m_totF, type, moduleHash,  std::move(value));
}

void PixelChargeCalibCondData::setQ2TotG(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value)
{
   setValue( m_maxModuleHash, m_totG, type, moduleHash, std::move(value) );
}

void PixelChargeCalibCondData::setTotRes1(unsigned int moduleHash, std::vector<float> &&value) {
   setValue(m_maxModuleHash, m_totRes1, moduleHash,std::move(value));
}

void PixelChargeCalibCondData::setTotRes2(unsigned int moduleHash, std::vector<float> &&value) {
   setValue(m_maxModuleHash, m_totRes2,moduleHash, std::move(value));
}

int PixelChargeCalibCondData::getAnalogThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const
{
  const chipThreshold &typeMap = m_analogThreshold.at(diodeIndex(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }

  std::stringstream error;
  error << "PixelChargeCalibCondData::getAnalogThreshold(" << static_cast<int>(type) << ", " << moduleHash << ", " << FE << "): array out of bounds";
  throw std::range_error(error.str());
}

int PixelChargeCalibCondData::getAnalogThresholdSigma(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const
{
  const chipThreshold &typeMap = m_analogThresholdSigma.at(diodeIndex(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
     return fe_vec.at(FE);
  }

  std::stringstream error;
  error << "PixelChargeCalibCondData::getAnalogThresholdSigma(" << static_cast<int>(type) << ", " << moduleHash << ", " << FE << "): array out of bounds";
  throw std::range_error(error.str());
}

int PixelChargeCalibCondData::getAnalogThresholdNoise(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const
{
  const chipThreshold &typeMap = m_analogThresholdNoise.at(diodeIndex(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }

  std::stringstream error;
  error << "PixelChargeCalibCondData::getAnalogThresholdNoise(" << static_cast<int>(type) << ", " << moduleHash << ", " << FE << "): array out of bounds";
  throw std::range_error(error.str());
}

int PixelChargeCalibCondData::getInTimeThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const
{
  const chipThreshold &typeMap = m_inTimeThreshold.at(diodeIndex(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }

  std::stringstream error;
  error << "PixelChargeCalibCondData::getInTimeThreshold(" << static_cast<int>(type) << ", " << moduleHash << ", " << FE << "): array out of bounds";
  throw std::range_error(error.str());
}

float PixelChargeCalibCondData::getQ2TotA(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const
{
  const chipCharge &typeMap = m_totA.at(diodeIndex(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }

  std::stringstream error;
  error << "PixelChargeCalibCondData::getQ2TotA(" << static_cast<int>(type) << ", " << moduleHash << ", " << FE << "): array out of bounds";
  throw std::range_error(error.str());
}

float PixelChargeCalibCondData::getQ2TotE(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const
{
  const chipCharge &typeMap = m_totE.at(diodeIndex(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }

  std::stringstream error;
  error << "PixelChargeCalibCondData::getQ2TotE(" << static_cast<int>(type) << ", " << moduleHash << ", " << FE << "): array out of bounds";
  throw std::range_error(error.str());
}

float PixelChargeCalibCondData::getQ2TotC(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const
{
  const chipCharge &typeMap = m_totC.at(diodeIndex(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }

  std::stringstream error;
  error << "PixelChargeCalibCondData::getQ2TotC(" << static_cast<int>(type) << ", " << moduleHash << ", " << FE << "): array out of bounds";
  throw std::range_error(error.str());
}

float PixelChargeCalibCondData::getQ2TotF(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const
{
  const chipCharge &typeMap = m_totF.at(diodeIndex(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }

  std::stringstream error;
  error << "PixelChargeCalibCondData::getQ2TotF(" << static_cast<int>(type) << ", " << moduleHash << ", " << FE << "): array out of bounds";
  throw std::range_error(error.str());
}

float PixelChargeCalibCondData::getQ2TotG(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const
{
  const chipCharge &typeMap = m_totG.at(diodeIndex(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }

  std::stringstream error;
  error << "PixelChargeCalibCondData::getQ2TotG(" << static_cast<int>(type) << ", " << moduleHash << ", " << FE << "): array out of bounds";
  throw std::range_error(error.str());
}

float PixelChargeCalibCondData::getTotRes(unsigned int moduleHash, unsigned int FE, float Q) const
{
  float res1{};
  {
  const auto &fe_vec = m_totRes1.at(moduleHash);
  if (FE < fe_vec.size()) {
    res1 = fe_vec[FE];
  } else {
    std::stringstream error;
    error << "PixelChargeCalibCondData::getTotRes(" << moduleHash << ", " << FE << "): res1 array out of bounds";
    throw std::range_error(error.str());
  }
  }

  float res2{};
  {
  const auto &fe_vec = m_totRes2.at(moduleHash);
  if (FE < fe_vec.size()) {
    res2 = fe_vec[FE];
  } else {
    std::stringstream error;
    error << "PixelChargeCalibCondData::getTotRes(" << moduleHash << ", " << FE << "): res2 array out of bounds";
    throw std::range_error(error.str());
  }
  }
  return res1 + res2 * Q;
}

float PixelChargeCalibCondData::getToT(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE, float Q) const
{
  if (getCalibrationStrategy(moduleHash) == CalibrationStrategy::LUTFEI4) {
    return getToTLUTFEI4(moduleHash, FE, Q);
  }

  float paramA = getQ2TotA(type, moduleHash, FE);
  float paramE = getQ2TotE(type, moduleHash, FE);
  float paramC = getQ2TotC(type, moduleHash, FE);
  float tot = 0.0;
  if (paramC + Q != 0.0) {
    tot = paramA * (paramE + Q) / (paramC + Q);
  }

  // Protection for large charge
  float exth = 1e5;    // the calibration function is analytically connected at threshold exth.
  if (Q>exth && getCalibrationStrategy(moduleHash)==CalibrationStrategy::RUN3PIX) {
    float paramF = getQ2TotF(type, moduleHash, FE);
    float paramG = getQ2TotG(type, moduleHash, FE);
    if (paramF != 0.0f){
      tot = (Q-paramG)/paramF;
    }
  }
  return tot;
}

float PixelChargeCalibCondData::getCharge(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE, float ToT) const
{
  if (getCalibrationStrategy(moduleHash) == CalibrationStrategy::LUTFEI4) {
    return getChargeLUTFEI4(moduleHash, FE, ToT);
  }

  float paramA = getQ2TotA(type, moduleHash, FE);
  float paramE = getQ2TotE(type, moduleHash, FE);
  float paramC = getQ2TotC(type, moduleHash, FE);
  float charge = 0.0;
  if (std::fabs(paramA) > 0.0 && std::fabs(ToT / paramA - 1.0) > 0.0) {
    charge = (paramC * ToT / paramA - paramE) / (1.0 - ToT / paramA);
  }

  // Protection for small charge
  float threshold = getAnalogThreshold(type,moduleHash,FE);
  if (charge<threshold && getCalibrationStrategy(moduleHash)==CalibrationStrategy::RUN3PIX) { charge=threshold; }

  // Protection for large charge
  float exth = 1e5;    // the calibration function is analytically connected at threshold exth.
  if (charge>exth && getCalibrationStrategy(moduleHash)==CalibrationStrategy::RUN3PIX) {
    float paramF = getQ2TotF(type, moduleHash, FE);
    float paramG = getQ2TotG(type, moduleHash, FE);
    charge = paramF*ToT+paramG;
  }
  return charge;
}

void PixelChargeCalibCondData::setCalibrationStrategy(unsigned int moduleHash, CalibrationStrategy strategy)
{
  m_calibrationStrategy[moduleHash] = strategy;
}

PixelChargeCalibCondData::CalibrationStrategy PixelChargeCalibCondData::getCalibrationStrategy(unsigned int moduleHash) const
{
  auto itr = m_calibrationStrategy.find(moduleHash);
  if (itr != m_calibrationStrategy.end()) {
    return itr->second;
  }
  return CalibrationStrategy::RUN1PIX;
}

void PixelChargeCalibCondData::setTot2Charges(unsigned int moduleHash, IBLModule charges)
{
  m_tot2Charges[moduleHash] = std::move(charges);
}

const PixelChargeCalibCondData::IBLCalibration &PixelChargeCalibCondData::getTot2Charges(unsigned int moduleHash, unsigned int FE) const
{
  auto it = m_tot2Charges.find(moduleHash);
  if (it != m_tot2Charges.end() && FE < it->second.size()) {
    return it->second.at(FE);
  }

  std::stringstream error;
  error << "PixelChargeCalibCondData::getTot2Charges(" << moduleHash << ", " << FE << "): array out of bounds";
  throw std::range_error(error.str());
}

float PixelChargeCalibCondData::getChargeLUTFEI4(unsigned int moduleHash, unsigned int FE, unsigned int ToT) const
{
  if (ToT < 1 || ToT > IBLCalibrationSize) {
    std::stringstream error;
    error << "PixelChargeCalibCondData::getChargeLUTFEI4(" << moduleHash << ", " << FE << ", " << ToT << "): array out of bounds";
    throw std::runtime_error(error.str());
  }

  const IBLCalibration &charges = getTot2Charges(moduleHash,FE);
  return charges[ToT - 1];
}

float PixelChargeCalibCondData::getToTLUTFEI4(unsigned int moduleHash, unsigned int FE, float Q) const
{
  int tot = -1;
  float minDiff = FLT_MAX;
  for (size_t t = 0; t < IBLCalibrationSize; t++) {
    float charge = getChargeLUTFEI4(moduleHash, FE, t + 1);
    float diff = std::fabs(charge - Q);
    if (diff < minDiff) {
      minDiff = diff;
      tot = t + 1;
    }
  }
  return tot;
}
