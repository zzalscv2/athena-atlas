# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTrkToolInterfaces )

# External dependencies:
find_package( Acts COMPONENTS Core )

# External dependencies:
# Component(s) in the package:
atlas_add_library( ActsTrkToolInterfacesLib
                   ActsTrkToolInterfaces/*.h
                   INTERFACE
                   PUBLIC_HEADERS ActsTrkToolInterfaces
                   LINK_LIBRARIES
                   GaudiKernel
                   BeamSpotConditionsData
                   ActsCore
                   ActsTrkEventLib
                   SiSpacePointFormationLib
                   InDetReadoutGeometry
		   InDetIdentifier
                   xAODInDetMeasurement
                   TrkFitterUtils
                   )
