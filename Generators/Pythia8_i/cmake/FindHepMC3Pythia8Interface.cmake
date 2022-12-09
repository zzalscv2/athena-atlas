# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# CMake module for finding the location of the Pythia8ToHepMC3.h header in
# an LCG release.
#

# LCG include(s).
include( LCGFunctions )

# Declare the external module.
lcg_external_module( NAME HepMC3Pythia8Interface
   INCLUDE_NAMES "Pythia8ToHepMC3.h"
   INCLUDE_SUFFIXES
      "include"
      "share/HepMC3/interfaces/pythia8/include/Pythia8"
   SEARCH_PATHS ${HEPMC3_LCGROOT} )

# Handle the standard find_package arguments.
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( HepMC3Pythia8Interface
   DEFAULT_MSG HEPMC3PYTHIA8INTERFACE_INCLUDE_DIR )
mark_as_advanced( HEPMC3PYTHIA8INTERFACE_FOUND
   HEPMC3PYTHIA8INTERFACE_INCLUDE_DIR
   HEPMC3PYTHIA8INTERFACE_INCLUDE_DIRS )

# Set up the RPM dependency.
lcg_need_rpm( hepmc3
   FOUND_NAME HEPMC3PYTHIA8INTERFACE
   VERSION_NAME HEPMC3 )
