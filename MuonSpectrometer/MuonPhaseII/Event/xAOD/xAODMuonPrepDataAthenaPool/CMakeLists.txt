# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonPrepDataAthenaPool )

# External dependencies:
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_poolcnv_library( xAODMuonPrepDataAthenaPoolCnv
   FILES xAODMuonPrepData/MdtDriftCircleContainer.h xAODMuonPrepData/MdtDriftCircleAuxContainer.h xAODMuonPrepData/RpcStripContainer.h xAODMuonPrepData/RpcStripAuxContainer.h xAODMuonPrepData/TgcStripContainer.h xAODMuonPrepData/TgcStripAuxContainer.h xAODMuonPrepData/MMClusterContainer.h xAODMuonPrepData/MMClusterAuxContainer.h         
   TYPES_WITH_NAMESPACE xAOD::MdtDriftCircleContainer xAOD::MdtDriftCircleAuxContainer xAOD::RpcStripContainer xAOD::RpcStripAuxContainer xAOD::TgcStripContainer xAOD::TgcStripAuxContainer xAOD::MMClusterContainer xAOD::MMClusterAuxContainer
   CNV_PFX xAOD
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthenaKernel
                  AthenaPoolCnvSvcLib AthenaPoolUtilities xAODMuonPrepData)
