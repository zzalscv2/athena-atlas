# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PerfMonComps )

# External dependencies:
find_package( AIDA )
find_package( Boost )
find_package( Python COMPONENTS Development )
find_package( ROOT COMPONENTS Core ROOTTPython
   cppyy${Python_VERSION_MAJOR}_${Python_VERSION_MINOR} )
find_package( TBB )
find_package( nlohmann_json )
find_package( psutil )

# Component(s) in the package:
atlas_add_component( PerfMonComps
   src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${AIDA_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   ${Python_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${Python_LIBRARIES}
   ${CMAKE_DL_LIBS} ${TBB_LIBRARIES} AthenaBaseComps AthenaKernel RootUtilsPyROOT CxxUtils
   PerfMonEvent PerfMonKernel SGTools StoreGateLib GaudiKernel
   AthDSoCallBacks nlohmann_json::nlohmann_json)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
