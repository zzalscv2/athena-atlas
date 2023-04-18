# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1MBTS )

# Component(s) in the package:
atlas_add_component( TrigT1MBTS
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel TileEvent 
                     TileIdentifier TrigConfInterfaces TrigConfL1Data TrigConfData TrigT1Interfaces )
