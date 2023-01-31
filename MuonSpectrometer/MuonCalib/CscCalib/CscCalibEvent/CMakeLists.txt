# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CscCalibEvent )

# Component(s) in the package:
atlas_add_library( CscCalibEvent
                   src/*.cxx
                   PUBLIC_HEADERS CscCalibEvent
                   LINK_LIBRARIES AthContainers AthenaKernel Identifier MuonIdHelpersLib EventContainers )

atlas_add_dictionary( CscCalibEventDict
                      CscCalibEvent/CscCalibEventDict.h
                      CscCalibEvent/selection.xml
                      LINK_LIBRARIES CscCalibEvent )
