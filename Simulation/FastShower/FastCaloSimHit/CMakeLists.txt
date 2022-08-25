# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FastCaloSimHit )

# Component(s) in the package:
atlas_add_library( FastCaloSimHitLib
                   src/*.cxx
                   PUBLIC_HEADERS FastCaloSimHit
                   LINK_LIBRARIES AthenaBaseComps GaudiKernel CaloInterfaceLib LArSimEvent TileSimEvent StoreGateLib PileUpToolsLib TileConditionsLib
                   PRIVATE_LINK_LIBRARIES CaloEvent CaloIdentifier CxxUtils EventInfo TileEvent )

atlas_add_component( FastCaloSimHit
                     src/components/*.cxx
                     LINK_LIBRARIES FastCaloSimHitLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-ignore=F401,F821 )
