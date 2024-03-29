################################################################################
# Package: TauAnalysisTools
################################################################################

# Declare the package name:
atlas_subdir( TauAnalysisTools )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )

# Libraries in the package:
if( XAOD_STANDALONE )
   atlas_add_library( TauAnalysisToolsLib
      TauAnalysisTools/*.h Root/*.cxx
	  PUBLIC_HEADERS TauAnalysisTools
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODEgamma xAODMuon xAODJet
      xAODTau xAODTracking xAODTruth TruthUtils PATCoreLib PATInterfaces
      tauRecToolsLib MCTruthClassifierLib AsgAnalysisInterfaces
      PRIVATE_LINK_LIBRARIES AthLinks xAODEventInfo xAODMetaData PathResolver TruthUtils )
else()
   atlas_add_library( TauAnalysisToolsLib
      TauAnalysisTools/*.h Root/*.cxx
	  PUBLIC_HEADERS TauAnalysisTools
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODEgamma xAODMuon xAODJet
      xAODTau xAODTracking xAODTruth TruthUtils PATCoreLib PATInterfaces
      ElectronPhotonSelectorToolsLib tauRecToolsLib MCTruthClassifierLib
      AsgAnalysisInterfaces
      PRIVATE_LINK_LIBRARIES AthLinks xAODEventInfo xAODMetaData PathResolver TruthUtils )
endif()

if( NOT XAOD_STANDALONE )
   atlas_add_component( TauAnalysisTools
      src/*.cxx src/components/*.cxx
      LINK_LIBRARIES AthenaBaseComps AsgTools xAODTau GaudiKernel
      TauAnalysisToolsLib )
endif()

atlas_add_dictionary( TauAnalysisToolsDict
   TauAnalysisTools/TauAnalysisToolsDict.h
   TauAnalysisTools/selection.xml
   LINK_LIBRARIES TauAnalysisToolsLib )

atlas_add_dictionary( TauAnalysisToolsEnumDict
   TauAnalysisTools/Enums.h
   TauAnalysisTools/selection-enums.xml )

# Executable(s) in the package:
if( XAOD_STANDALONE )
   atlas_add_executable( TauAnalysisToolsExample
      util/TauAnalysisToolsExample.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODCore xAODEventInfo
      xAODTau AsgTools PileupReweightingLib TauAnalysisToolsLib )
endif()

# Test(s) in the package:
if( NOT XAOD_STANDALONE )
   atlas_add_test( ut_TauAnalysisTools_test
      SOURCES test/ut_TauAnalysisTools_test.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AthAnalysisBaseCompsLib POOLRootAccessLib
      GaudiKernel AsgTools xAODTau TauAnalysisToolsLib CxxUtils
      POST_EXEC_SCRIPT nopost.sh )
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py )
