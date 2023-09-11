# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigConfHLTData )

# Component(s) in the package:
atlas_add_library( TrigConfHLTData
   TrigConfHLTData/*.h Root/*.cxx
   PUBLIC_HEADERS TrigConfHLTData
   LINK_LIBRARIES TrigConfL1Data TrigConfHLTUtilsLib )
