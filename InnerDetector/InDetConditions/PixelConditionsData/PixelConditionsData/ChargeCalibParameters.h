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
#include <iosfwd>
 
namespace PixelChargeCalib{
  
  struct LegacyFitParameters{
    float A = 0.f;
    float E = 0.f;
    float C = 0.f;
    LegacyFitParameters() = default;
    LegacyFitParameters(float a, float e, float c):A(a), E(e), C(c){
      //nop
    }
    bool operator == (const LegacyFitParameters & o){
      return ((o.A == A) and (o.E == E) and (o.C == C));
    }
    bool operator != (const LegacyFitParameters & o){
      return not operator == (o);
    }
    ///Return Time-over-threshold given charge Q
    float ToT(float Q) const{
      if ((C + Q) != 0.0f) {
        return A * (E + Q) / (C + Q);
      }
      return 0.f;
    }
    //return Charge, given time-over-Threshold
    float Q(float tot) const{
      if (std::fabs(A) != 0.0f && std::fabs(tot / A - 1.f) != 0.0f) {
        return  (C * tot / A - E) / (1.f - tot / A);
      }
      return 0.f;
    }
    
  };
 
  struct LinearFitParameters{
    float F = 0.f;
    float G = 0.f;
    LinearFitParameters() = default;
    LinearFitParameters(float f, float g):F(f), G(g){
      //nop
    }
    bool operator == (const LinearFitParameters & o){
      return ((o.F == F) and (o.G == G) );
    }
    bool operator != (const LinearFitParameters & o){
      return not operator == (o);
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
    Thresholds() = default;
    Thresholds (int v, int s, int n, int i):value(v), sigma(s), noise(n), inTimeValue(i){
      //nop
    }
    bool operator == (const Thresholds & o){
      return ((o.value == value) and (o.sigma == sigma) and (o.noise == noise) and (o.inTimeValue == inTimeValue));
    }
    bool operator != (const Thresholds & o){
      return not operator == (o);
    }
  };
  
  struct Resolutions{
    float res1 = 0.f;
    float res2 = 0.f;
    Resolutions() = default;
    Resolutions(float r1, float r2):res1(r1), res2(r2){
      //nop
    }
    bool operator == (const Resolutions & o){
      return ((o.res1 == res1) and (o.res2 == res2) );
    }
    bool operator != (const Resolutions & o){
      return not operator == (o);
    }
    float total(float Q) const {
      return res1 + res2 * Q;
    }
  };
  
  std::ostream & operator << (std::ostream & out, const LegacyFitParameters & legFitPar);
  std::ostream & operator << (std::ostream & out, const LinearFitParameters & linFitParam);
  std::ostream & operator << (std::ostream & out, const Thresholds & t);
  std::ostream & operator << (std::ostream & out, const Resolutions & r); 
}

#endif
