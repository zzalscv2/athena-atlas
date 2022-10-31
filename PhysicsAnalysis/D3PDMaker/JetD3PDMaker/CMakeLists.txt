# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( JetD3PDMaker )

# External dependencies:
find_package( Boost )

# Component(s) in the package:
atlas_add_component( JetD3PDMaker
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} CaloEvent CaloGeoHelpers CaloIdentifier AthenaBaseComps AthenaKernel Navigation StoreGateLib Identifier FourMomUtils GaudiKernel D3PDMakerInterfaces D3PDMakerUtils JetTagEvent JetTagInfo JetEvent Particle TileConditionsLib TileEvent TileIdentifier VxVertex )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )

