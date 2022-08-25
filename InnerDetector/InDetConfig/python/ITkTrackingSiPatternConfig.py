# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

def ITkSiTrackMaker_xkCfg(flags, name="ITkSiTrackMaker", **kwargs) :
    acc = ComponentAccumulator()

    from InDetConfig.SiDetElementsRoadToolConfig import ITkSiDetElementsRoadMaker_xkCfg
    ITkSiDetElementsRoadMaker = acc.popToolsAndMerge(ITkSiDetElementsRoadMaker_xkCfg(flags))

    from InDetConfig.SiCombinatorialTrackFinderToolConfig import ITkSiCombinatorialTrackFinder_xkCfg
    track_finder = acc.popToolsAndMerge(ITkSiCombinatorialTrackFinder_xkCfg(flags))

    kwargs.setdefault("useSCT", flags.ITk.Tracking.ActivePass.useITkStrip)
    kwargs.setdefault("usePixel", flags.ITk.Tracking.ActivePass.useITkPixel)
    kwargs.setdefault("RoadTool", ITkSiDetElementsRoadMaker)
    kwargs.setdefault("CombinatorialTrackFinder", track_finder)
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
    kwargs.setdefault("useBremModel", flags.Detector.EnableCalo and flags.ITk.Tracking.doBremRecovery and flags.ITk.Tracking.ActivePass.extension == "") # Disabled for second passes in reco
    kwargs.setdefault("doCaloSeededBrem", flags.ITk.Tracking.doCaloSeededBrem and flags.Detector.EnableCalo)
    kwargs.setdefault("doHadCaloSeedSSS", flags.ITk.Tracking.doHadCaloSeededSSS and flags.Detector.EnableCalo)
    if kwargs["useBremModel"] and kwargs["doCaloSeededBrem"]:
        from InDetConfig.InDetCaloClusterROISelectorConfig import ITkCaloClusterROIPhiRZContainerMakerCfg
        acc.merge(ITkCaloClusterROIPhiRZContainerMakerCfg(flags))
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

    if flags.ITk.Tracking.doStoreTrackSeeds:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        extrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
        acc.addPublicTool(extrapolator)  # TODO: migrate to private?

        kwargs.setdefault("SeedToTrackConversion", CompFactory.InDet.SeedToTrackConversionTool(
            name="ITkSeedToTrackConversion",
            Extrapolator=extrapolator,
            OutputName=f"SiSPSeedSegments{flags.ITk.Tracking.ActivePass.extension}"))
        kwargs.setdefault("SeedSegmentsWrite", True)

    ITkSiTrackMaker = CompFactory.InDet.SiTrackMaker_xk(name = name+flags.ITk.Tracking.ActivePass.extension, **kwargs)
    acc.setPrivateTools(ITkSiTrackMaker)
    return acc

def ITkSiSPSeededTrackFinderCfg(flags, name="ITkSiSpTrackFinder", **kwargs) :
    acc = ComponentAccumulator()

    ITkSiTrackMaker = acc.popToolsAndMerge(ITkSiTrackMaker_xkCfg(flags))
    from TrkConfig.TrkExRungeKuttaPropagatorConfig import ITkPropagatorCfg
    ITkPropagator = acc.popToolsAndMerge(ITkPropagatorCfg(flags))
    acc.addPublicTool(ITkPropagator)
    from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolNoHoleSearchCfg
    ITkTrackSummaryToolNoHoleSearch = acc.getPrimaryAndMerge(ITkTrackSummaryToolNoHoleSearchCfg(flags))

    if "SeedsTool" not in kwargs:
        ITkSiSpacePointsSeedMaker = None

        if flags.ITk.Tracking.ActivePass.extension != "ConversionFinding" and flags.Acts.TrackFinding.useSiSpacePointSeedMaker:
            from ActsTrkSeedingTool.ActsTrkSeedingToolConfig import ActsTrkSiSpacePointsSeedMakerCfg
            ITkSiSpacePointsSeedMaker = acc.popToolsAndMerge(ActsTrkSiSpacePointsSeedMakerCfg(flags))
        else:
            from InDetConfig.SiSpacePointsSeedToolConfig import ITkSiSpacePointsSeedMakerCfg
            ITkSiSpacePointsSeedMaker = acc.popToolsAndMerge(ITkSiSpacePointsSeedMakerCfg(flags))

        kwargs.setdefault("SeedsTool", ITkSiSpacePointsSeedMaker)

    #
    # --- Setup Track finder using space points seeds
    #
    kwargs.setdefault("TrackTool", ITkSiTrackMaker)
    kwargs.setdefault("PropagatorTool", ITkPropagator)
    if flags.ITk.Tracking.ActivePass.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", 'ITkPRDtoTrackMap'+ flags.ITk.Tracking.ActivePass.extension)
    kwargs.setdefault("TrackSummaryTool", ITkTrackSummaryToolNoHoleSearch)
    kwargs.setdefault("useZvertexTool", False)
    kwargs.setdefault("useZBoundFinding", flags.ITk.Tracking.ActivePass.doZBoundary)
    kwargs.setdefault("ITKGeometry", True)
    kwargs.setdefault("SpacePointsSCTName", "ITkStripSpacePoints")
    kwargs.setdefault("SpacePointsPixelName", "ITkPixelSpacePoints")

    if flags.ITk.Tracking.doFastTracking :
        kwargs.setdefault("doFastTracking", True)

        if 'InDetEtaDependentCutsSvc' not in kwargs :
            from InDet.InDetEtaDependentCutsConfig import ITkEtaDependentCutsSvcCfg
            acc.merge(ITkEtaDependentCutsSvcCfg(flags))
            kwargs.setdefault("InDetEtaDependentCutsSvc", acc.getService("ITkEtaDependentCutsSvc"+flags.ITk.Tracking.ActivePass.extension))

    ITkSiSPSeededTrackFinder = CompFactory.InDet.SiSPSeededTrackFinder(name = name+flags.ITk.Tracking.ActivePass.extension, **kwargs)
    acc.addEventAlgo(ITkSiSPSeededTrackFinder)
    return acc

