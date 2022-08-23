/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "ROBPrefetchingAlg.h"

// Trigger includes
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

// System includes
#include <unordered_set>
#include <vector>
#include <sstream>

namespace {
  std::string robSetToString(const std::unordered_set<uint32_t>& robs) {
    std::ostringstream ss;
    bool first{true};
    ss << "[";
    for (const uint32_t rob : robs) {
      if (first) {first=false;}
      else {ss << ", ";}
      ss << "0x" << std::hex << rob << std::dec;
    }
    ss << "]";
    return ss.str();
  }
}

ROBPrefetchingAlg::ROBPrefetchingAlg(const std::string& name, ISvcLocator* pSvcLocator)
: AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode ROBPrefetchingAlg::initialize() {
  ATH_CHECK(m_inputDecisions.initialize());
  ATH_CHECK(m_regionSelectorTools.retrieve());
  ATH_CHECK(m_robDataProviderSvc.retrieve());
  ATH_MSG_DEBUG("Will consume " << m_inputDecisions.size() << " input decision collection(s):");
  if (msgLvl(MSG::DEBUG)) {
    for (const auto& decisionContKey : m_inputDecisions) {
      ATH_MSG_DEBUG("- " << decisionContKey.key());
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode ROBPrefetchingAlg::execute(const EventContext& eventContext) const {
  using namespace TrigCompositeUtils;
  std::unordered_set<uint32_t> robsToPrefetch;
  std::vector<uint32_t> robsInRoI; // reused in the loop for each RoI

  // Loop over all input Decision collections to extract RoIs
  for (const auto& decisionContKey : m_inputDecisions) {
    SG::ReadHandle<DecisionContainer> decisionCont{decisionContKey, eventContext};
    ATH_CHECK(decisionCont.isValid());
    ATH_MSG_DEBUG("Processing " << decisionCont->size() << " decisions in " << decisionContKey.key());
    for (const Decision* decision : *decisionCont) {

      ElementLink<TrigRoiDescriptorCollection> roiEL = findLink<TrigRoiDescriptorCollection>(decision, roiString()).link;
      if (!roiEL.isValid()) {
        ATH_MSG_WARNING("No " << roiString() << " link in " << decisionContKey.key() << " decision " << *decision);
        continue;
      }

      const TrigRoiDescriptor& roi = **roiEL;
      ATH_MSG_DEBUG("Processing decision " << *decision);
      ATH_MSG_DEBUG("Processing RoI: " << roi);

      for (const auto& tool : m_regionSelectorTools) {
        robsInRoI.clear();
        tool->ROBIDList(roi, robsInRoI);
        robsToPrefetch.insert(robsInRoI.begin(),robsInRoI.end());
      }
    }
  }

  // Prefetch all ROBs collected above
  ATH_MSG_DEBUG("Prefetching " << robsToPrefetch.size() << " ROBs: " << robSetToString(robsToPrefetch));
  std::vector<uint32_t> robsToPrefetchVec{robsToPrefetch.begin(),robsToPrefetch.end()};
  m_robDataProviderSvc->addROBData(robsToPrefetchVec, name());

  return StatusCode::SUCCESS;
}
