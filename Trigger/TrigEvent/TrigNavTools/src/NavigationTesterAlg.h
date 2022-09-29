/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"
#include "TriggerMatchingTool/IIParticleRetrievalTool.h"

#include <vector>

namespace Trig {
    class NavigationTesterAlg : public AthAlgorithm
    {
    public:
        NavigationTesterAlg(const std::string &name, ISvcLocator *pSvcLocator);
        ~NavigationTesterAlg() override = default;

        StatusCode initialize() override;
        StatusCode execute() override;

    private:
        ToolHandle<Trig::IIParticleRetrievalTool> m_tool1{
            this, "RetrievalTool1", "", "The first retrieval tool to test"};
        ToolHandle<Trig::IIParticleRetrievalTool> m_tool2{
            this, "RetrievalTool2", "", "The second retrieval tool to test"};
        Gaudi::Property<std::vector<std::string>> m_chains{
            this, "Chains", {}, "The chains to test"};
        Gaudi::Property<bool> m_failOnDifference{
            this, "FailOnDifference", false,
            "Return FAILURE if the navigation does not compare equal"};
    }; //> end class AthAlgorithm
}
