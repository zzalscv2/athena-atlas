################################################################################
# Package: STgcClusterization
################################################################################

# Declare the package name:
atlas_subdir( STgcClusterization )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( STgcClusterizationLib
                   src/*.cxx
                   PUBLIC_HEADERS STgcClusterization
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES GaudiKernel MuonPrepRawData MuonIdHelpersLib  MuonCondData
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps EventPrimitives MuonReadoutGeometry)

atlas_add_component( STgcClusterization
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} GaudiKernel MuonPrepRawData AthenaBaseComps EventPrimitives MuonIdHelpersLib STgcClusterizationLib )

