///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Jet_p6.h
// Header file for class Jet_p6
// Author: P.A Delsart
// Date:   December 2009
/////////////////////////////////////////////////////////////////// 
#ifndef JETEVENTTPCNV_JET_P6_H 
#define JETEVENTTPCNV_JET_P6_H 

// STL includes
#include <vector>

// DataModelAthenaPool includes
#include "DataModelAthenaPool/Navigable_p2.h"

#include "AthenaPoolUtilities/TPObjRef.h"

#include "DataModelAthenaPool/ElementLink_p3.h"
#include "DataModelAthenaPool/AthenaBarCode_p1.h"

// DataModelAthenaPool
#include "DataModelAthenaPool/DataLink_p2.h"

#include "JetEventTPCnv/JetConverterBase.h"

class JetAssociationBase;

class Jet_p6
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
  Jet_p6() = default;
  
  /** Destructor: 
   */
  ~Jet_p6() = default;
  
  // copy and move constructor defaulted
  Jet_p6(const Jet_p6& other) noexcept = default;
  Jet_p6(Jet_p6&& other) noexcept = default;

  // copy and move assignment defaulted
  Jet_p6 & operator=(const Jet_p6 &) noexcept = default;
  Jet_p6 & operator=(Jet_p6 &&) noexcept = default;

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 
  
  /// the navigable part 
  Navigable_p2<uint32_t, float> m_nav;
  
  /// the 4-mom part
  // P4PxPyPzE_p1 m_momentum;
  // we'll store the 4 momenta as floats. Additionally, we'll store both signal states
  // calibrated and uncalibrated, if available. Store in own struct "mom" with 4 floats
  //  float px, py, pz, m;
  //  std::vector<mom> m_momentum;
  // JetConverterTypes::momentum            m_momentum;
  // JetConverterTypes::signalState_pers_t  m_rawSignal;
  
  // We use a fixed array to store the 3x4-momentum for efficiency reasons
  // in the form px,py,pz,e and in the order FINAL, EMSCALE, CONSTITSCALE
  float m_momentum[12] = {0};
  
  

  // The Particle base stuff -- since Jet now inherits from particle base.
  // ParticleBase_p1 m_partBase;
  // replace the above by what's only usefull for jets :
  int m_dataType{0};
  /// link to the particle's origin (vertex)
  ElementLinkInt_p3 m_origin;



  /** we store the recoStatus for jets here
   */
  int m_constituentSigState{0};
  unsigned int m_RoIword{0};

  unsigned int  m_constituentsN{0};

  /// storing what jet algorithm the jet belongs to
  unsigned int m_author{0};

  
  

  short m_jetId{0}; // WARNING the transiant part is a unsigned int. However this index jets.
                    // indexing moments for more than 32768 jets doesn't make sens.
  
  /// JetTagInfoBase objects
  std::vector<TPObjRef> m_tagJetInfo;
  
  /// JetAssociationBase objects
  std::vector<TPObjRef> m_associations;
  
  
  
};

#endif //> JETEVENTTPCNV_JET_P6_H
