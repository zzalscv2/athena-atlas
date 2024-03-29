# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigHLTJetHypo )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( TrigHLTJetHypoLib
                   src/*.cxx
                   src/TrigHLTJetHypoUtils/*.cxx
                   PUBLIC_HEADERS TrigHLTJetHypo
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES DecisionHandlingLib GaudiKernel xAODJet FourMomUtils
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps AthenaMonitoringKernelLib DecisionHandlingLib TrigCompositeUtilsLib TrigSteeringEvent xAODEventInfo xAODTrigger CaloEvent TrigBjetHypoLib )

atlas_add_component( TrigHLTJetHypo
                     src/components/*.cxx
                     LINK_LIBRARIES TrigHLTJetHypoLib )

# Install files from the package:
atlas_install_python_modules( python/*.py
			      POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-select=ATL901)  # 901: check for print()

# Tests:

atlas_add_test( TrigHLTJetHypoTool SCRIPT python -m TrigHLTJetHypo.TrigJetHypoToolConfig
				POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( hypotoolTool SCRIPT python -m TrigHLTJetHypo.hypoToolTests
			     	POST_EXEC_SCRIPT nopost.sh )
