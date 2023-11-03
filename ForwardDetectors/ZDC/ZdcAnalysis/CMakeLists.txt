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
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgDataHandlesLib AsgTools CxxUtils xAODEventInfo xAODForward xAODTrigL1Calo TrigDecisionInterface TrigDecisionToolLib ZdcUtilsLib
                   PRIVATE_LINK_LIBRARIES PathResolver nlohmann_json::nlohmann_json )

atlas_add_dictionary (ZdcAnalysisDict
		     	ZdcAnalysis/ZdcAnalysisDict.h
			ZdcAnalysis/selection.xml
			LINK_LIBRARIES ZdcAnalysisLib)

if (NOT XAOD_STANDALONE)
   atlas_add_component( ZdcAnalysis
      src/components/*.cxx
      INCLUDE_DIRS
      LINK_LIBRARIES ZdcAnalysisLib )
endif()

atlas_install_data( data/* )
