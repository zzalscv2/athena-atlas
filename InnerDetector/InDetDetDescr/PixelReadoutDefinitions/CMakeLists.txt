# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PixelReadoutDefinitions )

find_package( Boost COMPONENTS unit_test_framework)

# Component(s) in the package:
atlas_add_library( PixelReadoutDefinitionsLib
                   PixelReadoutDefinitions/*.h
                   INTERFACE
                   PUBLIC_HEADERS PixelReadoutDefinitions )
                   
atlas_add_test( PixelReadoutDefinitions_test
  SOURCES test/PixelReadoutDefinitions_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} PixelReadoutDefinitionsLib
  POST_EXEC_SCRIPT "nopost.sh" )
