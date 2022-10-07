/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IsoCloseByCaloDecorAlg_H
#define IsoCloseByCaloDecorAlg_H

// Gaudi & Athena basics
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "IsolationSelection/IIsolationCloseByCorrectionTool.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
/**
 * @brief The algorithm selects Inner detector tracks from leptons that are poluting the isolation of
 *        other close-by leptons. The tracks can be either written to a VIEW_ELEMENTS container or
 *        marked as passed for the track particle thinning of a particular derivation *
 */

namespace CP {

    class IsoCloseByCaloDecorAlg : public AthReentrantAlgorithm {
    public:
        IsoCloseByCaloDecorAlg(const std::string& name, ISvcLocator* svcLoc);

        StatusCode execute(const EventContext& ctx) const override;
        StatusCode initialize() override;

    private:
        /// Input containers to retrieve from the storegate
        SG::ReadHandleKey<xAOD::IParticleContainer> m_primPartKey{this, "PrimaryContainer", ""};

        /// These tools shall be configured to pick up the same Inner detector tracks as for the isolation building
        ToolHandle<CP::IIsolationCloseByCorrectionTool> m_closeByCorrTool{this, "IsoCloseByCorrectionTool", "",
                                                                          "The isolation close by correction tool."};

        Gaudi::Property<bool> m_decorClust{this, "decorateClusters", true, "Decorates the associated cluster information"};
        Gaudi::Property<bool> m_decorPflow{this, "decoratePFlowObj", true, "Decorates the associated pflow obj information"};
        ///
        SG::WriteDecorHandleKeyArray<xAOD::IParticleContainer> m_decorKeys{
            this, "DecorationKeys", {}, "List of xAOD decorations added to the particle"};
    };
}  // namespace CP
#endif