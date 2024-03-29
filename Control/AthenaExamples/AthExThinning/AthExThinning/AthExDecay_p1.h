///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// AthExDecay_p1.h 
// Header file for class AthExDecay_p1
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHEXTHINNING_AthExDECAY_P1_H 
#define ATHEXTHINNING_AthExDECAY_P1_H 

// STL includes
#include <vector>

// Forward declaration
class AthExDecayCnv_p1;

#include "DataModelAthenaPool/ElementLink_p1.h"
#include "AthExThinning/AthExParticles_p1.h"

class AthExDecay_p1
{ 

  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class AthExDecayCnv_p1;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /** Default constructor: 
   */
  AthExDecay_p1();

  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

  ElementLinkInt_p1 m_p1;
  ElementLinkInt_p1 m_p2;
  ElementLinkInt_p1 m_l1;
  ElementLinkInt_p1 m_l2;
}; 

#endif //> ATHEXTHINNING_AthExDECAY_P1_H
