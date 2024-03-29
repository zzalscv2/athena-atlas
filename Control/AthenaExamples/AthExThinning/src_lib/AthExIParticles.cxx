///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// AthExIParticles.cxx 
// Implementation file for class AthExIParticles
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes

// AthExThinning includes
#include "AthExThinning/AthExIParticles.h"
// explicit instantiation
//template class DataVector<AthExIParticle>;

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
AthExIParticles::AthExIParticles() :
  DataVector<AthExIParticle>()
{}

AthExIParticles::AthExIParticles( const AthExIParticles& rhs ) :
  DataVector<AthExIParticle>( rhs )
{}

AthExIParticles& AthExIParticles::operator=( const AthExIParticles& rhs )
{
  if ( this != &rhs ) {
    DataVector<AthExIParticle>::operator=(rhs);
  }
  return *this;
}

AthExIParticles::AthExIParticles( const SG::OwnershipPolicy own ) :
  DataVector<AthExIParticle>( own )
{}

// Destructor
///////////////
AthExIParticles::~AthExIParticles()
{}
