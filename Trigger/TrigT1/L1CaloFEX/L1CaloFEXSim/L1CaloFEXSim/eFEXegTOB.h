/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXegTOB.h  -  
//                              -------------------
//     begin                : 17 01 2020
//     email                : tong.qiu@cern.ch
//  **************************************************************************


#pragma once
#include "AthenaKernel/CLASS_DEF.h"

namespace LVL1 {
  class eFEXegTOB
  {
    //eFEXegAlgo class description below:
    /** The eFEXegAlgo.h class store the energy, the location and the isolation variables of eFEX TOBs
     */
  private:
    unsigned int m_eta;
    unsigned int m_phi;
    unsigned int m_ET;
    unsigned int m_Reta_Core;
    unsigned int m_Reta_Env;
    unsigned int m_Rhad_EM;
    unsigned int m_Rhad_Had;
    unsigned int m_Wstot_Num;
    unsigned int m_Wstot_Den;
    unsigned int m_FPGA_ID;
    unsigned int m_eFEX_ID;
    bool m_seed_UnD;
    unsigned int m_seed;
    uint32_t m_tobword;
    uint32_t m_xtobword0;
    uint32_t m_xtobword1;
    
  public:
    eFEXegTOB();
    ~eFEXegTOB() {};
    
    inline unsigned  int getEta() const {return m_eta;}
    inline unsigned  int getPhi() const {return m_phi;}
    inline unsigned  int getET() const {return m_ET;}
    inline unsigned  int getFPGAID() const {return m_FPGA_ID;}
    inline unsigned  int geteFEXID() const {return m_eFEX_ID;}
    // if seed is above (higher phi) central supercell of the seed
    inline bool getSeedUnD() const {return m_seed_UnD;}
    // seed index in eta
    inline unsigned  int getSeed() const {return m_seed;}
    inline unsigned  int getRetaCore() const {return m_Reta_Core;}
    inline unsigned  int getRetaEnv() const {return m_Reta_Env;}
    inline unsigned  int getRhadEM() const {return m_Rhad_EM;}
    inline unsigned  int getRhadHad() const {return m_Rhad_Had;}
    inline unsigned  int getWstotNum() const {return m_Wstot_Num;}
    inline unsigned  int getWstotDen() const {return m_Wstot_Den;}
    inline uint32_t getTobword() const {return m_tobword;}
    inline uint32_t getxTobword0() const {return m_xtobword0;}
    inline uint32_t getxTobword1() const {return m_xtobword1;}
    
    void setEta(unsigned int);
    void setPhi(unsigned int);
    void setET(unsigned int);
    void setFPGAID(unsigned int);
    void seteFEXID(unsigned int);
    void setSeedUnD(bool);
    void setSeed(unsigned int);
    void setRetaCore(unsigned int);
    void setRetaEnv(unsigned int);
    void setRhadEM(unsigned int);
    void setRhadHad(unsigned int);
    void setWstotNum(unsigned int);
    void setWstotDen(unsigned int);
    void setTobword(uint32_t);
    void setxTobword0(uint32_t);
    void setxTobword1(uint32_t);
  };
  
} // end of namespace

CLASS_DEF( LVL1::eFEXegTOB, 32202261 , 1 )
