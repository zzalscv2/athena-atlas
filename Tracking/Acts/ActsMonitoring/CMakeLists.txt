# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsMonitoring )

# Component(s) in the package:
atlas_add_component( ActsMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES 
		        AthenaBaseComps 
			AthenaKernel 
			GaudiKernel 
			Identifier 
			InDetIdentifier 
			InDetReadoutGeometry 
			StoreGateLib 
			ActsTrkEventLib 
			xAODInDetMeasurement
		     	AthenaMonitoringKernelLib 
			AthenaMonitoringLib 
			TrkTruthData 
			ActsTrkToolInterfacesLib 
			InDetRecToolInterfaces
		     	PixelReadoutGeometryLib 
			ReadoutGeometryBase 
			InDetReadoutGeometry 
			SCT_ReadoutGeometry 
			ActsTrkEventCnvLib
			TrkValHistUtils )

atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
