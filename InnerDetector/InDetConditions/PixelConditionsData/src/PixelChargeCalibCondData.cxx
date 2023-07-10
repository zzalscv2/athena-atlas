/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelConditionsData/ChargeCalibParameters.h"
#include <stdexcept>
#include <sstream>
#include <cfloat> //for FLT_MAX

using namespace PixelChargeCalib;
using InDetDD::enum2uint; //in PixelReadoutDefinitions.h; will use ADL anyway,but make it explicit

namespace{
  const std::out_of_range 
  generateError(const char * functionName, InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE){
    std::stringstream error;
    error << "PixelChargeCalibCondData::"<< functionName << "("<<enum2uint(type)<< ", " << moduleHash << ", " << FE << "): out of bounds";
    return std::out_of_range(error.str());
  }
  const std::out_of_range 
  generateError(const char * functionName, unsigned int moduleHash, unsigned int FE, unsigned int val){
    std::stringstream error;
    error << "PixelChargeCalibCondData::"<< functionName << "(" << moduleHash << ", " << FE << ", "<< val << "): out of bounds";
    return std::out_of_range(error.str());
  }
  const std::out_of_range 
  generateError(const char * functionName,  unsigned int moduleHash, unsigned int FE){
    std::stringstream error;
    error << "PixelChargeCalibCondData::"<< functionName << "(" << moduleHash << ", " << FE << "): out of bounds";
    return std::out_of_range(error.str());
  }
  const std::out_of_range 
  generateError(const char * functionName,  unsigned int moduleHash){
    std::stringstream error;
    error << "PixelChargeCalibCondData::"<< functionName << "(" << moduleHash << "): out of bounds";
    return std::out_of_range(error.str());
  }
}

PixelChargeCalibCondData::PixelChargeCalibCondData(std::size_t max_module_hash) : m_maxModuleHash(max_module_hash){
  //nop
}

void 
PixelChargeCalibCondData::setThresholds(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::Thresholds> & thresholds){
  const auto n  = thresholds.size();
  std::vector<int> v1;
  std::vector<int> v2;
  std::vector<int> v3;
  std::vector<int> v4;
  v1.reserve(n);
  v2.reserve(n);
  v3.reserve(n);
  v4.reserve(n);
  for (const auto& th : thresholds){
    v1.push_back(th.value); //values are copied here
    v2.push_back(th.sigma);
    v3.push_back(th.noise);
    v4.push_back(th.inTimeValue);
  }
  setValue(m_maxModuleHash, m_analogThreshold, type, moduleHash, std::move(v1)); //...and then moved
  setValue(m_maxModuleHash, m_analogThresholdSigma, type, moduleHash, std::move(v2));
  setValue(m_maxModuleHash, m_analogThresholdNoise, type, moduleHash, std::move(v3));
  setValue(m_maxModuleHash, m_inTimeThreshold,type, moduleHash, std::move(v4));
}

PixelChargeCalib::Thresholds 
PixelChargeCalibCondData::getThresholds(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  auto idx = enum2uint(type);
  const chipThreshold &typeMap1 = m_analogThreshold.at(idx);
  const chipThreshold &typeMap2 = m_analogThresholdSigma.at(idx);
  const chipThreshold &typeMap3 = m_analogThresholdNoise.at(idx);
  const chipThreshold &typeMap4 = m_inTimeThreshold.at(idx);
  //
  const auto &val = typeMap1.at(moduleHash);
  if (FE < val.size()) {
    const auto &sig = typeMap2.at(moduleHash);
    const auto &noi = typeMap3.at(moduleHash);
    const auto &inT = typeMap4.at(moduleHash);
    return PixelChargeCalib::Thresholds {val[FE],sig[FE],noi[FE], inT[FE]};
  }
  throw generateError(__func__, type, moduleHash, FE);
}

