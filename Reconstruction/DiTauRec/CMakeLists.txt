################################################################################
# Package: DiTauRec
################################################################################

# Declare the package name:
atlas_subdir( DiTauRec )

# Declare the package's dependencies:
if( NOT XAOD_ANALYSIS )
  atlas_depends_on_subdirs( PUBLIC
    Control/AthenaBaseComps
    Event/xAOD/xAODEgamma
    Event/xAOD/xAODJet
    Event/xAOD/xAODMuon
    Event/xAOD/xAODTau
    GaudiKernel
    PhysicsAnalysis/MuonID/MuonSelectorTools
    Reconstruction/Jet/JetEDM
    Tracking/TrkTools/TrkToolInterfaces
    PRIVATE
    Tools/PathResolver
    Calorimeter/CaloEvent
    Event/xAOD/xAODCaloEvent
    Event/xAOD/xAODTracking
    Event/FourMomUtils
    Reconstruction/tauRec
    Reconstruction/RecoTools/RecoToolInterfaces
    PhysicsAnalysis/MuonID/MuonIDAnalysis/MuonMomentumCorrections
    Tracking/TrkVertexFitter/TrkVertexFitterInterfaces
    Reconstruction/tauRecTools
    Tracking/TrkEvent/TrkParametersIdentificationHelpers )
  

else()
atlas_depends_on_subdirs( PUBLIC
                          Control/AthenaBaseComps
                          Event/xAOD/xAODEgamma
                          Event/xAOD/xAODJet
                          Event/xAOD/xAODMuon
                          Event/xAOD/xAODTau
                          GaudiKernel
                          PhysicsAnalysis/MuonID/MuonSelectorTools
                          Reconstruction/Jet/JetEDM
                          Tracking/TrkTools/TrkToolInterfaces
                          PRIVATE
                          Tools/PathResolver
                          Calorimeter/CaloEvent
                          Event/xAOD/xAODTracking
                          Event/xAOD/xAODCaloEvent
                          Event/FourMomUtils
                          Reconstruction/tauRec
                          Reconstruction/RecoTools/RecoToolInterfaces
                          Reconstruction/RecoTools/ITrackToVertex
                          Tracking/TrkVertexFitter/TrkVertexFitterInterfaces
                          Tracking/TrkVertexFitter/TrkVertexFitters
                          InnerDetector/InDetRecTools/InDetTrackSelectionTool
                          Reconstruction/tauRecTools
                          Reconstruction/MVAUtils
                          Tracking/TrkEvent/TrkParametersIdentificationHelpers )
endif()

# External dependencies:
find_package( FastJet )

if(XAOD_ANALYSIS)
atlas_add_root_dictionary( DiTauRecLib DiTauRecLibCintDict
  ROOT_HEADERS
  DiTauRec/ILepHadBuilder.h
  DiTauRec/HadElBuilder.h
  DiTauRec/HadMuBuilder.h
  Root/LinkDef.h
  EXTERNAL_PACKAGES ROOT
  )
endif()

# Component(s) in the package:

if( NOT XAOD_ANALYSIS )
  atlas_add_component( DiTauRec
    src/*.cxx
    src/components/*.cxx
    INCLUDE_DIRS ${FASTJET_INCLUDE_DIRS}
    LINK_LIBRARIES ${FASTJET_LIBRARIES} AthenaBaseComps xAODEgamma xAODJet xAODMuon xAODTau GaudiKernel MuonSelectorToolsLib JetEDM TrkToolInterfaces CaloEvent xAODCaloEvent xAODTracking RecoToolInterfaces tauRec tauRecToolsLib MuonMomentumCorrectionsLib TrkParametersIdentificationHelpers TrkVertexFitterInterfaces TrkVertexFittersLib ITrackToVertex InDetTrackSelectionToolLib FourMomUtils MVAUtils PathResolver )
else()

atlas_add_library( DiTauRecLib
  DiTauRec/*.h Root/*.cxx ${DiTauRecLibCintDict}
  PUBLIC_HEADERS DiTauRec
  LINK_LIBRARIES AsgTools xAODEgamma xAODJet xAODMuon xAODTau MuonSelectorToolsLib JetEDM xAODTracking RecoToolInterfaces tauRecToolsLib
  )

if( NOT XAOD_STANDALONE )
  atlas_add_component(DiTauRec Root/components/*.cxx LINK_LIBRARIES DiTauRecLib)
endif()

endif()

# Install files from the package:
atlas_install_headers( DiTauRec )
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py )

