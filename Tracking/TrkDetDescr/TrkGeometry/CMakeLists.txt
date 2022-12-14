# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkGeometry )

# Component(s) in the package:
atlas_add_library( TrkGeometry
                   src/*.cxx
                   PUBLIC_HEADERS TrkGeometry
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel AthContainers CxxUtils GeoPrimitives GaudiKernel 
		   TrkDetDescrUtils TrkGeometrySurfaces TrkSurfaces TrkVolumes TrkEventPrimitives TrkNeutralParameters TrkParameters
                   PRIVATE_LINK_LIBRARIES EventPrimitives )
if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
  # Don't allow clang to make optimizations that may introduce FPEs.
  # Fixes some FPEs observed in the clang15 build.
  set_target_properties( TrkGeometry PROPERTIES COMPILE_FLAGS "-ffp-exception-behavior=strict" )
endif()

atlas_add_dictionary( TrkGeometryDict
                      TrkGeometry/TrkGeometryDict.h
                      TrkGeometry/selection.xml
                      LINK_LIBRARIES AthContainers TrkGeometry )
