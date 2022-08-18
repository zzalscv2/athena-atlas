/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerMatchingTool/TrigMatchTestAlg.h"
#include "AsgDataHandles/ReadHandle.h"
#include "TrigCompositeUtils/KFromNItr.h"
#include "TrigCompositeUtils/ChainNameParser.h"
#include "TrigCompositeUtils/ProductItr.h"

#include <algorithm>
#include <iterator>
#include <iomanip>

namespace Trig {

    TrigMatchTestAlg::TrigMatchTestAlg(const std::string &name, ISvcLocator *pSvcLocator) :
        EL::AnaAlgorithm(name, pSvcLocator)
    {
        declareProperty("TrigDecisionTool", m_tdt, "The TrigDecisionTool");
        declareProperty("OfflineElectrons", m_offlineKeys["e"] = "Electrons", "The offline electrons");
        declareProperty("OfflinePhotons", m_offlineKeys["g"] = "Photons", "The offline photons");
        declareProperty("OfflineMuons", m_offlineKeys["mu"] = "Muons", "The offline muons");
        declareProperty("OfflineTaus", m_offlineKeys["tau"] = "TauJets", "The offline taus");
    }


    StatusCode TrigMatchTestAlg::initialize()
    {
        ATH_CHECK(m_tdt.retrieve());
        ATH_CHECK(m_matchingTool.retrieve());
        for (auto &p : m_offlineKeys)
            ATH_CHECK(p.second.initialize(SG::AllowEmpty));

        for (const std::string &chain : m_chains)
        {
            std::map<std::string, std::size_t> info;
            for (const ChainNameParser::LegInfo &legInfo : ChainNameParser::HLTChainInfo(chain))
                if (m_offlineKeys.count(legInfo.signature) && !m_offlineKeys[legInfo.signature].empty())
                    info[legInfo.signature] += legInfo.multiplicity;
            if (!info.empty())
            {
                m_chainInfos[chain] = std::move(info);
                m_chainData[chain];
            }
            else
                ATH_MSG_WARNING("No matchable items in chain " << chain);
        }

        if (m_chainInfos.empty())
        {
            ATH_MSG_ERROR("No matchable chains provided!");
            return StatusCode::FAILURE;
        }

        return StatusCode::SUCCESS;
    }

    StatusCode TrigMatchTestAlg::execute()
    {
        ++m_nEvents;
        // Retrieve the input containers
        std::map<std::string, const xAOD::IParticleContainer *> offlineInputs;
        for (const auto &p : m_offlineKeys)
        {
            if (p.second.empty())
                continue;
            auto handle = SG::makeHandle(p.second);
            if (!handle.isValid())
            {
                ATH_MSG_ERROR("Failed to retrieve " << p.second.key());
                return StatusCode::FAILURE;
            }
            offlineInputs[p.first] = handle.cptr();
        }

        for (const auto &chainPair : m_chainInfos)
        {
            const std::string &chain = chainPair.first;
            if (!m_tdt->isPassed(chain))
            {
                ATH_MSG_DEBUG("Chain " << chain << " not passed!");
                continue;
            }
            ++m_chainData[chain].nEventsPassed;
            // Now we have to build all the *offline* combinations
            std::vector<TrigCompositeUtils::KFromNItr> idxItrs;
            std::vector<const xAOD::IParticleContainer *> containers;
            for (const auto &sigPair : chainPair.second)
            {
                containers.push_back(offlineInputs.at(sigPair.first));
                idxItrs.emplace_back(sigPair.second, containers.back()->size());
            }
            TrigCompositeUtils::ProductItr<TrigCompositeUtils::KFromNItr> idxItr(
                    idxItrs, std::vector<TrigCompositeUtils::KFromNItr>(idxItrs.size()));
            bool matched = false;
            for (; !idxItr.exhausted(); ++idxItr)
            {
                std::vector<const xAOD::IParticle *> particles;
                for (std::size_t idx = 0; idx < containers.size(); ++idx)
                    std::transform(
                            idxItr->at(idx)->begin(), idxItr->at(idx)->end(), std::back_inserter(particles),
                            [container=containers.at(idx)] (std::size_t contIdx) { return container->at(contIdx); });
                if (m_matchingTool->match(particles, chain))
                {
                    matched = true;
                    ++m_chainData[chain].nEventsMatched;
                    break;
                }
            }
            ATH_MSG_DEBUG("Chain " << chain << " is matched? " << std::boolalpha << matched);
        }
        return StatusCode::SUCCESS;
    }
    
    StatusCode TrigMatchTestAlg::finalize()
    {
        ATH_MSG_INFO("Finalizing " << name() );
        ATH_MSG_INFO("Saw " << m_nEvents << " events");
        for (const auto &p : m_chainData)
            ATH_MSG_INFO("Chain " << p.first << " had " << p.second.nEventsPassed << " passed events of which " << p.second.nEventsMatched << " were matched");
        return StatusCode::SUCCESS;
    }


} //> end namespace Trig
