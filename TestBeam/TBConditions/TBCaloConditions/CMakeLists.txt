# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TBCaloConditions )

# Component(s) in the package:
atlas_add_library( TBCaloConditionsLib
                   TBCaloConditions/*.h
                   INTERFACE
                   PUBLIC_HEADERS TBCaloConditions
                   LINK_LIBRARIES GaudiKernel )

atlas_add_component( TBCaloConditions
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES GaudiKernel AthenaBaseComps StoreGateLib AthenaPoolUtilities CondDBObjects CxxUtils TBCaloConditionsLib )

# Install files from the package:
atlas_install_joboptions( share/*.py )

