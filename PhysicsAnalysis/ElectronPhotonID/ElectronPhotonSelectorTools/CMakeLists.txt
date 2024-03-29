# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ElectronPhotonSelectorTools )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore Hist MathMore )
find_package( lwtnn )
find_package( Eigen )

# Component(s) in the package:
atlas_add_library( ElectronPhotonSelectorToolsLib
  ElectronPhotonSelectorTools/*.h Root/*.cxx
  PUBLIC_HEADERS ElectronPhotonSelectorTools
  PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  INCLUDE_DIRS ${LWTNN_INCLUDE_DIRS}  ${EIGEN_INCLUDE_DIRS}
  LINK_LIBRARIES EgammaAnalysisInterfacesLib AsgTools xAODEgamma
  xAODTracking xAODEventInfo MVAUtils
  xAODHIEvent PATCoreAcceptLib AsgDataHandlesLib
  PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AsgMessagingLib FourMomUtils 
  xAODCaloEvent PathResolver EgammaAnalysisHelpersLib
  ${LWTNN_LIBRARIES} ${EIGEN_LIBRARIES} EventPrimitives )

if( NOT XAOD_STANDALONE )
   atlas_add_component( ElectronPhotonSelectorTools
      src/components/*.cxx
      LINK_LIBRARIES ElectronPhotonSelectorToolsLib )
endif()

atlas_add_dictionary( ElectronPhotonSelectorToolsDict
  ElectronPhotonSelectorTools/ElectronPhotonSelectorToolsCoreDict.h
  ElectronPhotonSelectorTools/selectionCore.xml)

atlas_add_dictionary( ElectronPhotonSelectorToolsPythonDict
  ElectronPhotonSelectorTools/ElectronPhotonSelectorToolsPythonDict.h
  ElectronPhotonSelectorTools/selectionPython.xml
  LINK_LIBRARIES ElectronPhotonSelectorToolsLib )

atlas_add_test( ut_EgammaPIDDefs_pytest
SCRIPT test/ut_EgammaPIDDefs_pytest.py )

atlas_add_test( ut_LikelihoodEnums_pytest
SCRIPT test/ut_LikelihoodEnums_pytest.py )

atlas_add_executable( EGIdentification_mem_check
   util/EGIdentification_mem_check.cxx
   LINK_LIBRARIES AsgMessagingLib AsgTools EgammaAnalysisInterfacesLib )

if( XAOD_STANDALONE )
   atlas_add_executable( EGIdentification_testEGIdentificationPoints
      util/testEGIdentificationPoints.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AsgMessagingLib AsgTools
      EgammaAnalysisInterfacesLib ElectronPhotonSelectorToolsLib
      PATCoreAcceptLib xAODCore xAODEgamma )
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( data/*.conf )
