# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SCT_G4_SD )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( GTest )
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelRead GeoModelDBManager )

# Component(s) in the package:
atlas_add_library( SCT_G4_SDLib
                   src/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS} ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} ${GEOMODEL_LIBRARIES} G4AtlasToolsLib InDetSimEvent MCTruth StoreGateLib GeoModelInterfaces GeoPrimitives )

atlas_add_library( SCT_G4_SD
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_LINK_LIBRARIES SCT_G4_SDLib )

# Test(s) in the package:
atlas_add_test( SCT_G4_SDToolConfig_test
                SCRIPT test/SCT_G4_SDToolConfig_test.py
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( SctSensorSD_gtest
                SOURCES test/SctSensorSD_gtest.cxx
                INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} G4AtlasToolsLib InDetSimEvent MCTruth StoreGateLib TestTools SCT_G4_SDLib
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( SctSensor_CTB_gtest
                SOURCES test/SctSensor_CTB_gtest.cxx
                INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} G4AtlasToolsLib InDetSimEvent MCTruth StoreGateLib TestTools SCT_G4_SDLib
                POST_EXEC_SCRIPT nopost.sh )


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/optionForTest.txt )
