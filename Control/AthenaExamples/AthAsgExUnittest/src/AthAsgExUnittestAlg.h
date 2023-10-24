// -*- mode: c++ -*-
//
//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//

#ifndef ATHASGEXUNITTEST_ATHASGEXUNITTESTALG_H
#define ATHASGEXUNITTEST_ATHASGEXUNITTESTALG_H 1

#include "AthAnalysisBaseComps/AthAnalysisAlgorithm.h"

#ifdef XAOD_ANALYSIS
#include "AsgTools/AnaToolHandle.h" //use asg::AnaToolHandle instead of regular ToolHandles for full dual-use experience!
#endif

#include "AthAsgExUnittest/IAthAsgExUnittestTool.h"


class AthAsgExUnittestAlg: public ::AthAnalysisAlgorithm {
public:

  AthAsgExUnittestAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~AthAsgExUnittestAlg();

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:

  int m_property;
  ToolHandle<IAthAsgExUnittestTool> m_tool;

};

#endif //> !ATHASGEXUNITTEST_ATHASGEXUNITTESTALG_H
