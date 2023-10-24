// -*- mode: c++ -*-
//
//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHASGEXUNITTEST_ATHEXUNITTESTTOOL_H
#define ATHASGEXUNITTEST_ATHEXUNITTESTTOOL_H 1

#include "AsgTools/AsgTool.h"
#include "AthAsgExUnittest/IAthAsgExUnittestTool.h"

class AthAsgExUnittestTool: public asg::AsgTool, public virtual IAthAsgExUnittestTool {
public:

  ASG_TOOL_CLASS( AthAsgExUnittestTool, IAthAsgExUnittestTool )
  // Add another constructor for non-athena use cases
  AthAsgExUnittestTool( const std::string& name );

  // Initialize is required by AsgTool base class
  virtual StatusCode initialize() override;

  // This tools method
  virtual double useTheProperty() override;

private:

  double m_nProperty;
  unsigned int m_enumProperty;

};

#endif //> !ATHASGEXUNITTEST_ATHEXUNITTESTTOOL_H
