# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ForwardTransport )

# External dependencies:
find_package( Geant4 )

# Component(s) in the package:
atlas_add_library( ForwardTransport
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} AtlasHepMCLib  TruthUtils ForwardTracker GaudiKernel GeneratorObjects G4AtlasToolsLib ForwardTransportSvcLib ISF_Event MCTruth )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )

