/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "ViewCreatorExtraPrefetchROITool.h"

ViewCreatorExtraPrefetchROITool::ViewCreatorExtraPrefetchROITool(const std::string& type, const std::string& name, const IInterface* parent)
: base_class(type, name, parent) {}

StatusCode ViewCreatorExtraPrefetchROITool::initialize() {
  ATH_CHECK(m_extraRoiWHK.initialize());
  ATH_CHECK(m_roiCreator.retrieve());
  ATH_CHECK(m_roiUpdater.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode ViewCreatorExtraPrefetchROITool::attachROILinks(TrigCompositeUtils::DecisionContainer& decisions, const EventContext& eventContext) const {
  using namespace TrigCompositeUtils;

  // Call the main RoI creator tool
  ATH_CHECK(m_roiCreator->attachROILinks(decisions, eventContext));

  // Record the updated RoI container
  SG::WriteHandle<TrigRoiDescriptorCollection> outputRois = createAndStoreNoAux(m_extraRoiWHK, eventContext);

  // Loop over all decisions and create updated RoI for each RoI linked by the main tool
  for (Decision* decision : decisions) {
    // Get the original RoI from the main tool
    const ElementLink<TrigRoiDescriptorCollection> roiEL = decision->objectLink<TrigRoiDescriptorCollection>(roiString());
    ATH_CHECK(roiEL.isValid());
    const TrigRoiDescriptor* originalRoi = *roiEL;

    // Execute the updater tool
    std::unique_ptr<TrigRoiDescriptor> updatedRoi = m_roiUpdater->execute(originalRoi, eventContext);

    // Add the updated (and merged if requested) RoI to the output container
    if (m_mergeWithOriginal.value()) {
      outputRois->push_back(std::make_unique<TrigRoiDescriptor>());
      outputRois->back()->setComposite(true);
      outputRois->back()->manageConstituents(true); // take ownership of the two objects added below
      outputRois->back()->push_back(new TrigRoiDescriptor(*originalRoi));
      outputRois->back()->push_back(updatedRoi.release());
    } else {
      outputRois->push_back(std::move(updatedRoi));
    }

    // Link the last element of the output container to the current decision object
    const ElementLink<TrigRoiDescriptorCollection> newRoiEL{*outputRois, outputRois->size()-1, eventContext};
    decision->setObjectLink(m_extraRoiLinkName, newRoiEL);
  }

  return StatusCode::SUCCESS;
}
