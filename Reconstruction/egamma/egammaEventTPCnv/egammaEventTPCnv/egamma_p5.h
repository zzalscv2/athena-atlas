///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// egamma_p5.h 
// Header file for class egamma_p5
// New cnv to include Ringer: W.S.Freund<wsfreund@mail.cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef EGAMMAEVENTTPCNV_EGAMMA_P5_H 
#define EGAMMAEVENTTPCNV_EGAMMA_P5_H 1

// STL includes
#include <vector>

// DataModelAthenaPool includes
#include "DataModelAthenaPool/ElementLink_p3.h"
#include "DataModelAthenaPool/ElementLinkVector_p1.h"

// EventCommonTPCnv includes
#include "EventCommonTPCnv/P4EEtaPhiMFloat_p2.h"

// ParticleEventTPCnv includes
#include "ParticleEventTPCnv/ParticleBase_p2.h"

// forward declarations
class egammaCnv_p5;

class egamma_p5 
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class egammaCnv_p5;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  egamma_p5();

  /** Destructor: 
   */
  ~egamma_p5();

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 

  /// the 4-mom part
  P4EEtaPhiMFloat_p2 m_momentum;

  /// the 4-mom part for the Cluster
  P4EEtaPhiMFloat_p2 m_momentumCluster;
                                                              
  /// the ParticleBase part
  ParticleBase_p2 m_particleBase;

  // the egamma part 

  /// links to clusters
  ElementLinkInt_p3 m_cluster;
  
  /// links to tracks
  ElementLinkIntVector_p1 m_trackParticle;
  // std::vector<ElementLinkInt_p3> m_trackParticle;

  /// links to tracks
  ElementLinkIntVector_p1 m_conversion;
  // std::vector<ElementLinkInt_p3> m_conversion;

  /// links to egDetails
  ElementLinkIntVector_p1 m_egDetails;
  // std::vector<ElementLinkInt_p3> m_egDetails;

  /// links to rings          
  ElementLinkInt_p3 m_rings;  

  // authors
  unsigned int m_author;

  /// egamma PID
  std::vector<unsigned int> m_egammaEnumPIDs;
  std::vector<double> m_egammaDblPIDs;
}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline egamma_p5::egamma_p5()
  : m_author(0)
{}

#endif //> EGAMMAEVENTTPCNV_EGAMMA_P5_H
