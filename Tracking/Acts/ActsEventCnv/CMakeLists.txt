# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsEventCnv)

# External dependencies:
find_package(Acts COMPONENTS Core)

# Component(s) in the package:
atlas_add_library(ActsEventCnvLib
    ActsEventCnv/*.h
    src/*.cxx
    PUBLIC_HEADERS ActsEventCnv
    INTERFACE
    LINK_LIBRARIES
    ActsCore
    ActsGeometryLib
    GaudiKernel
    StoreGateLib
    TrkTrack
    TrkParameters
    xAODMeasurementBase
    xAODTracking
    ActsTrkEventLib
)

atlas_add_component(ActsEventCnv
    src/ActsToTrkConverterTool.cxx
    src/components/*.cxx
    LINK_LIBRARIES
    ActsEventCnvLib
    AthenaBaseComps
    AthenaKernel
    TrkGeometry
    TrkSurfaces
    TrkTrackSummary
    TrkExUtils
)

atlas_add_test(ActsEventCnvTest
    SCRIPT test/testConversion.py
    PROPERTIES TIMEOUT 900
    POST_EXEC_SCRIPT noerror.sh)
