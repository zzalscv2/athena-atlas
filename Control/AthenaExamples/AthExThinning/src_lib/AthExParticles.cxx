///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// AthExParticles.cxx 
// Implementation file for class AthExParticles
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes

// AthExThinning includes
#include "AthExThinning/AthExParticles.h"

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
AthExParticles::AthExParticles() :
  DataVector<AthExParticle>()
{}

AthExParticles::AthExParticles( const AthExParticles& rhs ) :
  DataVector<AthExParticle>( rhs )
{}

AthExParticles& AthExParticles::operator=( const AthExParticles& rhs )
{
  if ( this != &rhs ) {
    DataVector<AthExParticle>::operator=(rhs);
  }
  return *this;
}

AthExParticles::AthExParticles( const SG::OwnershipPolicy own ) :
  DataVector<AthExParticle>( own )
{}

// Destructor
///////////////
AthExParticles::~AthExParticles()
{}
