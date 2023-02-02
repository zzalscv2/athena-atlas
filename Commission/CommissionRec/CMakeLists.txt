# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CommissionRec )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_component( CommissionRec
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel CommissionEvent LArSimEvent TrackRecordLib )
