/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "JetJvtEfficiency/NNJvtSelectionTool.h"
#include "PathResolver/PathResolver.h"
#include "AsgDataHandles/ReadDecorHandle.h"

#include <fstream>

namespace {
    const static std::map<std::string, std::string> workingPoints{
            {"FixedEffPt", "NNJVT.Cuts.FixedEffPt.Offline.Nonprompt_All_MaxW.json"},
            {"TightFwd", "NNJVT.Cuts.TightFwd.Offline.Nonprompt_All_MaxWeight.json"}};
}

namespace CP {
    StatusCode NNJvtSelectionTool::initialize() {
        ATH_CHECK(JvtSelectionToolBase::initialize());

        m_jvtAcc.emplace(m_jvtMoment.key());
        if (m_jetContainer.empty()) {
            ATH_MSG_WARNING("No JetContainer set. This behaviour is deprecated");
            ATH_CHECK(m_jvtMoment.initialize(false));
        }
        else {
            m_jvtMoment = m_jetContainer + "." + m_jvtMoment.key();
            ATH_CHECK(m_jvtMoment.initialize());
        }

        if (m_configFile.empty()) {
            auto itr = workingPoints.find(m_wp);
            if (itr == workingPoints.end()) {
                ATH_MSG_ERROR("Unknown NNJvt WP: " << m_wp.value());
                return StatusCode::FAILURE;
            }
            m_configFile = itr->second;
        }
        std::string file =
                m_configDir.empty() ? m_configFile.value() : (m_configDir + "/" + m_configFile);
        std::string resolved = PathResolverFindCalibFile(file);
        if (resolved.empty()) {
            ATH_MSG_ERROR("File " << file << " not found!");
            return StatusCode::FAILURE;
        }
        std::ifstream fcuts(resolved);
        if (!fcuts.is_open()) {
            ATH_MSG_ERROR("Failed to open " << resolved << "!");
            return StatusCode::FAILURE;
        }

        m_cutMap = JetPileupTag::NNJvtCutMap::fromJSON(fcuts);

        return StatusCode::SUCCESS;
    }

    bool NNJvtSelectionTool::select(const xAOD::IParticle *jet) const {
        return (*m_jvtAcc)(*jet) > m_cutMap(*jet);
    }
} // namespace CP