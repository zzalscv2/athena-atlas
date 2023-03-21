# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SiTrackMakerTool_xk package
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType


def SiTrackMaker_xkCfg(flags, name="InDetSiTrackMaker", **kwargs):
    # To produce BeamSpotData and AtlasFieldCacheCondObj
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = BeamSpotCondAlgCfg(flags)
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))

    if "RoadTool" not in kwargs:
        from InDetConfig.SiDetElementsRoadToolConfig import (
            SiDetElementsRoadMaker_xkCfg)
        kwargs.setdefault("RoadTool", acc.popToolsAndMerge(
            SiDetElementsRoadMaker_xkCfg(flags)))

    if "CombinatorialTrackFinder" not in kwargs:
        from InDetConfig.SiCombinatorialTrackFinderToolConfig import (
            SiCombinatorialTrackFinder_xkCfg)
        kwargs.setdefault("CombinatorialTrackFinder", acc.popToolsAndMerge(
            SiCombinatorialTrackFinder_xkCfg(flags)))

    kwargs.setdefault("useSCT",    flags.InDet.Tracking.ActiveConfig.useSCT)
    kwargs.setdefault("usePixel",  flags.InDet.Tracking.ActiveConfig.usePixel)

    kwargs.setdefault("pTmin",     flags.InDet.Tracking.ActiveConfig.minPT)
    kwargs.setdefault("pTminBrem", flags.InDet.Tracking.ActiveConfig.minPTBrem)
    kwargs.setdefault("pTminSSS",  flags.InDet.Tracking.ActiveConfig.pT_SSScut)
    kwargs.setdefault("nClustersMin",
                      flags.InDet.Tracking.ActiveConfig.minClusters)
    kwargs.setdefault("nHolesMax", flags.InDet.Tracking.ActiveConfig.nHolesMax)
    kwargs.setdefault("nHolesGapMax",
                      flags.InDet.Tracking.ActiveConfig.nHolesGapMax)
    kwargs.setdefault("SeedsFilterLevel",
                      flags.InDet.Tracking.ActiveConfig.seedFilterLevel)
    kwargs.setdefault("Xi2max", flags.InDet.Tracking.ActiveConfig.Xi2max)
    kwargs.setdefault("Xi2maxNoAdd",
                      flags.InDet.Tracking.ActiveConfig.Xi2maxNoAdd)
    kwargs.setdefault("nWeightedClustersMin",
                      flags.InDet.Tracking.ActiveConfig.nWeightedClustersMin)

    kwargs.setdefault("CosmicTrack", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("Xi2maxMultiTracks",
                      flags.InDet.Tracking.ActiveConfig.Xi2max)
    kwargs.setdefault("useSSSseedsFilter", True)
    kwargs.setdefault("doMultiTracksProd", True)

    kwargs.setdefault("useBremModel",
                      flags.Tracking.doBremRecovery and
                      flags.Detector.EnableCalo and
                      (flags.InDet.Tracking.ActiveConfig.extension in ["", "BLS"]))
    kwargs.setdefault("doCaloSeededBrem", flags.Tracking.doCaloSeededBrem)

    if kwargs["useBremModel"] and kwargs["doCaloSeededBrem"]:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            CaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(CaloClusterROIPhiRZContainerMakerCfg(flags))

    kwargs.setdefault("doHadCaloSeedSSS", flags.Tracking.doHadCaloSeededSSS)

    if kwargs["doHadCaloSeedSSS"]:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            HadCaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(HadCaloClusterROIPhiRZContainerMakerCfg(flags))

    kwargs.setdefault("phiWidth",
                      flags.InDet.Tracking.ActiveConfig.phiWidthBrem)
    kwargs.setdefault("etaWidth",
                      flags.InDet.Tracking.ActiveConfig.etaWidthBrem)
    kwargs.setdefault("EMROIPhiRZContainer", "InDetCaloClusterROIPhiRZ0GeV")
    kwargs.setdefault("HadROIPhiRZContainer", "InDetHadCaloClusterROIPhiRZ")
    kwargs.setdefault("UseAssociationTool",
                      flags.InDet.Tracking.ActiveConfig.usePrdAssociationTool)

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("TrackPatternRecoInfo",
                          'SiSpacePointsSeedMaker_Cosmic')

    elif flags.Reco.EnableHI:
        kwargs.setdefault("TrackPatternRecoInfo",
                          'SiSpacePointsSeedMaker_HeavyIon')

    elif flags.InDet.Tracking.ActiveConfig.extension == "LowPt":
        kwargs.setdefault("TrackPatternRecoInfo",
                          'SiSpacePointsSeedMaker_LowMomentum')

    elif (flags.InDet.Tracking.ActiveConfig.extension == "VeryLowPt" or
          (flags.InDet.Tracking.ActiveConfig.extension == "Pixel" and
           flags.Tracking.doMinBias)):
        kwargs.setdefault("TrackPatternRecoInfo",
                          'SiSpacePointsSeedMaker_VeryLowMomentum')

    elif flags.InDet.Tracking.ActiveConfig.extension == "BeamGas":
        kwargs.setdefault("TrackPatternRecoInfo",
                          'SiSpacePointsSeedMaker_BeamGas')

    elif flags.InDet.Tracking.ActiveConfig.extension == "Forward":
        kwargs.setdefault("TrackPatternRecoInfo",
                          'SiSpacePointsSeedMaker_ForwardTracks')

    elif (flags.InDet.Tracking.ActiveConfig.extension in
          ["LargeD0", "R3LargeD0", "LowPtLargeD0"]):
        kwargs.setdefault("TrackPatternRecoInfo",
                          'SiSpacePointsSeedMaker_LargeD0')

    else:
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSPSeededFinder')

    if (flags.Tracking.doStoreTrackSeeds and
            "SeedToTrackConversion" not in kwargs):
        from InDetConfig.SeedToTrackConversionToolConfig import (
            SeedToTrackConversionToolCfg)
        kwargs.setdefault("SeedToTrackConversion", acc.popToolsAndMerge(
            SeedToTrackConversionToolCfg(flags)))
        kwargs.setdefault("SeedSegmentsWrite", True)

    acc.setPrivateTools(CompFactory.InDet.SiTrackMaker_xk(
        name+flags.InDet.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def TrigSiTrackMaker_xkCfg(flags, name="TrigSiTrackMaker", **kwargs):
    # To produce BeamSpotData and AtlasFieldCacheCondObj
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = BeamSpotCondAlgCfg(flags)
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))

    if "RoadTool" not in kwargs:
        from InDetConfig.SiDetElementsRoadToolConfig import (
            SiDetElementsRoadMaker_xkCfg)
        kwargs.setdefault("RoadTool", acc.popToolsAndMerge(
            SiDetElementsRoadMaker_xkCfg(flags)))

    if "CombinatorialTrackFinder" not in kwargs:
        from InDetConfig.SiCombinatorialTrackFinderToolConfig import (
            SiCombinatorialTrackFinder_xk_Trig_Cfg)
        kwargs.setdefault("CombinatorialTrackFinder", acc.popToolsAndMerge(
            SiCombinatorialTrackFinder_xk_Trig_Cfg(flags)))

    kwargs.setdefault("pTmin", flags.InDet.Tracking.ActiveConfig.minPT)
    kwargs.setdefault("nClustersMin",
                      flags.InDet.Tracking.ActiveConfig.nClustersMin)
    kwargs.setdefault("nHolesMax", flags.InDet.Tracking.ActiveConfig.nHolesMax)
    kwargs.setdefault("nHolesGapMax",
                      flags.InDet.Tracking.ActiveConfig.nHolesGapMax)
    kwargs.setdefault("SeedsFilterLevel",
                      flags.InDet.Tracking.ActiveConfig.seedFilterLevel)
    kwargs.setdefault("Xi2max", flags.InDet.Tracking.ActiveConfig.Xi2max)
    kwargs.setdefault("Xi2maxNoAdd",
                      flags.InDet.Tracking.ActiveConfig.Xi2maxNoAdd)
    kwargs.setdefault("nWeightedClustersMin",
                      flags.InDet.Tracking.ActiveConfig.nWeightedClustersMin)
    kwargs.setdefault("Xi2maxMultiTracks",
                      flags.InDet.Tracking.ActiveConfig.Xi2max)
    kwargs.setdefault("UseAssociationTool", False)

    kwargs.setdefault("useBremModel",
                      flags.InDet.Tracking.ActiveConfig.name == "2023fix")

    kwargs.setdefault("CosmicTrack",
                      flags.InDet.Tracking.ActiveConfig.input_name == "cosmics")

    acc.setPrivateTools(CompFactory.InDet.SiTrackMaker_xk(name, **kwargs))
    return acc


def ITkSiTrackMaker_xkCfg(flags, name="ITkSiTrackMaker", **kwargs):
    # To produce BeamSpotData and AtlasFieldCacheCondObj
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = BeamSpotCondAlgCfg(flags)
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))

    if "RoadTool" not in kwargs:
        from InDetConfig.SiDetElementsRoadToolConfig import (
            ITkSiDetElementsRoadMaker_xkCfg)
        kwargs.setdefault("RoadTool", acc.popToolsAndMerge(
            ITkSiDetElementsRoadMaker_xkCfg(flags)))

    if "CombinatorialTrackFinder" not in kwargs:
        from InDetConfig.SiCombinatorialTrackFinderToolConfig import (
            ITkSiCombinatorialTrackFinder_xkCfg)
        kwargs.setdefault("CombinatorialTrackFinder", acc.popToolsAndMerge(
            ITkSiCombinatorialTrackFinder_xkCfg(flags)))

    kwargs.setdefault("useSCT", flags.ITk.Tracking.ActiveConfig.useITkStrip)
    kwargs.setdefault("usePixel", flags.ITk.Tracking.ActiveConfig.useITkPixel)
    kwargs.setdefault("etaBins", flags.ITk.Tracking.ActiveConfig.etaBins)
    kwargs.setdefault("pTBins", flags.ITk.Tracking.ActiveConfig.minPT)
    kwargs.setdefault("pTmin", flags.ITk.Tracking.ActiveConfig.minPT[0])
    kwargs.setdefault("pTminBrem",
                      flags.ITk.Tracking.ActiveConfig.minPTBrem[0])
    kwargs.setdefault("nClustersMin",
                      min(flags.ITk.Tracking.ActiveConfig.minClusters))
    kwargs.setdefault("nHolesMax",
                      flags.ITk.Tracking.ActiveConfig.nHolesMax[0])
    kwargs.setdefault("nHolesGapMax",
                      flags.ITk.Tracking.ActiveConfig.nHolesGapMax[0])
    kwargs.setdefault("SeedsFilterLevel",
                      flags.ITk.Tracking.ActiveConfig.seedFilterLevel)
    kwargs.setdefault("Xi2max", flags.ITk.Tracking.ActiveConfig.Xi2max[0])
    kwargs.setdefault("Xi2maxNoAdd",
                      flags.ITk.Tracking.ActiveConfig.Xi2maxNoAdd[0])
    kwargs.setdefault("nWeightedClustersMin",
                      flags.ITk.Tracking.ActiveConfig.nWeightedClustersMin[0])
    kwargs.setdefault("CosmicTrack", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("Xi2maxMultiTracks",
                      flags.ITk.Tracking.ActiveConfig.Xi2max[0])
    kwargs.setdefault("doMultiTracksProd", True)

    # Disabled for second passes in reco
    kwargs.setdefault("useBremModel",
                      flags.Detector.EnableCalo and
                      flags.Tracking.doBremRecovery and
                      flags.ITk.Tracking.ActiveConfig.extension == "")
    kwargs.setdefault("doCaloSeededBrem", flags.Tracking.doCaloSeededBrem)

    if kwargs["useBremModel"] and kwargs["doCaloSeededBrem"]:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            ITkCaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(ITkCaloClusterROIPhiRZContainerMakerCfg(flags))

    kwargs.setdefault("doHadCaloSeedSSS", flags.Tracking.doHadCaloSeededSSS)

    if kwargs["doHadCaloSeedSSS"]:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            ITkHadCaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(ITkHadCaloClusterROIPhiRZContainerMakerCfg(flags))

    kwargs.setdefault("phiWidth",
                      flags.ITk.Tracking.ActiveConfig.phiWidthBrem[0])
    kwargs.setdefault("etaWidth",
                      flags.ITk.Tracking.ActiveConfig.etaWidthBrem[0])
    kwargs.setdefault("EMROIPhiRZContainer", "ITkCaloClusterROIPhiRZ0GeV")
    kwargs.setdefault("HadROIPhiRZContainer", "ITkHadCaloClusterROIPhiRZ")
    kwargs.setdefault("UseAssociationTool",
                      flags.ITk.Tracking.ActiveConfig.usePrdAssociationTool)
    kwargs.setdefault("ITKGeometry", True)

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("TrackPatternRecoInfo",
                          'SiSpacePointsSeedMaker_Cosmic')

    elif flags.ITk.Tracking.ActiveConfig.extension == "ConversionFinding":
        kwargs.setdefault("TrackPatternRecoInfo",
                          'SiSpacePointsSeedMaker_ITkConversionTracks')

    elif flags.ITk.Tracking.ActiveConfig.extension == "LargeD0":
        kwargs.setdefault("TrackPatternRecoInfo",
                          'SiSpacePointsSeedMaker_LargeD0')

    elif flags.ITk.Tracking.ActiveConfig.extension == "LowPt":
        kwargs.setdefault("TrackPatternRecoInfo",
                          'SiSpacePointsSeedMaker_LowMomentum')

    else:
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSPSeededFinder')

    if (flags.Tracking.doStoreTrackSeeds and
            "SeedToTrackConversion" not in kwargs):
        from InDetConfig.SeedToTrackConversionToolConfig import (
            ITkSeedToTrackConversionToolCfg)
        kwargs.setdefault("SeedToTrackConversion", acc.popToolsAndMerge(
            ITkSeedToTrackConversionToolCfg(flags)))
        kwargs.setdefault("SeedSegmentsWrite", True)

    acc.setPrivateTools(CompFactory.InDet.SiTrackMaker_xk(
        name+flags.ITk.Tracking.ActiveConfig.extension, **kwargs))
    return acc
