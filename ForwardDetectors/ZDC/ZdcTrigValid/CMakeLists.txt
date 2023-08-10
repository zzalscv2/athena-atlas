# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ZdcTrigValid )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO Minuit Minuit2 HistPainter Graf )

# Component(s) in the package:
atlas_add_library( ZdcTrigValidLib
                   Root/*.cxx
                   PUBLIC_HEADERS ZdcTrigValid
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES  ${ROOT_LIBRARIES} AsgDataHandlesLib AsgTools TrigDecisionInterface TrigDecisionToolLib CxxUtils xAODEventInfo xAODForward xAODTrigL1Calo ZdcUtilsLib
		               PRIVATE_LINK_LIBRARIES PathResolver )

atlas_add_component( ZdcTrigValid
                     src/components/*.cxx
                     LINK_LIBRARIES ZdcTrigValidLib )

