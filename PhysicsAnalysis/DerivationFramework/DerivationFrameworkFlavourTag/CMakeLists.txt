# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkFlavourTag )

# External dependencies:
find_package( ROOT COMPONENTS Core RIO )

# Component(s) in the package:
atlas_add_component( DerivationFrameworkFlavourTag
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps FlavorTagDiscriminants GaudiKernel xAODJet xAODTracking xAODBTagging )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
