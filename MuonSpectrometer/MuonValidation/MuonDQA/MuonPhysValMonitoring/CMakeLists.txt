################################################################################
# Package: MuonPhysValMonitoring
################################################################################

# Declare the package name:
atlas_subdir( MuonPhysValMonitoring )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( MuonPhysValMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AtlasHepMCLib GaudiKernel AthenaMonitoringLib xAODBase xAODCore xAODEventInfo xAODMuon xAODTracking xAODTrigMuon xAODTrigger xAODTruth MuonHistUtils IsolationSelectionLib FourMomUtils MuonCombinedToolInterfaces TrkToolInterfaces TrkValHistUtils TrigDecisionToolLib MuonAnalysisInterfacesLib )

# Install files from the package:
atlas_install_scripts( macros/MuonValidation_*.py macros/MuonValidation_*.sh)
atlas_install_python_modules( python/*py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime( scripts/*py POST_BUILD_CMD ${ATLAS_FLAKE8} )

