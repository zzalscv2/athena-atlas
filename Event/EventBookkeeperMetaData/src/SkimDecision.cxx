/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// Implementation file for class SkimDecision
// Author: David Cote, September 2008. <david.cote@cern.ch>
/////////////////////////////////////////////////////////////////// 
 
#include "EventBookkeeperMetaData/SkimDecision.h"

////////////////
/// Constructors
////////////////

SkimDecision::SkimDecision()
  : m_isAccepted (true)
{ 
}

void SkimDecision::setName( const std::string& name ){ m_name=name; }
void SkimDecision::setIsAccepted( bool answer ){ m_isAccepted=answer; }
