# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonRegionSelector )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_component( MuonRegionSelector
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps IRegionSelector
                     RegSelLUT GaudiKernel Identifier CSCcablingLib StoreGateLib MuonCablingData
                     MuonCondData MuonTGC_CablingLib RPC_CondCablingLib
                     MuonReadoutGeometry MuonNSWCommonDecode MuonAGDDDescription  )
