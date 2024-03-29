/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERPHOMET_H
#define GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERPHOMET_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthMetaDataContainer.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

/// @brief Algorithm to skim the xAOD truth particle container for xAOD MET filter
///
/// This algorithm is used to copy and skim the particles from the xAOD TruthParticles container,
/// keeping just relevant MET particles from the event.
/// The design of this class heavily mirrors the DerivationFramework::TruthCollectionMaker.
///
/// @author Jeff Dandoy <Jeff.Dandoy@cern.ch>
class xAODTruthParticleSlimmerMET : public AthAlgorithm
{
public:
    /// Regular algorithm constructor
    xAODTruthParticleSlimmerMET(const std::string &name, ISvcLocator *svcLoc);
    /// Function initialising the algorithm
    virtual StatusCode initialize();
    /// Function executing the algorithm
    virtual StatusCode execute();

private:
    /// The key for the output xAOD truth containers
    std::string m_xaodTruthParticleContainerNameMET;
    std::string m_xaodTruthParticleContainerName;
    std::string m_xaodTruthEventContainerName;

    /// Selection values for keeping METs
    //double m_MET_pt_selection; //in GeV

    bool prompt( const xAOD::TruthParticle* tp ) const;

    ToolHandle<IMCTruthClassifier> m_classif;
}; // class xAODTruthParticleSlimmerMET

#endif //GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERPHOMET_H
