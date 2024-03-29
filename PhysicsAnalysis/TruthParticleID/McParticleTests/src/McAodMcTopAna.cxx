///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// McTopAna.cxx 
// Implementation file for class McTopAna
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// McParticleTests includes
#include "McAodMcTopAna.h"

// STL includes

// FrameWork includes
#include "Gaudi/Property.h"

namespace McAod {

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
McTopAna::McTopAna( const std::string& name, 
                    ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator )
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

}

// Destructor
///////////////
McTopAna::~McTopAna()
{}

// Athena Algorithm's Hooks
////////////////////////////
StatusCode McTopAna::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode McTopAna::finalize()
{
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode McTopAna::execute()
{  
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  return StatusCode::SUCCESS;
}


} //> end namespace McAod
