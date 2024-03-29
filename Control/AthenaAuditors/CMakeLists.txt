# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthenaAuditors )

# External dependencies:
find_package( Boost )
find_package( gdb COMPONENTS bfd iberty sframe )
find_package( gperftools )
find_package( libunwind )
find_package( ZLIB )

# Component(s) in the package:
atlas_add_component( AthenaAuditors
   src/*.h src/*.cxx src/components/*.cxx
   DEFINITIONS PACKAGE=AthenaAuditors
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${GDB_INCLUDE_DIRS} ${LIBUNWIND_INCLUDE_DIRS} ${GPERFTOOLS_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS}
   LINK_LIBRARIES ${GDB_LIBRARIES} ${LIBUNWIND_LIBRARIES} ${CMAKE_DL_LIBS} ${ZLIB_LIBRARIES}
   AthenaBaseComps AthenaKernel CxxUtils GaudiKernel )
