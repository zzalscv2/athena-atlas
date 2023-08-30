/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_isolationDecorator_H
#define DERIVATIONFRAMEWORK_isolationDecorator_H

#include <string>
#include <vector>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "ExpressionEvaluation/ExpressionParserUser.h"
#include "GaudiKernel/ToolHandle.h"
#include "RecoToolInterfaces/ICaloTopoClusterIsolationTool.h"
#include "RecoToolInterfaces/ITrackIsolationTool.h"

#include <StoreGate/ReadHandleKey.h>
#include <xAODTracking/TrackParticleContainer.h>

namespace DerivationFramework {
    class isolationDecorator : public ExpressionParserUser<AthAlgTool>, public IAugmentationTool {
    public:
        /** Constructor with parameters */
        isolationDecorator(const std::string& t, const std::string& n, const IInterface* p);

        /** Destructor */
        virtual ~isolationDecorator() = default;

        StatusCode initialize() override;

        StatusCode addBranches() const override;

    private:
        StatusCode decorate(const xAOD::IParticle* part, const int iso_type, const float val) const;
 
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_containerName{this, "TargetContainer", "InDetTrackParticles"};
        Gaudi::Property<std::string> m_selectionString{this, "SelectionString", "" };
        Gaudi::Property<std::string> m_prefix{this, "Prefix", "" };
        Gaudi::Property<std::string> m_selFlag{this, "SelectionFlag", "" };
        Gaudi::Property<int> m_selFlagValue{this, "SelectionFlagValue", 1};

        /// Athena configured tools
        ToolHandle<xAOD::ITrackIsolationTool> m_trackIsolationTool{this, "TrackIsolationTool", ""};
        ToolHandle<xAOD::ICaloTopoClusterIsolationTool> m_caloIsolationTool{this, "CaloIsolationTool" , ""};

        std::vector<xAOD::Iso::IsolationType> m_ptconeTypes{};
        Gaudi::Property<std::vector<int>> m_ptcones{this, "ptcones", {}};
        xAOD::TrackCorrection m_trkCorrList;
        std::vector<xAOD::Iso::IsolationType> m_topoetconeTypes;
        Gaudi::Property<std::vector<int>> m_topoetcones{this, "topoetcones", {}};
        xAOD::CaloCorrection m_caloCorrList;

        std::map<int, SG::AuxElement::Decorator<float> > m_decorators;
    };
}  // namespace DerivationFramework
#endif  //
