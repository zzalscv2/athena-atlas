# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MCTruthBase )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( XercesC )

# Component(s) in the package:
atlas_add_library( MCTruthBaseLib
                   src/*.cxx
                   OBJECT
                   PUBLIC_HEADERS MCTruthBase
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} ${XERCESC_LIBRARIES} ${CLHEP_LIBRARIES} AtlasHepMCLib AthenaKernel CxxUtils GaudiKernel StoreGateLib G4AtlasToolsLib TrackRecordLib
                   PRIVATE_LINK_LIBRARIES AthenaBaseComps
                   G4AtlasInterfaces MCTruth SimHelpers ISF_InterfacesLib
                   AtlasDetDescr ISF_Geant4Event
                 )

atlas_add_library(MCTruthBase
                  src/components/*.cxx
                  OBJECT
                  NO_PUBLIC_HEADERS
                  PRIVATE_LINK_LIBRARIES MCTruthBaseLib)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
