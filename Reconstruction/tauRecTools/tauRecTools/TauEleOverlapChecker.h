/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TauEleOverlapChecker_H
#define TAURECTOOLS_TauEleOverlapChecker_H

#include "tauRecTools/TauRecToolBase.h"

#include "AsgTools/ToolHandle.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauJet.h"

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadHandle.h"

/**
 * @brief Select only the tau within the removal direction. 
 * 
 * @author Qichen Dong
 *                                                                              
 */
class TauEleOverlapChecker : public TauRecToolBase {

public:
    ASG_TOOL_CLASS2(TauEleOverlapChecker, TauRecToolBase, ITauToolBase)
    /** @brief Constructor */ 
    TauEleOverlapChecker(const std::string& name);
    /** @brief Destructor */
    virtual ~TauEleOverlapChecker() = default;
    /** @brief Initialization of the tool **/
    virtual StatusCode initialize() override;
    /** @brief Execution of this tool */ 
    virtual StatusCode execute(xAOD::TauJet& tau) const override;

private:
    Gaudi::Property<double> m_checkingCone {this, "CheckingCone", 0.6, "Cone size of the checking, in dR"};
    SG::ReadHandleKey<xAOD::CaloClusterContainer>   m_removedClustersContainer {this,"Key_RemovedClustersContainer","RemovedClusters_EleRM","Removed Clusters input container"};
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_removedTracksContainer   {this,"Key_RemovedTracksContainer",  "RemovedTracks_EleRM",  "Removed Tracks input container"};
};

#endif // TAURECTOOLS_TauEleOverlapChecker_H
