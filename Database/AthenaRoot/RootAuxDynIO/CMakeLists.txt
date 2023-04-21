# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RootAuxDynIO )

find_package( ROOT COMPONENTS Core RIO Tree ROOTNTuple ROOTVecOps )
find_package( VDT )

# Component(s) in the package:
atlas_add_library( RootAuxDynIO
                   src/*.cxx
                   PUBLIC_HEADERS RootAuxDynIO
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}  ${VDT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES}
                     AthContainers AthContainersInterfaces AthContainersRoot  RootUtils
                   )
