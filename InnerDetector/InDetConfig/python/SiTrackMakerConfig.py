# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of SiTrackMakerTool_xk package
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

def SiTrackMaker_xkCfg(flags, name="InDetSiTrackMaker", **kwargs):
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc = BeamSpotCondAlgCfg(flags)             # To produce BeamSpotData
    acc.merge(AtlasFieldCacheCondAlgCfg(flags)) # To produce AtlasFieldCacheCondObj

    if "RoadTool" not in kwargs:
        from InDetConfig.SiDetElementsRoadToolConfig import SiDetElementsRoadMaker_xkCfg
        kwargs.setdefault("RoadTool", acc.popToolsAndMerge(
            SiDetElementsRoadMaker_xkCfg(flags)))

    if "CombinatorialTrackFinder" not in kwargs:
        from InDetConfig.SiCombinatorialTrackFinderToolConfig import SiCombinatorialTrackFinder_xkCfg
        kwargs.setdefault("CombinatorialTrackFinder", acc.popToolsAndMerge(
            SiCombinatorialTrackFinder_xkCfg(flags)))

    kwargs.setdefault("useSCT", flags.InDet.Tracking.ActivePass.useSCT)
    kwargs.setdefault("usePixel", flags.InDet.Tracking.ActivePass.usePixel)

    kwargs.setdefault("pTmin", flags.InDet.Tracking.ActivePass.minPT)
    kwargs.setdefault("pTminBrem", flags.InDet.Tracking.ActivePass.minPTBrem)
    kwargs.setdefault("pTminSSS", flags.InDet.Tracking.ActivePass.pT_SSScut)
    kwargs.setdefault("nClustersMin", flags.InDet.Tracking.ActivePass.minClusters)
    kwargs.setdefault("nHolesMax", flags.InDet.Tracking.ActivePass.nHolesMax)
    kwargs.setdefault("nHolesGapMax", flags.InDet.Tracking.ActivePass.nHolesGapMax)
    kwargs.setdefault("SeedsFilterLevel", flags.InDet.Tracking.ActivePass.seedFilterLevel)
    kwargs.setdefault("Xi2max", flags.InDet.Tracking.ActivePass.Xi2max)
    kwargs.setdefault("Xi2maxNoAdd", flags.InDet.Tracking.ActivePass.Xi2maxNoAdd)
    kwargs.setdefault("nWeightedClustersMin", flags.InDet.Tracking.ActivePass.nWeightedClustersMin)

    kwargs.setdefault("CosmicTrack", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("Xi2maxMultiTracks", flags.InDet.Tracking.ActivePass.Xi2max)
    kwargs.setdefault("useSSSseedsFilter", True)
    kwargs.setdefault("doMultiTracksProd", True)

    kwargs.setdefault("useBremModel", flags.InDet.Tracking.doBremRecovery \
                      and flags.Detector.EnableCalo \
                      and (flags.InDet.Tracking.ActivePass.extension=="" \
                           or flags.InDet.Tracking.ActivePass.extension=="BLS"))
    kwargs.setdefault("doCaloSeededBrem", flags.InDet.Tracking.doCaloSeededBrem and flags.Detector.EnableCalo)
    if kwargs["useBremModel"] and kwargs["doCaloSeededBrem"]:
        from InDetConfig.InDetCaloClusterROISelectorConfig import CaloClusterROIPhiRZContainerMakerCfg
        acc.merge(CaloClusterROIPhiRZContainerMakerCfg(flags))

    kwargs.setdefault("doHadCaloSeedSSS", flags.InDet.Tracking.doHadCaloSeededSSS and flags.Detector.EnableCalo)
    if kwargs["doHadCaloSeedSSS"]:
        from InDetConfig.InDetCaloClusterROISelectorConfig import HadCaloClusterROIPhiRZContainerMakerCfg
        acc.merge(HadCaloClusterROIPhiRZContainerMakerCfg(flags))

    kwargs.setdefault("phiWidth", flags.InDet.Tracking.ActivePass.phiWidthBrem)
    kwargs.setdefault("etaWidth", flags.InDet.Tracking.ActivePass.etaWidthBrem)
    kwargs.setdefault("EMROIPhiRZContainer", "InDetCaloClusterROIPhiRZ0GeV")
    kwargs.setdefault("HadROIPhiRZContainer", "InDetHadCaloClusterROIPhiRZ")
    kwargs.setdefault("UseAssociationTool", flags.InDet.Tracking.ActivePass.usePrdAssociationTool)

    if flags.InDet.Tracking.ActivePass.extension == "DBM":
        kwargs.setdefault("MagneticFieldMode", "NoField")
        kwargs.setdefault("useBremModel", False)
        kwargs.setdefault("doMultiTracksProd", False)
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSPSeededFinder')
        kwargs.setdefault("pTminSSS", -1)
        kwargs.setdefault("CosmicTrack", False)
        kwargs.setdefault("useSSSseedsFilter", False)
        kwargs.setdefault("doCaloSeededBrem", False)
        kwargs.setdefault("doHadCaloSeedSSS", False)

    elif flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_Cosmic')

    elif flags.Reco.EnableHI:
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_HeavyIon')
    
    elif flags.InDet.Tracking.ActivePass.extension == "LowPt":
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_LowMomentum')

    elif flags.InDet.Tracking.ActivePass.extension == "VeryLowPt" \
         or (flags.InDet.Tracking.ActivePass.extension == "Pixel" \
             and flags.InDet.Tracking.doMinBias):
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_VeryLowMomentum')

    elif flags.InDet.Tracking.ActivePass.extension == "BeamGas":
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_BeamGas')

    elif flags.InDet.Tracking.ActivePass.extension == "Forward":
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_ForwardTracks')

    elif flags.InDet.Tracking.ActivePass.extension == "LargeD0" \
         or flags.InDet.Tracking.ActivePass.extension == "R3LargeD0" \
         or flags.InDet.Tracking.ActivePass.extension == "LowPtLargeD0":
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_LargeD0')

    else:
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSPSeededFinder')

    if flags.InDet.Tracking.doStoreTrackSeeds and "SeedToTrackConversion" not in kwargs:
        from InDetConfig.SeedToTrackConversionToolConfig import SeedToTrackConversionToolCfg
        kwargs.setdefault("SeedToTrackConversion", acc.popToolsAndMerge(
            SeedToTrackConversionToolCfg(flags)))
        kwargs.setdefault("SeedSegmentsWrite", True)

    acc.setPrivateTools(CompFactory.InDet.SiTrackMaker_xk(name = name+flags.InDet.Tracking.ActivePass.extension, **kwargs))
    return acc

def TrigSiTrackMaker_xkCfg(flags, name="TrigSiTrackMaker", **kwargs):
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc = BeamSpotCondAlgCfg(flags)             # To produce BeamSpotData
    acc.merge(AtlasFieldCacheCondAlgCfg(flags)) # To produce AtlasFieldCacheCondObj

    if "RoadTool" not in kwargs:
        from InDetConfig.SiDetElementsRoadToolConfig import SiDetElementsRoadMaker_xkCfg
        kwargs.setdefault("RoadTool", acc.popToolsAndMerge(
            SiDetElementsRoadMaker_xkCfg(flags)))

    if "CombinatorialTrackFinder" not in kwargs:
        from InDetConfig.SiCombinatorialTrackFinderToolConfig import SiCombinatorialTrackFinder_xk_Trig_Cfg
        kwargs.setdefault("CombinatorialTrackFinder", acc.popToolsAndMerge(
            SiCombinatorialTrackFinder_xk_Trig_Cfg(flags)))

    kwargs.setdefault("pTmin", flags.InDet.Tracking.ActivePass.minPT)
    kwargs.setdefault("nClustersMin", flags.InDet.Tracking.ActivePass.minClusters)
    kwargs.setdefault("nHolesMax", flags.InDet.Tracking.ActivePass.nHolesMax)
    kwargs.setdefault("nHolesGapMax", flags.InDet.Tracking.ActivePass.nHolesGapMax)
    kwargs.setdefault("SeedsFilterLevel", flags.InDet.Tracking.ActivePass.seedFilterLevel)
    kwargs.setdefault("Xi2max", flags.InDet.Tracking.ActivePass.Xi2max)
    kwargs.setdefault("Xi2maxNoAdd", flags.InDet.Tracking.ActivePass.Xi2maxNoAdd)
    kwargs.setdefault("nWeightedClustersMin", flags.InDet.Tracking.ActivePass.nWeightedClustersMin)
    kwargs.setdefault("Xi2maxMultiTracks", flags.InDet.Tracking.ActivePass.Xi2max)
    kwargs.setdefault("UseAssociationTool", False)

    acc.setPrivateTools(CompFactory.InDet.SiTrackMaker_xk(name, **kwargs))
    return acc

def ITkSiTrackMaker_xkCfg(flags, name="ITkSiTrackMaker", **kwargs):
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc = BeamSpotCondAlgCfg(flags)             # To produce BeamSpotData
    acc.merge(AtlasFieldCacheCondAlgCfg(flags)) # To produce AtlasFieldCacheCondObj

    if "RoadTool" not in kwargs:
        from InDetConfig.SiDetElementsRoadToolConfig import ITkSiDetElementsRoadMaker_xkCfg
        kwargs.setdefault("RoadTool", acc.popToolsAndMerge(
            ITkSiDetElementsRoadMaker_xkCfg(flags)))

    if "CombinatorialTrackFinder" not in kwargs:
        from InDetConfig.SiCombinatorialTrackFinderToolConfig import ITkSiCombinatorialTrackFinder_xkCfg
        kwargs.setdefault("CombinatorialTrackFinder", acc.popToolsAndMerge(
            ITkSiCombinatorialTrackFinder_xkCfg(flags)))

    kwargs.setdefault("useSCT", flags.ITk.Tracking.ActivePass.useITkStrip)
    kwargs.setdefault("usePixel", flags.ITk.Tracking.ActivePass.useITkPixel)
    kwargs.setdefault("etaBins", flags.ITk.Tracking.ActivePass.etaBins)
    kwargs.setdefault("pTBins", flags.ITk.Tracking.ActivePass.minPT)
    kwargs.setdefault("pTmin", flags.ITk.Tracking.ActivePass.minPT[0])
    kwargs.setdefault("pTminBrem", flags.ITk.Tracking.ActivePass.minPTBrem[0])
    kwargs.setdefault("nClustersMin", min(flags.ITk.Tracking.ActivePass.minClusters))
    kwargs.setdefault("nHolesMax", flags.ITk.Tracking.ActivePass.nHolesMax[0])
    kwargs.setdefault("nHolesGapMax", flags.ITk.Tracking.ActivePass.nHolesGapMax[0])
    kwargs.setdefault("SeedsFilterLevel", flags.ITk.Tracking.ActivePass.seedFilterLevel)
    kwargs.setdefault("Xi2max", flags.ITk.Tracking.ActivePass.Xi2max[0])
    kwargs.setdefault("Xi2maxNoAdd", flags.ITk.Tracking.ActivePass.Xi2maxNoAdd[0])
    kwargs.setdefault("nWeightedClustersMin", flags.ITk.Tracking.ActivePass.nWeightedClustersMin[0])
    kwargs.setdefault("CosmicTrack", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("Xi2maxMultiTracks", flags.ITk.Tracking.ActivePass.Xi2max[0])
    kwargs.setdefault("doMultiTracksProd", True)

    kwargs.setdefault("useBremModel", flags.Detector.EnableCalo \
                      and flags.ITk.Tracking.doBremRecovery \
                      and flags.ITk.Tracking.ActivePass.extension == "") # Disabled for second passes in reco
    kwargs.setdefault("doCaloSeededBrem", flags.ITk.Tracking.doCaloSeededBrem and flags.Detector.EnableCalo)
    if kwargs["useBremModel"] and kwargs["doCaloSeededBrem"]:
        from InDetConfig.InDetCaloClusterROISelectorConfig import ITkCaloClusterROIPhiRZContainerMakerCfg
        acc.merge(ITkCaloClusterROIPhiRZContainerMakerCfg(flags))

    kwargs.setdefault("doHadCaloSeedSSS", flags.ITk.Tracking.doHadCaloSeededSSS and flags.Detector.EnableCalo)
    if kwargs["doHadCaloSeedSSS"]:
        from InDetConfig.InDetCaloClusterROISelectorConfig import ITkHadCaloClusterROIPhiRZContainerMakerCfg
        acc.merge(ITkHadCaloClusterROIPhiRZContainerMakerCfg(flags))

    kwargs.setdefault("phiWidth", flags.ITk.Tracking.ActivePass.phiWidthBrem[0])
    kwargs.setdefault("etaWidth", flags.ITk.Tracking.ActivePass.etaWidthBrem[0])
    kwargs.setdefault("EMROIPhiRZContainer", "ITkCaloClusterROIPhiRZ0GeV")
    kwargs.setdefault("HadROIPhiRZContainer", "ITkHadCaloClusterROIPhiRZ")
    kwargs.setdefault("UseAssociationTool", flags.ITk.Tracking.ActivePass.usePrdAssociationTool)
    kwargs.setdefault("ITKGeometry", True)

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_Cosmic')

    elif flags.ITk.Tracking.ActivePass.extension == "ConversionFinding":
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_ITkConversionTracks')

    elif flags.ITk.Tracking.ActivePass.extension == "LargeD0":
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_LargeD0')

    else:
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSPSeededFinder')


    if flags.ITk.Tracking.doStoreTrackSeeds and "SeedToTrackConversion" not in kwargs:
        from InDetConfig.SeedToTrackConversionToolConfig import ITkSeedToTrackConversionToolCfg
        kwargs.setdefault("SeedToTrackConversion", acc.popToolsAndMerge(
            ITkSeedToTrackConversionToolCfg(flags)))
        kwargs.setdefault("SeedSegmentsWrite", True)

    acc.setPrivateTools(CompFactory.InDet.SiTrackMaker_xk(name = name+flags.ITk.Tracking.ActivePass.extension, **kwargs))
    return acc
