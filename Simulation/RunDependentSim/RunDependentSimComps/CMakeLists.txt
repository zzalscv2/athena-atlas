# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RunDependentSimComps )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist MathCore Gpad Rint )

# Build the executables of the package:
atlas_add_executable( RunPileUpOverlayReuseToys src/RunPileUpOverlayReuseToys.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( share/RunDepTaskMaker.py share/PrintFirstJobForRun.py 
                       share/RunDepProfileGenerator.py share/RunPileUpOverlayReuseToys.C )

