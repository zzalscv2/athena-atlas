# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ZdcAnalysis )

# Declare the package name:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO Minuit Minuit2 HistPainter Graf Matrix )
find_package( nlohmann_json )

# Component(s) in the package:
atlas_add_library( ZdcAnalysisLib
                   Root/*.cxx
                   PUBLIC_HEADERS ZdcAnalysis
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} nlohmann_json::nlohmann_json AsgDataHandlesLib AsgTools CxxUtils xAODEventInfo xAODForward xAODTrigL1Calo TrigDecisionInterface TrigDecisionToolLib ZdcUtilsLib
		                   PRIVATE_LINK_LIBRARIES PathResolver )

atlas_add_component( ZdcAnalysis
		     src/components/*.cxx
		     LINK_LIBRARIES ZdcAnalysisLib )

atlas_install_data( data/* )
