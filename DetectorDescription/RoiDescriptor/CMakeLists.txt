# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RoiDescriptor )

# Component(s) in the package:
atlas_add_library( RoiDescriptor
                   Root/RoiDescriptor.cxx
                   Root/RoiUtil.cxx
                   PUBLIC_HEADERS RoiDescriptor
                   LINK_LIBRARIES IRegionSelector )
if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
  # Don't allow clang to make optimizations that may introduce FPEs.
  # Fixes some FPEs observed in the clang15 build.
  set_target_properties( RoiDescriptor PROPERTIES COMPILE_FLAGS "-ffp-exception-behavior=strict" )
endif()

atlas_add_dictionary( RoiDescriptorDict
                      RoiDescriptor/RoiDescriptorDict.h
                      RoiDescriptor/selection.xml
                      LINK_LIBRARIES RoiDescriptor )
