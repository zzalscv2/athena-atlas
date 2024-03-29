# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import BeamType, LHCPeriod, FlagEnum,HIMode


class PrimaryPassConfig(FlagEnum):
    VtxLumi = 'VtxLumi'
    VtxBeamSpot = 'VtxBeamSpot'
    Cosmics = 'Cosmics'
    HeavyIon = 'HeavyIon'
    HighPileup = 'HighPileup'
    UPC = 'UPC'
    HIP = 'HIP'
    MinBias = 'MinBias'
    RobustReco = 'RobustReco'
    Default = 'Main'


class ITkPrimaryPassConfig(FlagEnum):
    FTF = 'ITkFTF'
    FastTracking = 'ITkFast'
    Default = 'ITkMain'


class TrackFitterType(FlagEnum):
    DistributedKalmanFilter = 'DistributedKalmanFilter'
    GlobalChi2Fitter = 'GlobalChi2Fitter'
    GaussianSumFilter = 'GaussianSumFilter'


class KalmanUpdatorType(FlagEnum):
    KalmanUpdator = 'KalmanUpdator'
    KalmanUpdator_xk = 'KalmanUpdator_xk'
    KalmanUpdatorSMatrix = 'KalmanUpdatorSMatrix'
    KalmanWeightUpdator = 'KalmanWeightUpdator'
    KalmanUpdatorAmg = 'KalmanUpdatorAmg'


class PixelClusterSplittingType(FlagEnum):
    NeuralNet = 'NeuralNet'
    Truth = 'Truth'


class TrackingComponent(FlagEnum):
    AthenaChain = "AthenaChain"  # full Athena Chain (default)
    ActsChain = "ActsChain"  # full Acts Chain
    # Validation options
    ValidateActsClusters = "ValidateActsClusters"
    ValidateActsSpacePoints = "ValidateActsSpacePoints"
    ValidateActsSeeds = "ValidateActsSeeds"
    ValidateActsTracks = "ValidateActsTracks"
    ValidateActsAmbiguityResolution = "ValidateActsAmbiguityResolution"
    # Benchmarking
    BenchmarkSpot = "BenchmarkSpot"


