/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VALGRINDRTTJOBS_VALGRINDHELPERALG_H
#define VALGRINDRTTJOBS_VALGRINDHELPERALG_H

// Include files
#include "AthenaBaseComps/AthAlgorithm.h"

// #include "valgrind/valgrind.h"

#include <sys/types.h>
#include <unistd.h>

class ValgrindHelperAlg : public AthAlgorithm 
{
 public:
  
  ValgrindHelperAlg( const std::string& name, ISvcLocator* svcloc )
    : AthAlgorithm( name, svcloc )
    {
      // VALGRIND_PRINTF("%s", "in constructor...");
    };

  virtual ~ValgrindHelperAlg( )
    {
      // VALGRIND_PRINTF("%s", "in destructor...");
    };

  virtual StatusCode initialize( )
  {
    ATH_MSG_INFO ("ValgrindHelperAlg test: " << getpid() );
    // VALGRIND_PRINTF("%s", "in initialize...");
    return StatusCode::SUCCESS;
  };

  virtual StatusCode execute( )
  {
    // VALGRIND_PRINTF("%s", "in execute...");
    return StatusCode::SUCCESS;
  };

  virtual StatusCode finalize( )
  {
    // VALGRIND_PRINTF("%s", "in finalize...");
    return StatusCode::SUCCESS;
  };

};
#endif
