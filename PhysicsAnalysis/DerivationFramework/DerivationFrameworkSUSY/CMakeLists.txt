# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkSUSY )

# Component(s) in the package:
atlas_add_component( DerivationFrameworkSUSY
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaKernel DerivationFrameworkInterfaces ExpressionEvaluationLib GaudiKernel MCTruthClassifierLib RecoToolInterfaces xAODAssociations xAODEventInfo xAODJet xAODTracking xAODTruth ReweightUtilsLib)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

