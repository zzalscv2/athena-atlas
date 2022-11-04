///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Jet_p4.h 
// Header file for class Jet_p4
// Author: S.Binet<binet@cern.ch>
// Author: R.Seuster<seuster@cern.ch>
// Date:   March 2007
// Date:   Feb 2008
/////////////////////////////////////////////////////////////////// 
#ifndef JETEVENTTPCNV_JET_P4_H 
#define JETEVENTTPCNV_JET_P4_H 

// STL includes
#include <vector>

// DataModelAthenaPool includes
#include "DataModelAthenaPool/Navigable_p1.h"

// EventCommonTPCnv includes
#include "EventCommonTPCnv/P4PxPyPzE_p1.h"

#include "AthenaPoolUtilities/TPObjRef.h"

#include "ParticleEventTPCnv/ParticleBase_p1.h"

// forward declarations
class JetTagInfoBase;
class JetAssociationBase;

class JetCnv_p4;

class Jet_p4 
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class JetCnv_p4;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  Jet_p4() = default;

  /** Destructor:
   */
  ~Jet_p4() = default;
  
  // copy and move constructor defaulted
  Jet_p4(const Jet_p4& other) noexcept = default;
  Jet_p4(Jet_p4&& other) noexcept = default;

  // copy and move assignment defaulted
  Jet_p4 & operator=(const Jet_p4 &) noexcept = default;
  Jet_p4 & operator=(Jet_p4 &&) noexcept = default;

  ///////////////////////////////////////////////////////////////////
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 

  /// the navigable part 
  Navigable_p1<uint32_t, double> m_nav;

  /// the 4-mom part
  P4PxPyPzE_p1 m_momentum;
  
  /// The Particle base stuff -- since Jet now inherrits from particle base.
  ParticleBase_p1 m_partBase;

  unsigned int m_author{0};
  
  /// combined likelihoods
  unsigned int m_num_combinedLikelihood{0};

  /// JetMomentStore
  std::vector<float> m_shapeStore;
  
  /// JetTagInfoBase objects
  std::vector<TPObjRef> m_tagJetInfo;
  
  /// JetAssociationBase objects
  std::vector<TPObjRef> m_associations;

  // Signal states 
  char  m_ratioE{};  /**ratio for uncalibrated  E    */
  char  m_ratioPx{}; /**ratio for uncalibrated  p_x  */
  char  m_ratioPy{}; /**ratio for uncalibrated  p_y  */
  char  m_ratioPz{}; /**ratio for uncalibrated  p_z  */
}; 

#endif //> JETEVENTTPCNV_JET_P4_H
