# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AtlasCLHEPDict )

# External dependencies:
find_package( CLHEP )

atlas_add_dictionary( AtlasCLHEPDict
                      AtlasCLHEPDict/AtlasCLHEPDict.h
                      AtlasCLHEPDict/selection.xml
                      INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                      LINK_LIBRARIES ${CLHEP_LIBRARIES} )
