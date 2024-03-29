///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// egDetailContainer_p1.h 
// Header file for class egDetailContainer_p1
// Author: K.Cranmer<cranmer@cern.ch>
// Author: S.Binet<binet@cern.ch>
// Date:   December 2006
/////////////////////////////////////////////////////////////////// 
#ifndef EGAMMAEVENTTPCNV_EGDETAIL_P1_H 
#define EGAMMAEVENTTPCNV_EGDETAIL_P1_H 

// STL includes
#include <vector>
#include <string>

// forward declarations
class egDetailCnv_p1;

class egDetail_p1 
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  //  friend class egDetailCnv_p1;
  friend class egDetailContainerCnv_p1;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  egDetail_p1();

  /** Destructor: 
   */
  ~egDetail_p1();

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 

  /// egDetail Parameters and name of class
  std::string m_className;
  std::vector<unsigned int> m_egDetailEnumParams;
  std::vector<float> m_egDetailFloatParams;
}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline egDetail_p1::egDetail_p1()
{}

#endif //> EGAMMAEVENTTPCNV_EGDETAIL_P1_H
