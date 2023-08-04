# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetSimData )

# Component(s) in the package:
atlas_add_library( InDetSimData
                   src/InDetSimData.cxx
                   PUBLIC_HEADERS InDetSimData
                   LINK_LIBRARIES AthenaKernel Identifier GeneratorObjects AthAllocators)

atlas_add_dictionary( InDetSimDataDict
                      InDetSimData/InDetSimDataDict.h
                      InDetSimData/selection.xml
                      LINK_LIBRARIES InDetSimData )
