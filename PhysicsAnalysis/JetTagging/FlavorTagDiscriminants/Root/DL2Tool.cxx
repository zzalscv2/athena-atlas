/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/DL2Tool.h"
#include "FlavorTagDiscriminants/DL2HighLevel.h"

namespace FlavorTagDiscriminants {

  DL2Tool::DL2Tool(const std::string& name):
    asg::AsgTool(name),
    m_props(),
    m_dl2(nullptr)
  {
    declareProperty("nnFile", m_props.nnFile);
    declareProperty("flipTagConfig", m_props.flipTagConfig);
  }
  DL2Tool::~DL2Tool() {}

  StatusCode DL2Tool::initialize() {
    ATH_MSG_INFO("Initialize DL2 from: " + m_props.nnFile);
    FlipTagConfig flipConfig = FlipTagConfig::STANDARD;
    if (m_props.flipTagConfig.size() > 0) {
      flipConfig = flipTagConfigFromString(m_props.flipTagConfig);
    }
    m_dl2.reset(new DL2HighLevel(m_props.nnFile, flipConfig));
    return StatusCode::SUCCESS;
  }
  StatusCode DL2Tool::finalize() {
    return StatusCode::SUCCESS;
  }

  int DL2Tool::modifyJet(xAOD::Jet& jet) const {
    m_dl2->decorate(jet);
    return 0; // 0 means success
  }

}
