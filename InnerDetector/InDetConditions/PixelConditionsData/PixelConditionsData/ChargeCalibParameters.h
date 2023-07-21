/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ChargeCalibParameters.h
 * @author Shaun Roe
 * @date June, 2023
 * @brief Structs for holding charge calibration parameterisation and data
 */
#ifndef ChargeCalibParameters_h
#define ChargeCalibParameters_h
#include <cmath>
 
namespace PixelChargeCalib{
  
  struct LegacyFitParameters{
    float A = 0.f;
    float E = 0.f;
    float C = 0.f;
    LegacyFitParameters(float a, float e, float c):A(a), E(e), C(c){
      //nop
    }
    ///Return Time-over-threshold given charge Q
    float ToT(float Q) const{
      if ((C + Q) != 0.0) {
        return A * (E + Q) / (C + Q);
      }
      return 0.f;
    }
    //return Charge, given time-over-Threshold
    float Q(float tot) const{
      if (std::fabs(A) != 0.0 && std::fabs(tot / A - 1.0) != 0.0) {
        return  (C * tot / A - E) / (1.0 - tot / A);
      }
      return 0.f;
    }
    
  };
 
  struct LinearFitParameters{
    float F = 0.f;
    float G = 0.f;
    LinearFitParameters(float f, float g):F(f), G(g){
      //nop
    }
    float ToT(float Q) const{
      if (F != 0.0f){
        return (Q - G) / F;
      }
      return 0.f;
    }
    float Q(float tot) const {
     return  F * tot + G;
    }
  };
 
  struct Thresholds{
    int value = 0;
    int sigma = 0;
    int noise = 0;
    int inTimeValue = 0;
    Thresholds (int v, int s, int n, int i):value(v), sigma(s), noise(n), inTimeValue(i){
      //nop
    }
  };
  
  struct Resolutions{
    float res1 = 0.f;
    float res2 = 0.f;
    Resolutions() = default;
    Resolutions(float r1, float r2):res1(r1), res2(r2){
      //nop
    }
    float total(float Q) const {
      return res1 + res2 * Q;
    }
  };
}

#endif
