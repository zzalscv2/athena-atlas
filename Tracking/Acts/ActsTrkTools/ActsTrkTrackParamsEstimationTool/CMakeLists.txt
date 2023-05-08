# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTrkTrackParamsEstimationTool )

# External dependencies:
find_package( Acts COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( ActsTrkTrackParamsEstimationTool
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES
                     GaudiKernel
                     ActsCore
                     ActsTrkEventLib
                     ActsTrkToolInterfacesLib
                     ActsInteropLib
                     MagFieldConditions
                     )




