/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "JetJvtEfficiency/JvtSelectionTool.h"
#include "AsgDataHandles/ReadDecorHandle.h"

namespace {
    const static std::map<std::string, float> pflowWPMap{{"Medium", 0.2}, {"Tight", 0.5}};
    const static std::map<std::string, float> topoWPMap{
            {"Loose", 0.11}, {"Medium", 0.59}, {"Tight", 0.91}};
} // namespace

namespace CP {

    StatusCode JvtSelectionTool::initialize() {
        ATH_MSG_WARNING("Jvt is deprecated, please move to using NNJvt");
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

        if (m_wp != "Custom") {
            if (m_wp == "Default")
                m_wp = m_isPFlow.value() ? "Tight" : "Medium";
            m_jvtCutBorder = (!m_isPFlow.value() && m_wp == "Medium") ? 0.11 : -2.;
            const auto &map = m_isPFlow.value() ? pflowWPMap : topoWPMap;
            auto itr = map.find(m_wp);
            if (itr == map.end()) {
                ATH_MSG_ERROR("Invalid Jvt working point name");
                return StatusCode::FAILURE;
            }
            m_jvtCut = itr->second;
        }

        return StatusCode::SUCCESS;
    }

    bool JvtSelectionTool::select(const xAOD::IParticle *jet) const {
        float eta = (*m_etaAcc)(*jet);
        if (std::abs(eta) > 2.4 && std::abs(eta) < 2.5)
            return (*m_jvtAcc)(*jet) > m_jvtCutBorder;
        else
            return (*m_jvtAcc)(*jet) > m_jvtCut;
    }

} // namespace CP
