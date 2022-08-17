/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//              jFEXTOB - TOBs info in jFEX
//                              -------------------
//     begin                : 18 02 2021
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************


#ifndef JFEX_TOB_H
#define JFEX_TOB_H
#include "AthenaKernel/CLASS_DEF.h"

namespace LVL1 {
  class jFEXTOB
  {

  public:
    jFEXTOB();
    ~jFEXTOB() {};
    
    void initialize(uint8_t , uint8_t , uint32_t , uint , uint );
    
    void setFpga(uint8_t  x){m_fpga = x;};
    void setjFex(uint8_t  x){m_jfex = x;};
    void setWord(uint32_t x){m_word = x;};
    void setRes (uint     x){m_res  = x;};
    void setTTID(uint     x){m_ttid = x;};
    
    uint8_t  getFpga(){return m_fpga;};
    uint8_t  getjFex(){return m_jfex;};
    uint32_t getWord(){return m_word;};
    uint     getRes() {return m_res; };
    uint     getTTID(){return m_ttid;};
    

  private:
    uint8_t  m_fpga;
    uint8_t  m_jfex;
    uint32_t m_word;
    uint     m_res;
    uint     m_ttid;
    

  };
  

} //end of namespace

CLASS_DEF( LVL1::jFEXTOB , 133374173 , 1 )
#endif 
