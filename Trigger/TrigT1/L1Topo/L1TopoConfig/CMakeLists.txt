# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( L1TopoConfig )

# External dependencies:
find_package( Oracle )     # Oracle client libraries for DB access
find_package( cx_Oracle )  # Oracle python client

# Component(s) in the package:
atlas_add_library( L1TopoConfig
                   Root/*.cxx
                   PUBLIC_HEADERS L1TopoConfig
                   LINK_LIBRARIES TrigConfBase L1TopoCommon )

