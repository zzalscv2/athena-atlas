# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Pre-configuration setup for AthAnalysis.
#

# Child projects should not use the ATLAS CPack install scripts.
set( ATLAS_USE_CUSTOM_CPACK_INSTALL_SCRIPT FALSE
   CACHE BOOL "Don't use the custom ATLAS CPack install scripts" )
