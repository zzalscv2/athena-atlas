///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// Neutrino_p1.h 
// Header file for class Neutrino_p1
// Author: K.Cranmer<cranmer@cern.ch>
// Author: S.Binet<binet@cern.ch>
// Date:   December 2006
/////////////////////////////////////////////////////////////////// 
#ifndef PARTICLEEVENTTPCNV_NEUTRINO_P1_H 
#define PARTICLEEVENTTPCNV_NEUTRINO_P1_H 1

// EventCommonTPCnv includes
#include "EventCommonTPCnv/P4PxPyPzE_p1.h"

// ParticleEventTPCnv includes
#include "ParticleEventTPCnv/ParticleBase_p1.h"

// forward declarations
class NeutrinoCnv_p1;

class Neutrino_p1 
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class NeutrinoCnv_p1;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  Neutrino_p1();

  /** Destructor: 
   */
  ~Neutrino_p1() = default;

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 

  /// the 4-mom part
  P4PxPyPzE_p1 m_momentum;

  /// the ParticleBase part
  ParticleBase_p1 m_particleBase;

  // the Neutrino part 

}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline Neutrino_p1::Neutrino_p1()
{}

#endif //> PARTICLEEVENTTPCNV_NEUTRINO_P1_H
