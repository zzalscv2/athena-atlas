# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( EventLoopGrid )

# External dependencies of the package:
find_package( ROOT COMPONENTS Core RIO PyROOT Tree )

# Library in the package:
atlas_add_root_dictionary( EventLoopGrid EventLoopGridCintDict
   ROOT_HEADERS
   EventLoopGrid/PrunDriver.h
   Root/LinkDef.h
   EXTERNAL_PACKAGES ROOT )

atlas_add_library( EventLoopGrid
   EventLoopGrid/*.h Root/*.h Root/*.cxx ${EventLoopGridCintDict}
   PUBLIC_HEADERS EventLoopGrid
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES EventLoop
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} RootCoreUtils SampleHandler PathResolver )

target_compile_definitions (EventLoopGrid PUBLIC USE_CMAKE)

# Install files from the package:
atlas_install_data( data/* )

file (GLOB util_sources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/util/[a-zA-Z0-9]*.cxx")
foreach (source ${util_sources})
  string (REGEX REPLACE "util/(.*).cxx" "\\1" util ${source})
  message (${util} ${source} LINK_LIBRARIES EventLoop)
  atlas_add_executable (${util} ${source} LINK_LIBRARIES EventLoopGrid)
endforeach (source ${util_sources})