void 
PixelChargeCalibCondData::setLegacyFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::LegacyFitParameters> &parameters){
  const auto n  = parameters.size();
  std::vector<float> v1;
  std::vector<float> v2;
  std::vector<float> v3;
  v1.reserve(n);
  v2.reserve(n);
  v3.reserve(n);
  for (const auto& par : parameters){
    v1.push_back(par.A); //values are copied here
    v2.push_back(par.E);
    v3.push_back(par.C);
  }
  setValue(m_maxModuleHash, m_totA,type,moduleHash,std::move(v1));
  setValue(m_maxModuleHash, m_totE,type,moduleHash,std::move(v2));
  setValue(m_maxModuleHash, m_totC,type,moduleHash,std::move(v3));
}

PixelChargeCalib::LegacyFitParameters 
PixelChargeCalibCondData::getLegacyFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  auto idx = enum2uint(type);
  const chipCharge &typeMap1 = m_totA.at(idx);
  const chipCharge &typeMap2 = m_totE.at(idx);
  const chipCharge &typeMap3 = m_totC.at(idx);
  //
  const auto &A = typeMap1.at(moduleHash);
  if (FE < A.size()) {
    const auto &E = typeMap2.at(moduleHash);
    const auto &C = typeMap3.at(moduleHash);
    return {A[FE], E[FE], C[FE]};
  }
  throw generateError(__func__, type, moduleHash, FE);
}

void 
PixelChargeCalibCondData::setLinearFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::LinearFitParameters> &parameters){
  const auto n  = parameters.size();
  std::vector<float> v1;
  std::vector<float> v2;
  v1.reserve(n);
  v2.reserve(n);
  for (const auto& par : parameters){
    v1.push_back(par.F); //values are copied here
    v2.push_back(par.G);
  }
  setValue(m_maxModuleHash, m_totF,type,moduleHash,std::move(v1));
  setValue(m_maxModuleHash, m_totG,type,moduleHash,std::move(v2));
}



PixelChargeCalib::LinearFitParameters 
PixelChargeCalibCondData::getLinearFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  auto idx = enum2uint(type);
  const chipCharge &typeMap1 = m_totF.at(idx);
  const chipCharge &typeMap2 = m_totG.at(idx);
  //
  const auto &F = typeMap1.at(moduleHash);
  if (FE < F.size()) {
    const auto &G = typeMap2.at(moduleHash);
    return {F[FE], G[FE]};
  }
  throw generateError(__func__, type, moduleHash, FE);
}

void 
PixelChargeCalibCondData::setTotResolutions(unsigned int moduleHash, const std::vector<PixelChargeCalib::Resolutions> &value){
  const auto n = value.size();
  std::vector<float> value1;
  std::vector<float> value2;
  value1.reserve(n);
  value2.reserve(n);
  for (const auto& v : value){
    value1.push_back(v.res1); //values are copied here
    value2.push_back(v.res2);
  }
  setValue(m_maxModuleHash, m_totRes1, moduleHash,std::move(value1));
  setValue(m_maxModuleHash, m_totRes2, moduleHash,std::move(value2));
}


void 
PixelChargeCalibCondData::setAnalogThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value){
   setValue(m_maxModuleHash, m_analogThreshold, type, moduleHash, std::move(value));//no copying, just moved
}

void 
PixelChargeCalibCondData::setAnalogThresholdSigma(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value){
   setValue(m_maxModuleHash, m_analogThresholdSigma, type, moduleHash, std::move(value));
}

void 
PixelChargeCalibCondData::setAnalogThresholdNoise(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value){
   setValue(m_maxModuleHash, m_analogThresholdNoise, type, moduleHash, std::move(value));
}

void 
PixelChargeCalibCondData::setInTimeThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value){
   setValue(m_maxModuleHash, m_inTimeThreshold,type, moduleHash, std::move(value));
}

void PixelChargeCalibCondData::setQ2TotA(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value){
   setValue(m_maxModuleHash, m_totA,type,moduleHash,std::move(value));
}

