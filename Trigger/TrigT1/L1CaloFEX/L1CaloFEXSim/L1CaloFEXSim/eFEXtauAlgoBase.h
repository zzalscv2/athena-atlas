/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************************
//                           eFEXtauAlgoBase.h
//                          -------------------
//     begin                : 08 05 2023
//     email                : david.reikher@cern.ch, nicholas.andrew.luongo@cern.ch
//  *********************************************************************************/

#ifndef eFEXtauAlgoBase_H
#define eFEXtauAlgoBase_H

#include "AthenaBaseComps/AthAlgTool.h" 
#include "L1CaloFEXSim/eTowerContainer.h"
#include "L1CaloFEXToolInterfaces/IeFEXtauAlgo.h"

namespace LVL1 {
// Doxygen class description below:
/** The eFEXtauBDTAlgo class calculates the tau BDT TOB variables
 */

class eFEXtauAlgoBase : public AthAlgTool, virtual public IeFEXtauAlgo {

public:
  /** Constructors */
  eFEXtauAlgoBase(const std::string &type, const std::string &name,
                 const IInterface *parent);

  /** Destructor */
  virtual ~eFEXtauAlgoBase();

  virtual StatusCode safetyTest() override;

  virtual void compute() override {};
  virtual bool isCentralTowerSeed() override;
  virtual bool isBDT() override { return false; }
  virtual void setThresholds(std::vector<unsigned int> rHadThreshold,
                             std::vector<unsigned int> bdtThreshold,
                             unsigned int etThreshold,
                             unsigned int etThresholdForRHad) override;

  virtual void getRCore(std::vector<unsigned int> & rCoreVec) override;
  virtual unsigned int rCoreCore() override { return 0; }
  virtual unsigned int rCoreEnv() override { return 0; }
  virtual float getRealRCore() override;
  virtual void getRHad(std::vector<unsigned int> &rHadVec) override;
  virtual float getRealRHad() override;
  virtual void getSums(unsigned int seed, bool UnD, 
                       std::vector<unsigned int> & RcoreSums, 
                       std::vector<unsigned int> & Remums) override;
  virtual unsigned int getBDTScore() override { return 0; }
  virtual unsigned int getBDTCondition() override { return 0; };

protected:
  SG::ReadHandleKey<LVL1::eTowerContainer> m_eTowerContainerKey {this, "MyETowers", "eTowerContainer", "Input container for eTowers"};
  bool m_cellsSet = false;

  int m_eFexalgoTowerID[3][3];

  void buildLayers(int efex_id, int fpga_id, int central_eta);
  void setSCellPointers();
  virtual void setSupercellSeed() { }
  virtual void setUnDAndOffPhi() { }

  virtual bool getUnD() override { return 0; }
  virtual unsigned int getSeed() override { return 0; }

  unsigned int m_em0cells[3][3];
  unsigned int m_em1cells[12][3];
  unsigned int m_em2cells[12][3];
  unsigned int m_em3cells[3][3];
  unsigned int m_hadcells[3][3];
  unsigned int m_twrcells[3][3];

};

} // namespace LVL1

#endif
