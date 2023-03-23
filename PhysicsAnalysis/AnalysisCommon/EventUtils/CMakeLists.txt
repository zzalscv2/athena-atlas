# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( EventUtils )


# Component(s) in the package:
atlas_add_component( EventUtils
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AsgDataHandlesLib AsgTools AthAnalysisBaseCompsLib AthContainers AthContainersInterfaces AthenaBaseComps AthenaKernel CxxUtils EventInfo xAODBase xAODCaloEvent xAODCore xAODCutFlow xAODEgamma xAODEventInfo xAODJet xAODMuon xAODPFlow xAODParticleEvent xAODTau xAODTracking xAODTruth GaudiKernel PATCoreLib ExpressionEvaluationLib TrigDecisionToolLib DerivationFrameworkInterfaces )
