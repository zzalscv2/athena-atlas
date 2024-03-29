///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// CaloEnergy_p1.h 
// Header file for class CaloEnergy_p1
// Author: S.Binet<binet@cern.ch>
// Date:   March 2007
/////////////////////////////////////////////////////////////////// 
#ifndef RECTPCNV_CALOENERGY_P1_H 
#define RECTPCNV_CALOENERGY_P1_H 

// STL includes
#include <vector>

// RecTPCnv includes
#include "RecTPCnv/DepositInCalo_p1.h"

// forward declarations
class CaloEnergyCnv_p1;

class CaloEnergy_p1 
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class CaloEnergyCnv_p1;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  CaloEnergy_p1()
    : m_energyDeposit(0),
      m_energySigmaMinus(0),
      m_energySigmaPlus(0),
      m_energyLossType(0),
      m_caloLRLikelihood(0),
      m_caloMuonIdTag(0),
      m_fsrCandidateEnergy(0)
  {}

  /** Destructor: 
   */
  ~CaloEnergy_p1() = default;

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
public: 

  float          m_energyDeposit;
  float          m_energySigmaMinus;
  float          m_energySigmaPlus;
  int            m_energyLossType;
  float          m_caloLRLikelihood;
  unsigned short m_caloMuonIdTag;
  float          m_fsrCandidateEnergy;
  std::vector<DepositInCalo_p1> m_deposits; 
}; 

#endif //> RECTPCNV_CALOENERGY_P1_H
