# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArHitAlgs )

# External dependencies:
#find_package( CLHEP )

# Component(s) in the package:
atlas_add_component( LArHitAlgs
                     src/*.cxx src/components/*.cxx
                     #PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     #PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                     #LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel
                     LINK_LIBRARIES AthenaBaseComps AthenaKernel
                     CaloIdentifier Identifier LArSimEvent
                     StoreGateLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

