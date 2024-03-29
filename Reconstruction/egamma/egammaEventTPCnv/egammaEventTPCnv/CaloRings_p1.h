///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// CaloRings_p1.h 
// Header file for class CaloRings_p1
// Author: D.E.Ferreira de Lima<dferreir@mail.cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef RECTPCNV_CALORINGS_P1_H 
#define RECTPCNV_CALORINGS_P1_H 

// STL includes
#include <vector>

// DataModelAthenaPool includes
#include "DataModelAthenaPool/ElementLink_p3.h"
#include "DataModelAthenaPool/ElementLinkVector_p1.h"

// forward declarations
class CaloRingsCnv_p1;

class CaloRings_p1 
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class CaloRingsCnv_p1;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  CaloRings_p1();

  /** Destructor: 
   */
  ~CaloRings_p1();

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 

  /// Ring information
  std::vector<float>  m_rings;
  std::vector<unsigned int> m_nRings;

  std::vector<int> m_layers; // To be converted to CaloCell_ID::CaloSample
  std::vector<unsigned int> m_nLayers;

  std::vector<float> m_deltaEta;
  std::vector<float> m_deltaPhi;
}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline CaloRings_p1::CaloRings_p1()
{}

#endif //> RECTPCNV_EGAMMA_P1_H
