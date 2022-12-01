# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RegionSelector )

# Component(s) in the package:
atlas_add_library( RegionSelectorLib
                   src/*.cxx
                   PUBLIC_HEADERS RegionSelector
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel GaudiKernel IRegionSelector Identifier RegSelLUT
                   PRIVATE_LINK_LIBRARIES PathResolver )

atlas_add_component( RegionSelector
                     src/components/*.cxx
                     LINK_LIBRARIES RegionSelectorLib )

atlas_add_component( RegionSelectorTest
                     test/components/*.cxx
                     test/*.cxx
                     LINK_LIBRARIES RegionSelectorLib RoiDescriptor )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Define unit tests
foreach(det Pixel SCT TRT Tile)
   atlas_add_test( RegSelTool_${det}
                   SCRIPT python -m RegionSelector.RegSelToolConfig ${det}
                   POST_EXEC_SCRIPT noerror.sh )
endforeach()
# TODO: Fix and enable the tests below (move them to the loop above), ATLASRECTS-7289
foreach(det LAr RPC TGC MDT STGC MM)
   atlas_add_test( RegSelTool_${det}
                   SCRIPT python -m RegionSelector.RegSelToolConfig ${det}
                   POST_EXEC_SCRIPT noerror.sh
                   PROPERTIES DISABLED True )
endforeach()
# TODO: Once all detector configs are fixed, enable the combined test, ATLASRECTS-7289
atlas_add_test( RegSelTool_All
                SCRIPT python -m RegionSelector.RegSelToolConfig Pixel SCT TRT LAr Tile RPC TGC MDT STGC MM
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 300 DISABLED True )
