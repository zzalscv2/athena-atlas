# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkTruthTrackTools )

# External dependencies:
find_package( HepPDT )

# Component(s) in the package:
atlas_add_component( TrkTruthTrackTools
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${HEPPDT_INCLUDE_DIRS} 
                     LINK_LIBRARIES ${HEPPDT_LIBRARIES} AtlasHepMCLib TruthUtils AthenaBaseComps AtlasDetDescr GaudiKernel TrkSurfaces TrkEventUtils TrkEventPrimitives TrkPrepRawData TrkRIO_OnTrack TrkTrack TrkTruthData TrkTruthTrackInterfaces TrkExInterfaces TrkFitterInterfaces TrkToolInterfaces )
