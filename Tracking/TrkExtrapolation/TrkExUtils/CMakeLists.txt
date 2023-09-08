# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkExUtils )

find_package( GTest )

# Component(s) in the package:
atlas_add_library( TrkExUtils
                   src/*.cxx
                   PUBLIC_HEADERS TrkExUtils
                   LINK_LIBRARIES GeoPrimitives EventPrimitives GaudiKernel TrkDetDescrUtils TrkGeometry
		           TrkSurfaces TrkEventPrimitives TrkMaterialOnTrack TrkNeutralParameters TrkParameters CxxUtils
                   TrkPatternParameters )

atlas_add_test( MaterialInteraction
   SOURCES test/MaterialInteractionTest.cxx
   LINK_LIBRARIES
   TrkExUtils
   TrkGeometry
   TrkEventPrimitives
   GTest::gtest_main
   POST_EXEC_SCRIPT noerror.sh
)
