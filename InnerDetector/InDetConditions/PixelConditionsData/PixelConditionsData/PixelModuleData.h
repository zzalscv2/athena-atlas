/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsData/PixelModuleData.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date November, 2019
 * @brief Store pixel constant parameters in PixelModuleData.
 */

#ifndef PIXELMODULEDATA_H
#define PIXELMODULEDATA_H

#include <AthenaKernel/CLASS_DEF.h>
#include <AthenaKernel/CondCont.h>
#include "CLHEP/Units/SystemOfUnits.h"
#include <iosfwd>


class PixelModuleData 
{
  public:
    // stream insertion
    friend std::ostream & operator << (std::ostream &out, const PixelModuleData &c);
    friend std::istream & operator >> (std::istream &in, PixelModuleData &c);
    // Digitization parameters
    void setBunchSpace(double bunchSpace);
    double getBunchSpace() const;

    void setBarrelNumberOfBCID(const std::vector<int> &barrelNumberOfBCID);
    void setEndcapNumberOfBCID(const std::vector<int> &endcapNumberOfBCID);
    void setDBMNumberOfBCID(const std::vector<int> &DBMNumberOfBCID);
    int getNumberOfBCID(int barrel_ec, int layer) const;

    void setBarrelTimeOffset(const std::vector<double> &barrelTimeOffset);
    void setEndcapTimeOffset(const std::vector<double> &endcapTimeOffset);
    void setDBMTimeOffset(const std::vector<double> &DBMTimeOffset);
    double getTimeOffset(int barrel_ec, int layer) const;

    void setBarrelTimeJitter(const std::vector<double> &barrelTimeJitter);
    void setEndcapTimeJitter(const std::vector<double> &endcapTimeJitter);
    void setDBMTimeJitter(const std::vector<double> &DBMTimeJitter);
    double getTimeJitter(int barrel_ec, int layer) const;

    void setDefaultBarrelAnalogThreshold(const std::vector<int> &barrelAnalogThreshold);
    void setDefaultEndcapAnalogThreshold(const std::vector<int> &endcapAnalogThreshold);
    void setDefaultDBMAnalogThreshold(const std::vector<int> &DBMAnalogThreshold);
    int getDefaultAnalogThreshold(int barrel_ec, int layer) const;

    void setDefaultBarrelAnalogThresholdSigma(const std::vector<int> &barrelAnalogThresholdSigma);
    void setDefaultEndcapAnalogThresholdSigma(const std::vector<int> &endcapAnalogThresholdSigma);
    void setDefaultDBMAnalogThresholdSigma(const std::vector<int> &DBMAnalogThresholdSigma);
    int getDefaultAnalogThresholdSigma(int barrel_ec, int layer) const;

    void setDefaultBarrelAnalogThresholdNoise(const std::vector<int> &barrelAnalogThresholdNoise);
    void setDefaultEndcapAnalogThresholdNoise(const std::vector<int> &endcapAnalogThresholdNoise);
    void setDefaultDBMAnalogThresholdNoise(const std::vector<int> &DBMAnalogThresholdNoise);
    int getDefaultAnalogThresholdNoise(int barrel_ec, int layer) const;

    void setDefaultBarrelInTimeThreshold(const std::vector<int> &barrelInTimeThreshold);
    void setDefaultEndcapInTimeThreshold(const std::vector<int> &endcapInTimeThreshold);
    void setDefaultDBMInTimeThreshold(const std::vector<int> &DBMInTimeThreshold);
    int getDefaultInTimeThreshold(int barrel_ec, int layer) const;

    void setBarrelToTThreshold(const std::vector<int> &barrelToTThreshold);
    void setEndcapToTThreshold(const std::vector<int> &endcapToTThreshold);
    void setDBMToTThreshold(const std::vector<int> &DBMToTThreshold);
    int getToTThreshold(int barrel_ec, int layer) const;

    void setBarrelCrossTalk(const std::vector<double> &barrelCrossTalk);
    void setEndcapCrossTalk(const std::vector<double> &endcapCrossTalk);
    void setDBMCrossTalk(const std::vector<double> &DBMCrossTalk);
    double getCrossTalk(int barrel_ec, int layer) const;

    void setBarrelThermalNoise(const std::vector<double> &barrelThermalNoise);
    void setEndcapThermalNoise(const std::vector<double> &endcapThermalNoise);
    void setDBMThermalNoise(const std::vector<double> &DBMThermalNoise);
    double getThermalNoise(int barrel_ec, int layer) const;

    void setBarrelNoiseOccupancy(const std::vector<double> &barrelNoiseOccupancy);
    void setEndcapNoiseOccupancy(const std::vector<double> &endcapNoiseOccupancy);
    void setDBMNoiseOccupancy(const std::vector<double> &DBMNoiseOccupancy);
    double getNoiseOccupancy(int barrel_ec, int layer) const;

