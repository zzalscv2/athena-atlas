/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


//
// includes
//

#include <EventLoopTest/UnitTestAlg7.h>

#include <TH1.h>

//
// method implementations
//

namespace EL
{
  UnitTestAlg7 ::
  UnitTestAlg7 (const std::string& name,
                ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
  {
  }



  ::StatusCode UnitTestAlg7 ::
  initialize ()
  {
    ANA_CHECK (book (TH1F ("dummy_hist", "dummy_hist", 50, 0, 50)));
    return ::StatusCode::SUCCESS;
  }



  ::StatusCode UnitTestAlg7 ::
  execute ()
  {
    return ::StatusCode::SUCCESS;
  }



  ::StatusCode UnitTestAlg7 ::
  finalize ()
  {
    return ::StatusCode::SUCCESS;
  }
}
