/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXtauBDT.h  -
//                              -------------------
//     begin                : 15 09 2022
//     email                : david.reikher@cern.ch
//  ***************************************************************************/

#ifndef eFEXtauBDT_H
#define eFEXtauBDT_H

//#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXSim/eFEXBDT.h"
#include <vector>

class AthAlgTool;

namespace LVL1 {
// Doxygen class description below:
/** The eFEXtauBDT class calculates the tau TOB variables
 */

class eFEXtauBDT {

public:
  /** Constructors */
  eFEXtauBDT(AthAlgTool *log, std::string config_path);

  /** Destructor */
  virtual ~eFEXtauBDT();

  void next();

  void setPointerToSCell(int eta, int phi, int layer, unsigned int *sCellPtr);
  void setPointerToFracMultipliersParam(int index,
                                        unsigned int *fracMultipliers);
  void setPointerToBDTThresholdsParam(int index, unsigned int *bdtThresholds);
  void setPointerToETThresholdParam(unsigned int *etThreshold);
  void setPointerToETThresholdForFracParam(unsigned int *etThresholdForFrac);
  void buildBDTVariables();
  void computeBDTScore();
  void computeETEstimate();
  void computeEMETEstimate();
  void computeHADETEstimate();
  void computeFracCondition();
  void computeBDTCondition();
  void computeTowers();
  void computeIsCentralTowerSeed();
  void debugPrintSCellValues();
  void debugPrintBDTVariables();
  void debugPrintTowers();
  void initBDTVars();
  void initETPointers();
  void initEMETPointers();
  void initHADETPointers();
  void initTowersPointers();
  unsigned int getETEstimate() { return m_eTEstimate; }
  unsigned int getEMETEstimate() { return m_EM_eTEstimate; }
  unsigned int getEMETEstimateOverflow() { return m_EM_eTEstimateOverflow; }
  unsigned int getHADETEstimate() { return m_HAD_eTEstimate; }
  unsigned int getHADETEstimateOverflow() { return m_HAD_eTEstimateOverflow; }
  std::vector<unsigned int> &getBDTVars() { return m_bdtVars; }
  std::vector<unsigned int> &getTowers() { return m_towers; }
  std::vector<unsigned int> &getEMMultipliedByFracParams() {
    return m_emEtXMultiplier;
  }
  std::vector<unsigned int> &getEMMultipliedByFracParamsOverflow() {
    return m_emEtXMultiplierOverflow;
  }
  unsigned int getBDTScoreShifted() { return m_bdtScoreShifted; }

  // ======= These are the important getters ======
  // These are the fields in the algorithm's output TOB structure in the
  // VHDL implementation of eFEXFirmware (TriggerObjectCore_tau type in VHDL)
  unsigned int getET();
  unsigned int getTOBETOverflow() { return m_eTEstimateOverflow; }
  unsigned int getIsMax() { return m_isSeeded; }
  unsigned int getBDTCondition() { return m_bdtCondition; }
  unsigned int getFracCondition() { return m_fracCondition; }
  unsigned int getBDTScore() { return m_bdtScore; }
  // ================================================================================

  unsigned int multWithOverflow(unsigned int a, unsigned int b, bool &overflow,
                                int resultNBits);
  inline bool isOverflow(unsigned int number, int nBits);
  inline unsigned int BitLeftShift(unsigned int number, int by, int totalNBits);
  inline int flatTowerIndex(int eta, int phi);


private:
  void initPointers(const std::vector<std::vector<int>> &scells,
                    std::vector<unsigned int *> &ptr_list);
  unsigned int computeEstimate(std::vector<unsigned int *> &ptr_list,
                               bool &overflow, int resultNBits);

  unsigned int *superCellToPtr(int eta, int phi, int layer);

