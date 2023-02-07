# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TBDetDescrAlg )

# Component(s) in the package:
atlas_add_library( TBDetDescrAlgLib
                   src/TB*.cxx
                   PUBLIC_HEADERS TBDetDescrAlg
                   LINK_LIBRARIES AthenaBaseComps
                   PRIVATE_LINK_LIBRARIES DetDescrCnvSvcLib GaudiKernel TBDetDescr )

atlas_add_component( TBDetDescrAlg
                     src/components/*.cxx
                     LINK_LIBRARIES TBDetDescrAlgLib )

# Install files from the package:
atlas_install_joboptions( share/*.py )
# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

