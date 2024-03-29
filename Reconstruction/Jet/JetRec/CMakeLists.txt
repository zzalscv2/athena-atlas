# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( JetRec )

# Extra dependencies, based on the environment:
set( extra_libs )
if( NOT GENERATIONBASE )
   list( APPEND extra_libs xAODPFlow )
   if( NOT XAOD_ANALYSIS )
      list( APPEND extra_libs AthenaMonitoringKernelLib )
   endif()
   if( NOT XAOD_STANDALONE )
      list( APPEND extra_libs StoreGateLib )
   endif()
endif()

# External dependencies:
find_package( FastJet COMPONENTS fastjetplugins fastjettools siscone siscone_spherical )
find_package( FastJetContrib COMPONENTS VariableR RecursiveTools )
find_package( GTest )

# Component(s) in the package:
atlas_add_library( JetRecLib
  JetRec/*.h Root/*.h Root/*.cxx src/*.cxx
  PUBLIC_HEADERS JetRec
  INCLUDE_DIRS ${FASTJET_INCLUDE_DIRS} ${FASTJETCONTRIB_INCLUDE_DIRS}
  LINK_LIBRARIES ${FASTJET_LIBRARIES} ${FASTJETCONTRIB_LIBRARIES} AsgDataHandlesLib AnaAlgorithmLib AsgTools  AthLinks EventShapeInterface JetEDM JetInterface xAODBase xAODCaloEvent xAODCore xAODEventInfo xAODJet xAODMuon  ${extra_libs}
  PRIVATE_LINK_LIBRARIES CxxUtils xAODTracking )

if( NOT XAOD_STANDALONE )
  atlas_add_component( JetRec
    src/components/*.cxx
    LINK_LIBRARIES ${FASTJET_LIBRARIES} AthContainers AthenaBaseComps GaudiKernel JetEDM JetInterface JetRecLib StoreGateLib xAODJet xAODMuon AnaAlgorithmLib )
endif()

if( XAOD_STANDALONE )
  atlas_add_dictionary( JetRecDict
    JetRec/JetRecDict.h
    JetRec/selection.xml
    LINK_LIBRARIES JetRecLib )
endif()

# Test(s) in the package:
atlas_add_test( TestTests
  SOURCES
  # test/all_tests.cx
  test/TestTest.cxx
  INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
  LINK_LIBRARIES ${GTEST_LIBRARIES} JetRecLib
  POST_EXEC_SCRIPT nopost.sh )

if ( NOT XAOD_STANDALONE )
  atlas_add_test(JetTests
    SOURCES
    test/JetTest.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} JetRecLib
    POST_EXEC_SCRIPT nopost.sh )
endif()

if ( NOT XAOD_STANDALONE )
  atlas_add_test(PseudoJetContainerOffline
    SOURCES
    test/PseudoJetContainerOfflineTest.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} JetRecLib
    POST_EXEC_SCRIPT nopost.sh )
endif()

if ( NOT GENERATIONBASE AND NOT XAOD_STANDALONE )
  atlas_add_test(PseudoJetContainerTrigger
    SOURCES
    test/PseudoJetContainerTriggerTest.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} JetRecLib
    POST_EXEC_SCRIPT nopost.sh )
endif()

if ( NOT XAOD_STANDALONE )
  atlas_add_test(ExtractorUnitTests
    SOURCES
    test/ExtractorTest.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} JetRecLib
    POST_EXEC_SCRIPT nopost.sh )
endif()

atlas_add_test(JetClustererTest
  SOURCES
  test/JetClustererTest.cxx
  INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
  LINK_LIBRARIES ${GTEST_LIBRARIES} JetRecLib CxxUtils
  POST_EXEC_SCRIPT nopost.sh )

if ( NOT GENERATIONBASE AND NOT XAOD_STANDALONE AND NOT XAOD_ANALYSIS )
  atlas_add_test(JetRecAlg_Test
    SCRIPT share/JetRecAlgTestCfg.py
    POST_EXEC_SCRIPT nopost.sh )
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )
