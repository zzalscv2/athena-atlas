# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthExUnittest )

# External dependencies:
find_package( GTest )

# The component lib:
atlas_add_component( AthExUnittest
                     AthExUnittest/*.h src/*.cxx
                     src/components/*.cxx
                     Root/*.cxx
                     LINK_LIBRARIES GaudiKernel AthenaBaseComps )

# The test:
if( NOT SIMULATIONBASE AND NOT GENERATIONBASE AND NOT XAOD_ANALYSIS )
  atlas_add_test( gt_AthExUnittest
    SOURCES test/gt_AthExUnittest.cxx
    LINK_LIBRARIES GaudiKernel GoogleTestTools AthenaBaseComps CxxUtils
    POST_EXEC_SCRIPT nopost.sh )
endif()
