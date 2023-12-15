/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "L1TriggerResultMaker.h"
#include "L1TopoAlgorithms/cTauMultiplicity.h"
#include "xAODTrigger/eFexTauRoIAuxContainer.h"
#include "xAODTrigger/TrigCompositeAuxContainer.h"

namespace {
  template<class T> void makeLink(const SG::ReadHandleKey<T>& rhk,
                                  xAOD::TrigComposite& target,
                                  const std::string& linkName,
                                  const EventContext& eventContext) {
    ElementLink<T> link = ElementLink<T>(rhk.key(), 0, eventContext);
    target.typelessSetObjectLink(linkName,
                                 link.key(),
                                 ClassID_traits<T>::ID(),
                                 /*beginIndex =*/ 0);
  }
  template<class T> void makeLink(const SG::WriteHandleKey<T>& rhk,
                                  xAOD::TrigComposite& target,
                                  const std::string& linkName,
                                  const EventContext& eventContext) {
    ElementLink<T> link = ElementLink<T>(rhk.key(), 0, eventContext);
    target.typelessSetObjectLink(linkName,
                                 link.key(),
                                 ClassID_traits<T>::ID(),
                                 /*beginIndex =*/ 0);
  }
}

// =============================================================================
// Standard constructor
// =============================================================================
L1TriggerResultMaker::L1TriggerResultMaker(const std::string& name, ISvcLocator* svcLoc)
: AthReentrantAlgorithm(name, svcLoc) {}

