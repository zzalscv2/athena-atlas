# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CampaignsARTTests )

# Install the squirrels from the package
atlas_install_runtime( test/*.sh )
atlas_install_scripts( test/*.sh )
atlas_install_scripts( share/test_*.sh )
