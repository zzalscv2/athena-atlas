///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ITruthParticleVisitor.cxx 
// Implementation file for class ITruthParticleVisitor
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes

// McParticleKernel includes
#include "McParticleKernel/ITruthParticleVisitor.h"

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////

ITruthParticleVisitor::ITruthParticleVisitor()
{}

ITruthParticleVisitor::ITruthParticleVisitor( const ITruthParticleVisitor& )
{}

ITruthParticleVisitor&
ITruthParticleVisitor::operator=( const ITruthParticleVisitor& rhs )
{
  if ( this != &rhs ) {
  }
  return *this;
}

// Destructor
///////////////

ITruthParticleVisitor::~ITruthParticleVisitor()
{}
