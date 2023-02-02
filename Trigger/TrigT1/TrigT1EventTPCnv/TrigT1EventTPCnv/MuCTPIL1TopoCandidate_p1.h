/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1EVENTTPCNV_MuCTPIL1TopoCandidate_P1_H
#define TRIGT1EVENTTPCNV_MuCTPIL1TopoCandidate_P1_H

// Forward declaration of the converter:
class MuCTPIL1TopoCandidateCnv_p1;

/**
 *   @short Persistent representation of MuCTPIL1TopoCandidate
 *
 *          This is the first version of the persistent representation(s)
 *          of MuCTPIL1TopoCandidate. It stores the same data as the transient one,
 *          without having the easy accessor functions.
 *
 *   @author Anil Sonay
 */

struct MuCTPIL1TopoCandidate_p1 {
  std::string  m_sectorName {};
  unsigned int m_roiID {};
  unsigned int m_bcid {}; 
  unsigned int m_ptThresholdID {};
  unsigned int m_ptL1TopoCode {};
  unsigned int m_ptValue {};
  float m_eta {};
  float m_phi {};
  unsigned int m_etacode {};
  unsigned int m_phicode {};
  float m_etamin {};
  float m_etamax {};
  float m_phimin {};
  float m_phimax {};
  unsigned int m_roiWord {};
  unsigned int m_mioctID {};
  int m_ieta {};
  int m_iphi {};
  bool m_phiOvl {};
  bool m_is2cand {};
  int m_charge {};
  bool m_bw2or3 {};
  bool m_innerCoin {};
  bool m_goodMF {};
};

#endif // TRIGT1EVENTTPCNV_MuCTPIL1TopoCandidate_P1_H
