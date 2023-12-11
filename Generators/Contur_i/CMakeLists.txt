# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Declare the package name:

atlas_subdir( Contur_i )


# Component(s) in the package:

find_package( contur )
find_package( Rivet )
find_package( YODA )

atlas_install_scripts( share/setup* python/contur_unpack)

# Set up the runtime environment of Contur.
set( conturEnvironment_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
   CACHE PATH "Location of conturEnvironmentConfig.cmake" )
find_package( conturEnvironment )