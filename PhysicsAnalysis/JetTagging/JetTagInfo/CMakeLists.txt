# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( JetTagInfo )

# Component(s) in the package:
atlas_add_library( JetTagInfo
                   src/*.cxx
                   PUBLIC_HEADERS JetTagInfo
                   LINK_LIBRARIES AthLinks EventPrimitives JetEvent muonEvent Particle egammaEvent VxVertex
                   PRIVATE_LINK_LIBRARIES GaudiKernel )

atlas_add_dictionary( JetTagInfoDict
                      JetTagInfo/JetTagInfoDict.h
                      JetTagInfo/selection.xml
                      LINK_LIBRARIES JetTagInfo )
