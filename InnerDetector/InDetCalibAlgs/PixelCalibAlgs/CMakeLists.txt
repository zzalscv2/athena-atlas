# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PixelCalibAlgs )

# External dependencies:
find_package( ROOT COMPONENTS Graf Core Tree MathCore Hist RIO MathMore Physics
   Matrix Gpad )
   
find_package( Boost COMPONENTS unit_test_framework)
find_package( COOL COMPONENTS CoolKernel CoolApplication )
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )


# Libraries in the package:
atlas_add_library( PixelCalibAlgsLib
   PixelCalibAlgs/*.h src/*.cxx ChargeCalibration/common/*.h ChargeCalibration/common/*.cxx
   PUBLIC_HEADERS PixelCalibAlgs
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps CxxUtils GaudiKernel InDetConditionsSummaryService 
   InDetPrepRawData InDetReadoutGeometry PixelGeoModelLib PixelReadoutGeometryLib PixelConditionsData StoreGateLib
   PRIVATE_LINK_LIBRARIES AthenaPoolUtilities InDetIdentifier InDetRawData )

atlas_add_component( PixelCalibAlgs
   src/components/*.cxx
   LINK_LIBRARIES GaudiKernel InDetByteStreamErrors PixelConditionsData PixelCalibAlgsLib )

atlas_add_executable( makeInactiveModuleList
   Application/makeInactiveModuleList.C
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils PixelConditionsData PixelCalibAlgsLib )
   
atlas_add_executable( PixelCalib
   ChargeCalibration/pixel/PixelCalib.C
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils PixelCalibAlgsLib )
   
atlas_add_executable( MakeReferenceFile
   ChargeCalibration/pixel/MakeReferenceFile.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${COOL_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils ${COOL_LIBRARIES} ${CORAL_LIBRARIES} )


   
atlas_add_test( PixelMapping_test
   SOURCES ChargeCalibration/common/PixelMapping.cxx test/PixelMapping_test.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES} PathResolver
   POST_EXEC_SCRIPT "nopost.sh"
)

atlas_install_runtime ( share/HashVSdcsID.dat share/table_Run2.txt share/PixelMapping_Run2.dat ChargeCalibration/common/mapping.csv)
