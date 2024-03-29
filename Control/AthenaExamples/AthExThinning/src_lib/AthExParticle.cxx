///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// AthExParticle.cxx 
// Implementation file for class AthExParticle
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes

// AthExThinning includes
#include "AthExThinning/AthExParticle.h"

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////

AthExParticle::AthExParticle( double px, double py, double pz, double ene ) :
  AthExIParticle( ),
  m_px ( px ),
  m_py ( py ),
  m_pz ( pz ),
  m_ene( ene )
{}

AthExParticle::AthExParticle( const AthExParticle& rhs ) :
  AthExIParticle( rhs ),
  m_px ( rhs.m_px ),
  m_py ( rhs.m_py ),
  m_pz ( rhs.m_pz ),
  m_ene( rhs.m_ene )
{}

AthExParticle& AthExParticle::operator=( const AthExParticle& rhs )
{
  if ( this != &rhs ) {
    AthExIParticle::operator=( rhs );
    m_px  = rhs.m_px;
    m_py  = rhs.m_py;
    m_pz  = rhs.m_pz;
    m_ene = rhs.m_ene;
  }
  return *this;
}

// Destructor
///////////////
AthExParticle::~AthExParticle()
{}
