/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// IParticleAssocFilter.cxx 
// Implementation file for class IParticleAssocFilter
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes

// AnalysisAssociation includes
#include "AnalysisAssociation/IParticleAssocFilter.h"

/////////////////////////////////////////////////////////////////// 
/// Public methods: 
/////////////////////////////////////////////////////////////////// 

/// Constructors
////////////////
IParticleAssocFilter::IParticleAssocFilter() : 
  AssocFilter<IParticle,IParticle>()
{}

IParticleAssocFilter::IParticleAssocFilter( const IParticleAssocFilter& rhs ) :
  IAssocFilter<IParticle,IParticle>(rhs),
  AssocFilter<IParticle,IParticle>(rhs)
{}

IParticleAssocFilter& 
IParticleAssocFilter::operator=( const IParticleAssocFilter& rhs )
{
  if ( this != &rhs ) {
    AssocFilter<IParticle,IParticle>::operator=(rhs);
  }
  return *this;
}

/// Destructor
///////////////
IParticleAssocFilter::~IParticleAssocFilter() {}

