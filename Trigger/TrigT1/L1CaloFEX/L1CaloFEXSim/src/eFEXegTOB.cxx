/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXegTOB.cxx  -  
//                              -------------------
//     begin                : 17 01 2020
//     email                : tong.qiu@cern.ch
//  **************************************************************************


#include "L1CaloFEXSim/eFEXegTOB.h"


LVL1::eFEXegTOB::eFEXegTOB():
  m_eta{99999},
  m_phi{99999},
  m_ET{99999},
  m_Reta_Core{99999},
  m_Reta_Env{99999},
  m_Rhad_EM{99999},
  m_Rhad_Had{99999},
  m_Wstot_Num{99999},
  m_Wstot_Den{99999},
  m_FPGA_ID{99999},
  m_eFEX_ID{99999},
  m_seed_UnD{true},
  m_seed{99999},
  m_tobword{0},
  m_xtobword0{0},
  m_xtobword1{0}
{}

void LVL1::eFEXegTOB::setEta(unsigned int eta) {
  m_eta = eta;
}

void LVL1::eFEXegTOB::setPhi(unsigned int phi) {
  m_phi = phi;
}

void LVL1::eFEXegTOB::setET(unsigned int et) {
  m_ET = et;
}

void LVL1::eFEXegTOB::setFPGAID(unsigned int fpgaid) {
  m_FPGA_ID = fpgaid;
}

void LVL1::eFEXegTOB::seteFEXID(unsigned int efexid) {
  m_eFEX_ID = efexid;
}

void LVL1::eFEXegTOB::setSeedUnD(bool seedund) {
  m_seed_UnD = seedund;
}

void LVL1::eFEXegTOB::setSeed(unsigned int seed) {
  m_seed = seed;
}

void LVL1::eFEXegTOB::setRetaCore(unsigned int retaCore) {
  m_Reta_Core = retaCore;
}

void LVL1::eFEXegTOB::setRetaEnv(unsigned int retaEnv) {
  m_Reta_Env = retaEnv;
}

void LVL1::eFEXegTOB::setRhadEM(unsigned int rhadEM) {
  m_Rhad_EM = rhadEM;
}

void LVL1::eFEXegTOB::setRhadHad(unsigned int rhadHad) {
  m_Rhad_Had = rhadHad;
}

void LVL1::eFEXegTOB::setWstotNum(unsigned int wstot_Num) {
  m_Wstot_Num = wstot_Num;
}

void LVL1::eFEXegTOB::setWstotDen(unsigned int wstot_Den) {
  m_Wstot_Den = wstot_Den;
}

void LVL1::eFEXegTOB::setTobword(uint32_t tobword) {
  m_tobword = tobword;
}

void LVL1::eFEXegTOB::setxTobword0(uint32_t tobword) {
  m_xtobword0 = tobword;
}

void LVL1::eFEXegTOB::setxTobword1(uint32_t tobword) {
  m_xtobword1 = tobword;
}
