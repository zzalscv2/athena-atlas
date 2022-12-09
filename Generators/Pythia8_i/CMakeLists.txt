# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( Pythia8_i )

# External dependencies:
find_package( Boost )
find_package( CLHEP )
find_package( Pythia8 )

# Remove the --as-needed linker flags:
atlas_disable_as_needed()

# Component(s) in the package:
atlas_add_library( Pythia8_iLib
   Pythia8_i/*.h src/*.cxx
   src/Pythia8Custom/*.h src/Pythia8Custom/*.cxx
   src/UserHooks/*.h src/UserHooks/*.cxx
   src/UserProcesses/*.cxx
   src/UserResonances/*.cxx
   PUBLIC_HEADERS Pythia8_i
   DEFINITIONS ${CLHEP_DEFINITIONS}
   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${PYTHIA8_INCLUDE_DIRS}
   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${PYTHIA8_LIBRARIES} GaudiKernel
                  GeneratorModulesLib
   PRIVATE_DEFINITIONS PY8INCLUDE_DIR=\"${PYTHIA8_INCLUDE_DIR}\"
   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} AthenaKernel GeneratorObjects
                          PathResolver )

atlas_add_component( Pythia8_i
   src/components/*.cxx
   LINK_LIBRARIES Pythia8_iLib )

# Install files from the package:
atlas_install_joboptions( share/common/*.py
                          share/example/*.py )

set( Pythia8Environment_DIR ${CMAKE_CURRENT_SOURCE_DIR}
   CACHE PATH "Location of Pythia8Environment.cmake" )
find_package( Pythia8Environment )