  unsigned int *m_em0cells[3][3]{};
  unsigned int *m_em1cells[12][3]{};
  unsigned int *m_em2cells[12][3]{};
  unsigned int *m_em3cells[3][3]{};
  unsigned int *m_hadcells[3][3]{};
  unsigned int *m_fracMultipliers[3]{};
  unsigned int *m_bdtThresholds[3]{};
  unsigned int *m_etThreshold{};
  unsigned int *m_etThresholdForFrac{};
  unsigned int m_bdtScore = 0;
  unsigned int m_bdtScoreShifted = 0;
  unsigned int m_eTEstimate = 0;
  bool m_eTEstimateOverflow = false;
  unsigned int m_EM_eTEstimate = 0;
  bool m_EM_eTEstimateOverflow = 0;
  unsigned int m_HAD_eTEstimate = 0;
  bool m_HAD_eTEstimateOverflow = false;
  unsigned int m_fracCondition = 0;
  unsigned int m_bdtCondition = 0;
  bool m_isSeeded = false;
  unsigned int m_hadEstimateShifted = 0;
  std::vector<unsigned int> m_emEtXMultiplier;
  std::vector<unsigned int> m_emEtXMultiplierOverflow;
  std::vector<unsigned int> m_towers;

  // Final computed BDT variables
  std::vector<unsigned int> m_bdtVars;

  // BDT variables are sums of supercells. The pointers to the memory locations
  // containing the supercell ET values for each variable are stored in the
  // following vector. Index in the vector corresponds to index of the variable.
  std::vector<std::vector<unsigned int *>> m_bdtVarComputeSCellPointers;

  // List of pointers to supercells that participate in the sum to estimate the
  // ET
  std::vector<unsigned int *> m_eTComputeSCellPointers;
  std::vector<unsigned int *> m_EM_eTComputeSCellPointers;
  std::vector<unsigned int *> m_HAD_eTComputeSCellPointers;
  std::vector<std::vector<unsigned int *>> m_towersComputeSCellPointers;

  bool m_bdtVarsComputed = false;
  eFEXBDT m_bdt;
  AthAlgTool *m_log;

  const std::vector<std::vector<int>> m_locMap{
      {0, 0, 0}, {1, 0, 0},  {2, 0, 0},  {0, 1, 0}, {1, 1, 0}, {2, 1, 0},
      {0, 2, 0}, {1, 2, 0},  {2, 2, 0},  {0, 0, 1}, {1, 0, 1}, {2, 0, 1},
      {3, 0, 1}, {4, 0, 1},  {5, 0, 1},  {6, 0, 1}, {7, 0, 1}, {8, 0, 1},
      {9, 0, 1}, {10, 0, 1}, {11, 0, 1}, {0, 1, 1}, {1, 1, 1}, {2, 1, 1},
      {3, 1, 1}, {4, 1, 1},  {5, 1, 1},  {6, 1, 1}, {7, 1, 1}, {8, 1, 1},
      {9, 1, 1}, {10, 1, 1}, {11, 1, 1}, {0, 2, 1}, {1, 2, 1}, {2, 2, 1},
      {3, 2, 1}, {4, 2, 1},  {5, 2, 1},  {6, 2, 1}, {7, 2, 1}, {8, 2, 1},
      {9, 2, 1}, {10, 2, 1}, {11, 2, 1}, {0, 0, 2}, {1, 0, 2}, {2, 0, 2},
      {3, 0, 2}, {4, 0, 2},  {5, 0, 2},  {6, 0, 2}, {7, 0, 2}, {8, 0, 2},
      {9, 0, 2}, {10, 0, 2}, {11, 0, 2}, {0, 1, 2}, {1, 1, 2}, {2, 1, 2},
      {3, 1, 2}, {4, 1, 2},  {5, 1, 2},  {6, 1, 2}, {7, 1, 2}, {8, 1, 2},
      {9, 1, 2}, {10, 1, 2}, {11, 1, 2}, {0, 2, 2}, {1, 2, 2}, {2, 2, 2},
      {3, 2, 2}, {4, 2, 2},  {5, 2, 2},  {6, 2, 2}, {7, 2, 2}, {8, 2, 2},
      {9, 2, 2}, {10, 2, 2}, {11, 2, 2}, {0, 0, 3}, {1, 0, 3}, {2, 0, 3},
      {0, 1, 3}, {1, 1, 3},  {2, 1, 3},  {0, 2, 3}, {1, 2, 3}, {2, 2, 3},
      {0, 0, 4}, {1, 0, 4},  {2, 0, 4},  {0, 1, 4}, {1, 1, 4}, {2, 1, 4},
      {0, 2, 4}, {1, 2, 4},  {2, 2, 4}};
};

} // namespace LVL1

#endif
