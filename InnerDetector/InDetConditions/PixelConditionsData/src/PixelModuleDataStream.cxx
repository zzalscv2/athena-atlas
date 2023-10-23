/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelModuleDataStream.h"
#include "PixelConditionsData/PixelModuleData.h"
#include "PixelConditionsData/PixelConditionsDataStringUtils.h"
#include "PathResolver/PathResolver.h"
#include <vector>
#include <type_traits>
#include <sstream>

using namespace PixelConditionsData;

namespace{
  template <typename T>
  struct is_container : std::false_type { };
  
  template <typename T> 
  struct is_container<std::vector<T> > : std::true_type { };
  
  template <class T>
  std::ostream &
  operator << (std::ostream & out, const std::vector<T> & v){
    const std::string delim = is_container<T>()?"":" ";
    const std::string LF = is_container<T>()?"":"\n";
    for (size_t i{};i!=v.size();++i){
      out<<v[i]<<delim;
    }
    out<<LF;
    return out;
  }
  
  
  std::istream &
  operator >> (std::istream & in, std::vector<std::vector<float>> & v){
    std::istream::sentry s(in);
    if (s){
      v.clear();
      constexpr size_t bufferSize=500;
      char buffer[bufferSize];
      in.getline(buffer, bufferSize-1, '\n');
      if (buffer[0] =='#') in.getline(buffer, bufferSize);
      std::istringstream is(buffer);
      //assumes (for now) only three entries...which is currently the case
      v.emplace_back(std::istream_iterator<float>(is),std::istream_iterator<float>());
      in.getline(buffer, bufferSize-1, '\n');
      std::istringstream is2(buffer);
      v.emplace_back(std::istream_iterator<float>(is2),std::istream_iterator<float>());
      in.getline(buffer, bufferSize-1, '\n');
      std::istringstream is3(buffer);
      v.emplace_back(std::istream_iterator<float>(is3),std::istream_iterator<float>());
    }
    return in;
  }
  template <typename T>
  std::istream &
  operator >> (std::istream & in, std::vector<T> & v){
    std::istream::sentry s(in);
    if (s){
      v.clear();
      constexpr size_t bufferSize=500;
      char buffer[bufferSize]={};
      in.getline(buffer, bufferSize-1,'\n');
      if (buffer[0] =='#') in.getline(buffer, bufferSize-1,'\n');
      std::string s(buffer);
      std::istringstream iss(s);
      v.assign(std::istream_iterator<T>(iss), std::istream_iterator<T>());
      if (in.fail()){
        throw std::runtime_error("error on streaming input to PixelModuleData");
      }
    }
    return in;
  }
}

