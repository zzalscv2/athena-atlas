# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigMuonBackExtrapolator )

# Component(s) in the package:
atlas_add_component( TrigMuonBackExtrapolator
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps TrigMuonToolInterfaces )

