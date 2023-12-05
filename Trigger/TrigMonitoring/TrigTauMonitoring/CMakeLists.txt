# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigTauMonitoring )

find_package( Boost )

# Component(s) in the package:
atlas_add_component( TrigTauMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaMonitoringKernelLib AthenaMonitoringLib CxxUtils xAODTau xAODTrigger xAODTruth LArRecEvent TruthUtils ${Boost_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
