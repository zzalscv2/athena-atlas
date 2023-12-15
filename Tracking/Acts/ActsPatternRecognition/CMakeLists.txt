# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsPatternRecognition )

# External dependencies:
find_package( Acts COMPONENTS Core )

atlas_add_component( ActsPatternRecognition
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES
                       GaudiKernel
		       StoreGateLib
		       AthenaBaseComps
		       AthenaMonitoringKernelLib
		       ActsCore
		       ActsInteropLib
		       ActsToolInterfacesLib
		       ActsGeometryInterfacesLib
		       ActsEventLib
		       ActsEventCnvLib		     
                       BeamSpotConditionsData
		       MagFieldConditions
		       SiSPSeededTrackFinderData
		       InDetRecToolInterfaces
		       )