    void setBarrelDisableProbability(const std::vector<double> &barrelDisableProbability);
    void setEndcapDisableProbability(const std::vector<double> &endcapDisableProbability);
    void setDBMDisableProbability(const std::vector<double> &DBMDisableProbability);
    double getDisableProbability(int barrel_ec, int layer) const;

    void setBarrelNoiseShape(const std::vector<std::vector<float>> &barrelNoiseShape);
    void setEndcapNoiseShape(const std::vector<std::vector<float>> &endcapNoiseShape);
    void setDBMNoiseShape(const std::vector<std::vector<float>> &DBMNoiseShape);
    const std::vector<float> &getNoiseShape(int barrel_ec, int layer) const;

    void setFEI3BarrelLatency(const std::vector<int> &FEI3BarrelLatency);
    void setFEI3EndcapLatency(const std::vector<int> &FEI3EndcapLatency);
    int getFEI3Latency(int barrel_ec, int layer) const;

    void setFEI3BarrelHitDuplication(const std::vector<bool> &FEI3BarrelHitDuplication);
    void setFEI3EndcapHitDuplication(const std::vector<bool> &FEI3EndcapHitDuplication);
    bool getFEI3HitDuplication(int barrel_ec, int layer) const;

    void setFEI3BarrelSmallHitToT(const std::vector<int> &FEI3BarrelSmallHitToT);
    void setFEI3EndcapSmallHitToT(const std::vector<int> &FEI3EndcapSmallHitToT);
    int getFEI3SmallHitToT(int barrel_ec, int layer) const;

    void setFEI3BarrelTimingSimTune(const std::vector<int> &FEI3BarrelTimingSimTune);
    void setFEI3EndcapTimingSimTune(const std::vector<int> &FEI3EndcapTimingSimTune);
    int getFEI3TimingSimTune(int barrel_ec, int layer) const;

    void setFEI4BarrelHitDiscConfig(const std::vector<int> &FEI4BarrelHitDiscConfig);
    void setFEI4EndcapHitDiscConfig(const std::vector<int> &FEI4EndcapHitDiscConfig);
    int getFEI4HitDiscConfig(int barrel_ec, int layer) const;
    int getFEI4OverflowToT(int barrel_ec, int layer) const;

    void setFEI4ChargScaling(float scaleFEI4);
    void setUseFEI4SpecialScalingFunction(bool useFEI4SpecialScalingFunction);
    float getFEI4ChargScaling() const;
    bool getUseFEI4SpecialScalingFunction() const;

    void setFEI4ToTSigma(const std::vector<double> &FEI4ToTSigma);
    double getFEI4ToTSigma(int tot) const;

    // Charge calibration parameters
    void setDefaultQ2TotA(float paramA);
    void setDefaultQ2TotE(float paramE);
    void setDefaultQ2TotC(float paramC);
    float getDefaultQ2TotA() const;
    float getDefaultQ2TotE() const;
    float getDefaultQ2TotC() const;

    void setPIXLinearExtrapolation(bool doLinearExtrapolation);
    bool getPIXLinearExtrapolation() const;

    // Lorentz angle correction
    void setBarrelLorentzAngleCorr(const std::vector<double> &barrelLorentzAngleCorr);
    void setEndcapLorentzAngleCorr(const std::vector<double> &endcapLorentzAngleCorr);
    double getLorentzAngleCorr(int barrel_ec, int layer) const;

    // DCS parameters
    void setDefaultBiasVoltage(float biasVoltage);
    float getDefaultBiasVoltage() const;

    void setDefaultBarrelBiasVoltage(const std::vector<float> &barrelBiasVoltage);
    void setDefaultEndcapBiasVoltage(const std::vector<float> &endcapBiasVoltage);
    void setDefaultDBMBiasVoltage(const std::vector<float> &DBMBiasVoltage);
    float getDefaultBiasVoltage(int barrel_ec, int layer) const;

    void setDefaultTemperature(float temperature);
    float getDefaultTemperature() const;

    // Radiation damage fluence maps
    void setFluenceLayer(const std::vector<double> &fluenceLayer);
    const std::vector<double> &getFluenceLayer() const;

    void setRadSimFluenceMapList(const std::vector<std::string> &radSimFluenceMapList);
    const std::vector<std::string> &getRadSimFluenceMapList() const;

    void setFluenceLayer3D(const std::vector<double> &fluenceLayer);
    const std::vector<double> &getFluenceLayer3D() const;

    void setRadSimFluenceMapList3D(const std::vector<std::string> &radSimFluenceMapList3D);
    const std::vector<std::string> &getRadSimFluenceMapList3D() const;

    // Cabling parameters
    void setCablingMapToFile(bool cablingMapToFile);
    bool getCablingMapToFile() const;

