# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTrkSpacePointFormationTool )

# External dependencies:
find_package(Acts COMPONENTS Core)

atlas_add_component( ActsTrkSpacePointFormationTool
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES
                     ActsCore
                     AthenaBaseComps
                     GaudiKernel
                     ActsTrkEventLib
                     ActsTrkEventCnvLib
                     ActsTrkToolInterfacesLib
                     InDetCondTools
                     PixelReadoutGeometryLib
                     ReadoutGeometryBase
                     SCT_ReadoutGeometry
                     SiSpacePointFormationLib
                     xAODInDetMeasurement
                     )
