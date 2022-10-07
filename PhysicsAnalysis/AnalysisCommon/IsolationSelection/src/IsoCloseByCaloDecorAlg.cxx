
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "IsoCloseByCaloDecorAlg.h"

#include <IsolationSelection/IsolationCloseByCorrectionTool.h>
#include <StoreGate/ReadDecorHandle.h>
#include <StoreGate/ReadHandle.h>

namespace CP {
    static const FloatDecorator dec_assocCaloEta{IsolationCloseByCorrectionTool::caloDecors()[0]};
    static const FloatDecorator dec_assocCaloPhi{IsolationCloseByCorrectionTool::caloDecors()[1]};
    static const FloatDecorator dec_assocCaloEne{IsolationCloseByCorrectionTool::caloDecors()[2]};
    static const BoolDecorator dec_assocCaloIsDec{IsolationCloseByCorrectionTool::caloDecors()[3]};

    static const FloatDecorator dec_assocPflowEta{IsolationCloseByCorrectionTool::pflowDecors()[0]};
    static const FloatDecorator dec_assocPflowPhi{IsolationCloseByCorrectionTool::pflowDecors()[1]};
    static const FloatDecorator dec_assocPflowEne{IsolationCloseByCorrectionTool::pflowDecors()[2]};
    static const BoolDecorator dec_assocPflowIsDec{IsolationCloseByCorrectionTool::pflowDecors()[3]};

    IsoCloseByCaloDecorAlg::IsoCloseByCaloDecorAlg(const std::string& name, ISvcLocator* svcLoc) : AthReentrantAlgorithm(name, svcLoc) {}
    StatusCode IsoCloseByCaloDecorAlg::initialize() {
        ATH_CHECK(m_primPartKey.initialize());
        ATH_CHECK(m_closeByCorrTool.retrieve());
        if (m_decorClust) {
            for (const std::string& decor : IsolationCloseByCorrectionTool::caloDecors()) {
                m_decorKeys.emplace_back(m_primPartKey.key() + "." + decor);
            }
        }
        if (m_decorPflow) {
            for (const std::string& decor : IsolationCloseByCorrectionTool::pflowDecors()) {
                m_decorKeys.emplace_back(m_primPartKey.key() + "." + decor);
            }
        }

        ATH_CHECK(m_decorKeys.initialize());
        if (!m_decorClust && !m_decorPflow) {
            ATH_MSG_FATAL("Nothing is done by me. It's bogous scheduling me");
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }

    StatusCode IsoCloseByCaloDecorAlg::execute(const EventContext& ctx) const {
        SG::ReadHandle<xAOD::IParticleContainer> readHandle{m_primPartKey, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve particle collection " << m_primPartKey.fullKey());
            return StatusCode::FAILURE;
        }
        for (const xAOD::IParticle* part : *readHandle) {
            float eta{0.f}, phi{0.f}, ene{0.f};
            if (m_decorClust) {
                m_closeByCorrTool->associateCluster(part, eta, phi, ene);
                dec_assocCaloEta(*part) = eta;
                dec_assocCaloPhi(*part) = phi;
                dec_assocCaloEne(*part) = ene;
                dec_assocCaloIsDec(*part) = true;
            }
            if (!m_decorPflow) continue;
            m_closeByCorrTool->associateFlowElement(ctx, part, eta, phi, ene);
            dec_assocPflowEta(*part) = eta;
            dec_assocPflowPhi(*part) = phi;
            dec_assocPflowEne(*part) = ene;
            dec_assocPflowIsDec(*part) = true;
        }
        return StatusCode::SUCCESS;
    }
}  // namespace CP
