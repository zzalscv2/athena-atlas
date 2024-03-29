# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( IOVSvc )

# External dependencies:
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_library( IOVSvcLib
                   src/*.cxx
                   PUBLIC_HEADERS IOVSvc
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel GaudiKernel SGTools
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PersistentDataModel StoreGateLib xAODEventInfo RootUtils )

atlas_add_component( IOVSvc
                     src/components/*.cxx
                     LINK_LIBRARIES IOVSvcLib )

# Install files from the package:
atlas_install_joboptions( share/IOVSvc.txt share/IOVSvc.py )

# Tests in the package:
atlas_add_test( IOVSvcTool_test
                SOURCES
                test/IOVSvcTool_test.cxx
                LINK_LIBRARIES IOVSvcLib StoreGateLib TestTools
                LOG_IGNORE_PATTERN "^HistogramPersis.* INFO|^IOVSvc +DEBUG|^IOVSvcTool +DEBUG|0x[0-9a-f]{4,}"
                ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/share" )

atlas_add_test( IOVSvc_test
                SOURCES
                test/IOVSvc_test.cxx
                LINK_LIBRARIES IOVSvcLib TestTools
                LOG_IGNORE_PATTERN "^HistogramPersis.* INFO|^IOVSvc +DEBUG|^IOVSvcTool +DEBUG" 
                ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/share" )
