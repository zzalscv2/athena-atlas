# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( JetTagTools )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread
    MathMore Minuit Minuit2 Matrix Physics HistPainter Rint RooFitCore RooFit )

# Component(s) in the package:
atlas_add_library( JetTagToolsLib
    JetTagTools/*.h
    src/TrackSelector.cxx
    src/HistoHelperRoot.cxx src/NewLikelihoodTool.cxx
    src/BasicTrackGradeFactory.cxx src/DetailedTrackGradeFactory.cxx
    src/GradedTrack.cxx src/IPTag.cxx
    src/SVForIPTool.cxx src/SVTag.cxx 
    src/JetFitterInputWriter.cxx src/JetFitterDummyClassifier.cxx
    src/MV2Tag.cxx
    src/JetFitterVariablesFactory.cxx src/MSVVariablesFactory.cxx
    src/MultiSVTag.cxx
    src/MultivariateTagManager.cxx
    src/TagNtupleDumper.cxx
    src/ClassifiedTrackTaggerTool.cxx
    src/CTTDecorCheckInTool.cxx

    PUBLIC_HEADERS JetTagTools
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
    PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
    LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel CxxUtils GaudiKernel
    GeoPrimitives ITrackToVertex InDetRecToolInterfaces JetTagCalibrationLib JetTagInfo
    MVAUtils xAODBTagging xAODJet xAODTracking InDetVKalVxInJetToolLib
    PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} InDetTrackSelectionToolLib
    Navigation Particle ParticleJetToolsLib TrkEventPrimitives TrkLinks TrkNeuralNetworkUtilsLib
    TrkParameters TrkTrack TrkTrackLink TrkTrackSummary TrkVertexFitterInterfaces VxJetVertex
    VxSecVertex VxVertex TestTools)

atlas_add_component( JetTagTools
    src/components/*.cxx
    LINK_LIBRARIES JetTagToolsLib )

# Install files from the package:
atlas_install_runtime( share/*.root share/*.txt )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )


# Test(s) in the package:
atlas_add_test( ClassifiedTrackTaggerTool_test
                SOURCES test/ClassifiedTrackTaggerTool_test.cxx
                INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel CxxUtils EventPrimitives 
                GaudiKernel MagFieldConditions TestTools TrkVertexFitterInterfaces xAODTracking InDetVKalVxInJetToolLib xAODJet
                LOG_IGNORE_PATTERN "Booking|ClassIDSvc|weight file|create data set info" )
atlas_install_joboptions( share/*.txt )
