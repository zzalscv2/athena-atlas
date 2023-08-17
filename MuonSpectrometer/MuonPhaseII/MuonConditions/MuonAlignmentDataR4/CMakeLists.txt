################################################################################
# Package: MuonAlignmentDataR4
################################################################################

# Declare the package name:
atlas_subdir( MuonAlignmentDataR4 )

set( extra_libs )
if( NOT SIMULATIONBASE )
  set( extra_libs MuonNSWAsBuilt  )
endif()

# Component(s) in the package:
atlas_add_library( MuonAlignmentDataR4
                   src/*.cxx
                   PUBLIC_HEADERS MuonAlignmentDataR4
                   LINK_LIBRARIES ${extra_libs} MuonAlignmentData Identifier ActsGeometryInterfacesLib )

