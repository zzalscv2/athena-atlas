# $Id: CMakeLists.txt 797759 2017-02-18 01:24:37Z zmarshal $
################################################################################
# Package: DerivationFrameworkAnalysisTests
################################################################################

# Declare the package name:
atlas_subdir( DerivationFrameworkAnalysisTests )

if( XAOD_STANDALONE )
   set( xaod_access_deps Control/xAODRootAccess )
   set( xaod_access_lib xAODRootAccess )
# ... for AthAnalysisBase (Athena calls this POOLRootAccess)
else()
   set( xaod_access_deps PhysicsAnalysis/POOLRootAccess )
   set( xaod_access_lib POOLRootAccessLib )
endif()

# External dependencies:
find_package( Boost COMPONENTS program_options)
find_package( ROOT COMPONENTS Core Tree Hist )

# Executable(s) in the package:
atlas_add_executable( TruthDerivationTester
   util/TruthDerivationTester.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${xaod_access_lib} AsgTools PATInterfaces
   PMGAnalysisInterfacesLib xAODEventInfo xAODTruth xAODJet xAODMissingET ${Boost_LIBRARIES})
