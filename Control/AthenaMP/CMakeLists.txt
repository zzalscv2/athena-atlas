# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthenaMP )

# Component(s) in the package:
atlas_add_component( AthenaMP
                     src/*.cxx
                     src/components/*.cxx
                     src/memory-profiler/getPss.cc
                     LINK_LIBRARIES  AthenaBaseComps AthenaInterprocess AthenaMPToolsLib GaudiKernel StoreGateLib )

atlas_add_executable( getSharedMemory
                      src/memory-profiler/getSharedMemory.cc
                      src/memory-profiler/getPss.cc
                      LINK_LIBRARIES CxxUtils )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
