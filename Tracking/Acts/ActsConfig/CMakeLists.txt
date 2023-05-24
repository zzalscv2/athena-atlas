# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( TrackingComponents
   SCRIPT test/TrackingComponents.py )
