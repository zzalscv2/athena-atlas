# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GeneratorFilters )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree RIO Physics )

# Component(s) in the package:
atlas_add_library( GeneratorFiltersLib
                   src/*.cxx
                   PUBLIC_HEADERS GeneratorFilters
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AthContainers AthenaBaseComps AtlasHepMCLib GaudiKernel GeneratorModulesLib MCTruthClassifierLib xAODJet xAODTruth
                   PRIVATE_LINK_LIBRARIES AthLinks AthenaKernel CxxUtils GeneratorObjects StoreGateLib TruthUtils )

atlas_add_component( GeneratorFilters
                     src/components/*.cxx
                     LINK_LIBRARIES GeneratorFiltersLib )

# Install job option fragments from the package:
atlas_install_joboptions( share/common/*.py )

