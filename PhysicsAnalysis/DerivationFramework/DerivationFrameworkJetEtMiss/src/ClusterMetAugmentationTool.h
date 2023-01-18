/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ClusterMetAugmentationTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: Alex Bunka (alexandertarasbunka@cern.ch)
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_CLUSTERMETAUGMENTATIONTOOL_H
#define DERIVATIONFRAMEWORK_CLUSTERMETAUGMENTATIONTOOL_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "JetInterface/IJetSelector.h"
#include "JetInterface/IJetModifier.h"
#include "xAODJet/JetContainer.h"
#include <AsgTools/AnaToolHandle.h>
#include <TRandom3.h>

namespace DerivationFramework {

    class ClusterMetAugmentationTool : public AthAlgTool, public IAugmentationTool {
    public:
        ClusterMetAugmentationTool(const std::string& t, const std::string& n, const IInterface* p);

        StatusCode initialize();
        StatusCode finalize();
        virtual StatusCode addBranches() const;

    private:
        TRandom3* m_ranNumGen;
        std::string m_jetContainerName;
        std::string m_electronContainerName;
        std::string m_muonContainerName;
    };
}

#endif
