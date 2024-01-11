
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include <cassert>
#include "LArElecCalib/LArProvenance.h"

int main (int /*argc*/, char** /*argv*/) {
  uint16_t val;
  val=(LArProv::DEFAULTRECO | LArProv::SATURATED);

  assert(LArProv::test(val,LArProv::PEAKOFC)==true);
  assert(LArProv::test(val,LArProv::PEAKAVG)==false);
  assert(LArProv::test(val,LArProv::RAMPDB)==true);
  assert(LArProv::test(val,LArProv::PEDSAMPLEZERO)==false);
  assert(LArProv::test(val,LArProv::SATURATED)==true);
  assert(LArProv::test(val,LArProv::MASKED)==false);

  LArProv::LArProvenance check=static_cast< LArProv::LArProvenance>(LArProv::PEAKCUBIC | LArProv::RAMPDB | LArProv::PEDDB);
  assert(LArProv::test(val,check)==false);
  assert(LArProv::test((uint16_t)check,(LArProv::LArProvenance)val)==false);
  
}
