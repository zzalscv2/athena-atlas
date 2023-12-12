/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */
#ifndef PixelNoiseFunctions_h
#define PixelNoiseFunctions_h


#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include <vector>

class SiChargedDiodeCollection;
class SiTotalCharge;
class PixelModuleData;
class PixelChargeCalibCondData;

namespace CLHEP{
  class HepRandomEngine;
}


namespace PixelDigitization{
  void crossTalk(double crossTalk, SiChargedDiodeCollection& chargedDiodes) ;
  
  void thermalNoise(double thermalNoise, SiChargedDiodeCollection& chargedDiodes,
    CLHEP::HepRandomEngine* rndmEngine);
                    
  void randomNoise(SiChargedDiodeCollection& chargedDiodes, const PixelModuleData *moduleData,
    const PixelChargeCalibCondData *chargeCalibData, CLHEP::HepRandomEngine* rndmEngine, 
    InDetDD::IPixelReadoutManager * pixelReadout);
    
  void 
  randomNoise(SiChargedDiodeCollection& chargedDiodes, const double totalNoiseOccupancy, 
    const std::vector<float> &noiseShape, float overflowToT,
    const PixelChargeCalibCondData *chargeCalibData, CLHEP::HepRandomEngine* rndmEngine, 
    InDetDD::IPixelReadoutManager * pixelReadout);
  
  //randomly disables certain elements, using moduleData to get probability
  void randomDisable(SiChargedDiodeCollection& chargedDiodes,
    const PixelModuleData *moduleData,
    CLHEP::HepRandomEngine* rndmEngine);
  
  //randomly disables certain elements, probability as a parameter          
  void randomDisable(SiChargedDiodeCollection& chargedDiodes,
    double disableProbability, CLHEP::HepRandomEngine* rndmEngine);
                     
  double getG4Time(const SiTotalCharge& totalCharge) ;
}//namespace
  
  #endif