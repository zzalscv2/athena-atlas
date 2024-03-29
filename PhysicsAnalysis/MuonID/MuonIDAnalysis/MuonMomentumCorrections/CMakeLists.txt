################################################################################
# Package: MuonMomentumCorrections
################################################################################

# Declare the package name:
atlas_subdir( MuonMomentumCorrections )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree RIO Physics )

# Libraries in the package:
atlas_add_library( MuonMomentumCorrectionsLib
   MuonMomentumCorrections/*.h Root/*.cxx
   PUBLIC_HEADERS MuonMomentumCorrections
   INCLUDE_DIRS  ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES  ${ROOT_LIBRARIES} AsgTools xAODMuon MuonAnalysisInterfacesLib FourMomUtils
   PATInterfaces AsgDataHandlesLib AsgMessagingLib
   PRIVATE_LINK_LIBRARIES xAODEventInfo PathResolver )

if( NOT XAOD_STANDALONE )
   atlas_add_component( MuonMomentumCorrections
      src/*.h src/*.cxx src/components/*.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps xAODMuon xAODCore MuonAnalysisInterfacesLib  FourMomUtils
      xAODBase GaudiKernel MuonSelectorToolsLib MuonMomentumCorrectionsLib StoreGateLib AsgMessagingLib PathResolver)
endif()

atlas_add_dictionary( MuonMomentumCorrectionsDict
   MuonMomentumCorrections/MuonMomentumCorrectionsDict.h
   MuonMomentumCorrections/selection.xml
   LINK_LIBRARIES MuonMomentumCorrectionsLib )

# Executable(s) in the package:
atlas_add_executable( MCAST_Tester
   util/MCAST_Tester.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess
   xAODEventInfo xAODMuon xAODCore PATInterfaces xAODCore AsgTools AsgAnalysisInterfaces FourMomUtils
   MuonSelectorToolsLib MuonMomentumCorrectionsLib  MuonAnalysisInterfacesLib AsgMessagingLib  PathResolver)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

