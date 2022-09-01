# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkBPhys )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore )
find_package( HepPDT )

# Component(s) in the package:
atlas_add_component( DerivationFrameworkBPhys
   DerivationFrameworkBPhys/*.h src/*.cxx src/components/*.cxx
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${HEPPDT_INCLUDE_DIRS}
   LINK_LIBRARIES xAODMuon AthenaBaseComps JpsiUpsilonToolsLib
   MuonSelectorToolsLib TrkVertexAnalysisUtilsLib
   xAODTracking xAODBPhysLib AthenaBaseComps AthenaKernel RecoToolInterfaces EventPrimitives
   DerivationFrameworkInterfaces BPhysToolsLib TrackVertexAssociationToolLib
   xAODBase xAODMetaData  AsgTools CaloInterfaceLib TrkVertexFitterInterfaces
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} ${HEPPDT_LIBRARIES} TrackToCaloLib InDetConversionFinderToolsLib
   xAODEventInfo AthenaPoolUtilities xAODPrimitives TrigDecisionToolLib
   ITrackToVertex TrkVKalVrtFitterLib
   InDetTrackSelectionToolLib  InDetV0FinderLib TrkV0FitterLib )


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8})
atlas_install_joboptions( share/*.py )
