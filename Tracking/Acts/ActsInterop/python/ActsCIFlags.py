# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Flags used in CI tests

from InDetConfig.ITkConfigFlags import TrackingComponent

def actsWorkflowFlags(flags):
    """flags for Reco_tf with CA used in CI tests: add Acts workflow to reco sequence"""
    flags.Reco.EnableHGTDExtension = False
    flags.DQ.useTrigger = False
    flags.Acts.doAnalysis = True
    flags.Acts.doMonitoring = True
    flags.ITk.Tracking.recoChain = [TrackingComponent.AthenaChain,
                                    TrackingComponent.ActsChain]
    flags.Output.HISTFileName = "ActsMonitoringOutput.root" 

def actsValidateClustersFlags(flags):
    """flags for Reco_tf with CA used in CI tests: use cluster conversion [xAOD -> InDet] with both Athena and Acts sequences"""
    flags.Reco.EnableHGTDExtension = False
    flags.ITk.Tracking.recoChain = [TrackingComponent.ValidateActsClusters]

def actsValidateSpacePointsFlags(flags):
    """flags for Reco_tf with CA used in CI tests: use for validating space point formation with Acts"""
    flags.Reco.EnableHGTDExtension = False
    flags.ITk.Tracking.recoChain = [TrackingComponent.ValidateActsSpacePoints]
    
def actsValidateSeedsFlags(flags):
    """flags for Reco_tf with CA used in CI tests: use SiSpacePointSeedMaker tool during reconstruction"""
    flags.Reco.EnableHGTDExtension = False
    flags.ITk.Tracking.recoChain = [TrackingComponent.ValidateActsSeeds]
    flags.Tracking.writeSeedValNtuple = True

def actsValidateOrthogonalSeedsFlags(flags):
    """flags for Reco_tf with CA used in CI tests: use SiSpacePointSeedMaker tool during reconstruction (orthogonal seeding)"""
    from ActsInterop.ActsConfigFlags import SeedingStrategy
    flags.Acts.SeedingStrategy = SeedingStrategy.Orthogonal
    actsValidateSeedsFlags(flags)

def actsValidateTracksFlags(flags):
    """flags for Reco_tf with CA used in CI tests: use ActsTrkFinding during reconstruction"""
    flags.Reco.EnableHGTDExtension = False
    flags.ITk.Tracking.MainPass.doAmbiguityProcessorTrackFit = False
    flags.ITk.Tracking.recoChain = [TrackingComponent.ValidateActsTracks]

def actsArtFlags(flags):
    """flags for Reco_tf with CA used in ART test: add Acts workflow to reco sequence"""
    flags.Reco.EnableHGTDExtension = False
    flags.Acts.doMonitoring = True
    flags.ITk.Tracking.recoChain = [TrackingComponent.AthenaChain,
                                    TrackingComponent.ActsChain]