//declared as a friend in the PixelModuleData class
std::ostream & 
operator << (std::ostream &out, const PixelModuleData &c){

  const std::string LF{"\n"};
  //NB: in the following, vector variables are not followed by a linefeed;
  // it's already in the vector stream insertion operator in anonymous namespace (above)
  out<<"#bunchSpace"<<LF;
  out<<c.m_bunchSpace<<LF;
  //
  out<<"#barrelNumberOfBCID"<<LF;
  out<<c.m_barrelNumberOfBCID;
  out<<"#endcapNumberOfBCID"<<LF;
  out<<c.m_endcapNumberOfBCID;
  out<<"#DBMNumberOfBCID"<<LF;
  out<<c.m_DBMNumberOfBCID;
  //
  out<<"#barrelTimeOffset"<<LF;
  out<<c.m_barrelTimeOffset;
  out<<"#endcapTimeOffset"<<LF;
  out<<c.m_endcapTimeOffset;
  out<<"#DBMTimeOffset"<<LF;
  out<<c.m_DBMTimeOffset;
  //
  out<<"#barrelTimeJitter"<<LF;
  out<<c.m_barrelTimeJitter;
  out<<"#endcapTimeJitter"<<LF;
  out<<c.m_endcapTimeJitter;
  out<<"#DBMTimeJitter"<<LF;
  out<<c.m_DBMTimeJitter;
  //
  out<<"#defaultBarrelAnalogThreshold"<<LF;
  out<<c.m_defaultBarrelAnalogThreshold;
  out<<"#defaultEndcapAnalogThreshold"<<LF;
  out<<c.m_defaultEndcapAnalogThreshold;
  out<<"#defaultDBMAnalogThreshold"<<LF;
  out<<c.m_defaultDBMAnalogThreshold;
  //
  out<<"#defaultBarrelAnalogThresholdSigma"<<LF;
  out<<c.m_defaultBarrelAnalogThresholdSigma;
  out<<"#defaultEndcapAnalogThresholdSigma"<<LF;
  out<<c.m_defaultEndcapAnalogThresholdSigma;
  out<<"#defaultDBMAnalogThresholdSigma"<<LF;
  out<<c.m_defaultDBMAnalogThresholdSigma;
  //
  out<<"#defaultBarrelAnalogThresholdNoise"<<LF;
  out<<c.m_defaultBarrelAnalogThresholdNoise;
  out<<"#defaultEndcapAnalogThresholdNoise"<<LF;
  out<<c.m_defaultEndcapAnalogThresholdNoise;
  out<<"#defaultDBMAnalogThresholdNoise"<<LF;
  out<<c.m_defaultDBMAnalogThresholdNoise;
  //
  out<<"#defaultBarrelInTimeThreshold"<<LF;
  out<<c.m_defaultBarrelInTimeThreshold;
  out<<"#defaultEndcapInTimeThreshold"<<LF;
  out<<c.m_defaultEndcapInTimeThreshold;
  out<<"#defaultDBMInTimeThreshold"<<LF;
  out<<c.m_defaultDBMInTimeThreshold;
  //
  out<<"#barrelToTThreshold"<<LF;
  out<<c.m_barrelToTThreshold;
  out<<"#endcapToTThreshold"<<LF;
  out<<c.m_endcapToTThreshold;
  out<<"#DBMToTThreshold"<<LF;
  out<<c.m_DBMToTThreshold;
  //
  out<<"#barrelCrossTalk"<<LF;
  out<<c.m_barrelCrossTalk; 
  out<<"#endcapCrossTalk"<<LF;
  out<<c.m_endcapCrossTalk; 
  out<<"#DBMCrossTalk"<<LF;
  out<<c.m_DBMCrossTalk;
  //
  out<<"#barrelThermalNoise"<<LF;
  out<<c.m_barrelThermalNoise;
  out<<"#endcapThermalNoise"<<LF;
  out<<c.m_endcapThermalNoise;
  out<<"#DBMThermalNoise"<<LF;
  out<<c.m_DBMThermalNoise;
  //
  out<<"#barrelNoiseOccupancy"<<LF;
  out<<c.m_barrelNoiseOccupancy;
  out<<"#endcapNoiseOccupancy"<<LF;
  out<<c.m_endcapNoiseOccupancy;
  out<<"#DBMNoiseOccupancy"<<LF;
  out<<c.m_DBMNoiseOccupancy;
  //
  out<<"#barrelDisableProbability"<<LF;
  out<<c.m_barrelDisableProbability;
  out<<"#endcapDisableProbability"<<LF;
  out<<c.m_endcapDisableProbability;
  out<<"#DBMDisableProbability"<<LF;
  out<<c.m_DBMDisableProbability;
  //
  out<<"#barrelNoiseShape"<<LF;
  out<<c.m_barrelNoiseShape;
  out<<"#endcapNoiseShape"<<LF;
  out<<c.m_endcapNoiseShape;
  out<<"#DBMNoiseShape"<<LF;
  out<<c.m_DBMNoiseShape;
  //
  out<<"#FEI3BarrelLatency"<<LF;
  out<<c.m_FEI3BarrelLatency;
  out<<"#FEI3EndcapLatency"<<LF;
  out<<c.m_FEI3EndcapLatency;
  //
  out<<"#FEI3BarrelHitDuplication"<<LF;
  out<<c.m_FEI3BarrelHitDuplication;
  out<<"#FEI3EndcapHitDuplication"<<LF;
  out<<c.m_FEI3EndcapHitDuplication;
  //
  out<<"#FEI3BarrelSmallHitToT"<<LF;
  out<<c.m_FEI3BarrelSmallHitToT;
  out<<"#FEI3EndcapSmallHitToT"<<LF;
  out<<c.m_FEI3EndcapSmallHitToT;
  //
  out<<"#FEI3BarrelTimingSimTune"<<LF;
  out<<c.m_FEI3BarrelTimingSimTune;
  out<<"#FEI3EndcapTimingSimTune"<<LF;
  out<<c.m_FEI3EndcapTimingSimTune;
  //
  out<<"#FEI4BarrelHitDiscConfig"<<LF;
  out<<c.m_FEI4BarrelHitDiscConfig;
  out<<"#FEI4EndcapHitDiscConfig"<<LF;
  out<<c.m_FEI4EndcapHitDiscConfig;
  //
  out<<"#scaleFEI4"<<LF;
  out<<c.m_scaleFEI4<<LF;
  out<<"#useFEI4SpecialScalingFunction"<<LF;
  out<<c.m_useFEI4SpecialScalingFunction<<LF;
  out<<"#FEI4ToTSigma"<<LF;
  out<<c.m_FEI4ToTSigma;
  //
  out<<"#paramA"<<LF;
  out<<c.m_paramA<<LF;
  out<<"#paramE"<<LF;
  out<<c.m_paramE<<LF;
  out<<"#paramC"<<LF;
  out<<c.m_paramC<<LF;
  out<<"#doLinearExtrapolation"<<LF;
  out<<c.m_doLinearExtrapolation<<LF;
  //
  out<<"#barrelLorentzAngleCorr"<<LF;
  out<<c.m_barrelLorentzAngleCorr;
  out<<"#endcapLorentzAngleCorr"<<LF;
  out<<c.m_endcapLorentzAngleCorr;
  //
  out<<"#biasVoltage"<<LF;
  out<<c.m_biasVoltage<<LF;
  out<<"#temperature"<<LF;
  out<<c.m_temperature<<LF;
  //
  out<<"#barrelBiasVoltage"<<LF;
  out<<c.m_barrelBiasVoltage;
  out<<"#endcapBiasVoltage"<<LF;
  out<<c.m_endcapBiasVoltage;
  out<<"#DBMBiasVoltage"<<LF;
  out<<c.m_DBMBiasVoltage;
  //
  out<<"#fluenceLayer"<<LF;
  out<<c.m_fluenceLayer;
  out<<"#radSimFluenceMapList"<<LF;
  out<<c.m_radSimFluenceMapList;
  //
  out<<"#fluenceLayer3D"<<LF;
  out<<c.m_fluenceLayer3D;
  out<<"#radSimFluenceMapList3D"<<LF;
  out<<c.m_radSimFluenceMapList3D;
  //
  out<<"#cablingMapToFile"<<LF;
  out<<c.m_cablingMapToFile<<LF;
  out<<"#cablingMapFileName"<<LF;
  out<<c.m_cablingMapFileName;
  //
  out<<"#distortionInputSource"<<LF;
  out<<c.m_distortionInputSource<<LF;
  out<<"#distortionVersion"<<LF;
  out<<c.m_distortionVersion<<LF;
  out<<"#distortionR1"<<LF;
  out<<c.m_distortionR1<<LF;
  out<<"#distortionR2"<<LF;
  out<<c.m_distortionR2<<LF;
  out<<"#distortionTwist"<<LF;
  out<<c.m_distortionTwist<<LF;
  out<<"#distortionMeanR"<<LF;
  out<<c.m_distortionMeanR<<LF;
  out<<"#distortionRMSR"<<LF;
  out<<c.m_distortionRMSR<<LF;
  out<<"#distortionMeanTwist"<<LF;
  out<<c.m_distortionMeanTwist<<LF;
  out<<"#distortionRMSTwist"<<LF;
  out<<c.m_distortionRMSTwist<<LF;
  out<<"#distortionWriteToFile"<<LF;
  out<<c.m_distortionWriteToFile<<LF;
  out<<"#distortionFileName"<<LF;
  out<<c.m_distortionFileName;
  return out;
}