// =============================================================================
// Implementation of AthReentrantAlgorithm::initialize
// =============================================================================
StatusCode L1TriggerResultMaker::initialize() {
  ATH_MSG_DEBUG("Initialising " << name());
  ATH_CHECK(m_l1TriggerResultWHKey.initialize());
  ATH_CHECK(m_muRoIKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_eFexEMRoIKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_eFexTauRoIKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_jFexFwdElRoIKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_jFexTauRoIKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_cTauRoIKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_cjTauLinkKey.initialize(!m_cjTauLinkKey.empty()));
  ATH_CHECK(m_jFexSRJetRoIKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_jFexLRJetRoIKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_gFexSRJetRoIKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_gFexLRJetRoIKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_thresholdPatternTools.retrieve());
  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of AthReentrantAlgorithm::execute
// =============================================================================
StatusCode L1TriggerResultMaker::execute(const EventContext& eventContext) const {
  ATH_MSG_DEBUG("Executing " << name());

  // Create and record the L1TriggerResult container
  SG::WriteHandle<xAOD::TrigCompositeContainer> l1trHandle(m_l1TriggerResultWHKey, eventContext);
  auto cont = std::make_unique<xAOD::TrigCompositeContainer>();
  auto auxcont = std::make_unique<xAOD::TrigCompositeAuxContainer>();
  cont->setStore(auxcont.get());
  ATH_CHECK(l1trHandle.record(std::move(cont), std::move(auxcont)));
  ATH_MSG_DEBUG("Recorded L1TriggerResult with key " << m_l1TriggerResultWHKey.key());

  // Create new L1TriggerResult in the container
  l1trHandle->push_back(std::make_unique<xAOD::TrigComposite>());

  // For all RoI types, find it in the event store and link to the L1TriggerResult
  auto retrieveAndLink = [this, &eventContext, &l1trHandle](auto key) -> StatusCode {
    // Skip disabled inputs
    if (key.empty()) {return StatusCode::SUCCESS;}
    // Retrieve the L1 xAOD container to verify it exists
    auto handle = SG::makeHandle(key, eventContext);
    ATH_CHECK(handle.isValid());
    // Link the L1 xAOD container (actually its first element) to L1TriggerResult
    ATH_MSG_DEBUG(key.key() << " size: " << handle->size());
    if (not handle->empty()) {
      makeLink(key, *(l1trHandle->back()), key.key(), eventContext);
    }
    return StatusCode::SUCCESS;
  };

  ATH_CHECK(retrieveAndLink(m_muRoIKey));
  ATH_CHECK(retrieveAndLink(m_eFexEMRoIKey));
  ATH_CHECK(retrieveAndLink(m_eFexTauRoIKey));
  ATH_CHECK(retrieveAndLink(m_jFexFwdElRoIKey));
  ATH_CHECK(retrieveAndLink(m_jFexTauRoIKey));
  ATH_CHECK(retrieveAndLink(m_jFexSRJetRoIKey));
  ATH_CHECK(retrieveAndLink(m_jFexLRJetRoIKey));
  ATH_CHECK(retrieveAndLink(m_gFexSRJetRoIKey));
  ATH_CHECK(retrieveAndLink(m_gFexLRJetRoIKey));

  // Create combined Taus and link them to the L1TR
  ATH_CHECK(createCombinedTauRoIs(*(l1trHandle->back()), eventContext));

  for (const auto& tool: m_thresholdPatternTools) {
    ATH_CHECK(tool->decorateThresholds(eventContext));
  }

  return StatusCode::SUCCESS;
}

// =============================================================================
// Create the combined Tau container matching eTau to jTau
// =============================================================================
StatusCode L1TriggerResultMaker::createCombinedTauRoIs(xAOD::TrigComposite& l1tr, const EventContext& eventContext) const {
  // Skip if keys set to empty
  if (m_cTauRoIKey.empty() or m_cjTauLinkKey.empty()) {return StatusCode::SUCCESS;}

  // Create handles
  using jTauLink_t = ElementLink<xAOD::jFexTauRoIContainer>;
  SG::ReadHandle<xAOD::eFexTauRoIContainer> eTauRoIs{m_eFexTauRoIKey, eventContext};
  SG::ReadHandle<xAOD::jFexTauRoIContainer> jTauRoIs{m_jFexTauRoIKey, eventContext};
  SG::WriteHandle<xAOD::eFexTauRoIContainer> cTauRoIs{m_cTauRoIKey, eventContext};
  SG::WriteDecorHandle<xAOD::eFexTauRoIContainer, jTauLink_t> cjTauLink{m_cjTauLinkKey, eventContext};
  ATH_CHECK(eTauRoIs.isValid());
  ATH_CHECK(jTauRoIs.isValid());

  // Create and record the new eTau container for cTaus
  ATH_CHECK(cTauRoIs.record(std::make_unique<xAOD::eFexTauRoIContainer>(),
                            std::make_unique<xAOD::eFexTauRoIAuxContainer>()));

  // Match jTaus to eTaus and add the resulting cTaus to the container
  // Unmatched eTaus get added as cTau with invalid link to jTau, ATR-25927
  size_t i_eTau{0};
  size_t n_matched{0};
  for (const xAOD::eFexTauRoI* eTau : *eTauRoIs) {
    // Add new eTau to the cTau container
    cTauRoIs->push_back(std::make_unique<xAOD::eFexTauRoI>());
    // Copy over all variables from the original eTau
    *cTauRoIs->back() = *eTau;

    const size_t i_jTau = TCS::cTauMultiplicity::cTauMatching(*eTau, *jTauRoIs);
    if (i_jTau==std::numeric_limits<size_t>::max()) {
      ATH_MSG_DEBUG("No matching jTau for eTau index " << i_eTau);
      // Add an invalid link to jTau
      cjTauLink(*cTauRoIs->back()) = jTauLink_t{};
    } else {
      ++n_matched;
      ATH_MSG_DEBUG("Matched jTau index " << i_jTau << " to eTau index " << i_eTau);
      // Link the matched jTau
      cjTauLink(*cTauRoIs->back()) = jTauLink_t{m_jFexTauRoIKey.key(), i_jTau, eventContext};
    }
    ++i_eTau;
  }

  // Link the cTaus to the L1TriggerResult
  ATH_MSG_DEBUG(m_eFexTauRoIKey.key() << " size: " << eTauRoIs->size());
  ATH_MSG_DEBUG(m_jFexTauRoIKey.key() << " size: " << jTauRoIs->size());
  ATH_MSG_DEBUG(m_cTauRoIKey.key() << " size: " << cTauRoIs->size() << ", matched: " << n_matched);
  if (not cTauRoIs->empty()) {
    makeLink(m_cTauRoIKey, l1tr, m_cTauRoIKey.key(), eventContext);
  }

  return StatusCode::SUCCESS;
}
