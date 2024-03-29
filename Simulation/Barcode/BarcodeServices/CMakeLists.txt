################################################################################
# Package: BarcodeServices
################################################################################

# Declare the package name:
atlas_subdir( BarcodeServices )

# External dependencies:
find_package( ROOT COMPONENTS Cint Core Tree MathCore Hist RIO pthread MathMore Minuit Minuit2 Matrix Physics HistPainter Rint )
find_package( Boost COMPONENTS unit_test_framework)

# tag ROOTBasicLibs was not recognized in automatic conversion in cmt2cmake

# Component(s) in the package:
atlas_add_root_dictionary( BarcodeServicesLib
                           BarcodeServicesLibDictSource
                           ROOT_HEADERS BarcodeServices/BitCalculator.h BarcodeServices/TruthDressRoom.h Root/LinkDef.h
                           EXTERNAL_PACKAGES ROOT )

atlas_add_library( BarcodeServicesLib
                   Root/*.cxx
                   ${BarcodeServicesLibDictSource}
                   PUBLIC_HEADERS BarcodeServices
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel )

atlas_add_component( BarcodeServices
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps TruthUtils GaudiKernel BarcodeEventLib BarcodeInterfacesLib BarcodeServicesLib )


atlas_add_test(BitCalculator_test
  SOURCES test/BitCalculator_test.cxx Root/BitCalculator.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} BarcodeServicesLib
  POST_EXEC_SCRIPT "nopost.sh" )
  
  
# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

