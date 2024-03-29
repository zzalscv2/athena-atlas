# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PixelConditionsTools )

atlas_add_library( PixelConditionsToolsLib
                   PixelConditionsTools/*.h
                   INTERFACE
                   PUBLIC_HEADERS PixelConditionsTools
                   LINK_LIBRARIES GaudiKernel InDetConditionsSummaryService )

# Component(s) in the package:
atlas_add_component( PixelConditionsTools
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaKernel GaudiKernel Identifier InDetByteStreamErrors InDetConditionsSummaryService InDetIdentifier PixelConditionsData InDetReadoutGeometry PixelReadoutGeometryLib PixelConditionsToolsLib StoreGateLib )

# Install files from the package:
atlas_install_joboptions( share/*.py )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime( share/*.txt share/*.py )