def ITkSiSPSeededTrackFinderROIConvCfg(flags, name="ITkSiSpTrackFinderROIConv", **kwargs) :
    from InDetConfig.InDetCaloClusterROISelectorConfig import ITkCaloClusterROIPhiRZContainerMakerCfg
    acc = ITkCaloClusterROIPhiRZContainerMakerCfg(flags)

    from RegionSelector.RegSelToolConfig import regSelTool_ITkStrip_Cfg
    RegSelTool_ITkStrip   = acc.popToolsAndMerge(regSelTool_ITkStrip_Cfg(flags))

    kwargs.setdefault("RegSelTool_Strip", RegSelTool_ITkStrip)
    kwargs.setdefault("useITkConvSeeded", True)
    kwargs.setdefault("EMROIPhiRZContainer", "ITkCaloClusterROIPhiRZ15GeVUnordered")

    acc.merge(ITkSiSPSeededTrackFinderCfg(flags, name, **kwargs))
    return acc

def ITkCopyAlgForAmbiCfg(flags, name="ITkCopyAlgForAmbi", InputTrackCollection = None, OutputTrackCollection = None, **kwargs) :
    acc = ComponentAccumulator()

    kwargs.setdefault("CollectionName", InputTrackCollection)
    kwargs.setdefault("AliasName", OutputTrackCollection)
    ITkCopyAlgForAmbi = CompFactory.Trk.TrkCollectionAliasAlg (name = name+flags.ITk.Tracking.ActivePass.extension, **kwargs)
    acc.addEventAlgo(ITkCopyAlgForAmbi)
    return acc

# ------------------------------------------------------------
#
# ----------- Setup Si Pattern for New tracking
#
# ------------------------------------------------------------
def ITkTrackingSiPatternCfg(flags,
                            InputCollections = None,
                            ResolvedTrackCollectionKey = None,
                            SiSPSeededTrackCollectionKey = None ,
                            ClusterSplitProbContainer=''):
    acc = ComponentAccumulator()
    #
    # --- get list of already associated hits (always do this, even if no other tracking ran before)
    #
    if flags.ITk.Tracking.ActivePass.usePrdAssociationTool:
        from InDetConfig.InDetTrackPRD_AssociationConfig import ITkTrackPRD_AssociationCfg
        acc.merge(ITkTrackPRD_AssociationCfg(flags,
                                             name = 'ITkTrackPRD_Association' + flags.ITk.Tracking.ActivePass.extension,
                                             TracksName = list(InputCollections)))

    # ------------------------------------------------------------
    #
    # ----------- SiSPSeededTrackFinder
    #
    # ------------------------------------------------------------

    SiSPSeededTrackFinderCfg = ITkSiSPSeededTrackFinderCfg
    if flags.ITk.Tracking.ActivePass.extension == "ConversionFinding":
        SiSPSeededTrackFinderCfg = ITkSiSPSeededTrackFinderROIConvCfg
    acc.merge(SiSPSeededTrackFinderCfg( flags,
                                        TracksLocation = SiSPSeededTrackCollectionKey))
    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------

    if flags.ITk.Tracking.doFastTracking:
        acc.merge(ITkCopyAlgForAmbiCfg( flags,
                                        InputTrackCollection = SiSPSeededTrackCollectionKey,
                                        OutputTrackCollection = ResolvedTrackCollectionKey ))

    else:
        from TrkConfig.TrkAmbiguitySolverConfig import ITkTrkAmbiguityScoreCfg, ITkTrkAmbiguitySolverCfg
        acc.merge(ITkTrkAmbiguityScoreCfg( flags,
                                           SiSPSeededTrackCollectionKey = SiSPSeededTrackCollectionKey,
                                           ClusterSplitProbContainer = ClusterSplitProbContainer))

        acc.merge(ITkTrkAmbiguitySolverCfg(flags,
                                           ResolvedTrackCollectionKey = ResolvedTrackCollectionKey))

    return acc

