# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetGNNTracking )

# Component(s) in the package:
atlas_add_component( InDetGNNTracking
    src/*.cxx
    src/components/*.cxx
    LINK_LIBRARIES AthenaBaseComps GaudiKernel InDetRecToolInterfaces
    PathResolver AthOnnxruntimeServiceLib AthOnnxruntimeUtilsLib
    TrkTrack StoreGateLib TrkExInterfaces TrkSpacePoint TrkFitterInterfaces
)
