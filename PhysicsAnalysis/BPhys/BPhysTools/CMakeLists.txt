#
# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
#

# $Id: CMakeLists.txt 805745 2017-05-31 17:23:48Z wwalko $
#
# Build configuration for the package.
#
#********************************************
set(extra_dep)
if( XAOD_STANDALONE )
  set(extra_dep)
else()
  set( extra_dep
         GaudiKernel
         Control/AthenaKernel
     )
endif()
#********************************************
set(extra_libs)
if( XAOD_STANDALONE )
  set(extra_libs)
else()
  set( extra_libs
         GaudiKernel
         AthenaKernel
     )
endif()
#********************************************
# The name of the package:
atlas_subdir( BPhysTools )


# Used external(s):
find_package( ROOT COMPONENTS Core Physics Matrix )

# Build the main library of the package: 
atlas_add_library( BPhysToolsLib
		   BPhysTools/*.h Root/*.cxx src/*.cxx
		   PUBLIC_HEADERS BPhysTools
		   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
		   LINK_LIBRARIES ${ROOT_LIBRARIES} 
		   xAODTracking
		   xAODBPhysLib
		   AsgTools
       ${extra_libs}
		   xAODEventInfo
		   )

if(NOT XAOD_STANDALONE)
atlas_add_component( BPhysTools
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} 
		                 xAODTracking
		                 xAODBPhysLib
		                 AsgTools
                     ${extra_libs}
                     BPhysToolsLib
		                 xAODEventInfo
		                 )
endif()

# Build the dictionary 
atlas_add_dictionary( BPhysToolsDict
  BPhysTools/BPhysToolsDict.h
  BPhysTools/selection.xml
  LINK_LIBRARIES BPhysToolsLib
  )  


# Executables in util subdirectory
file (GLOB util_sources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/util/*.cxx")
foreach (source ${util_sources})
   string (REGEX REPLACE "util/(.*).cxx" "\\1" util ${source})
   atlas_add_executable (${util} ${source} LINK_LIBRARIES BPhysToolsLib)
endforeach (source ${util_sources})
 
