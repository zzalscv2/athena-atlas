///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// CaloEnergy_p2.h 
// Header file for class CaloEnergy_p2
// Author: Ketevi A. Assamagan<ketevi@bnl.gov>
// Date:   february 2008
/////////////////////////////////////////////////////////////////// 
#ifndef RECTPCNV_CALOENERGY_P2_H 
#define RECTPCNV_CALOENERGY_P2_H 

// STL includes
#include <vector>

// RecTPCnv includes
#include "RecTPCnv/DepositInCalo_p2.h"
#include "TrkEventTPCnv/TrkMaterialOnTrack/EnergyLoss_p1.h"

// forward declarations
class CaloEnergyCnv_p2;

class CaloEnergy_p2 
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class CaloEnergyCnv_p2;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  CaloEnergy_p2()
    : m_energyLossType(0),
      m_caloLRLikelihood(0),
      m_caloMuonIdTag(0),
      m_fsrCandidateEnergy(0),
      m_etCore(0)
  {}

  /** Destructor: 
   */
  ~CaloEnergy_p2() = default;

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private:

  Trk::EnergyLoss_p1  m_energyLoss;

private: 

  int            m_energyLossType;
  float          m_caloLRLikelihood;
  unsigned short m_caloMuonIdTag;
  float          m_fsrCandidateEnergy;
  std::vector<DepositInCalo_p2> m_deposits; 
  float          m_etCore; 

}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

#endif //> RECTPCNV_CALOENERGY_P1_H
