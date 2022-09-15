/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGENERICALGS_ROBPrefetchingAlg_h
#define TRIGGENERICALGS_ROBPrefetchingAlg_h

// Trigger includes
#include "TrigCompositeUtils/TrigCompositeUtils.h"

// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "IRegionSelector/IRegSelTool.h"
#include "StoreGate/ReadHandleKeyArray.h"

// System includes
#include <string>
#include <vector>
#include <unordered_set>

/**
 *  @class ROBPrefetchingAlg
 *  @brief Algorithm taking a list of decision objects, extracting RoI from each and prefetching ROBs
 *  (i.e. calling ROBDataProviderSvc::addROBData) for all these RoIs from specific detectors configured
 *  in the @c RegionSelectorTools property.
 **/
class ROBPrefetchingAlg : public AthReentrantAlgorithm {
public:
  ROBPrefetchingAlg(const std::string& name, ISvcLocator* svcLoc);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& eventContext) const override;

private:
  /// Array of input decisions from which RoIs will be extracted
  SG::ReadHandleKeyArray<TrigCompositeUtils::DecisionContainer> m_inputDecisions {
    this, "ROBPrefetchingInputDecisions", {}, "Input Decisions"};

  /// Array of RegionSelector tools for RoI->ROBs mapping, one for each detector to be prefetched
  ToolHandleArray<IRegSelTool> m_regionSelectorTools {
    this, "RegionSelectorTools", {}, "Region Selector tools"};

  /// The ROB data provider service used to prefetch the ROBs
  ServiceHandle<IROBDataProviderSvc> m_robDataProviderSvc {
    this, "ROBDataProviderSvc", "ROBDataProviderSvc", "Name of the ROB data provider"};

  /// Choose the name of the RoI link to follow
  Gaudi::Property<std::string> m_roiLinkName {
    this, "RoILinkName", TrigCompositeUtils::roiString(), "The string identifying links from Decisions to RoIs"};

  /// ChainFilter to select decisions for specific active chains
  Gaudi::Property<std::vector<TrigCompositeUtils::DecisionID>> m_chainFilterVec {
    this, "ChainFilter", {}, "Apply prefetching only to RoIs from Decisions where chains from this list are active. Empty list means no filtering."};

  /// ChainFilter as unordered_set for faster lookup
  std::unordered_set<TrigCompositeUtils::DecisionID> m_chainFilter;
};

#endif // TRIGGENERICALGS_ROBPrefetchingAlg_h
