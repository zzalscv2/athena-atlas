# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkSiSPSeededTrackFinderCfg(flags, name="ITkSiSpTrackFinder", **kwargs) :
    acc = ComponentAccumulator()

    from InDetConfig.SiTrackMakerConfig import ITkSiTrackMaker_xkCfg
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

