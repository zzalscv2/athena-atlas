# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import Format


def InDetRecPreProcessingSiliconCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    # ------------------------------------------------------------
    #
    # ----------- Data-Preparation stage
    #
    # ------------------------------------------------------------
    #
    # ----------- PrepRawData creation from Raw Data Objects
    #

    #
    # --- Slim BCM RDOs by zero-suppressing
    #
    if flags.Detector.EnableBCM:
        if flags.Input.Format is Format.BS:
            from BCM_RawDataByteStreamCnv.BCM_RawDataByteStreamCnvConfig import (
                BCM_RawDataProviderAlgCfg)
            acc.merge(BCM_RawDataProviderAlgCfg(flags))
        from InDetConfig.BCM_ZeroSuppressionConfig import BCM_ZeroSuppressionCfg
        acc.merge(BCM_ZeroSuppressionCfg(flags))

    #
    # -- Pixel Clusterization
    #
    if (flags.Detector.EnablePixel and
        (flags.Input.Format is Format.BS
         or 'PixelRDOs' in flags.Input.Collections)):

        #
        # --- PixelClusterization algorithm
        #
        from InDetConfig.InDetPrepRawDataFormationConfig import (
            PixelClusterizationCfg)
        acc.merge(PixelClusterizationCfg(flags))
        if flags.InDet.doSplitReco:
            from InDetConfig.InDetPrepRawDataFormationConfig import (
                PixelClusterizationPUCfg)
            acc.merge(PixelClusterizationPUCfg(flags))
    #
    # --- SCT Clusterization
    #
    if (flags.Detector.EnableSCT and
        (flags.Input.Format is Format.BS
         or 'SCT_RDOs' in flags.Input.Collections)):

        #
        # --- SCT_Clusterization algorithm
        #
        from InDetConfig.InDetPrepRawDataFormationConfig import (
            SCTClusterizationCfg)
        acc.merge(SCTClusterizationCfg(flags))
        if flags.InDet.doSplitReco:
            from InDetConfig.InDetPrepRawDataFormationConfig import (
                SCTClusterizationPUCfg)
            acc.merge(SCTClusterizationPUCfg(flags))

    #
    # ----------- form SpacePoints from clusters in SCT and Pixels
    #
    #
    from InDetConfig.SiSpacePointFormationConfig import (
        InDetSiTrackerSpacePointFinderCfg)
    acc.merge(InDetSiTrackerSpacePointFinderCfg(flags))

    # this truth must only be done if you do PRD and SpacePointformation
    # If you only do the latter (== running on ESD) then the needed input (simdata)
    # is not in ESD but the resulting truth (clustertruth) is already there ...
    if (flags.InDet.doTruth and
        (not flags.Detector.EnableSCT or
         'SCT_SDO_Map' in flags.Input.Collections) and
        (not flags.Detector.EnablePixel or
         'PixelSDO_Map' in flags.Input.Collections)):

        from InDetConfig.InDetTruthAlgsConfig import (
            InDetPRD_MultiTruthMakerSiCfg)
        acc.merge(InDetPRD_MultiTruthMakerSiCfg(flags))
        if flags.InDet.doSplitReco:
            from InDetConfig.InDetTruthAlgsConfig import (
                InDetPRD_MultiTruthMakerSiPUCfg)
            acc.merge(InDetPRD_MultiTruthMakerSiPUCfg(flags))

    return acc


def ITkRecPreProcessingSiliconCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    # ------------------------------------------------------------
    #
    # ----------- Data-Preparation stage
    #
    # ------------------------------------------------------------
    #
    # ----------- PrepRawData creation from Raw Data Objects
    #

    #
    # --- Slim BCM RDOs by zero-suppressing
    #
    if flags.Detector.EnableBCMPrime:
        from InDetConfig.BCM_ZeroSuppressionConfig import BCM_ZeroSuppressionCfg
        acc.merge(BCM_ZeroSuppressionCfg(flags))

    #
    # --- Deducing configuration from the flags
    #
    from ActsConfig.TrackingComponentConfigurer import (
        TrackingComponentConfigurer)
    configuration_settings = TrackingComponentConfigurer(flags)

    #
    # -- Clusterization Algorithms
    #
    if configuration_settings.doAthenaCluster:
        from InDetConfig.InDetPrepRawDataFormationConfig import (
            AthenaTrkClusterizationCfg)
        acc.merge(AthenaTrkClusterizationCfg(flags))

    if configuration_settings.doActsCluster:
        from ActsConfig.ActsTrkClusterizationConfig import (
            ActsTrkClusterizationCfg)
        acc.merge(ActsTrkClusterizationCfg(flags))

    #
    # ---  Cluster EDM converters
    #
    if configuration_settings.doAthenaToActsCluster:
        if (not flags.Detector.EnableITkPixel or
                not flags.Detector.EnableITkStrip):
            raise RuntimeError(
                "Cluster EDM converter (InDet -> xAOD) must be activated for both Pixel and Strips")
        #
        # --- InDet -> xAOD Cluster EDM converter
        #
        from InDetConfig.InDetPrepRawDataFormationConfig import (
            ITkInDetToXAODClusterConversionCfg)
        acc.merge(ITkInDetToXAODClusterConversionCfg(flags))

    if configuration_settings.doActsToAthenaCluster:
        if (not flags.Detector.EnableITkPixel or
                not flags.Detector.EnableITkStrip):
            raise RuntimeError(
                "Cluster EDM converter (xAOD -> InDet) must be activated for both Pixel and Strips")
        #
        # --- xAOD -> InDet Cluster EDM converter
        #
        from InDetConfig.InDetPrepRawDataFormationConfig import (
            ITkXAODToInDetClusterConversionCfg)
        acc.merge(ITkXAODToInDetClusterConversionCfg(flags))

    #
    # ----------- form SpacePoints from clusters in SCT and Pixels
    #
    if configuration_settings.doAthenaSpacePoint:
        from InDetConfig.SiSpacePointFormationConfig import (
            ITkSiTrackerSpacePointFinderCfg)
        acc.merge(ITkSiTrackerSpacePointFinderCfg(flags))

    if configuration_settings.doActsSpacePoint:
        from ActsConfig.ActsTrkSpacePointFormationConfig import (
            ActsTrkSpacePointFormationCfg)
        acc.merge(ActsTrkSpacePointFormationCfg(flags))

    #
    # --- Space Point EDM converters
    #
    if configuration_settings.AthenaToActsSpacePointConverter:
        if (not flags.Detector.EnableITkPixel or
                not flags.Detector.EnableITkStrip):
            raise RuntimeError(
                "Space Point EDM converter (Trk -> xAOD) must be activated for both Pixel and Strips")

        #
        # --- Trk -> xAOD Space Point EDM converter
        from SiSpacePointFormation.SiSpacePointFormationConfig import (
            TrkToXAODSpacePointConversionCfg)
        acc.merge(TrkToXAODSpacePointConversionCfg(flags))

    # this truth must only be done if you do PRD and SpacePointformation
    # If you only do the latter (== running on ESD) then the needed input (simdata)
    # is not in ESD but the resulting truth (clustertruth) is already there ...
    if flags.ITk.doTruth:
        from InDetConfig.InDetTruthAlgsConfig import ITkPRD_MultiTruthMakerSiCfg
        acc.merge(ITkPRD_MultiTruthMakerSiCfg(flags))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN2

    flags.Tracking.doPixelClusterSplitting = True

    numThreads = 1
    flags.Concurrency.NumThreads = numThreads
    flags.Concurrency.NumConcurrentEvents = numThreads

    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    top_acc.merge(InDetRecPreProcessingSiliconCfg(flags))

    iovsvc = top_acc.getService('IOVDbSvc')
    iovsvc.OutputLevel = 5

    top_acc.printConfig()
    top_acc.run(25)
    top_acc.store(open("test_SiliconPreProcessing.pkl", "wb"))
