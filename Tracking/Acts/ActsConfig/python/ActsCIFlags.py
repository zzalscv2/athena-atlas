# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Flags used in CI tests

from TrkConfig.TrkConfigFlags import TrackingComponent

def actsWorkflowFlags(flags):
    """flags for Reco_tf with CA used in CI tests: add Acts workflow to reco sequence"""
    flags.Reco.EnableHGTDExtension = False
    flags.DQ.useTrigger = False
    flags.Acts.doAnalysis = True
    flags.Acts.doMonitoring = True
    flags.Tracking.recoChain = [TrackingComponent.AthenaChain,
                                TrackingComponent.ActsChain]
    flags.Output.HISTFileName = "ActsMonitoringOutput.root" 

def actsValidateClustersFlags(flags):
    """flags for Reco_tf with CA used in CI tests: use cluster conversion [xAOD -> InDet] with both Athena and Acts sequences"""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.recoChain = [TrackingComponent.ValidateActsClusters]

def actsTrkValidateSpacePointsFlags(flags):
    """flags for Reco_tf with CA used in CI tests: use for validating Athena-based space point formation"""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.recoChain = [TrackingComponent.ValidateActsSpacePoints]

def actsCoreValidateSpacePointsFlags(flags):
    """flags for Reco_tf with CA used in CI tests: use for validating ACTS-based space point formation"""
    from ActsConfig.ActsConfigFlags import SpacePointStrategy
    flags.Acts.SpacePointStrategy = SpacePointStrategy.ActsCore
    actsTrkValidateSpacePointsFlags(flags)
    
def actsValidateSeedsFlags(flags):
    """flags for Reco_tf with CA used in CI tests: use SiSpacePointSeedMaker tool during reconstruction"""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.recoChain = [TrackingComponent.ValidateActsSeeds]
    flags.Tracking.writeSeedValNtuple = True

def actsValidateOrthogonalSeedsFlags(flags):
    """flags for Reco_tf with CA used in CI tests: use SiSpacePointSeedMaker tool during reconstruction (orthogonal seeding)"""
    from ActsConfig.ActsConfigFlags import SeedingStrategy
    flags.Acts.SeedingStrategy = SeedingStrategy.Orthogonal
    actsValidateSeedsFlags(flags)

def actsValidateTracksFlags(flags):
    """flags for Reco_tf with CA used in CI tests: use ActsTrkFinding during reconstruction"""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.ITkMainPass.doAmbiguityProcessorTrackFit = False
    flags.Tracking.recoChain = [TrackingComponent.ValidateActsTracks]

def actsArtFlags(flags):
    """flags for Reco_tf with CA used in ART test: add Acts workflow to reco sequence"""
    flags.Reco.EnableHGTDExtension = False
    flags.Acts.doMonitoring = True
    flags.Tracking.recoChain = [TrackingComponent.AthenaChain,
                                TrackingComponent.ActsChain]
