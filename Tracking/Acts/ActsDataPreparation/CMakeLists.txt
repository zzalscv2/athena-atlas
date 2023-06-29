# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsDataPreparation)

# External dependencies:
find_package(Acts COMPONENTS Core)
find_package(Eigen)

atlas_add_component( ActsDataPreparation
                     src/*.cxx
                     src/components/*.cxx
		     INCLUDE_DIRS
		        ${EIGEN_INCLUDE_DIRS}
                     LINK_LIBRARIES
		        ${EIGEN_LIBRARIES}
			ActsCore
                     	ActsToolInterfacesLib
                     	AthenaBaseComps
                     	AthenaMonitoringKernelLib
                     	GaudiKernel
		     	InDetConditionsSummaryService
		     	InDetIdentifier
			InDetReadoutGeometry
		     	InDetRawData
			SiClusterizationToolLib
		     	xAODInDetMeasurement
			ActsEventLib
			ActsEventCnvLib
			BeamSpotConditionsData
			MagFieldConditions
			SCT_ReadoutGeometry                     
			PixelReadoutGeometryLib
			SiSpacePointFormationLib
			InDetCondTools
			ReadoutGeometryBase
			)
