/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef REGIONSELECTOR_REGSELTOOLTESTER_H
#define REGIONSELECTOR_REGSELTOOLTESTER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "IRegionSelector/IRegSelTool.h"

/**
 *  @class RegSelToolTester
 *  @brief Algorithm holding an array of RegionSelector tools which retrieves them in initialize()
 *  and calls them in execute() with dummy input RoIs to test their functionality
 **/
class RegSelToolTester : public AthReentrantAlgorithm {
public:
  RegSelToolTester(const std::string& name, ISvcLocator* svcLoc);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& eventContext) const override;

private:
  ToolHandleArray<IRegSelTool> m_regionSelectorTools {
    this, "RegionSelectorTools", {}, "Region Selector tools to configure and call"};
};

#endif // REGIONSELECTOR_REGSELTOOLTESTER_H
