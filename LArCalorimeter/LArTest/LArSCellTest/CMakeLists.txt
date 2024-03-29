# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArSCellTest )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( LArSCellTest
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} GaudiKernel StoreGateLib CxxUtils
                     CaloDetDescrLib CaloEvent CaloIdentifier AthenaBaseComps xAODEgamma xAODEventInfo
                     xAODTracking xAODTrigger xAODTrigCalo xAODTrigL1Calo 
                     xAODTruth LArCablingLib LArRawEvent RecoToolInterfaces TruthUtils)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )


