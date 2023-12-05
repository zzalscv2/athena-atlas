/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_SIMFIBERHIT_P1_H
#define ZDC_SIMFIBERHIT_P1_H

#include "Identifier/Identifier.h"

class ZDC_SimFiberHit_p1 {

 public:
  
  ZDC_SimFiberHit_p1() {
    m_Nphotons=0;
    m_Edep=0;
  };
  
  friend class ZDC_SimFiberHitCnv_p1;
  
 private:
  
  Identifier m_ID;
  int   m_Nphotons;
  float m_Edep;
};

#endif
