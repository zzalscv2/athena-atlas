# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HIEventUtils )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( HIEventUtilsLib
                   Root/*.cxx
                   PUBLIC_HEADERS HIEventUtils
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgDataHandlesLib AsgTools CaloGeoHelpers InDetTrackSelectionToolLib PATCoreAcceptLib xAODForward xAODHIEvent xAODTracking xAODTrigL1Calo  
                   PRIVATE_LINK_LIBRARIES AsgMessagingLib CxxUtils xAODEventInfo )

if (NOT XAOD_STANDALONE)
  atlas_add_component( HIEventUtils
                       src/*.cxx src/components/*.cxx
                       LINK_LIBRARIES AthenaBaseComps GaudiKernel HIEventUtilsLib
                       PRIVATE_LINK_LIBRARIES PathResolver)
endif ()

# Install files from the package:
atlas_install_joboptions( share/*.py )
