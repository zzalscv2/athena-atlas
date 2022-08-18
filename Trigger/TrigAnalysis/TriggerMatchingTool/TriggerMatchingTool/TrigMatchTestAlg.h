/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGERMATCHINGTOOL_TRIGMATCHTESTALG_H
#define TRIGGERMATCHINGTOOL_TRIGMATCHTESTALG_H

#include "AnaAlgorithm/AnaAlgorithm.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "xAODBase/IParticleContainer.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "TriggerMatchingTool/IMatchingTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

#include <vector>
#include <string>
#include <map>

namespace Trig {
    class TrigMatchTestAlg : public EL::AnaAlgorithm {
    public:
        TrigMatchTestAlg(const std::string &name, ISvcLocator *pSvcLocator);
        virtual ~TrigMatchTestAlg() override = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute() override;
        virtual StatusCode finalize() override;
    private:
        Gaudi::Property<std::vector<std::string>> m_chains{
            this, "Chains", {}, "List of trigger chains to check"};
        std::map<std::string, SG::ReadHandleKey<xAOD::IParticleContainer>> m_offlineKeys;
        ToolHandle<Trig::TrigDecisionTool> m_tdt{"Trig::TrigDecisionTool/TrigDecisionTool"};
        ToolHandle<Trig::IMatchingTool> m_matchingTool{
            this, "MatchingTool", "Trig::R3MatchingTool/MatchingTool", "The trigger matching tool"};
        std::map<std::string, std::map<std::string, std::size_t>> m_chainInfos;

        struct MatchData {
            std::size_t nEventsPassed{0};
            std::size_t nEventsMatched{0};
        };
        std::size_t m_nEvents{0};
        std::map<std::string, MatchData> m_chainData;

    }; //> end class TrigMatchTestAlg
}

#endif //> !TRIGGERMATCHINGTOOL_TRIGMATCHTESTALG_H
