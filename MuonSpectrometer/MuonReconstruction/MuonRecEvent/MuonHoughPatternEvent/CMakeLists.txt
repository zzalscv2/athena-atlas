################################################################################
# Package: MuonHoughPatternEvent
################################################################################

# Declare the package name:
atlas_subdir( MuonHoughPatternEvent )

# External dependencies:
find_package( ROOT COMPONENTS Hist Core Tree MathCore RIO pthread )

# tag ROOTBasicLibs was not recognized in automatic conversion in cmt2cmake

# Component(s) in the package:
atlas_add_library( MuonHoughPatternEvent
                   src/*.cxx
                   PUBLIC_HEADERS MuonHoughPatternEvent
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES AthContainers GeoPrimitives AthenaBaseComps
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils TrkDetElementBase TrkSurfaces TrkPrepRawData GeoPrimitives )

