# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RootUtils )

# External dependencies:
find_package( Boost COMPONENTS thread )
find_package( Python COMPONENTS Development )
find_package( ROOT COMPONENTS RIO Hist Tree Core MathCore ROOTTPython
   cppyy${Python_VERSION_MAJOR}_${Python_VERSION_MINOR} )

set( ROOT_LIBRARIES_PYROOT ${ROOT_LIBRARIES} )
unset( ROOT_FOUND )
find_package( ROOT COMPONENTS RIO Hist Tree Core MathCore )

# Component(s) in the package:
atlas_generate_reflex_dictionary( dict_cpp RootUtils
   HEADER ${CMAKE_CURRENT_SOURCE_DIR}/RootUtils/RootUtilsDict.h
   SELECTION ${CMAKE_CURRENT_SOURCE_DIR}/RootUtils/selection.xml
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES CxxUtils )

atlas_add_library( RootUtils
   src/*.cxx ${dict_cpp}
   PUBLIC_HEADERS RootUtils
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${Boost_LIBRARIES} CxxUtils AthDSoCallBacks )
add_dependencies( RootUtils RootUtilsDictGen )

atlas_add_library( RootUtilsPyROOT
   src/pyroot/*.cxx
   PUBLIC_HEADERS RootUtils
   INCLUDE_DIRS ${Python_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
   ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${Python_LIBRARIES} ${Boost_LIBRARIES} ${ROOT_LIBRARIES_PYROOT}
   CxxUtils )

atlas_add_dictionary( RootUtilsPyROOTDict
   RootUtils/RootUtilsPyROOTDict.h
   RootUtils/selection_PyROOT.xml
   LINK_LIBRARIES RootUtilsPyROOT )

# Test(s) in the package:
atlas_add_test( Type_test
   SOURCES test/Type_test.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} RootUtils
   PROPERTIES TIMEOUT 300 )

atlas_add_test( TSMethodCall_test
   SOURCES test/TSMethodCall_test.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${Boost_LIBRARIES} RootUtils )

atlas_add_test( TTreePatch_test
   SCRIPT ${Python_EXECUTABLE}
          ${CMAKE_CURRENT_SOURCE_DIR}/test/TTreePatch_t.py )

atlas_add_test( WithRootErrorHandler_test
   SOURCES test/WithRootErrorHandler_test.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
   LINK_LIBRARIES ${ROOT_LIBRARIES} RootUtils )

atlas_add_test( TRandomTLS_test
   SOURCES test/TRandomTLS_test.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils RootUtils
   POST_EXEC_SCRIPT nopost.sh )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
