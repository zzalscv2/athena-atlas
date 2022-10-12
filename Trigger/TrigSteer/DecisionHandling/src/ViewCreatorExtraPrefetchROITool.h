/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DECISIONHANDLING_VIEWCREATOREXTRAPREFETCHROITOOL_H
#define DECISIONHANDLING_VIEWCREATOREXTRAPREFETCHROITOOL_H

#include "DecisionHandling/IViewCreatorROITool.h"
#include "HLTSeeding/IRoiUpdaterTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

/**
 * @class ViewCreatorExtraPrefetchROITool
 * RoI provider wrapper tool which calls another RoI provider tool first, and then uses its output RoI
 * to create a new RoI using the RoiUpdaterTool. This new RoI is saved in a different output container
 * and can be used to prefetch ROBs instead of the original one. This solution was invented to prefetch
 * ROBs for the second Tau reco step already during the first Tau reco step, see ATR-26419.
 **/
class ViewCreatorExtraPrefetchROITool : public extends<AthAlgTool, IViewCreatorROITool> {
public:
  ViewCreatorExtraPrefetchROITool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual StatusCode initialize() override;
  virtual StatusCode attachROILinks(TrigCompositeUtils::DecisionContainer& decisions, const EventContext& eventContext) const override;

private:
  SG::WriteHandleKey<TrigRoiDescriptorCollection> m_extraRoiWHK{
    this, "ExtraPrefetchRoIsKey", "", "Name of the extra RoI collection to be used for prefetching"};
  Gaudi::Property<std::string> m_extraRoiLinkName{
    this, "PrefetchRoIsLinkName", "prefetchRoI", "Name of the link from a decision object to the RoI for prefetching"};
  Gaudi::Property<bool> m_mergeWithOriginal{
    this, "MergeWithOriginal", true, "Make the output RoI be a super-RoI combining the original and updated ones"};

  ToolHandle<IViewCreatorROITool> m_roiCreator { this, "RoiCreator", "", "The main RoI creator tool" };
  ToolHandle<IRoiUpdaterTool> m_roiUpdater { this, "RoiUpdater", "", "RoI Updater" };
};

#endif // DECISIONHANDLING_VIEWCREATOREXTRAPREFETCHROITOOL_H
