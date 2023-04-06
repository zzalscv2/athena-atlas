//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


/** This algrithm produces a column-wise NTuple
    out of a LAruA2MeVDB. Only the finalize
    method is used, initalize and execute are empty.
    The key of the object is given by the jobOption 
    'ContainerKey'. 
   * @author Walter Lampl
   * 11. 8. 2005 

*/

#ifndef LARUA2MEV2NTUPLE_H
#define LARUA2MEV2NTUPLE_H
#include "LArCalibTools/LArCond2NtupleBase.h"
#include "LArElecCalib/ILAruA2MeV.h"
#include "LArElecCalib/ILArDAC2uA.h"
#include "StoreGate/ReadHandleKey.h"

class LAruA2MeV2Ntuple : public LArCond2NtupleBase
{
 public:
  LAruA2MeV2Ntuple(const std::string & name, ISvcLocator * pSvcLocator);
  ~LAruA2MeV2Ntuple();

 //standard algorithm methods
  StatusCode initialize(); 
  virtual StatusCode stop();
  StatusCode finalize(){return StatusCode::SUCCESS;}
 private:
  SG::ReadCondHandleKey<ILAruA2MeV> m_uA2MeVKey{this,"uA2MeVKey","LAruA2MeV"};
  SG::ReadCondHandleKey<ILArDAC2uA> m_DAC2uAKey{this,"DAC2uAKey","LArDAC2uA"};
 
};

#endif
