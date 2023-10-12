/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConfigCondAlg.h"
#include "Identifier/IdentifierHash.h"
#include "GaudiKernel/EventIDRange.h"
#include <memory>
#include <sstream>

#include <fstream>

#include "PathResolver/PathResolver.h"

PixelConfigCondAlg::PixelConfigCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode PixelConfigCondAlg::initialize() {
  ATH_MSG_DEBUG("PixelConfigCondAlg::initialize()");

  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PixelConfigCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("PixelConfigCondAlg::execute()");

  SG::WriteCondHandle<PixelModuleData> writeHandle(m_writeKey, ctx);
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS; 
  }

  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelModuleData> writeCdo(std::make_unique<PixelModuleData>());

  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT,                     0,                       
                                              0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM,   EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, 
                          EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};

  // Digitization parameters
  writeCdo -> setBunchSpace(m_bunchSpace);
  writeCdo -> setBarrelNumberOfBCID(m_BarrelNumberOfBCID);
  writeCdo -> setEndcapNumberOfBCID(m_EndcapNumberOfBCID);
  writeCdo -> setDBMNumberOfBCID(m_DBMNumberOfBCID);
  writeCdo -> setBarrelTimeOffset(m_BarrelTimeOffset);
  writeCdo -> setEndcapTimeOffset(m_EndcapTimeOffset);
  writeCdo -> setDBMTimeOffset(m_DBMTimeOffset);
  writeCdo -> setBarrelTimeJitter(m_BarrelTimeJitter);
  writeCdo -> setEndcapTimeJitter(m_EndcapTimeJitter);
  writeCdo -> setDBMTimeJitter(m_DBMTimeJitter);

  writeCdo -> setDefaultBarrelAnalogThreshold(m_BarrelAnalogThreshold);
  writeCdo -> setDefaultEndcapAnalogThreshold(m_EndcapAnalogThreshold);
  writeCdo -> setDefaultDBMAnalogThreshold(m_DBMAnalogThreshold);
  writeCdo -> setDefaultBarrelAnalogThresholdSigma(m_BarrelAnalogThresholdSigma);
  writeCdo -> setDefaultEndcapAnalogThresholdSigma(m_EndcapAnalogThresholdSigma);
  writeCdo -> setDefaultDBMAnalogThresholdSigma(m_DBMAnalogThresholdSigma);
  writeCdo -> setDefaultBarrelAnalogThresholdNoise(m_BarrelAnalogThresholdNoise);
  writeCdo -> setDefaultEndcapAnalogThresholdNoise(m_EndcapAnalogThresholdNoise);
  writeCdo -> setDefaultDBMAnalogThresholdNoise(m_DBMAnalogThresholdNoise);
  writeCdo -> setDefaultBarrelInTimeThreshold(m_BarrelInTimeThreshold);
  writeCdo -> setDefaultEndcapInTimeThreshold(m_EndcapInTimeThreshold);
  writeCdo -> setDefaultDBMInTimeThreshold(m_DBMInTimeThreshold);
  writeCdo -> setBarrelThermalNoise(m_BarrelThermalNoise);
  writeCdo -> setEndcapThermalNoise(m_EndcapThermalNoise);
  writeCdo -> setDBMThermalNoise(m_DBMThermalNoise);
  writeCdo -> setFEI4BarrelHitDiscConfig(m_FEI4BarrelHitDiscConfig);
  writeCdo -> setFEI4EndcapHitDiscConfig(m_FEI4EndcapHitDiscConfig);
  writeCdo -> setFEI4ChargScaling(m_chargeScaleFEI4);
  writeCdo -> setUseFEI4SpecialScalingFunction(m_UseFEI4SpecialScalingFunction);
  writeCdo -> setFEI4ToTSigma(m_FEI4ToTSigma);

  // Charge calibration parameters
  writeCdo -> setDefaultQ2TotA(m_CalibrationParameterA);
  writeCdo -> setDefaultQ2TotE(m_CalibrationParameterE);
  writeCdo -> setDefaultQ2TotC(m_CalibrationParameterC);
  writeCdo -> setPIXLinearExtrapolation(m_doPIXLinearExtrapolation);

  // DCS parameters
  writeCdo -> setDefaultBiasVoltage(m_biasVoltage);
  writeCdo -> setDefaultTemperature(m_temperature);

  // Distortion parameters
  writeCdo -> setDistortionInputSource(m_distortionInputSource);
  writeCdo -> setDistortionVersion(m_distortionVersion);
  writeCdo -> setDistortionR1(m_distortionR1);
  writeCdo -> setDistortionR2(m_distortionR2);
  writeCdo -> setDistortionTwist(m_distortionTwist);
  writeCdo -> setDistortionMeanR(m_distortionMeanR);
  writeCdo -> setDistortionRMSR(m_distortionRMSR);
  writeCdo -> setDistortionMeanTwist(m_distortionMeanTwist);
  writeCdo -> setDistortionRMSTwist(m_distortionRMSTwist);
  writeCdo -> setDistortionWriteToFile(m_distortionWriteToFile);
  writeCdo -> setDistortionFileName(m_distortionFileName);

  // Cabling parameters
  writeCdo -> setCablingMapToFile(m_cablingMapToFile);
  writeCdo -> setCablingMapFileName(m_cablingMapFileName);

  // mapping files for radiation damage simulation
  std::vector<std::string> mapsPath_list;
  std::vector<std::string> mapsPath_list3D;

  // Year-dependent conditions
  int currentRunNumber = ctx.eventID().run_number();
  std::string filename = getFileName(currentRunNumber);
  std::ifstream indata(filename.c_str());

  std::string sline;
  std::vector<std::string> lBuffer;
  std::string multiline = "";
  while (getline(indata,sline)) {
    if (!sline.empty()) {
      if (sline.find("//")==std::string::npos) {
        if (sline.find("{")!=std::string::npos && sline.find("}")!=std::string::npos) {
          lBuffer.push_back(sline);
        }
        else if (sline.find("{")!=std::string::npos) {
          multiline = sline;
        }
        else if (sline.find("}")!=std::string::npos) {
          multiline += sline;
          lBuffer.push_back(multiline);
        }
        else {
          multiline += sline;
        }
      }
    }
  }

  writeCdo -> setBarrelToTThreshold(getParameterInt("BarrelToTThreshold", lBuffer));
  writeCdo -> setFEI3BarrelLatency(getParameterInt("FEI3BarrelLatency", lBuffer));
  writeCdo -> setFEI3BarrelHitDuplication(getParameterBool("FEI3BarrelHitDuplication", lBuffer));
  writeCdo -> setFEI3BarrelSmallHitToT(getParameterInt("FEI3BarrelSmallHitToT", lBuffer));
  writeCdo -> setFEI3BarrelTimingSimTune(getParameterInt("FEI3BarrelTimingSimTune", lBuffer));
  writeCdo -> setBarrelCrossTalk(getParameterDouble("BarrelCrossTalk", lBuffer));
  writeCdo -> setBarrelNoiseOccupancy(getParameterDouble("BarrelNoiseOccupancy", lBuffer));
  writeCdo -> setBarrelDisableProbability(getParameterDouble("BarrelDisableProbability", lBuffer));
  writeCdo -> setBarrelLorentzAngleCorr(getParameterDouble("BarrelLorentzAngleCorr", lBuffer));
  writeCdo -> setDefaultBarrelBiasVoltage(getParameterFloat("BarrelBiasVoltage", lBuffer));

  writeCdo -> setEndcapToTThreshold(getParameterInt("EndcapToTThreshold", lBuffer));
  writeCdo -> setFEI3EndcapLatency(getParameterInt("FEI3EndcapLatency", lBuffer));
  writeCdo -> setFEI3EndcapHitDuplication(getParameterBool("FEI3EndcapHitDuplication", lBuffer));
  writeCdo -> setFEI3EndcapSmallHitToT(getParameterInt("FEI3EndcapSmallHitToT", lBuffer));
  writeCdo -> setFEI3EndcapTimingSimTune(getParameterInt("FEI3EndcapTimingSimTune", lBuffer));
  writeCdo -> setEndcapCrossTalk(getParameterDouble("EndcapCrossTalk", lBuffer));
  writeCdo -> setEndcapNoiseOccupancy(getParameterDouble("EndcapNoiseOccupancy", lBuffer));
  writeCdo -> setEndcapDisableProbability(getParameterDouble("EndcapDisableProbability", lBuffer));
  writeCdo -> setEndcapLorentzAngleCorr(getParameterDouble("EndcapLorentzAngleCorr", lBuffer));
  writeCdo -> setDefaultEndcapBiasVoltage(getParameterFloat("EndcapBiasVoltage", lBuffer));

  writeCdo -> setEndcapNoiseShape({getParameterFloat("PixelNoiseShape", lBuffer),
                                   getParameterFloat("PixelNoiseShape", lBuffer),
                                   getParameterFloat("PixelNoiseShape", lBuffer)});

  // Radiation damage simulation
  writeCdo -> setFluenceLayer(getParameterDouble("BarrelFluence", lBuffer));
  std::vector<std::string> barrelFluenceFile = getParameterString("BarrelRadiationFile", lBuffer);
  for (auto fluence : barrelFluenceFile) {
    mapsPath_list.push_back(PathResolverFindCalibFile(fluence));
  }

  if (currentRunNumber<m_Run1IOV) {    // RUN1
    writeCdo -> setBarrelNoiseShape({getParameterFloat("BLayerNoiseShape", lBuffer),
                                     getParameterFloat("PixelNoiseShape", lBuffer),
                                     getParameterFloat("PixelNoiseShape", lBuffer)});
  }
  else {     // RUN2
    writeCdo -> setDBMToTThreshold(getParameterInt("DBMToTThreshold", lBuffer));
    writeCdo -> setDBMCrossTalk(getParameterDouble("DBMCrossTalk", lBuffer));
    writeCdo -> setDBMNoiseOccupancy(getParameterDouble("DBMNoiseOccupancy", lBuffer));
    writeCdo -> setDBMDisableProbability(getParameterDouble("DBMDisableProbability", lBuffer));
    writeCdo -> setDefaultDBMBiasVoltage(getParameterFloat("DBMBiasVoltage", lBuffer));

    writeCdo -> setBarrelNoiseShape({getParameterFloat("IBLNoiseShape", lBuffer),
                                     getParameterFloat("BLayerNoiseShape", lBuffer),
                                     getParameterFloat("PixelNoiseShape", lBuffer),
                                     getParameterFloat("PixelNoiseShape", lBuffer)});

    writeCdo -> setDBMNoiseShape({getParameterFloat("IBLNoiseShape", lBuffer),
                                  getParameterFloat("IBLNoiseShape", lBuffer),
                                  getParameterFloat("IBLNoiseShape", lBuffer)});

    // Radiation damage simulation for 3D sensor
    writeCdo -> setFluenceLayer3D(getParameterDouble("3DFluence", lBuffer));
    std::vector<std::string> barrel3DFluenceFile = getParameterString("3DRadiationFile", lBuffer);
    for (auto fluence3D : barrel3DFluenceFile) {
      mapsPath_list3D.push_back(PathResolverFindCalibFile(fluence3D));
    }
  }
  writeCdo -> setRadSimFluenceMapList(mapsPath_list);
  writeCdo -> setRadSimFluenceMapList3D(mapsPath_list3D);

  //=======================
  // Combine time interval
  //=======================
  EventIDRange rangeW{start, stop};
  if (rangeW.stop().isValid() && rangeW.start()>rangeW.stop()) {
    ATH_MSG_FATAL("Invalid intersection rangeW: " << rangeW);
    return StatusCode::FAILURE;
  }

  if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelModuleData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
}

