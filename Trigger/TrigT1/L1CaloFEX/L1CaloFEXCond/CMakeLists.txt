# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( L1CaloFEXCond )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat eformat_write )

# Package library:
atlas_add_library( L1CaloFEXCondLib
                   L1CaloFEXCond/*.h src/*.cxx
                   PUBLIC_HEADERS L1CaloFEXCond
                   LINK_LIBRARIES AthContainers AthLinks AthenaBaseComps AthenaKernel CxxUtils GaudiKernel StoreGateLib Identifier AthenaPoolUtilities
                   PRIVATE_LINK_LIBRARIES TrigConfData SGTools PathResolver)

# Component(s) in the package:
atlas_add_component( L1CaloFEXCond
                     src/components/*.cxx
                     LINK_LIBRARIES L1CaloFEXCondLib)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
