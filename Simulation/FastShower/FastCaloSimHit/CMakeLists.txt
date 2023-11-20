# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FastCaloSimHit )

# Component(s) in the package:

atlas_add_component( FastCaloSimHit
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel CaloInterfaceLib LArSimEvent TileSimEvent StoreGateLib PileUpToolsLib TileConditionsLib CaloEvent CaloIdentifier CxxUtils EventInfo TileEvent )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-ignore=F401,F821 )
