/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARFSAMPL2NTUPLE_H
#define LARFSAMPL2NTUPLE_H

#include "LArCalibTools/LArCond2NtupleBase.h"
#include "LArElecCalib/ILArfSampl.h"
#include "StoreGate/ReadCondHandleKey.h"
class LArfSampl2Ntuple : public LArCond2NtupleBase
{
 public:
  LArfSampl2Ntuple(const std::string & name, ISvcLocator * pSvcLocator);
  ~LArfSampl2Ntuple();

  //standard algorithm methods
  StatusCode initialize();
  virtual StatusCode stop();
  StatusCode finalize(){return StatusCode::SUCCESS;}
 private:
  SG::ReadCondHandleKey<ILArfSampl> m_contKey{this,"ContainerKey","LArfSamplSym"};
  
};

#endif
