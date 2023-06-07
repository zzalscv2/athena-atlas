/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           IeFEXtauAlgo.h  -
//                              -------------------
//     begin                : 12 05 2020
//     email                :  nicholas.andrew.luongo@cern.ch
//  ***************************************************************************/

#ifndef IeFEXtauAlgo_H
#define IeFEXtauAlgo_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"
#include "L1CaloFEXSim/eTowerContainer.h"

namespace LVL1 {
  
/*
Interface definition for eFEXtauAlgo
*/

  static const InterfaceID IID_IeFEXtauAlgo("LVL1::IeFEXtauAlgo", 1, 0);

  class IeFEXtauAlgo : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    virtual StatusCode safetyTest() = 0;
    virtual void setup(int inputTable[3][3], int efex_id, int fpga_id, int central_eta) = 0;
    virtual void compute() = 0;
    
    virtual bool isCentralTowerSeed() = 0;
    virtual std::unique_ptr<eFEXtauTOB> getTauTOB() = 0;
    virtual unsigned int rCoreCore() = 0;
    virtual unsigned int rCoreEnv() = 0;
    virtual void getRCore(std::vector<unsigned int> & rCoreVec) = 0;
    virtual float getRealRCore() = 0;
    virtual unsigned int rHadCore() = 0;
    virtual unsigned int rHadEnv() = 0;
    virtual void getRHad(std::vector<unsigned int> & rHadVec) = 0;
    virtual float getRealRHad() = 0;
    virtual unsigned int getEt() = 0;
    virtual unsigned int getBitwiseEt() = 0;
    virtual bool getUnD() = 0;
    virtual unsigned int getSeed() = 0;
    virtual void getSums(unsigned int seed, bool UnD, std::vector<unsigned int> & RcoreSums, 
                         std::vector<unsigned int> & RemSums) = 0;
    virtual unsigned int getBDTScore() = 0;
    virtual unsigned int getBDTCondition() = 0;
    virtual bool isBDT() = 0;
    virtual void setThresholds(std::vector<unsigned int> rHadThreshold, std::vector<unsigned int> bdtThreshold, unsigned int etThreshold, unsigned int etThresholdForRHad) = 0;

  private:

  };

  inline const InterfaceID& LVL1::IeFEXtauAlgo::interfaceID()
  {
    return IID_IeFEXtauAlgo;
  }

} // end of namespace

#endif
