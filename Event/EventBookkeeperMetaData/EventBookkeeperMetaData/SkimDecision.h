/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// Header file for class SkimDecision
// Author: David Cote, September 2008. <david.cote@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef SKIMDECISION_H 
#define SKIMDECISION_H 
 
#include "AthenaKernel/CLASS_DEF.h"
#include <string>

class SkimDecision
{
 public:
  // Default Constructor
  SkimDecision();

  //  Destructor
  ~SkimDecision();

  // get() and set() methods
  std::string getName() const { return m_name; }
  void setName( const std::string& name );

  bool isAccepted() const { return m_isAccepted; }
  void setIsAccepted( bool answer );

 private:
  std::string m_name;
  bool m_isAccepted;
};

 inline 
 SkimDecision::~SkimDecision()
 {}

//this is automatically generated by: 'clid -m SkimDecision'
CLASS_DEF( SkimDecision , 2337118 , 1 )

#endif //> SKIMDECISION_H
