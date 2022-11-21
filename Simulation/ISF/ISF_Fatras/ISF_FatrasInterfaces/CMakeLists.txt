# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_FatrasInterfaces )

# Component(s) in the package:
atlas_add_library( ISF_FatrasInterfaces
                   ISF_FatrasInterfaces/*.h
                   INTERFACE
                   PUBLIC_HEADERS ISF_FatrasInterfaces
                   LINK_LIBRARIES AthContainers CxxUtils GeoPrimitives GaudiKernel ISF_Event TrkEventPrimitives TrkNeutralParameters TrkParameters TrkExInterfaces TrkExUtils )
