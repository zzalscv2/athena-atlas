# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCondSvc )

# Component(s) in the package:
atlas_add_library( MuonCondSvcLib
                   src/*.cxx
                   PUBLIC_HEADERS MuonCondSvc
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel AthenaPoolUtilities GaudiKernel MuonAlignmentData MuonCondInterface MuonIdHelpersLib StoreGateLib
                   PRIVATE_LINK_LIBRARIES Identifier )

atlas_add_component( MuonCondSvc
                     src/components/*.cxx
                     LINK_LIBRARIES MuonCondSvcLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
