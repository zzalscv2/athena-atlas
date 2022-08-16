/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//              jFEXtauTOB - TOBs Tau Algorithm in jFEX
//                              -------------------
//     begin                : 18 02 2021
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************

#include "L1CaloFEXSim/jFEXtauTOB.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"

namespace LVL1 {

jFEXtauTOB::jFEXtauTOB():
  m_fpga{0},
  m_jfex{0},
  m_word{0},
  m_res{0},
  m_ttid{0}
{}


void jFEXtauTOB::initialize(uint8_t fpga, uint8_t jfex, uint32_t word, uint res, uint ttid )
{
    setFpga (fpga);
    setjFex (jfex);
    setWord (word);
    setRes  (res);
    setTTID (ttid);

}


uint jFEXtauTOB::getEt(){
    return ((m_word >> FEXAlgoSpaceDefs::jTau_etBit ) & 0x7ff);
}


    
}