def createTrackingConfigFlags():
    icf = AthConfigFlags()

    # Turn running of truth matching on and off (by default on for MC off for data)
    icf.addFlag("Tracking.doTruth", lambda prevFlags: prevFlags.Input.isMC)

    # control which fitter to be used
    icf.addFlag("Tracking.trackFitterType",
                TrackFitterType.GlobalChi2Fitter, type=TrackFitterType)
    # control which measurement updator to load as InDetUpdator
    icf.addFlag("Tracking.kalmanUpdator",
                KalmanUpdatorType.KalmanUpdatorSMatrix, type=KalmanUpdatorType)

    icf.addFlag("Tracking.materialInteractions", lambda prevFlags:
                prevFlags.Beam.Type is not BeamType.SingleBeam)
    # Control which type of particle hypothesis to use for the material interactions
    # 0=non-interacting,1=electron,2=muon,3=pion,4=kaon,5=proton. See ParticleHypothesis.h for full definition.
    icf.addFlag("Tracking.materialInteractionsType", lambda prevFlags:
                2 if prevFlags.Beam.Type is BeamType.Cosmics else 3)

    # Turn on running of Brem Recovery in tracking
    icf.addFlag("Tracking.doBremRecovery", lambda prevFlags: (
        prevFlags.Detector.EnableCalo and
        not (prevFlags.Tracking.PrimaryPassConfig in
             [PrimaryPassConfig.VtxLumi, PrimaryPassConfig.VtxBeamSpot] or
             prevFlags.Tracking.doLowMu or
             prevFlags.Beam.Type is not BeamType.Collisions or
             not prevFlags.BField.solenoidOn)))
    icf.addFlag("Tracking.doCaloSeededBrem", lambda prevFlags: (
        prevFlags.Detector.EnableCalo and prevFlags.Tracking.doBremRecovery))
    icf.addFlag("Tracking.phiWidthBrem", 0.3)
    icf.addFlag("Tracking.etaWidthBrem", 0.2)
    # Use Recover SSS to Calo ROIs
    icf.addFlag("Tracking.doHadCaloSeededSSS", False)
    # Use Calo ROIs to seed specific cuts for the ambi
    icf.addFlag("Tracking.doCaloSeededAmbi", lambda prevFlags:
                prevFlags.Detector.EnableCalo)
    # control if the shared hits are recorded in TrackPatricles
    icf.addFlag("Tracking.doSharedHits", True)
    # Switch for running TIDE Ambi
    icf.addFlag("Tracking.doTIDE_Ambi", lambda prevFlags:
                not (prevFlags.Beam.Type is BeamType.Cosmics))
    # Use simple position and error estimate for on-track pixel cluster
    icf.addFlag("Tracking.doPixelDigitalClustering", False)
    # Try to split pixel clusters
    icf.addFlag("Tracking.doPixelClusterSplitting",
                lambda prevFlags: not (prevFlags.Beam.Type is BeamType.Cosmics))
    # choose splitter type: NeuralNet or AnalogClus
    icf.addFlag("Tracking.pixelClusterSplittingType", lambda prevFlags:
                PixelClusterSplittingType.NeuralNet
                if prevFlags.GeoModel.Run <= LHCPeriod.Run3
                else PixelClusterSplittingType.Truth,
                type=PixelClusterSplittingType)
    # Cut value for splitting clusters into two parts
    icf.addFlag("Tracking.pixelClusterSplitProb1",
                lambda prevFlags: (
                    0.5 if prevFlags.GeoModel.Run is LHCPeriod.Run1 else 0.55))
    # Cut value for splitting clusters into three parts
    icf.addFlag("Tracking.pixelClusterSplitProb2",
                lambda prevFlags: (
                    0.5 if prevFlags.GeoModel.Run is LHCPeriod.Run1 else 0.45))
    # Skip ambiguity solver in hadronic ROI
    icf.addFlag("Tracking.doSkipAmbiROI", False)

    # Guaranteed not-subtracted topo clusters even in heavy ions
    icf.addFlag("Tracking.TopoClusters", "CaloTopoClusters")
    icf.addFlag("Tracking.EgammaTopoClusters", "egammaTopoClusters")

    # Express track parameters wrt. to : 'BeamLine','BeamSpot','Vertex' (first primary vertex)
    icf.addFlag("Tracking.perigeeExpression", lambda prevFlags:
                "Vertex" if (prevFlags.Tracking.PrimaryPassConfig is
                             PrimaryPassConfig.HeavyIon)
                else "BeamLine")

    # Tracking passes/configurations scheduled

    def doLargeD0(flags):
        if flags.GeoModel.Run <= LHCPeriod.Run3:
            return not ((flags.Beam.Type in
                         [BeamType.SingleBeam, BeamType.Cosmics]) or
                        flags.Tracking.PrimaryPassConfig in [
                            PrimaryPassConfig.HeavyIon,
                            PrimaryPassConfig.UPC,
                            PrimaryPassConfig.HIP,
                            PrimaryPassConfig.VtxLumi,
                            PrimaryPassConfig.VtxBeamSpot,
                            PrimaryPassConfig.HighPileup])
        else:  # LRT disabled by default for Run4 for now
            return False

    icf.addFlag("Tracking.doLargeD0", doLargeD0)
    icf.addFlag("Tracking.storeSeparateLargeD0Container", True)

    # Special configuration for low-mu runs
    icf.addFlag("Tracking.doLowMu", False)
    # Turn running of doLowPt second pass on and off
    icf.addFlag("Tracking.doLowPt",
                lambda prevFlags: prevFlags.Tracking.doLowMu)

    # Turn on to save the Track Seeds in a xAOD track collecting for development studies
    icf.addFlag("Tracking.doStoreTrackSeeds", False)
    # Save SiSP tracks (input to the ambiguity solver)
    icf.addFlag("Tracking.doStoreSiSPSeededTracks", False)
    # Turn writing of seed validation ntuple on and off
    icf.addFlag("Tracking.writeSeedValNtuple", False)
    # Save xAOD TrackMeasurementValidation + TrackStateValidation containers
    icf.addFlag("Tracking.writeExtendedSi_PRDInfo", False)
    icf.addFlag("Tracking.writeExtendedTRT_PRDInfo", False)

    # Toggle track slimming
    icf.addFlag("Tracking.doSlimming", lambda prevFlags:
                not (prevFlags.Beam.Type in
                     [BeamType.SingleBeam, BeamType.Cosmics] or
                     prevFlags.Tracking.PrimaryPassConfig in [
                         PrimaryPassConfig.VtxLumi,
                         PrimaryPassConfig.HighPileup]))

    ####################################################################

    # The following flags are only used in InDet configurations for now
    # No corresponding ITk config is available yet

    def cutLevel(flags):
        if flags.Tracking.PrimaryPassConfig is PrimaryPassConfig.HeavyIon:
            return 4
        elif flags.Tracking.doLowMu:
            return 3
        elif flags.Beam.Type is BeamType.Cosmics:
            return 8
        elif flags.Tracking.doMinBias:
            return 12
        else:
            return 19

    # Control cuts and settings for different lumi to limit CPU and disk space
    icf.addFlag("Tracking.cutLevel", cutLevel)

    # Turn on InDetRecStatistics
    icf.addFlag("Tracking.doStats", False)
    # Switch for track observer tool
    icf.addFlag("Tracking.doTIDE_AmbiTrackMonitoring", False)
    # use beam spot position in pixel NN
    icf.addFlag("Tracking.useBeamSpotInfoNN", True)
    # Threshold for NN cut in large D0 tracking for tracks in ambi
    icf.addFlag("Tracking.nnCutLargeD0Threshold", -1.0)
    # Use broad cluster errors for Pixel
    icf.addFlag("Tracking.useBroadPixClusterErrors", False)
    # Use broad cluster errors for SCT
    icf.addFlag("Tracking.useBroadSCTClusterErrors", False)

    # Tracking passes/configurations scheduled

    # Turn running of track segment creation in pixel on and off
    icf.addFlag("Tracking.doTrackSegmentsPixel",
                lambda prevFlags: (
                    prevFlags.Detector.EnablePixel and (
                        prevFlags.Tracking.doMinBias or
                        prevFlags.Tracking.doLowMu or
                        prevFlags.Beam.Type is BeamType.Cosmics)))
    # Turn running of track segment creation in SCT on and off
    icf.addFlag("Tracking.doTrackSegmentsSCT",
                lambda prevFlags: (
                    prevFlags.Detector.EnableSCT and (
                        prevFlags.Tracking.doLowMu or
                        prevFlags.Beam.Type is BeamType.Cosmics)))
    # Turn running of track segment creation in TRT on and off
    icf.addFlag("Tracking.doTrackSegmentsTRT",
                lambda prevFlags: (
                    prevFlags.Detector.EnableTRT and
                    (prevFlags.Tracking.doLowMu or
                     prevFlags.Beam.Type is BeamType.Cosmics)))
    # turn on / off TRT extensions
    icf.addFlag("Tracking.doTRTExtension",
                lambda prevFlags: prevFlags.Detector.EnableTRT)
    # control to run TRT Segment finding (do it always after new tracking!)
    icf.addFlag("Tracking.doTRTSegments",
                lambda prevFlags: (prevFlags.Detector.EnableTRT and
                                   (prevFlags.Tracking.doBackTracking or
                                    prevFlags.Tracking.doTRTStandalone)))
    # Turn running of backtracking on and off
    icf.addFlag("Tracking.doBackTracking", lambda prevFlags: (
        prevFlags.Detector.EnableTRT and
        not(prevFlags.Beam.Type in [BeamType.SingleBeam, BeamType.Cosmics] or
            prevFlags.Tracking.PrimaryPassConfig in [
                PrimaryPassConfig.HeavyIon,
                PrimaryPassConfig.VtxLumi,
                PrimaryPassConfig.VtxBeamSpot,
                PrimaryPassConfig.HighPileup])))
    # control TRT Standalone
    icf.addFlag("Tracking.doTRTStandalone", lambda prevFlags: (
        prevFlags.Detector.EnableTRT and
        not(prevFlags.Tracking.PrimaryPassConfig in [
            PrimaryPassConfig.HeavyIon,
            PrimaryPassConfig.VtxLumi,
            PrimaryPassConfig.VtxBeamSpot,
            PrimaryPassConfig.HighPileup])))

    # Turn running of doForwardTracks pass on and off
    icf.addFlag("Tracking.doForwardTracks", lambda prevFlags: (
        prevFlags.Detector.EnablePixel and
        not(prevFlags.Beam.Type in [BeamType.SingleBeam, BeamType.Cosmics] or
            prevFlags.Tracking.PrimaryPassConfig in [
                PrimaryPassConfig.HeavyIon,
                PrimaryPassConfig.VtxLumi,
                PrimaryPassConfig.VtxBeamSpot,
                PrimaryPassConfig.HighPileup] or
            prevFlags.Tracking.doMinBias or
            prevFlags.Tracking.doLowMu)))
    icf.addFlag("Tracking.doTrackSegmentsDisappearing",
                lambda prevFlags: (
                    not((prevFlags.Tracking.PrimaryPassConfig is
                         PrimaryPassConfig.HeavyIon) or
                        prevFlags.Beam.Type is BeamType.Cosmics)))

    # Turn running of doVeryLowPt third pass on and off
    icf.addFlag("Tracking.doVeryLowPt", False)
    # Turn running of doLargeD0 second pass down to 100 MeV on and off
    icf.addFlag("Tracking.doLowPtLargeD0", False)
    # Switch for running LowPtRoI settings
    icf.addFlag("Tracking.doLowPtRoI", False)
    # Switch for running UPC settings
    icf.addFlag("Tracking.doUPC", lambda prevFlags: prevFlags.Reco.HIMode is HIMode.UPC)
    # Switch for running HIP settings
    icf.addFlag("Tracking.doHIP", lambda prevFlags: prevFlags.Reco.HIMode is HIMode.HIP)
    # Switch for running MinBias settings (UPC or HIP turn this ON)
    icf.addFlag("Tracking.doMinBias", lambda prevFlags:
                prevFlags.Tracking.doUPC or prevFlags.Tracking.doHIP)
    # Turn running of BeamGas second pass on and off
    icf.addFlag("Tracking.doBeamGas",
                lambda prevFlags: prevFlags.Beam.Type is BeamType.SingleBeam)
    # Special reconstruction for BLS physics
    icf.addFlag("Tracking.doBLS", False)

    # Special pass using truth information for pattern recognition, runs in parallel to/instead of the first pass
    icf.addFlag("Tracking.doPseudoTracking", False)
    # Special pass using truth information for pattern recognition, removes assumed in-efficencies applied to PseudoTracking
    icf.addFlag("Tracking.doIdealPseudoTracking", False)

    ####################################################################

    # The following flags are only used in ITk configurations

    # Turn running of ITk FastTracking on and off
    icf.addFlag("Tracking.doITkFastTracking", False)

    # Turn running of Conversion second tracking pass on and off
    icf.addFlag("Tracking.doITkConversion",
                lambda prevFlags: not prevFlags.Tracking.doITkFastTracking)

    # Allows TrigFastTrackFinder to be run as an offline algorithm by replacing
    # SiSPSeededTrackFinder
    icf.addFlag("Tracking.useITkFTF", False)

    # enable reco steps
    icf.addFlag("Tracking.recoChain", [TrackingComponent.AthenaChain])

    ####################################################################

    # Tracking pass flags

    # InDet

    from TrkConfig.TrackingPassFlags import (
        createTrackingPassFlags, createHighPileupTrackingPassFlags,
        createMinBiasTrackingPassFlags, createUPCTrackingPassFlags,
        createHIPTrackingPassFlags, createLargeD0TrackingPassFlags,
        createR3LargeD0TrackingPassFlags, createLowPtLargeD0TrackingPassFlags,
        createLowPtTrackingPassFlags, createVeryLowPtTrackingPassFlags,
        createLowPtRoITrackingPassFlags, createForwardTracksTrackingPassFlags,
        createBeamGasTrackingPassFlags, createVtxLumiTrackingPassFlags,
        createVtxBeamSpotTrackingPassFlags, createCosmicsTrackingPassFlags,
        createHeavyIonTrackingPassFlags, createPixelTrackingPassFlags,
        createDisappearingTrackingPassFlags, createSCTTrackingPassFlags,
        createTRTTrackingPassFlags, createTRTStandaloneTrackingPassFlags,
        createRobustRecoTrackingPassFlags)

    def primaryPass(flags):
        if flags.Beam.Type is BeamType.Cosmics:
            return PrimaryPassConfig.Cosmics
        elif flags.Reco.EnableHI:
            if flags.Tracking.doUPC: #For UPC
                return PrimaryPassConfig.UPC
            elif flags.Tracking.doHIP: #For HIP
                return PrimaryPassConfig.HIP
            else: #For HI (default)
                return PrimaryPassConfig.HeavyIon
        elif flags.Tracking.doMinBias:
             return PrimaryPassConfig.MinBias
        else:
            return PrimaryPassConfig.Default

    icf.addFlag("Tracking.PrimaryPassConfig", lambda prevFlags:
                primaryPass(prevFlags), type=PrimaryPassConfig)

    # Set up for first tracking pass, updated for second passes
    icf.addFlagsCategory("Tracking.MainPass",
                         createTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.HighPileupPass",
                         createHighPileupTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.UPCPass",
                         createUPCTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.HIPPass",
                         createHIPTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.MinBiasPass",
                         createMinBiasTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.LargeD0Pass",
                         createLargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.R3LargeD0Pass",
                         createR3LargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.LowPtLargeD0Pass",
                         createLowPtLargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.LowPtPass",
                         createLowPtTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.VeryLowPtPass",
                         createVeryLowPtTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.LowPtRoIPass",
                         createLowPtRoITrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.ForwardPass",
                         createForwardTracksTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.BeamGasPass",
                         createBeamGasTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.VtxLumiPass",
                         createVtxLumiTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.VtxBeamSpotPass",
                         createVtxBeamSpotTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.CosmicsPass",
                         createCosmicsTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.HeavyIonPass",
                         createHeavyIonTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.PixelPass",
                         createPixelTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.DisappearingPass",
                         createDisappearingTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.SCTPass",
                         createSCTTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.TRTPass",
                         createTRTTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.TRTStandalonePass",
                         createTRTStandaloneTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("Tracking.RobustRecoPass",
                         createRobustRecoTrackingPassFlags, prefix=True)

    # ITk

    from TrkConfig.TrackingPassFlags import (
        createITkTrackingPassFlags, createITkLargeD0TrackingPassFlags,
        createITkConversionTrackingPassFlags,
        createITkFastTrackingPassFlags, createITkLargeD0FastTrackingPassFlags,
        createITkFTFPassFlags, createITkLowPtTrackingPassFlags)

    def itkPrimaryPass(flags):
        if flags.Tracking.useITkFTF:
            return ITkPrimaryPassConfig.FTF
        elif flags.Tracking.doITkFastTracking:
            return ITkPrimaryPassConfig.FastTracking
        else:
            return ITkPrimaryPassConfig.Default

    icf.addFlag("Tracking.ITkPrimaryPassConfig", lambda prevFlags:
                itkPrimaryPass(prevFlags), type=ITkPrimaryPassConfig)

    icf.addFlagsCategory ("Tracking.ITkMainPass",
                          createITkTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkLargeD0Pass",
                          createITkLargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkConversionPass",
                          createITkConversionTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkLowPt",
                          createITkLowPtTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkFastPass",
                          createITkFastTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkLargeD0FastPass",
                          createITkLargeD0FastTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkFTFPass",
                          createITkFTFPassFlags, prefix=True)

    # Acts
    from ActsConfig.ActsTrackingPassFlags import (
        createActsTrackingPassFlags,
        createValidateActsClustersTrackingPassFlags,
        createValidateActsSpacePointsTrackingPassFlags,
        createValidateActsSeedsTrackingPassFlags,
        createValidateActsTracksTrackingPassFlags,
        createValidateActsAmbiguityResolutionTrackingPassFlags,
        createActsBenchmarkSpotTrackingPassFlags
    )

    icf.addFlagsCategory ("Tracking.ITkActsPass",
                          createActsTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkValidateActsClustersPass",
                          createValidateActsClustersTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkValidateActsSpacePointsPass",
                          createValidateActsSpacePointsTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkValidateActsSeedsPass",
                          createValidateActsSeedsTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkValidateActsTracksPass",
                          createValidateActsTracksTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkValidateActsAmbiguityResolutionPass",
                          createValidateActsAmbiguityResolutionTrackingPassFlags, prefix=True)
    icf.addFlagsCategory ("Tracking.ITkActsBenchmarkSpotPass",
                          createActsBenchmarkSpotTrackingPassFlags, prefix=True)

    ####################################################################

    # Vertexing flags
    from TrkConfig.VertexFindingFlags import createPriVertexingFlags
    icf.addFlagsCategory("Tracking.PriVertex",
                         createPriVertexingFlags, prefix=True)

    # Turn on the primary vertex reconstruction
    icf.addFlag("Tracking.doVertexFinding",
                lambda prevFlags: prevFlags.Beam.Type is not BeamType.Cosmics)
    # Turn on the secondary vertex V0 finder
    icf.addFlag("Tracking.doV0Finder", False)

    return icf