//declared as a friend in the PixelModuleData class (dangerously skirting the interface)
std::istream & 
operator >> (std::istream &in, PixelModuleData &c){
  std::istream::sentry s(in);
  if (s){
    //this is rather unforgiving, and should only be used with the format given by the ostream
    //insertion operator
    std::string label;
    in.ignore(100, '\n');
    in>>c.m_bunchSpace;
    in.ignore(100,'\n');
    in>>c.m_barrelNumberOfBCID;
    in.ignore(100,'\n');
    in>>c.m_endcapNumberOfBCID;
    in.ignore(100,'\n');
    in>>c.m_DBMNumberOfBCID;
    in.ignore(100,'\n');
    in>>c.m_barrelTimeOffset;
    in.ignore(100,'\n');
    in>>c.m_endcapTimeOffset;
    in.ignore(100,'\n');
    in>>c.m_DBMTimeOffset;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelTimeJitter;
    in.ignore(100,'\n');
    in>>c.m_endcapTimeJitter;
    in.ignore(100,'\n');
    in>>c.m_DBMTimeJitter;
    //
    in.ignore(100,'\n');
    in>>c.m_defaultBarrelAnalogThreshold;
    in.ignore(100,'\n');
    in>>c.m_defaultEndcapAnalogThreshold;
    in.ignore(100,'\n');
    in>>c.m_defaultDBMAnalogThreshold;
    //
    in.ignore(100,'\n');
    in>>c.m_defaultBarrelAnalogThresholdSigma;
    in.ignore(100,'\n');
    in>>c.m_defaultEndcapAnalogThresholdSigma;
    in.ignore(100,'\n');
    in>>c.m_defaultDBMAnalogThresholdSigma;
    //
    in.ignore(100,'\n');
    in>>c.m_defaultBarrelAnalogThresholdNoise;
    in.ignore(100,'\n');
    in>>c.m_defaultEndcapAnalogThresholdNoise;
    in.ignore(100,'\n');
    in>>c.m_defaultDBMAnalogThresholdNoise;
    //
    in.ignore(100,'\n');
    in>>c.m_defaultBarrelInTimeThreshold;
    in.ignore(100,'\n');
    in>>c.m_defaultEndcapInTimeThreshold;
    in.ignore(100,'\n');
    in>>c.m_defaultDBMInTimeThreshold;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelToTThreshold;
    in.ignore(100,'\n');
    in>>c.m_endcapToTThreshold;
    in.ignore(100,'\n');
    in>>c.m_DBMToTThreshold;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelCrossTalk; 
    in.ignore(100,'\n');
    in>>c.m_endcapCrossTalk; 
    in.ignore(100,'\n');
    in>>c.m_DBMCrossTalk;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelThermalNoise;
    in.ignore(100,'\n');
    in>>c.m_endcapThermalNoise;
    in.ignore(100,'\n');
    in>>c.m_DBMThermalNoise;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelNoiseOccupancy;
    in.ignore(100,'\n');
    in>>c.m_endcapNoiseOccupancy;
    in.ignore(100,'\n');
    in>>c.m_DBMNoiseOccupancy;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelDisableProbability;
    in.ignore(100,'\n');
    in>>c.m_endcapDisableProbability;
    in.ignore(100,'\n');
    in>>c.m_DBMDisableProbability;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelNoiseShape;
    in.ignore(100,'\n');
    in>>c.m_endcapNoiseShape;
    in.ignore(100,'\n');
    in>>c.m_DBMNoiseShape;
    //
    in.ignore(100,'\n');
    in>>c.m_FEI3BarrelLatency;
    in.ignore(100,'\n');
    in>>c.m_FEI3EndcapLatency;
    //
    in.ignore(100,'\n');
    in>>c.m_FEI3BarrelHitDuplication;
    in.ignore(100,'\n');
    in>>c.m_FEI3EndcapHitDuplication;
    //
    in.ignore(100,'\n');
    in>>c.m_FEI3BarrelSmallHitToT;
    in.ignore(100,'\n');
    in>>c.m_FEI3EndcapSmallHitToT;
    //
    in.ignore(100,'\n');
    in>>c.m_FEI3BarrelTimingSimTune;
    in.ignore(100,'\n');
    in>>c.m_FEI3EndcapTimingSimTune;
    //
    in.ignore(100,'\n');
    in>>c.m_FEI4BarrelHitDiscConfig;
    in.ignore(100,'\n');
    in>>c.m_FEI4EndcapHitDiscConfig;
    //something magic about having to stream to a string at this point, 
    //instead of using 'ignore'
    in>>label;
    in>>c.m_scaleFEI4;
    in>>label;
    in>>c.m_useFEI4SpecialScalingFunction;
    in>>label;
    in>>c.m_FEI4ToTSigma;
    //
    in>>label;
    in>>c.m_paramA;
    in>>label;
    in>>c.m_paramE;
    in>>label;
    in>>c.m_paramC;
    in>>label;
    in>>c.m_doLinearExtrapolation;
    //
    in>>label;
    in>>c.m_barrelLorentzAngleCorr;
    in>>label;
    in>>c.m_endcapLorentzAngleCorr;
    //
    in>>label;
    in>>c.m_biasVoltage;
    in>>label;
    in>>c.m_temperature;
    //
    in>>label;
    in>>c.m_barrelBiasVoltage;
    in>>label;
    in>>c.m_endcapBiasVoltage;
    in>>label;
    in>>c.m_DBMBiasVoltage;
    //
    in>>label;
    in>>c.m_fluenceLayer;
    in>>label;
    in>>c.m_radSimFluenceMapList;
    //
    in>>label;
    in>>c.m_fluenceLayer3D;
    in>>label;
    in>>c.m_radSimFluenceMapList3D;
    //
    in>>label;
    in>>c.m_cablingMapToFile;
    in>>label;
    in>>c.m_cablingMapFileName;
    //
    in>>label;
    in>>c.m_distortionInputSource;
    in>>label;
    in>>c.m_distortionVersion;
    in>>label;
    in>>c.m_distortionR1;
    in>>label;
    in>>c.m_distortionR2;
    in>>label;
    in>>c.m_distortionTwist;
    in>>label;
    in>>c.m_distortionMeanR;
    in>>label;
    in>>c.m_distortionRMSR;
    in>>label;
    in>>c.m_distortionMeanTwist;
    in>>label;
    in>>c.m_distortionRMSTwist;
    in>>label;
    in>>c.m_distortionWriteToFile;
    in>>label;
    in>>c.m_distortionFileName;
  }
  return in;
}

