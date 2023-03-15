/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGENERICALGS_TimeBurner_h
#define TRIGGENERICALGS_TimeBurner_h

#include "DecisionHandling/HypoBase.h"
#include "GaudiKernel/IAlgTool.h"

/**
 *  @class TimeBurner
 *  @brief Reject-all hypo algorithm sleeping for some time and doing nothing else
 **/
class TimeBurner : public HypoBase {
  public:
    /// Standard constructor
    TimeBurner(const std::string& name, ISvcLocator* svcLoc);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& eventContext) const override;

  private:
    Gaudi::Property<unsigned int> m_sleepTimeMillisec {
      this, "SleepTimeMillisec", 0, "Time to sleep in each execution [ms]"
    };

    // Unused dummy property to pass duck-test of HypoAlgs
    ToolHandleArray<IAlgTool> m_hypoTools{this, "HypoTools", {}};
};

#endif // TRIGGENERICALGS_TimeBurner_h
