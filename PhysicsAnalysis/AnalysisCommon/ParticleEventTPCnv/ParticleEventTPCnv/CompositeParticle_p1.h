///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// CompositeParticle_p1.h 
// Header file for class CompositeParticle_p1
// Author: S.Binet<binet@cern.ch>
// Date:   Apr 2008
/////////////////////////////////////////////////////////////////// 
#ifndef PARTICLEEVENTTPCNV_COMPOSITEPARTICLE_P1_H 
#define PARTICLEEVENTTPCNV_COMPOSITEPARTICLE_P1_H 

// DataModelAthenaPool includes
#include "DataModelAthenaPool/Navigable_p1.h"
#include "DataModelAthenaPool/AthenaBarCode_p1.h"

// EventCommonTPCnv includes
#include "EventCommonTPCnv/P4PxPyPzE_p1.h"

// ParticleEventTPCnv includes
#include "ParticleEventTPCnv/ParticleBase_p1.h"

// forward declarations
class CompositeParticleCnv_p1;

class CompositeParticle_p1 
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class CompositeParticleCnv_p1;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  CompositeParticle_p1();

  /** Destructor: 
   */
  ~CompositeParticle_p1() = default;

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 

  /// the navigable part 
  Navigable_p1<uint32_t, double> m_nav;

  /// the 4-mom part
  P4PxPyPzE_p1 m_momentum;

  /// the iparticle-part
  ParticleBase_p1 m_particle;
  
}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline CompositeParticle_p1::CompositeParticle_p1()
{}

#endif //> PARTICLEEVENTTPCNV_COMPOSITEPARTICLE_P1_H
