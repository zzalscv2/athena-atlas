# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PerfMonKernel )

# External dependencies:
find_package( nlohmann_json )

# Component(s) in the package:
atlas_add_library( PerfMonKernel
                   src/*.cxx
                   PUBLIC_HEADERS PerfMonKernel
                   LINK_LIBRARIES  GaudiKernel nlohmann_json::nlohmann_json )

