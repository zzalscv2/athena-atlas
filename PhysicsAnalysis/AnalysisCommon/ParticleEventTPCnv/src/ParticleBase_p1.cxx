///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ParticleBase_p1.cxx 
// Implementation file for class ParticleBase_p1
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// ParticleEventTPCnv includes
#include "ParticleEventTPCnv/ParticleBase_p1.h"

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////

// Need to initialize these to handle the case where an instance of this
// is added to another _p class.
ParticleBase_p1::ParticleBase_p1()
  : m_charge (0),
    m_hasCharge (false),
    m_pdgId (0),
    m_hasPdgId (false),
    m_dataType (0)
{
}