    void setCablingMapFileName(const std::string &cablingMapFileName);
    const std::string &getCablingMapFileName() const;

    // Distortion parameters
    void setDistortionInputSource(int distortionInputSource);
    int getDistortionInputSource() const;

    void setDistortionVersion(int distortionVersion);
    int getDistortionVersion() const;

    void setDistortionR1(double distortionR1);
    double getDistortionR1() const;

    void setDistortionR2(double distortionR2);
    double getDistortionR2() const;

    void setDistortionTwist(double distortionTwist);
    double getDistortionTwist() const;

    void setDistortionMeanR(double distortionMeanR);
    double getDistortionMeanR() const;

    void setDistortionRMSR(double distortionRMSR);
    double getDistortionRMSR() const;

    void setDistortionMeanTwist(double distortionMeanTwist);
    double getDistortionMeanTwist() const;

    void setDistortionRMSTwist(double distortionRMSTwist);
    double getDistortionRMSTwist() const;

    void setDistortionWriteToFile(bool distortionWriteToFile);
    bool getDistortionWriteToFile() const;

    void setDistortionFileName(const std::string &distortionFileName);
    const std::string &getDistortionFileName() const;


  private:
    //default settings are from PixelConfigCondAlg.h
    double m_bunchSpace{25.0};
    //
    std::vector<int> m_barrelNumberOfBCID{1,1,1,1};
    std::vector<int> m_endcapNumberOfBCID{1,1,1};
    std::vector<int> m_DBMNumberOfBCID{1,1,1};
    //
    std::vector<double> m_barrelTimeOffset{5.0,5.0,5.0,5.0};
    std::vector<double> m_endcapTimeOffset{5.0,5.0,5.0};
    std::vector<double> m_DBMTimeOffset{5.0,5.0,5.0};
    //
    std::vector<double> m_barrelTimeJitter{0.0,0.0,0.0,0.0};
    std::vector<double> m_endcapTimeJitter{0.0,0.0,0.0};
    std::vector<double> m_DBMTimeJitter{0.0,0.0,0.0};
    //defaults are for RUN2 2015/2016
    std::vector<int> m_defaultBarrelAnalogThreshold{-1,-1,-1,-1};
    std::vector<int> m_defaultEndcapAnalogThreshold{-1,-1,-1};
    std::vector<int> m_defaultDBMAnalogThreshold{-1,-1,-1};
    //
    std::vector<int> m_defaultBarrelAnalogThresholdSigma{45,35,30,30};
    std::vector<int> m_defaultEndcapAnalogThresholdSigma{30,30,30};
    std::vector<int> m_defaultDBMAnalogThresholdSigma{70,70,70};
    //
    std::vector<int> m_defaultBarrelAnalogThresholdNoise{130,150,160,160};
    std::vector<int> m_defaultEndcapAnalogThresholdNoise{150,150,150};
    std::vector<int> m_defaultDBMAnalogThresholdNoise{190,190,190};
    //
    std::vector<int> m_defaultBarrelInTimeThreshold{2000,5000,5000,5000};
    std::vector<int> m_defaultEndcapInTimeThreshold{5000,5000,5000};
    std::vector<int> m_defaultDBMInTimeThreshold{1200,1200,1200};
    //
    std::vector<int> m_barrelToTThreshold{-1, 5, 5, 5};
    std::vector<int> m_endcapToTThreshold{ 5, 5, 5};
    std::vector<int> m_DBMToTThreshold{-1,-1,-1};
    //
    std::vector<double> m_barrelCrossTalk{0.30,0.12,0.12,0.12}; 
    std::vector<double> m_endcapCrossTalk{0.06,0.06,0.06}; 
    std::vector<double> m_DBMCrossTalk{0.06,0.06,0.06};
    //
    std::vector<double> m_barrelThermalNoise{160.0,160.0,160.0,160.0};
    std::vector<double> m_endcapThermalNoise{160.0,160.0,160.0};
    std::vector<double> m_DBMThermalNoise{160.0,160.0,160.0};
    //
    std::vector<double> m_barrelNoiseOccupancy{5e-8,5e-8,5e-8,5e-8};
    std::vector<double> m_endcapNoiseOccupancy{5e-8,5e-8,5e-8};
    std::vector<double> m_DBMNoiseOccupancy{5e-8,5e-8,5e-8};
    //
    std::vector<double> m_barrelDisableProbability{9e-3,9e-3,9e-3,9e-3};
    std::vector<double> m_endcapDisableProbability{9e-3,9e-3,9e-3};
    std::vector<double> m_DBMDisableProbability{9e-3,9e-3,9e-3};
    //
    const  std::vector<float> m_BLayerNoiseShape{0.0, 0.0, 0.0, 0.0, 0.2204, 0.5311, 0.7493, 0.8954, 0.9980, 1.0};
    const  std::vector<float> m_PixNoiseShape{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.2418, 0.4397, 0.5858, 0.6949, 0.7737, 0.8414, 0.8959, 0.9414, 0.9828, 1.0};
    const  std::vector<float> m_IBLNoiseShape{0.0, 0.0330, 0.0, 0.3026, 0.5019, 0.6760, 0.8412, 0.9918, 0.9918, 0.9918, 0.9918, 0.9918, 0.9918, 0.9918, 0.9918, 0.9918, 1.0};
    std::vector<std::vector<float>> m_barrelNoiseShape{m_BLayerNoiseShape, m_PixNoiseShape ,m_PixNoiseShape} ;
    std::vector<std::vector<float>> m_endcapNoiseShape{m_PixNoiseShape, m_PixNoiseShape, m_PixNoiseShape};
    std::vector<std::vector<float>> m_DBMNoiseShape{m_IBLNoiseShape,m_IBLNoiseShape,m_IBLNoiseShape};
    //
    std::vector<int>  m_FEI3BarrelLatency{0,151,256,256};
    std::vector<int>  m_FEI3EndcapLatency{256,256,256};
    //
    std::vector<bool> m_FEI3BarrelHitDuplication{false,false,false,false};
    std::vector<bool> m_FEI3EndcapHitDuplication{false,false,false};
    
