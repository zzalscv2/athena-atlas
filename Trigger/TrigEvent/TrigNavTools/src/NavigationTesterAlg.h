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
        ToolHandle<Trig::IIParticleRetrievalTool> m_toolRun2{
            this, "RetrievalToolRun2Nav", "", "The tool configured to use Run 2 format"};
        ToolHandle<Trig::IIParticleRetrievalTool> m_toolRun3{
            this, "RetrievalToolRun3Nav", "", "The tool configured for fetching RUn 3 format"};
        Gaudi::Property<std::vector<std::string>> m_chains{
            this, "Chains", {}, "The chains to test"};
        Gaudi::Property<bool> m_failOnDifference{
            this, "FailOnDifference", false,
            "Return FAILURE if the navigation does not compare equal"};
    }; //> end class AthAlgorithm
}
