# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArHitAlgs )

# Component(s) in the package:
atlas_add_component( LArHitAlgs
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaKernel
                     CaloIdentifier Identifier LArSimEvent
                     StoreGateLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

