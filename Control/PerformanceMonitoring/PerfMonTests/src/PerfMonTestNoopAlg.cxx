///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// PerfMonTestNoopAlg.cxx 
// Implementation file for class PerfMonTest::NoopAlg
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes

// FrameWork includes
#include "Gaudi/Property.h"

// PerfMonTests includes
#include "PerfMonTestNoopAlg.h"

using namespace PerfMonTest;

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
NoopAlg::NoopAlg( const std::string& name, 
		  ISvcLocator* pSvcLocator ) : 
  AthAlgorithm( name,    pSvcLocator )
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

}

// Destructor
///////////////
NoopAlg::~NoopAlg()
{ 
  ATH_MSG_DEBUG ( "Calling destructor" ) ;
}

// Athena Algorithm's Hooks
////////////////////////////
StatusCode NoopAlg::initialize()
{
  ATH_MSG_INFO ( "Initializing " << name() << "..." ) ;

  // Get pointer to StoreGateSvc and cache it :
  ATH_CHECK( evtStore().retrieve() );
  return StatusCode::SUCCESS;
}

StatusCode NoopAlg::finalize()
{
  ATH_MSG_INFO ( "Finalizing " << name() << "..." ) ;

  return StatusCode::SUCCESS;
}

StatusCode NoopAlg::execute()
{  
  ATH_MSG_DEBUG ( "Executing " << name() << "..." ) ;

  return StatusCode::SUCCESS;
}
