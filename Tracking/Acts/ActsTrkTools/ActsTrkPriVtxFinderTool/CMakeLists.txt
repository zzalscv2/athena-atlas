# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTrkPriVtxFinderTool )

# External dependencies:
find_package( Acts COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( ActsTrkPriVtxFinderTool
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES 
                     ActsCore
                     ActsGeometryInterfacesLib
                     ActsGeometryLib
                     ActsInteropLib
                     AthenaBaseComps
                     BeamSpotConditionsData
                     GaudiKernel
                     InDetRecToolInterfaces
                     InDetTrackSelectionToolLib
                     TrkLinks
                     TrkParticleBase
                     TrkTrack
                     TrkTrackLink
                     xAODTracking )
