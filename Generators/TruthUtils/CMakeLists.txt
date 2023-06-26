# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TruthUtils )

# Decide what to do about HepMC(3) / AtlasHepMC.
set( extra_libs )
if( NOT XAOD_STANDALONE )
   list( APPEND extra_libs AtlasHepMCLib )
   message( STATUS "Linking TruthUtils against AtlasHepMC" )
endif()

# Component(s) in the package:
atlas_add_library( TruthUtils
   INTERFACE
   PUBLIC_HEADERS TruthUtils
   LINK_LIBRARIES  ${extra_libs}
   )

# Install files from the package:
atlas_install_runtime( share/*.txt )

