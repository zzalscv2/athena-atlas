# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AFP_GlobReco )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore Hist RIO )

atlas_add_component( AFP_GlobReco
                     src/*.cxx src/components/*.cxx AFP_GlobReco/*.h 
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                     LINK_LIBRARIES ${ROOT_LIBRARIES}  AthContainers AthenaBaseComps AthenaKernel GaudiKernel PathResolver StoreGateLib xAODForward )


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )   

atlas_install_runtime( share/*.txt )


