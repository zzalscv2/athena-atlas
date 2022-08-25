# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

def SiTrackMaker_xkCfg(flags, name="InDetSiTrackMaker", **kwargs) :
    acc = ComponentAccumulator()

    kwargs.setdefault("useSCT", flags.InDet.Tracking.ActivePass.useSCT)
    kwargs.setdefault("usePixel", flags.InDet.Tracking.ActivePass.usePixel)

    from InDetConfig.SiDetElementsRoadToolConfig import SiDetElementsRoadMaker_xkCfg
    kwargs.setdefault("RoadTool", acc.popToolsAndMerge(SiDetElementsRoadMaker_xkCfg(flags)))

    from InDetConfig.SiCombinatorialTrackFinderToolConfig import SiCombinatorialTrackFinder_xkCfg
    kwargs.setdefault("CombinatorialTrackFinder", acc.popToolsAndMerge(SiCombinatorialTrackFinder_xkCfg(flags)))

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
    kwargs.setdefault("useBremModel", flags.InDet.Tracking.doBremRecovery and flags.Detector.EnableCalo
                      and (flags.InDet.Tracking.ActivePass.extension=="" or flags.InDet.Tracking.ActivePass.extension=="BLS") )
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

    elif flags.InDet.Tracking.ActivePass.extension == "VeryLowPt" or (flags.InDet.Tracking.ActivePass.extension == "Pixel" and flags.InDet.Tracking.doMinBias):
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_VeryLowMomentum')

    elif flags.InDet.Tracking.ActivePass.extension == "BeamGas":
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_BeamGas')

    elif flags.InDet.Tracking.ActivePass.extension == "Forward":
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_ForwardTracks')

    elif flags.InDet.Tracking.ActivePass.extension == "LargeD0" or flags.InDet.Tracking.ActivePass.extension == "R3LargeD0" or flags.InDet.Tracking.ActivePass.extension == "LowPtLargeD0":
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSpacePointsSeedMaker_LargeD0')

    else:
        kwargs.setdefault("TrackPatternRecoInfo", 'SiSPSeededFinder')

    if flags.InDet.Tracking.doStoreTrackSeeds:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("SeedToTrackConversion", CompFactory.InDet.SeedToTrackConversionTool(
            name="InDet_SeedToTrackConversion",
            Extrapolator=acc.getPrimaryAndMerge(InDetExtrapolatorCfg(flags)),
            OutputName=f"SiSPSeedSegments{flags.InDet.Tracking.ActivePass.extension}",
        ))
        kwargs.setdefault("SeedSegmentsWrite", True)

    InDetSiTrackMaker = CompFactory.InDet.SiTrackMaker_xk(name = name+flags.InDet.Tracking.ActivePass.extension, **kwargs)
    acc.setPrivateTools(InDetSiTrackMaker)
    return acc

def SiSPSeededTrackFinderCfg(flags, name="InDetSiSpTrackFinder", **kwargs) :
    acc = ComponentAccumulator()

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import InDetPropagatorCfg
    InDetPropagator = acc.popToolsAndMerge(InDetPropagatorCfg(flags))
    acc.addPublicTool(InDetPropagator)

    #
    # --- Setup Track finder using space points seeds
    #
    kwargs.setdefault("TrackTool", acc.popToolsAndMerge(SiTrackMaker_xkCfg(flags)))
    kwargs.setdefault("PropagatorTool", InDetPropagator)
    from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolNoHoleSearchCfg
    kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(InDetTrackSummaryToolNoHoleSearchCfg(flags)))

    if "SeedsTool" not in kwargs:
        from InDetConfig.SiSpacePointsSeedToolConfig import SiSpacePointsSeedMakerCfg
        kwargs.setdefault("SeedsTool", acc.popToolsAndMerge(
            SiSpacePointsSeedMakerCfg(flags)))

    kwargs.setdefault("useMBTSTimeDiff", flags.Reco.EnableHI) # Heavy-ion config

    if flags.InDet.Tracking.ActivePass.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", 'InDetPRDtoTrackMap'+ flags.InDet.Tracking.ActivePass.extension)

    if flags.InDet.Tracking.ActivePass.extension == "Forward":
        kwargs.setdefault("useZvertexTool", flags.Reco.EnableHI) # For heavy-ion
        kwargs.setdefault("useZBoundFinding", False)
    elif flags.InDet.Tracking.ActivePass.extension == "DBM":
        kwargs.setdefault("useZvertexTool", False)
        kwargs.setdefault("useZBoundFinding", False)
    else:
        kwargs.setdefault("useZvertexTool", flags.Reco.EnableHI) # For heavy-ion
        kwargs.setdefault("useZBoundFinding", flags.InDet.Tracking.ActivePass.doZBoundary)
    
    #
    # --- Z-coordinates primary vertices finder (only for collisions)
    #
    if kwargs["useZvertexTool"] and "ZvertexTool" not in kwargs:
        from InDetConfig.SiZvertexToolConfig import SiZvertexMaker_xkCfg
        kwargs.setdefault("ZvertexTool", acc.popToolsAndMerge(
            SiZvertexMaker_xkCfg(flags)))

    if flags.Reco.EnableHI:
        kwargs.setdefault("FreeClustersCut",2) #Heavy Ion optimization from Igor

    InDetSiSPSeededTrackFinder = CompFactory.InDet.SiSPSeededTrackFinder(name = name+flags.InDet.Tracking.ActivePass.extension, **kwargs)
    acc.addEventAlgo(InDetSiSPSeededTrackFinder)
    return acc

# ------------------------------------------------------------
#
# ----------- Setup Si Pattern for New tracking
#
# ------------------------------------------------------------
def TrackingSiPatternCfg(flags,
                         InputCollections = None,
                         ResolvedTrackCollectionKey = None,
                         SiSPSeededTrackCollectionKey = None ,
                         ClusterSplitProbContainer=''):
    acc = ComponentAccumulator()
    #
    # --- get list of already associated hits (always do this, even if no other tracking ran before)
    #
    if flags.InDet.Tracking.ActivePass.usePrdAssociationTool:
        from InDetConfig.InDetTrackPRD_AssociationConfig import InDetTrackPRD_AssociationCfg
        acc.merge(InDetTrackPRD_AssociationCfg(flags,
                                               name = 'InDetTrackPRD_Association' + flags.InDet.Tracking.ActivePass.extension,
                                               TracksName = list(InputCollections)))

    # ------------------------------------------------------------
    #
    # ----------- SiSPSeededTrackFinder
    #
    # ------------------------------------------------------------

    acc.merge(SiSPSeededTrackFinderCfg( flags,
                                        TracksLocation = SiSPSeededTrackCollectionKey))

    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------

    from TrkConfig.TrkAmbiguitySolverConfig import TrkAmbiguityScoreCfg, TrkAmbiguitySolverCfg
    acc.merge(TrkAmbiguityScoreCfg( flags,
                                    SiSPSeededTrackCollectionKey = SiSPSeededTrackCollectionKey,
                                    ClusterSplitProbContainer = ClusterSplitProbContainer))

    acc.merge(TrkAmbiguitySolverCfg(flags,
                                    ResolvedTrackCollectionKey = ResolvedTrackCollectionKey,
                                    ClusterSplitProbContainer = ClusterSplitProbContainer))

    return acc


