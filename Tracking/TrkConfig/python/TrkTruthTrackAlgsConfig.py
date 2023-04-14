# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkTruthTrackAlgs package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthTrackingCfg(flags, name='InDetTruthTrackCreation', **kwargs):
    acc = ComponentAccumulator()

    if "PRD_TruthTrajectoryBuilder" not in kwargs:
        from InDetConfig.InDetTruthToolsConfig import (
            InDetPRD_TruthTrajectoryBuilderCfg)
        InDetPRD_TruthTrajectoryBuilder = acc.popToolsAndMerge(
            InDetPRD_TruthTrajectoryBuilderCfg(flags))
        acc.addPublicTool(InDetPRD_TruthTrajectoryBuilder)
        kwargs.setdefault('PRD_TruthTrajectoryBuilder',
                          InDetPRD_TruthTrajectoryBuilder)

    if "TruthTrackBuilder" not in kwargs:
        from InDetConfig.InDetTruthToolsConfig import InDetTruthTrackBuilderCfg
        InDetTruthTrackBuilder = acc.popToolsAndMerge(
            InDetTruthTrackBuilderCfg(flags))
        acc.addPublicTool(InDetTruthTrackBuilder)
        kwargs.setdefault('TruthTrackBuilder', InDetTruthTrackBuilder)

    kwargs.setdefault('OutputTrackCollection', 'InDetPseudoTracks')

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault('AssociationTool', acc.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrackSummaryToolCfg)
        TrackSummaryTool = acc.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags))
        acc.addPublicTool(TrackSummaryTool)
        kwargs.setdefault('TrackSummaryTool', TrackSummaryTool)

    if "PRD_TruthTrajectorySelectors" not in kwargs:
        trajectoryselectors = []
        if not flags.Tracking.doIdealPseudoTracking:
            from InDetConfig.InDetTruthToolsConfig import (
                InDetPRD_TruthTrajectorySelectorCfg)
            trajectoryselectors.append(acc.popToolsAndMerge(
                InDetPRD_TruthTrajectorySelectorCfg(flags)))
        kwargs.setdefault('PRD_TruthTrajectorySelectors', trajectoryselectors)

    acc.addEventAlgo(CompFactory.Trk.TruthTrackCreation(name, **kwargs))
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Disable calo for this test
    flags.Detector.EnableCalo = False

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    ################################ Aditional configurations ################################
    top_acc.merge(TruthTrackingCfg(flags))
    if flags.Tracking.doIdealPseudoTracking:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        top_acc.merge(InDetTrackTruthCfg(
            flags,
            Tracks='InDetPseudoTrackParticles',
            DetailedTruth='InDetPseudoTrackDetailedTruth',
            TracksTruth='InDetPseudoTrackTruthCollection'))

    flags.dump()

    ComponentAccumulator.debugMode = "trackCA trackEventAlgo ..."

    from AthenaCommon.Constants import DEBUG
    top_acc.foreach_component("AthEventSeq/*").OutputLevel = DEBUG
    top_acc.printConfig(withDetails=True, summariseProps=True)
    top_acc.store(open("TruthTrackingConfig.pkl", "wb"))

    import sys
    if "--norun" not in sys.argv:
        sc = top_acc.run(1)
        if sc.isFailure():
            sys.exit(-1)
