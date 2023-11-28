/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
*/

#ifndef TAUREC_TauElecSubtractAlg_H
#define TAUREC_TauElecSubtractAlg_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloClusterCellLinkContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "CaloUtils/CaloClusterStoreHelper.h"
#include "tauRecTools/TauRecToolBase.h"
#include "xAODTau/TauJet.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "AsgTools/PropertyWrapper.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "xAODEgamma/ElectronxAODHelpers.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

/**
 * @brief Algorithm that remove electron cluster and tracks from the 
 * tracks and cluster containers.
 */
class TauElecSubtractAlg : public AthReentrantAlgorithm
{
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    public:
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;

    private:
        SG::ReadHandleKey<xAOD::ElectronContainer>        m_elecInput               { this, "Key_ElectronsInput",             "", "Input electron container" };
        SG::ReadHandleKey<xAOD::CaloClusterContainer>     m_clustersInput           { this, "Key_ClustersInput",              "", "Input cluster container " };
        SG::WriteHandleKey<xAOD::CaloClusterContainer>    m_clustersOutput          { this, "Key_ClustersOutput",             "", "Output cluster container" };
        SG::ReadHandleKey<xAOD::TrackParticleContainer>   m_tracksInput             { this, "Key_IDTracksInput",              "", "Input track container   " };
        SG::WriteHandleKey<xAOD::TrackParticleContainer>  m_tracksOutput            { this, "Key_IDTracksOutput",             "", "Output track container  " };
        SG::WriteHandleKey<xAOD::CaloClusterContainer>    m_removedClustersOutput   { this, "Key_RemovedClustersOutput",      "", "Output removed clusters " };
        SG::WriteHandleKey<xAOD::TrackParticleContainer>  m_removedTracksOutput     { this, "Key_RemovedTracksOutput",        "", "Output removed tracks   " };

        // make sure standard jet TVA is run before this algorithm, hard coded for now
        SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_stdJetTVADecoKey     { this, "InDetTrackParticles_jetTVA_key","InDetTrackParticles.TTVA_AMVFVertices_forReco", "make sure standard jet TVA is run before this algorithm, hard coded for now"};

        ToolHandle<IAsgElectronLikelihoodTool>            m_eleLHSelectTool         { this, "ElectronLHTool",                 "", "Electron likelihood selection tool" };
        Gaudi::Property<bool>                             m_doNothing               { this, "doNothing", false, "If we just deep copy the containers (For Validation Only)"};
};

#endif // TAUREC_TauElecSubtractAlg_H
