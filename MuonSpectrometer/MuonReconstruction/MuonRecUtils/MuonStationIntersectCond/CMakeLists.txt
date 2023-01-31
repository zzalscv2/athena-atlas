# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( MuonStationIntersectCond )

# External dependencies.
find_package( ROOT COMPONENTS Core )

# Component(s) in the package.
atlas_add_library( MuonStationIntersectCondLib
   MuonStationIntersectCond/*.h src/*.cxx
   PUBLIC_HEADERS MuonStationIntersectCond
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES AthenaKernel GeoPrimitives Identifier MuonCondData MuonIdHelpersLib MuonReadoutGeometry TrkDriftCircleMath
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel GeoModelUtilities StoreGateLib )

atlas_add_component( MuonStationIntersectCond
   src/components/*.cxx
   LINK_LIBRARIES MuonStationIntersectCondLib )
