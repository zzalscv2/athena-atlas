# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ZdcMonitoring )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist MathCore )

# Component(s) in the package:
atlas_add_library(
    ZdcMonitoringLib
    src/*.cxx
    PUBLIC_HEADERS
        ZdcMonitoring
    INCLUDE_DIRS
        ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES
        ${ROOT_LIBRARIES}
        AthenaBaseComps
        AthenaMonitoringLib
        GaudiKernel
        StoreGateLib
        LumiBlockData
        TrigAnalysisInterfaces
        TrigDecisionInterface
        TrigDecisionToolLib
        TrigNavToolsLib
        xAODEventInfo
        xAODForward
        xAODHIEvent
        ZdcUtilsLib
        ZdcAnalysisLib
)


atlas_add_component(
    ZdcMonitoring
    src/components/*.cxx
    LINK_LIBRARIES
        ZdcMonitoringLib
)

## Install files from the package:
atlas_install_python_modules( python/*.py )