void PixelChargeCalibCondData::setQ2TotE(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value){
   setValue(m_maxModuleHash, m_totE, type, moduleHash,  std::move(value));
}

void 
PixelChargeCalibCondData::setQ2TotC(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value){
   setValue(m_maxModuleHash, m_totC,type,moduleHash,  std::move(value));
}

void 
PixelChargeCalibCondData::setQ2TotF(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value){
   setValue(m_maxModuleHash, m_totF, type, moduleHash,  std::move(value));
}

void 
PixelChargeCalibCondData::setQ2TotG(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<float> &&value){
   setValue( m_maxModuleHash, m_totG, type, moduleHash, std::move(value) );
}

void 
PixelChargeCalibCondData::setTotRes1(unsigned int moduleHash, std::vector<float> &&value) {
   setValue(m_maxModuleHash, m_totRes1, moduleHash,std::move(value));
}

void 
PixelChargeCalibCondData::setTotRes2(unsigned int moduleHash, std::vector<float> &&value) {
   setValue(m_maxModuleHash, m_totRes2,moduleHash, std::move(value));
}

int 
PixelChargeCalibCondData::getAnalogThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  const chipThreshold &typeMap = m_analogThreshold.at(enum2uint(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }
  throw generateError(__func__, type, moduleHash, FE);
}

