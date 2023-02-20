# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigAFPHypo )

# Component(s) in the package:
atlas_add_component( TrigAFPHypo
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaMonitoringKernelLib DecisionHandlingLib GaudiKernel PathResolver TrigCompositeUtilsLib xAODForward xAODJet xAODTracking xAODTrigger )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_runtime( optics/*.txt )
