# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigEgammaForwardHypo )

# Component(s) in the package:
atlas_add_component( TrigEgammaForwardHypo
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthViews AthenaBaseComps AthenaMonitoringKernelLib DecisionHandlingLib EgammaAnalysisInterfacesLib GaudiKernel LumiBlockCompsLib RecoToolInterfaces StoreGateLib TrigCompositeUtilsLib TrigSteeringEvent TrkCaloExtension xAODBase xAODCaloEvent xAODEgamma xAODEventInfo xAODTracking xAODTrigCalo xAODTrigEgamma xAODTrigRinger xAODTrigger AthOnnxruntimeServiceLib RingerSelectorToolsLib)


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-select=ATL900,ATL901 )
