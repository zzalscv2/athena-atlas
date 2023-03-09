# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTrkEventCnv )

# External dependencies:
find_package( Acts COMPONENTS Core  )

# Component(s) in the package:
atlas_add_library( ActsTrkEventCnvLib
                   ActsTrkEventCnv/*.h
                   src/*.cxx
                   PUBLIC_HEADERS ActsTrkEventCnv
                   INTERFACE
                   LINK_LIBRARIES
                   ActsCore
                   ActsGeometryLib
                   AthenaBaseComps
                   AthenaKernel
                   GaudiKernel
                   Identifier
                   StoreGateLib
                   TrkGeometry
                   TrkSurfaces
                   TrkTrackSummary
                   TrkTrack
                   xAODMeasurementBase
                   TrkExUtils
                   xAODTracking
                   )

atlas_add_component( ActsTrkEventCnv
                     src/ActsToTrkConverterTool.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES
                     ActsTrkEventCnvLib
                     )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( ActsTrkEventCnvTest
SCRIPT python -m ActsTrkEventCnv.ActsTrkEventCnvConfig --threads=1
PROPERTIES TIMEOUT 900
POST_EXEC_SCRIPT noerror.sh )