# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsTrkClusterization)

# External dependencies:

atlas_add_component( ActsTrkClusterization
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES
                     ActsTrkToolInterfacesLib
                     AthenaBaseComps
                     AthenaMonitoringKernelLib
                     GaudiKernel
		     InDetConditionsSummaryService
		     InDetIdentifier
		     InDetRawData
		     xAODInDetMeasurement
                     )

