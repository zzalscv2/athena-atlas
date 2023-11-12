# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTruth )

# External dependencies:
find_package( Acts COMPONENTS Core )
# find_package( ROOT COMPONENTS Core Tree RIO )

atlas_add_component( ActsTruth
                     src/*.cxx
                     src/components/*.cxx
		     INCLUDE_DIRS
                     LINK_LIBRARIES
                       GaudiKernel
                       StoreGateLib
                       AthenaBaseComps
                       Identifier
                       InDetSimData
                       GeneratorObjects
                       xAODInDetMeasurement
                       xAODMeasurementBase
                       xAODTruth
                       ActsEventLib
                       ActsGeometryLib
                       ActsInteropLib
                     )
