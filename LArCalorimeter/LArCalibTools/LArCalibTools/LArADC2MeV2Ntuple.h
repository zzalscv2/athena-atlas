//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARCALIBTOOLS_ADC2MEV2NTUPLE
#define LARCALIBTOOLS_ADC2MEV2NTUPLE
#include "LArCalibTools/LArCond2NtupleBase.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArRawConditions/LArADC2MeV.h"
#include "CxxUtils/checker_macros.h"

class ATLAS_NOT_THREAD_SAFE LArADC2MeV2Ntuple : public LArCond2NtupleBase 
{
 public:

  using LArCond2NtupleBase::LArCond2NtupleBase;

  //standard algorithm methods
  StatusCode initialize();
  virtual StatusCode stop();
  StatusCode finalize(){return StatusCode::SUCCESS;}

 private:
  SG::ReadCondHandleKey<LArADC2MeV> m_adc2MeVKey{this,"LArADC2MeVKey","LArADC2MeV","SG key of the resulting LArADC2MeV object"};
  std::string m_ntName;

};

#endif
