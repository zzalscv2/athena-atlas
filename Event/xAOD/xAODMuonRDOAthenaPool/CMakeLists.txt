# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonRDOAthenaPool )

# External dependencies:
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_poolcnv_library( xAODMuonRDOAthenaPoolCnv
   FILES xAODMuonRDO/NRPCRDOContainer.h xAODMuonRDO/NRPCRDOAuxContainer.h  xAODMuonRDO/NSWTPRDOContainer.h xAODMuonRDO/NSWTPRDOAuxContainer.h        
   TYPES_WITH_NAMESPACE xAOD::NRPCRDOContainer xAOD::NRPCRDOAuxContainer xAOD::NSWTPRDOContainer xAOD::NSWTPRDOAuxContainer
   CNV_PFX xAOD
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthenaKernel
                  AthenaPoolCnvSvcLib AthenaPoolUtilities xAODMuonRDO)
