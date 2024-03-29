# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the name of this package:
atlas_subdir( AthLinksSA )

# This package uses ROOT:
find_package( ROOT COMPONENTS Core )

# Build a library that other components can link against. The name
# is the same as for the standard AthLinks package (see README.md):
atlas_add_library( AthLinks
   AthLinks/*.h AthLinks/*.icc AthLinks/tools/*.h Root/*.cxx
   PUBLIC_HEADERS AthLinks
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccessInterfaces )

# Build a (Reflex) dictionary library:
atlas_add_dictionary( AthLinksDict
   AthLinks/AthLinksDict.h
   AthLinks/selection.xml
   LINK_LIBRARIES AthLinks )
