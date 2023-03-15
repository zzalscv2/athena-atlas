/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TimeBurner.h"

#include <thread>
#include <chrono>

TimeBurner::TimeBurner(const std::string& name, ISvcLocator* pSvcLocator)
: ::HypoBase(name, pSvcLocator) {}

StatusCode TimeBurner::initialize() {
  // we don't actually need the HypoTool
  for (auto& tool : m_hypoTools) tool.disable();
  return StatusCode::SUCCESS;
}

StatusCode TimeBurner::execute(const EventContext& eventContext) const {
  // Create a reject decision
  TrigCompositeUtils::createAndStore(decisionOutput(), eventContext);

  std::this_thread::sleep_for(std::chrono::milliseconds(m_sleepTimeMillisec.value()));

  return StatusCode::SUCCESS;
}
