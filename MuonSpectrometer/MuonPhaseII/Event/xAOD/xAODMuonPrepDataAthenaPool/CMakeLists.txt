# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonPrepDataAthenaPool )

# External dependencies:
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_poolcnv_library( xAODMuonPrepDataAthenaPoolCnv
   FILES xAODMuonPrepData/MdtDriftCircleContainer.h xAODMuonPrepData/MdtDriftCircleAuxContainer.h        
   TYPES_WITH_NAMESPACE xAOD::MdtDriftCircleContainer xAOD::MdtDriftCircleAuxContainer
   CNV_PFX xAOD
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthenaKernel
                  AthenaPoolCnvSvcLib AthenaPoolUtilities xAODMuonPrepData)