    std::vector<int>  m_FEI3BarrelSmallHitToT{-1,-1,-1,-1};
    std::vector<int>  m_FEI3EndcapSmallHitToT{-1,-1,-1};
    //
    std::vector<int>  m_FEI3BarrelTimingSimTune{-1,2015,2015,2015};
    std::vector<int>  m_FEI3EndcapTimingSimTune{2015,2015,2015};
    //
    std::vector<int>  m_FEI4BarrelHitDiscConfig{2,2,2};
    std::vector<int>  m_FEI4EndcapHitDiscConfig{2,2,2};

    float m_scaleFEI4{1.0};
    bool m_useFEI4SpecialScalingFunction{true};

    std::vector<double> m_FEI4ToTSigma{0.0,0.50,0.50,0.50,0.50,0.50,0.60,0.60,0.60,0.60,0.65,0.70,0.75,0.80,0.80,0.80,0.80};

    float m_paramA{70.2f};
    float m_paramE{-3561.25f};
    float m_paramC{26000.0f};
    //default should not matter, but for ITk algorithms might cause problems
    bool m_doLinearExtrapolation{false};

    std::vector<double> m_barrelLorentzAngleCorr{1.0,1.0,1.0,1.0};
    std::vector<double> m_endcapLorentzAngleCorr{1.0,1.0,1.0};

    float m_biasVoltage{150.f};
    float m_temperature{-7.f};

    std::vector<float> m_barrelBiasVoltage{80.0,350.0,200.0,150.0};
    std::vector<float> m_endcapBiasVoltage{150.0,150.0,150.0};
    std::vector<float> m_DBMBiasVoltage{500.0,500.0,500.0};

    std::vector<double> m_fluenceLayer{0.80e14, 1.61e14, 0.71e14, 0.48e14};
    std::vector<std::string> m_radSimFluenceMapList{"PixelDigitization/maps_IBL_PL_80V_fl0_8e14.root",
                                    "PixelDigitization/maps_PIX_350V_fl1_61e14.root",
                                    "PixelDigitization/maps_PIX_200V_fl0_71e14.root",
                                    "PixelDigitization/maps_PIX_150V_fl0_48e14.root"};

    std::vector<double> m_fluenceLayer3D{5.0e15};
    std::vector<std::string> m_radSimFluenceMapList3D{"PixelDigitization/TCAD_IBL_3Dsensors_efields/phi_5e15_160V.root"};

    bool        m_cablingMapToFile{false};
    std::string m_cablingMapFileName{"PixelCabling/Pixels_Atlas_IdMapping_2016.dat"};

    int    m_distortionInputSource{4};//database
    int    m_distortionVersion{-1};
    double m_distortionR1{0.1/CLHEP::meter};
    double m_distortionR2{  0.1/CLHEP::meter};
    double m_distortionTwist{ 0.0005};
    double m_distortionMeanR{0.12/CLHEP::meter,};
    double m_distortionRMSR{0.08/CLHEP::meter};
    double m_distortionMeanTwist{-0.0005};
    double m_distortionRMSTwist{0.0008};
    bool   m_distortionWriteToFile{false};
    std::string m_distortionFileName{ "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/TrackingCP/PixelDistortions/PixelDistortionsData_v2_BB.txt"};

};

CLASS_DEF( PixelModuleData , 345932873 , 1 )

CONDCONT_DEF( PixelModuleData, 578988393 );

#endif
