///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// PerfMonTestMallocAlg.cxx 
// Implementation file for class PerfMonTest::MallocAlg
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes
#include <vector>

// FrameWork includes
#include "Gaudi/Property.h"

// CLHEP includes
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Random/RandGauss.h"

// PerfMonTests includes
#include "PerfMonTestMallocAlg.h"

using namespace PerfMonTest;

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
MallocAlg::MallocAlg( const std::string& name, 
                      ISvcLocator* pSvcLocator ) : 
  AthAlgorithm( name,    pSvcLocator )
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

  declareProperty( "EvtNbr",
                   m_evtNbr = 10,
                   "event number at which to actually do stuff" );

  declareProperty( "UseStdVector",
                   m_useStdVector = false,
                   "switch between using a C-array and a std::vector");
}

// Destructor
///////////////
MallocAlg::~MallocAlg()
{ 
  ATH_MSG_DEBUG ( "Calling destructor" ) ;
}

// Athena Algorithm's Hooks
////////////////////////////
StatusCode MallocAlg::initialize()
{
  ATH_MSG_INFO ( "Initializing " << name() << "..." ) ;
  
  return StatusCode::SUCCESS;
}

StatusCode MallocAlg::finalize()
{
  ATH_MSG_INFO ( "Finalizing " << name() << "..." ) ;

  return StatusCode::SUCCESS;
}

StatusCode MallocAlg::execute()
{  
  ATH_MSG_DEBUG ( "Executing " << name() << "..." ) ;

  m_currentEvtNbr++;

  //typedef int ElemType;
  typedef float ElemType;

  if ( m_currentEvtNbr >= m_evtNbr ) {
    const std::size_t nmax = 1024*1024;

    if (m_useStdVector) {
      std::vector<ElemType> c1_array(nmax);
      for ( std::size_t i = 0; i!=nmax; ++i ) {
        c1_array[i] =  i;
      }
      
      if ( m_currentEvtNbr >= 2*m_evtNbr ) {
        std::vector<ElemType> c2_array(nmax);
        for ( std::size_t i = 0; i!=nmax; ++i ) {
          c2_array[i] =  -1*static_cast<int>(i);
        }
      }
      
    } else {
      ElemType c1_array[nmax];
      for ( std::size_t i = 0; i!=nmax; ++i ) {
        c1_array[i] =  i;
      }
      isUsed (&c1_array);

      if ( m_currentEvtNbr >= 2*m_evtNbr ) {
        ElemType c2_array[nmax];
        for ( std::size_t i = 0; i!=nmax; ++i ) {
          c2_array[i] =  -1*static_cast<int>(i);
        }
        // dummy stuff to silence gcc-warning.
        if (c2_array[0] > c1_array[0]) { 
          c2_array[0] = c1_array[0];
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}