std::vector<std::string> PixelConfigCondAlg::getParameterString(const std::string& varName, const std::vector<std::string>& buffer) const {
  std::string sParam = "";
  std::string sMessage = "";
  for (size_t i=0; i<buffer.size(); i++) {
    if (buffer[i].find(varName.c_str())!=std::string::npos) {
      ATH_MSG_DEBUG("PixelConfigCondAlg::getParameterString() " << i << " " << buffer[i]);
      std::istringstream iss(buffer[i]);
      std::string s;
      bool chkParam = false;
      bool chkMessage = false;
      while (iss >> s) {
        if (s.find("{")!=std::string::npos && s.find("}")!=std::string::npos) {
          sParam += s.substr(1,s.length()-1);
        }
        else if (s.find("{")!=std::string::npos) {
          sParam += s.substr(1,s.length());
          chkParam = true;
        }
        else if (s.find("}")!=std::string::npos) {
          sParam += s.substr(0,s.length()-1);
          chkParam = false;
          chkMessage = true;
        }
        else if (chkParam==true) {
          sParam += s;
        }
        else if (chkMessage==true) {
          sMessage += " " + s;
        }
      }
    }
  }

  if (sParam.empty()) {
    ATH_MSG_FATAL("PixelConfigCondAlg::getParameterString() Input variable was not found. " << varName);
  }

  std::vector<std::string> vParam;
  int offset = 0; 
  for (;;) {
    auto pos = sParam.find(",",offset);
    if (pos==std::string::npos) {
      vParam.push_back(sParam.substr(offset,pos));
      break;
    }
    vParam.push_back(sParam.substr(offset,pos-offset));
    offset = pos + 1;
  }

  std::vector<std::string> vvParam;
  for (auto param : vParam) {
    if (param.find("\"")!=std::string::npos) {
      if (vParam.size()==1) {
        vvParam.push_back(param.substr(1,param.length()-3));
      }
      else {
        vvParam.push_back(param.substr(1,param.length()-2));
      }
    }
    else {
      vvParam.push_back(param);
    }
  }
  return vvParam;
}

