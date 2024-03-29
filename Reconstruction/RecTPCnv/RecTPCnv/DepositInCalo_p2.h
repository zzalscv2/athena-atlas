///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// DepositInCalo_p2.h 
// Header file for class DepositInCalo_p2
// Author: Ketevi A. Assamagan
// Date:   January 2009
/////////////////////////////////////////////////////////////////// 
#ifndef RECTPCNV_DEPOSITINCALO_P2_H 
#define RECTPCNV_DEPOSITINCALO_P2_H 

// STL includes

// forward declarations
class DepositInCaloCnv_p2;
class DepositInCalo_p1;

class DepositInCalo_p2 
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class DepositInCaloCnv_p2;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  DepositInCalo_p2()
    : m_subCaloId(0),
      m_energyDeposited(0),
      m_muonEnergyLoss(0),
      m_etDeposited(0)
  {}

  /// Allow conversion from DepositInCalo_p1.
  DepositInCalo_p2 (const DepositInCalo_p1& other);

  /** Destructor: 
   */
  ~DepositInCalo_p2() = default;

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 

  /** calorimeter sampling identifier */
  unsigned short m_subCaloId;

  /** energy Desposited */
  float m_energyDeposited;

  /** energy Loss of the muons computed using the extrapolator */
  float m_muonEnergyLoss;

  /** energy deposited Et */
  float m_etDeposited;
}; 

#endif //> RECTPCNV_DEPOSITINCALO_P1_H