int 
PixelChargeCalibCondData::getAnalogThresholdSigma(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  const chipThreshold &typeMap = m_analogThresholdSigma.at(enum2uint(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
     return fe_vec.at(FE);
  }
  throw generateError(__func__, type, moduleHash, FE);
}

int 
PixelChargeCalibCondData::getAnalogThresholdNoise(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  const chipThreshold &typeMap = m_analogThresholdNoise.at(enum2uint(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }
  throw generateError(__func__, type, moduleHash, FE);
}

int 
PixelChargeCalibCondData::getInTimeThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  const chipThreshold &typeMap = m_inTimeThreshold.at(enum2uint(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }
  throw generateError(__func__, type, moduleHash, FE);
}

float 
PixelChargeCalibCondData::getQ2TotA(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  const chipCharge &typeMap = m_totA.at(enum2uint(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }
  throw generateError(__func__, type, moduleHash, FE);
}

float 
PixelChargeCalibCondData::getQ2TotE(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  const chipCharge &typeMap = m_totE.at(enum2uint(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }
  throw generateError(__func__, type, moduleHash, FE);
}

float 
PixelChargeCalibCondData::getQ2TotC(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  const chipCharge &typeMap = m_totC.at(enum2uint(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }
  throw generateError(__func__, type, moduleHash, FE);
}

float 
PixelChargeCalibCondData::getQ2TotF(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  const chipCharge &typeMap = m_totF.at(enum2uint(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }
  throw generateError(__func__, type, moduleHash, FE);
}

float 
PixelChargeCalibCondData::getQ2TotG(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  const chipCharge &typeMap = m_totG.at(enum2uint(type));
  const auto &fe_vec = typeMap.at(moduleHash);
  if (FE < fe_vec.size()) {
    return fe_vec[FE];
  }
  throw generateError(__func__, type, moduleHash, FE);
}

float 
PixelChargeCalibCondData::getTotRes(unsigned int moduleHash, unsigned int FE, float Q) const{
  Resolutions r;
  if (const auto &fe_vec = m_totRes1.at(moduleHash); FE < fe_vec.size()) {
    r.res1 = fe_vec[FE];
  } else {
    throw generateError(__func__, moduleHash, FE);
  }
  //
  
  if (const auto &fe_vec = m_totRes2.at(moduleHash);FE < fe_vec.size()) {
    r.res2 = fe_vec[FE];
  } else {
    throw generateError(__func__, moduleHash, FE);
  }
  
  return r.total(Q);
}

float 
PixelChargeCalibCondData::getToT(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE, float Q) const{
  if (getCalibrationStrategy(moduleHash) == CalibrationStrategy::LUTFEI4) {
    return getToTLUTFEI4(moduleHash, FE, Q);
  }
  LegacyFitParameters legacy{getQ2TotA(type, moduleHash, FE), getQ2TotE(type, moduleHash, FE), getQ2TotC(type, moduleHash, FE)};
  float tot = legacy.ToT(Q);

  // Protection for large charge
  float exth = 1e5;    // the calibration function is analytically connected at threshold exth.
  if (Q>exth && getCalibrationStrategy(moduleHash)==CalibrationStrategy::RUN3PIX) {
    LinearFitParameters lin{getQ2TotF(type, moduleHash, FE), getQ2TotG(type, moduleHash, FE)};
    tot = lin.ToT(Q);
  }
  return tot;
}

float 
PixelChargeCalibCondData::getCharge(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE, float ToT) const{
  if (getCalibrationStrategy(moduleHash) == CalibrationStrategy::LUTFEI4) {
    return getChargeLUTFEI4(moduleHash, FE, ToT);
  }
  LegacyFitParameters legacy{ getQ2TotA(type, moduleHash, FE), getQ2TotE(type, moduleHash, FE), getQ2TotC(type, moduleHash, FE)};
 
  float charge = legacy.Q(ToT);

  // Protection for small charge
  float threshold = getAnalogThreshold(type,moduleHash,FE);
  if (charge<threshold && getCalibrationStrategy(moduleHash)==CalibrationStrategy::RUN3PIX) { charge=threshold; }
  // Protection for large charge
  float exth = 1e5;    // the calibration function is analytically connected at threshold exth.
  if (charge>exth && getCalibrationStrategy(moduleHash)==CalibrationStrategy::RUN3PIX) {
    LinearFitParameters lin  = {getQ2TotF(type, moduleHash, FE), getQ2TotG(type, moduleHash, FE)};
    charge = lin.Q(ToT);
  }
  return charge;
}

void 
PixelChargeCalibCondData::setCalibrationStrategy(unsigned int moduleHash, CalibrationStrategy strategy){ 
  if (moduleHash > m_maxModuleHash) {
    throw generateError(__func__, moduleHash, static_cast<int>(strategy));
  }
  m_calibrationStrategy[moduleHash] = strategy;
}

PixelChargeCalibCondData::CalibrationStrategy 
PixelChargeCalibCondData::getCalibrationStrategy(unsigned int moduleHash) const{
  if (moduleHash > m_maxModuleHash){
    throw generateError(__func__, moduleHash);
  }
  auto itr = m_calibrationStrategy.find(moduleHash);
  if (itr != m_calibrationStrategy.end()) {
    return itr->second;
  }
  return CalibrationStrategy::RUN1PIX;
}

void 
PixelChargeCalibCondData::setTot2Charges(unsigned int moduleHash, IBLModule charges){ 
  if (moduleHash > m_maxModuleHash){
    throw generateError(__func__, moduleHash);
  }
  m_tot2Charges[moduleHash] = std::move(charges);
}

const PixelChargeCalibCondData::IBLCalibration &
PixelChargeCalibCondData::getTot2Charges(unsigned int moduleHash, unsigned int FE) const{
  auto it = m_tot2Charges.find(moduleHash);
  if (it != m_tot2Charges.end() && FE < it->second.size()) {
    return it->second.at(FE);
  }
  throw generateError(__func__, moduleHash,FE);
}

float 
PixelChargeCalibCondData::getChargeLUTFEI4(unsigned int moduleHash, unsigned int FE, unsigned int ToT) const{
  if (ToT < 1 || ToT > IBLCalibrationSize) {
    throw generateError(__func__, moduleHash,FE, ToT);
  }

  const IBLCalibration &charges = getTot2Charges(moduleHash,FE);
  return charges[ToT - 1];
}

float 
PixelChargeCalibCondData::getToTLUTFEI4(unsigned int moduleHash, unsigned int FE, float Q) const{
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
