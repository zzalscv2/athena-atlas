# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetSurveyConstraintTool )

# External dependencies:
find_package( AIDA )

# Component(s) in the package:
atlas_add_component( InDetSurveyConstraintTool
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${AIDA_INCLUDE_DIRS}
                     LINK_LIBRARIES ${AIDA_LIBRARIES} AthenaBaseComps GeoPrimitives InDetAlignGenToolsLib Identifier EventPrimitives GaudiKernel AthenaKernel AtlasDetDescr CxxUtils DetDescrConditions InDetIdentifier InDetReadoutGeometry RegistrationServicesLib StoreGateLib )
