################################################################################
# Package: MuonAlignmentData
################################################################################

# Declare the package name:
atlas_subdir( MuonAlignmentData )

# External dependencies:
find_package( CLHEP )

set( extra_libs )
if( NOT SIMULATIONBASE )
  set( extra_libs MuonNSWAsBuilt  )
endif()

# Component(s) in the package:
atlas_add_library( MuonAlignmentData
                   src/*.cxx
                   PUBLIC_HEADERS MuonAlignmentData
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${extra_libs} AthContainers Identifier 
                   GeoPrimitives MuonIdHelpersLib )

