# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimLRT )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist RIO )

# Component(s) in the package:
atlas_add_library( FPGATrackSimLRTLib
    src/*.cxx FPGATrackSimLRT/*.h
    PUBLIC_HEADERS         FPGATrackSimLRT
    LINK_LIBRARIES         AthenaBaseComps GaudiKernel FPGATrackSimBanksLib FPGATrackSimHoughLib FPGATrackSimObjectsLib
    PRIVATE_INCLUDE_DIRS   ${ROOT_INCLUDE_DIRS}
    PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} FPGATrackSimMapsLib
)

atlas_add_component( FPGATrackSimLRT
    src/components/*.cxx
    LINK_LIBRARIES              FPGATrackSimLRTLib
)

