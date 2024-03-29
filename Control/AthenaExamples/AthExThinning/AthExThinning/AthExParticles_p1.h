///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// AthExParticles_p1.h 
// Header file for class AthExParticles_p1
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHEXTHINNING_AthExPARTICLES_P1_H 
#define ATHEXTHINNING_AthExPARTICLES_P1_H 

// STL includes
#include <vector>

// Forward declaration
class AthExParticlesCnv_p1;

class AthExParticle_p1
{
public:
 AthExParticle_p1( double px = 0., double py  = 0., 
		   double pz = 0., double ene = 0. ) :
    m_px ( px ),
    m_py ( py ),
    m_pz ( pz ),
    m_ene( ene )
  {}

  double m_px;
  double m_py;
  double m_pz;
  double m_ene;
};

class AthExParticles_p1
{ 

  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class AthExParticlesCnv_p1;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /** Default constructor: 
   */
  AthExParticles_p1();

  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

  std::vector< AthExParticle_p1 > m_particles;
}; 
#endif //> ATHEXTHINNING_AthExPARTICLES_P1_H
