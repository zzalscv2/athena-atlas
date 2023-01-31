///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// AthExElephantino_p1.h 
// Header file for class AthExElephantino_p1
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHEXTHINNING_AthExELEPHANTINO_P1_H 
#define ATHEXTHINNING_AthExELEPHANTINO_P1_H 


// Forward declaration
class AthExElephantinoCnv_p1;

// DataModel includes
#include "DataModelAthenaPool/ElementLinkVector_p1.h"

class AthExElephantino_p1
{ 

  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class AthExElephantinoCnv_p1;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /** Default constructor: 
   */
  AthExElephantino_p1();

  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

  ElementLinkIntVector_p1 m_legs;
  ElementLinkIntVector_p1 m_ears;
}; 

#endif //> ATHEXTHINNING_AthExELEPHANTINO_P1_H
