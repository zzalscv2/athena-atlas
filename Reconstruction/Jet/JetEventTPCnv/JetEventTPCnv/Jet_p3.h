///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Jet_p2.h 
// Header file for class Jet_p2
// Author: S.Binet<binet@cern.ch>
// Author: R.Seuster<seuster@cern.ch>
// Date:   March 2007
// Date:   Feb 2008
/////////////////////////////////////////////////////////////////// 
#ifndef JETEVENTTPCNV_JET_P3_H 
#define JETEVENTTPCNV_JET_P3_H 

// STL includes
#include <vector>

// DataModelAthenaPool includes
#include "DataModelAthenaPool/Navigable_p1.h"

// EventCommonTPCnv includes
#include "EventCommonTPCnv/P4PxPyPzE_p1.h"

// forward declarations
class JetTagInfoBase;
class JetAssociationBase;

class JetCnv_p3;

class Jet_p3 
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class JetCnv_p3;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  Jet_p3() = default;

  /** Destructor:
   */
  ~Jet_p3();
  
  // copy and move constructor defaulted
  Jet_p3(const Jet_p3& other) noexcept = default;
  Jet_p3(Jet_p3&& other) noexcept = default;

  // copy and move assignment defaulted
  Jet_p3 & operator=(const Jet_p3 &) noexcept = default;
  Jet_p3 & operator=(Jet_p3 &&) noexcept = default;

  ///////////////////////////////////////////////////////////////////
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 

  /// the navigable part 
  Navigable_p1<uint32_t, double> m_nav;

  /// the 4-mom part
  P4PxPyPzE_p1 m_momentum;
  
  unsigned int m_author{0};
  
  /// combined likelihoods
  unsigned int m_num_combinedLikelihood{0};

  /// JetMomentStore
  std::vector<float> m_shapeStore;
  
  /// JetTagInfoBase objects
  std::vector<const JetTagInfoBase*> m_tagJetInfo;
  
  /// JetAssociationBase objects
  std::vector<const JetAssociationBase*> m_associations;

  /// True if we own the objects pointed to from the above collections.
  // This should be true for the case of reading, but false for writing.
  // Therefore, it is set to true in the default constructor.
  // When the T->P converter puts pointers in one of these objects,
  // it should also clear this flag.
  bool m_ownPointers{true};

  
  // Signal states 
  char  m_ratioE{0};  /**ratio for uncalibrated  E    */
  char  m_ratioPx{0}; /**ratio for uncalibrated  p_x  */
  char  m_ratioPy{0}; /**ratio for uncalibrated  p_y  */
  char  m_ratioPz{0}; /**ratio for uncalibrated  p_z  */

}; 

#endif //> JETEVENTTPCNV_JET_P3_H
