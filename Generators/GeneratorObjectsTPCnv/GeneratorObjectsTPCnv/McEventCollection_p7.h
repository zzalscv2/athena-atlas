///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// McEventCollection_p7.h 
// Header file for class McEventCollection_p1
// Author: S.Binet<binet@cern.ch>
// Date:   October 2006
/////////////////////////////////////////////////////////////////// 
#ifndef GENERATOROBJECTSTPCNV_MCEVENTCOLLECTION_p7_H 
#define GENERATOROBJECTSTPCNV_MCEVENTCOLLECTION_p7_H 

// STL includes
#include <vector>

// GeneratorObjectsTPCnv includes
#include "GeneratorObjectsTPCnv/GenEvent_p7.h"
#include "GeneratorObjectsTPCnv/GenParticle_p7.h"
#include "GeneratorObjectsTPCnv/GenVertex_p7.h"

// Forward declaration
class McEventCollectionCnv_p7;

class McEventCollection_p7
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class McEventCollectionCnv_p7;
  
  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /** Default constructor: 
   */
  McEventCollection_p7();

  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

  /** The vector of persistent representation of GenEvents
   */
  std::vector<GenEvent_p7> m_genEvents;

  /** The vector of persistent representation of GenVertices
   */
  std::vector<GenVertex_p7> m_genVertices;

  /** The vector of persistent representation of GenParticles
   */
  std::vector<GenParticle_p7> m_genParticles;

};

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline McEventCollection_p7::McEventCollection_p7() :
  m_genEvents   ( ),
  m_genVertices ( ),
  m_genParticles( )
{}

#endif //> GENERATOROBJECTSTPCNV_MCEVENTCOLLECTION_p7_H
