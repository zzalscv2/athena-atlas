///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Jet_p5.h
// Header file for class Jet_p5
// Author: R.Seuster<seuster@cern.ch>
// Date:   September 2008
/////////////////////////////////////////////////////////////////// 
#ifndef JETEVENTTPCNV_JET_P5_H 
#define JETEVENTTPCNV_JET_P5_H 

// STL includes
#include <vector>

// DataModelAthenaPool includes
#include "DataModelAthenaPool/Navigable_p1.h"

#include "AthenaPoolUtilities/TPObjRef.h"

#include "ParticleEventTPCnv/ParticleBase_p1.h"

#include "JetEventTPCnv/JetConverterBase.h"

class JetTagInfoBase;
class JetAssociationBase;

class Jet_p5
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 
  
  // Make the templated JetConverterBase class our friend
  template <class PERS>
  friend class JetConverterBase;
  
  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 
  
  /** Default constructor: 
   */
  Jet_p5() = default;
  
  /** Destructor:
   */
  ~Jet_p5() = default;
  
  // copy and move constructor defaulted
  Jet_p5(const Jet_p5& other) noexcept = default;
  Jet_p5(Jet_p5&& other) noexcept = default;

  // copy and move assignment defaulted
  Jet_p5 & operator=(const Jet_p5 &) noexcept = default;
  Jet_p5 & operator=(Jet_p5 &&) noexcept = default;

  ///////////////////////////////////////////////////////////////////
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 
  
  /// the navigable part 
  Navigable_p1<uint32_t, double> m_nav;
  
  /// the 4-mom part
  // P4PxPyPzE_p1 m_momentum;
  // we'll store the 4 momenta as floats. Additionally, we'll store both signal states
  // calibrated and uncalibrated, if available. Store in own struct "mom" with 4 floats
  //  float px, py, pz, m;
  //  std::vector<mom> m_momentum;
  JetConverterTypes::momentum            m_momentum;
  JetConverterTypes::signalState_pers_t  m_rawSignal;
  
  /// The Particle base stuff -- since Jet now inherits from particle base.
  ParticleBase_p1 m_partBase;
  
  /// storing what jet algorithm the jet belongs to
  unsigned int m_author{0};
  
  /// combined likelihoods
  unsigned int m_num_combinedLikelihood{0};
  
  /// JetMomentStore
  std::vector<float> m_shapeStore;
  
  /// JetTagInfoBase objects
  std::vector<TPObjRef> m_tagJetInfo;
  
  /// JetAssociationBase objects
  std::vector<TPObjRef> m_associations;
  
  /** we store the recoStatus for jets here
   */
  unsigned int m_recoStatus{0};

  bool         m_usedForTrigger{false};

  unsigned int  m_constituentsN{0};
  
};

#endif //> JETEVENTTPCNV_JET_P5_H
