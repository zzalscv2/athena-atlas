/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENT_LOOP_UNIT_TEST_ALG7_H
#define EVENT_LOOP_UNIT_TEST_ALG7_H

#include <EventLoopTest/Global.h>

#include <AnaAlgorithm/AnaAlgorithm.h>

namespace EL
{
  /// \brief a \ref AnaAlgorithm for testing the configuration on the
  /// worker node

  class UnitTestAlg7 final : public AnaAlgorithm
  {
    //
    // public interface
    //

  public:
    UnitTestAlg7 (const std::string& name,
                  ISvcLocator* pSvcLocator);



    //
    // interface inherited from Algorithm
    //

  private:
    virtual ::StatusCode initialize () override;

  private:
    virtual ::StatusCode execute () override;

  private:
    virtual ::StatusCode finalize () override;



    //
    // private interface
    //
  };
}

#endif
