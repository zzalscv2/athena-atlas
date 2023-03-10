# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTrkFinding )

# External dependencies:
find_package( Acts COMPONENTS Core )

atlas_add_component( ActsTrkFinding
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES
                     GaudiKernel
                     BeamSpotConditionsData
                     ActsCore
                     ActsTrkEvent
                     ActsTrkToolInterfacesLib
                     MagFieldConditions
                     ActsGeometryInterfacesLib
                     SiSPSeededTrackFinderData
                     AthenaMonitoringKernelLib
                     ActsTrkEventCnvLib
                     )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
