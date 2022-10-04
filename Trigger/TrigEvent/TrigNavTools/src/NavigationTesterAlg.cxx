/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#include "NavigationTesterAlg.h"
#include <set>
#include <algorithm>
#include <iterator>


// anonymous namespace for convenience functions
namespace {
    std::set<std::set<const xAOD::IParticle *>> vectorToSet(
            const std::vector<std::vector<const xAOD::IParticle *>> &vec)
    {
        std::set<std::set<const xAOD::IParticle *>> ret;
        for (const std::vector<const xAOD::IParticle *> &combination : vec)
            ret.emplace(combination.begin(), combination.end());
        return ret;
    }

    std::ostream &operator<<(std::ostream &os, const xAOD::IParticle *p)
    {
        return os << "["
            << "type = " << p->type() << ", "
            << "pt = " << p->pt() << ", "
            << "eta = " << p->eta() << ", "
            << "phi = " << p->phi()
            << "]";

    }

    // Define printing operators for the set and IParticle pointers
    template <typename T>
    std::ostream &operator<<(std::ostream &os, const std::set<T> &s)
    {
        os << "{";
        for (auto itr = s.begin(); itr != s.end(); ++itr)
        {
            if (itr != s.begin())
                os << ", ";
            os << *itr;
        }
        return os << "}";
    }
}

namespace Trig {

    NavigationTesterAlg::NavigationTesterAlg(const std::string &name, ISvcLocator *pSvcLocator) :
        AthAlgorithm(name, pSvcLocator)
    {}

    StatusCode NavigationTesterAlg::initialize()
    {
        ATH_CHECK(m_toolRun2.retrieve());
        ATH_CHECK(m_toolRun3.retrieve());
        if (m_chains.size() == 0)
            ATH_MSG_WARNING("No chains provided, algorithm will be no-op");
        return StatusCode::SUCCESS;
    }

    StatusCode NavigationTesterAlg::execute()
    {
	/*** TODO ***/
        //bool good = true;
        for (const std::string &chain : m_chains)
        {
            ATH_MSG_DEBUG("Begin testing chain " << chain);

            // We assume that the navigation is ultimately a set of element links
            // We're comparing two types of navigation but they should both point to the same
            // objects.
            // We don't care about the order of the combinations, or the order within the
            // combinations, we just care that they are the same. Therefore, we can convert the
            // vectors to sets and just look at the differences between them
            std::vector<std::vector<const xAOD::IParticle *>> vecCombinationsRun2;
            ATH_MSG_DEBUG("###### checking features of CHAIN " << chain);
            ATH_CHECK(m_toolRun2->retrieveParticles(vecCombinationsRun2, chain));
            auto combsRun2 = vectorToSet(vecCombinationsRun2);
            ATH_MSG_DEBUG("Run 2 size " << combsRun2.size());
            for (auto& c : combsRun2 ) {
                ATH_MSG_DEBUG(c);
            }
            std::vector<std::vector<const xAOD::IParticle *>> vecCombinationsRun3;
            ATH_CHECK(m_toolRun3->retrieveParticles(vecCombinationsRun3, chain));
            auto combsRun3 = vectorToSet(vecCombinationsRun3);
            ATH_MSG_DEBUG("Run 3 size " << combsRun3.size());

            for (auto& c : combsRun3 ) {
                ATH_MSG_DEBUG(c);
            }
            if ( combsRun2.size() != combsRun3.size()) {
                if ( m_failOnDifference ) {
                    ATH_MSG_ERROR("Different count of feature accessed from chain " << chain  
                                << " using Run 2 navigation " << combsRun2.size() 
                                << " Run 3 navigation " << combsRun3.size() );
                    return StatusCode::FAILURE;
                }

            }
        }

	    /*** TODO ***/
        //     ATH_CHECK(m_toolRun3->retrieveParticles(vecCombinationsRun3, chain));
        //     std::set<std::set<const xAOD::IParticle *>> combos1 = vectorToSet(vecCombinationsRun2);
        //     std::set<std::set<const xAOD::IParticle *>> combos2 = vectorToSet(vecCombinationsRun3);
        //     ATH_MSG_DEBUG("Tool 1 retrieved " << combos1.size() << " combinations, tool 2 retrieved " << combos2.size());
        //     std::vector<std::set<const xAOD::IParticle *>> onlyIn1;
        //     std::vector<std::set<const xAOD::IParticle *>> onlyIn2;
        //     std::set_difference(
        //             combos1.begin(), combos1.end(),
        //             combos2.begin(), combos2.end(),
        //             std::back_inserter(onlyIn1));
        //     std::set_difference(
        //             combos2.begin(), combos2.end(),
        //             combos1.begin(), combos1.end(),
        //             std::back_inserter(onlyIn2));
        //     if (onlyIn1.size() || onlyIn2.size())
        //     {
        //         good = false;
        //         ATH_MSG_WARNING("Difference found for chain " << chain);
        //         if (onlyIn1.size())
        //         {
        //             ATH_MSG_WARNING("Only from tool 1:");
        //             for (const auto &combo : onlyIn1)
        //                 ATH_MSG_WARNING("\t" << combo);
        //         }
        //         if (onlyIn2.size())
        //         {
        //             ATH_MSG_WARNING("Only from tool 2:");
        //             for (const auto &combo : onlyIn2)
        //                 ATH_MSG_WARNING("\t" << combo);
        //         }
        //     }
        //     else
        //         ATH_MSG_DEBUG("Found particles match for chain " << chain);
        // }
        // if (m_failOnDifference.value() and !good)
        // {
        //     ATH_MSG_ERROR("Differences found!");
        //     return StatusCode::FAILURE;
        // }
        return StatusCode::SUCCESS;
    }
} //> end namespace Trig