std::vector<double> PixelConfigCondAlg::getParameterDouble(const std::string& varName, const std::vector<std::string>& buffer) const {
  std::vector<std::string> varString = getParameterString(varName, buffer);
  std::vector<double> varDouble;
  for (auto var : varString) {
    varDouble.push_back(std::stod(var,nullptr));
  }
  return varDouble;
}

std::vector<float> PixelConfigCondAlg::getParameterFloat(const std::string& varName, const std::vector<std::string>& buffer) const {
  std::vector<std::string> varString = getParameterString(varName, buffer);
  std::vector<float> varFloat;
  for (auto var : varString) {
    varFloat.push_back(std::stof(var,nullptr));
  }
  return varFloat;
}

std::vector<int> PixelConfigCondAlg::getParameterInt(const std::string& varName, const std::vector<std::string>& buffer) const {
  std::vector<std::string> varString = getParameterString(varName, buffer);
  std::vector<int> varInt;
  for (auto var : varString) {
    varInt.push_back(std::stoi(var,nullptr));
  }
  return varInt;
}

std::vector<bool> PixelConfigCondAlg::getParameterBool(const std::string& varName, const std::vector<std::string>& buffer) const {
  std::vector<std::string> varString = getParameterString(varName, buffer);
  std::vector<bool> varBool;
  for (auto var : varString) {
    if (var.find("False")!=std::string::npos || var.find("false")!=std::string::npos) {
      varBool.push_back(0);
    }
    else if (var.find("True")!=std::string::npos || var.find("true")!=std::string::npos) {
      varBool.push_back(1);
    }
    else {
      ATH_MSG_FATAL("PixelConfigCondAlg::getParameterBool() No matching boolean string " << var);
    }
  }
  return varBool;
}

std::string PixelConfigCondAlg::getFileName(const int currentRunNumber) const {
  if (m_usePrivateFileName.empty()) {
    std::ifstream indata(PathResolverFindCalibFile(m_conditionsFolder+m_conditionsFileName));
    int runNumber = 0;
    std::string subfilename;
    indata >> runNumber;
    while (currentRunNumber>=runNumber) {
      indata >> subfilename;
      if (indata.eof()) { break; }
      indata >> runNumber;
    }
    ATH_MSG_DEBUG("PixelConfigCondAlg::getFileName() RunNumber=" << currentRunNumber << " IOV=" << runNumber << " filename=" << subfilename);
    return PathResolverFindCalibFile(m_conditionsFolder+subfilename);
  }
  else {
    return m_usePrivateFileName;
  }
}

