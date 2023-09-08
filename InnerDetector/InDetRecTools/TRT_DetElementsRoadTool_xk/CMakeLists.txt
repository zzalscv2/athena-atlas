# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TRT_DetElementsRoadTool_xk )

# Component(s) in the package:
atlas_add_component( TRT_DetElementsRoadTool_xk
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaKernel GaudiKernel InDetRecToolInterfaces MagFieldConditions MagFieldElements StoreGateLib TRT_ReadoutGeometry TrkExInterfaces TrkGeometry TrkParameters TrkSurfaces CxxUtils )