std::istream & operator >>(SoshiFormat & f, PixelModuleData & md){
  std::istream *i = f.m_is;
  std::string sline;
  std::vector<std::string> lBuffer;
  std::string multiline = "";
  std::vector<std::string> mapsPath_list;
  std::vector<std::string> mapsPath_list3D;
  while (getline(*i,sline)) {
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

  md.setBarrelToTThreshold(getParameter<int>("BarrelToTThreshold", lBuffer));
  md.setFEI3BarrelLatency(getParameter<int>("FEI3BarrelLatency", lBuffer));
  md.setFEI3BarrelHitDuplication(getParameter<bool>("FEI3BarrelHitDuplication", lBuffer));
  md.setFEI3BarrelSmallHitToT(getParameter<int>("FEI3BarrelSmallHitToT", lBuffer));
  md.setFEI3BarrelTimingSimTune(getParameter<int>("FEI3BarrelTimingSimTune", lBuffer));
  md.setBarrelCrossTalk(getParameter<double>("BarrelCrossTalk", lBuffer));
  md.setBarrelNoiseOccupancy(getParameter<double>("BarrelNoiseOccupancy", lBuffer));
  md.setBarrelDisableProbability(getParameter<double>("BarrelDisableProbability", lBuffer));
  md.setBarrelLorentzAngleCorr(getParameter<double>("BarrelLorentzAngleCorr", lBuffer));
  md.setDefaultBarrelBiasVoltage(getParameter<float>("BarrelBiasVoltage", lBuffer));

  md.setEndcapToTThreshold(getParameter<int>("EndcapToTThreshold", lBuffer));
  md.setFEI3EndcapLatency(getParameter<int>("FEI3EndcapLatency", lBuffer));
  md.setFEI3EndcapHitDuplication(getParameter<bool>("FEI3EndcapHitDuplication", lBuffer));
  md.setFEI3EndcapSmallHitToT(getParameter<int>("FEI3EndcapSmallHitToT", lBuffer));
  md.setFEI3EndcapTimingSimTune(getParameter<int>("FEI3EndcapTimingSimTune", lBuffer));
  md.setEndcapCrossTalk(getParameter<double>("EndcapCrossTalk", lBuffer));
  md.setEndcapNoiseOccupancy(getParameter<double>("EndcapNoiseOccupancy", lBuffer));
  md.setEndcapDisableProbability(getParameter<double>("EndcapDisableProbability", lBuffer));
  md.setEndcapLorentzAngleCorr(getParameter<double>("EndcapLorentzAngleCorr", lBuffer));
  md.setDefaultEndcapBiasVoltage(getParameter<float>("EndcapBiasVoltage", lBuffer));

  md.setEndcapNoiseShape({getParameter<float>("PixelNoiseShape", lBuffer),
                                   getParameter<float>("PixelNoiseShape", lBuffer),
                                   getParameter<float>("PixelNoiseShape", lBuffer)});

  // Radiation damage simulation
  md.setFluenceLayer(getParameter<double>("BarrelFluence", lBuffer));
  std::vector<std::string> barrelFluenceFile = getParameterString("BarrelRadiationFile", lBuffer);
  for (const auto & fluence : barrelFluenceFile) {
    mapsPath_list.push_back(PathResolverFindCalibFile(fluence));
  }

  if (f.run1) {    // RUN1
    md.setBarrelNoiseShape({getParameter<float>("BLayerNoiseShape", lBuffer),
                                     getParameter<float>("PixelNoiseShape", lBuffer),
                                     getParameter<float>("PixelNoiseShape", lBuffer)});
  } else {     // RUN2
    md.setDBMToTThreshold(getParameter<int>("DBMToTThreshold", lBuffer));
    md.setDBMCrossTalk(getParameter<double>("DBMCrossTalk", lBuffer));
    md.setDBMNoiseOccupancy(getParameter<double>("DBMNoiseOccupancy", lBuffer));
    md.setDBMDisableProbability(getParameter<double>("DBMDisableProbability", lBuffer));
    md.setDefaultDBMBiasVoltage(getParameter<float>("DBMBiasVoltage", lBuffer));

    md.setBarrelNoiseShape({getParameter<float>("IBLNoiseShape", lBuffer),
                                     getParameter<float>("BLayerNoiseShape", lBuffer),
                                     getParameter<float>("PixelNoiseShape", lBuffer),
                                     getParameter<float>("PixelNoiseShape", lBuffer)});

    md.setDBMNoiseShape({getParameter<float>("IBLNoiseShape", lBuffer),
                                  getParameter<float>("IBLNoiseShape", lBuffer),
                                  getParameter<float>("IBLNoiseShape", lBuffer)});

    // Radiation damage simulation for 3D sensor
    md.setFluenceLayer3D(getParameter<double>("3DFluence", lBuffer));
    std::vector<std::string> barrel3DFluenceFile = getParameterString("3DRadiationFile", lBuffer);
    for (const auto & fluence3D : barrel3DFluenceFile) {
      mapsPath_list3D.push_back(PathResolverFindCalibFile(fluence3D));
    }
  }
  md.setRadSimFluenceMapList(mapsPath_list);
  md.setRadSimFluenceMapList3D(mapsPath_list3D);
  return *i;

}

