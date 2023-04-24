# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SiSpacePointsSeedTool_xk package
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType


def SiSpacePointsSeedMakerCfg(flags, name="InDetSpSeedsMaker", **kwargs):
    acc = ComponentAccumulator()
    #
    # --- Space points seeds maker, use different ones for cosmics and collisions
    #
    if flags.Beam.Type is BeamType.Cosmics:
        SiSpacePointsSeedMaker = (
            CompFactory.InDet.SiSpacePointsSeedMaker_Cosmic)
    elif flags.Reco.EnableHI:
        SiSpacePointsSeedMaker = (
            CompFactory.InDet.SiSpacePointsSeedMaker_HeavyIon)
    elif flags.Tracking.ActiveConfig.isLowPt:
        SiSpacePointsSeedMaker = (
            CompFactory.InDet.SiSpacePointsSeedMaker_LowMomentum)
    elif flags.Tracking.ActiveConfig.extension == "BeamGas":
        SiSpacePointsSeedMaker = (
            CompFactory.InDet.SiSpacePointsSeedMaker_BeamGas)
    else:
        SiSpacePointsSeedMaker = CompFactory.InDet.SiSpacePointsSeedMaker_ATLxk
        if flags.Tracking.writeSeedValNtuple:
            kwargs.setdefault("WriteNtuple", True)
            acc.addService(CompFactory.THistSvc(
                Output=["valNtuples DATAFILE='SeedMakerValidation.root' OPT='RECREATE'"]))

    kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minPT)
    kwargs.setdefault("maxdImpact",
                      flags.Tracking.ActiveConfig.maxPrimaryImpact)
    kwargs.setdefault("maxZ", flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("minZ", -flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("radMax", flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("RapidityCut", flags.Tracking.ActiveConfig.maxEta)

    kwargs.setdefault("usePixel", flags.Tracking.ActiveConfig.usePixel and
                      flags.Tracking.ActiveConfig.extension != "R3LargeD0")
    kwargs.setdefault("SpacePointsPixelName", 'PixelSpacePoints')
    kwargs.setdefault("useSCT", flags.Tracking.ActiveConfig.useSCT and
                      flags.Tracking.ActiveConfig.useSCTSeeding)
    kwargs.setdefault("SpacePointsSCTName", 'SCT_SpacePoints')
    kwargs.setdefault("useOverlapSpCollection",
                      flags.Tracking.ActiveConfig.useSCT and
                      flags.Tracking.ActiveConfig.useSCTSeeding)
    kwargs.setdefault("SpacePointsOverlapName", 'OverlapSpacePoints')

    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", (
            'InDetPRDtoTrackMap' + flags.Tracking.ActiveConfig.extension))

    if not flags.Reco.EnableHI and (
            flags.Tracking.ActiveConfig.extension == "" or
            flags.Tracking.ActiveConfig.extension == "Forward" or
            flags.Tracking.ActiveConfig.extension == "BLS"):
        kwargs.setdefault("maxdImpactPPS",
                          flags.Tracking.ActiveConfig.maxdImpactPPSSeeds)
        kwargs.setdefault("maxdImpactSSS",
                          flags.Tracking.ActiveConfig.maxdImpactSSSSeeds)
        kwargs.setdefault("maxSeedsForSpacePointStrips",
                          flags.Tracking.ActiveConfig.maxSeedsPerSP_Strips)
        kwargs.setdefault("maxSeedsForSpacePointPixels",
                          flags.Tracking.ActiveConfig.maxSeedsPerSP_Pixels)
        kwargs.setdefault("alwaysKeepConfirmedStripSeeds",
                          flags.Tracking.ActiveConfig.keepAllConfirmedStripSeeds)
        kwargs.setdefault("alwaysKeepConfirmedPixelSeeds",
                          flags.Tracking.ActiveConfig.keepAllConfirmedPixelSeeds)
        kwargs.setdefault("mindRadius", 10)
        kwargs.setdefault("maxSizeSP", 200)
        kwargs.setdefault("dImpactCutSlopeUnconfirmedSSS", 1.25)
        kwargs.setdefault("dImpactCutSlopeUnconfirmedPPP", 2.0)

    if flags.Reco.EnableHI:
        kwargs.setdefault("maxdImpactPPS",
                          flags.Tracking.ActiveConfig.maxdImpactPPSSeeds)
        kwargs.setdefault("maxdImpactSSS",
                          flags.Tracking.ActiveConfig.maxdImpactSSSSeeds)

    if flags.Beam.Type is not BeamType.Cosmics:
        kwargs.setdefault("maxRadius1",
                          0.75*flags.Tracking.ActiveConfig.radMax)
        kwargs.setdefault("maxRadius2",
                          flags.Tracking.ActiveConfig.radMax)
        kwargs.setdefault("maxRadius3",
                          flags.Tracking.ActiveConfig.radMax)

    if flags.Tracking.ActiveConfig.isLowPt:
        kwargs.setdefault("pTmax", flags.Tracking.ActiveConfig.maxPT)
        kwargs.setdefault("mindRadius", 4.0)

    if flags.Tracking.ActiveConfig.extension == "R3LargeD0":
        kwargs.setdefault("optimisePhiBinning", False)
        kwargs.setdefault("etaMax", flags.Tracking.ActiveConfig.maxEta)
        kwargs.setdefault("maxSeedsForSpacePointStrips",
                          flags.Tracking.ActiveConfig.maxSeedsPerSP_Strips)
        kwargs.setdefault("alwaysKeepConfirmedStripSeeds",
                          flags.Tracking.ActiveConfig.keepAllConfirmedStripSeeds)
        kwargs.setdefault("maxdRadius", 150)
        kwargs.setdefault("seedScoreBonusConfirmationSeed", -2000)
    elif flags.Tracking.ActiveConfig.extension == "Forward":
        kwargs.setdefault("checkEta", True)
        kwargs.setdefault("etaMin", flags.Tracking.ActiveConfig.minEta)
        kwargs.setdefault("etaMax", flags.Tracking.ActiveConfig.maxEta)

    acc.setPrivateTools(SiSpacePointsSeedMaker(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def ITkSiSpacePointsSeedMakerCfg(flags, name="ITkSpSeedsMaker", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minPTSeed)
    kwargs.setdefault("maxdImpact",
                      flags.Tracking.ActiveConfig.maxPrimaryImpactSeed)
    kwargs.setdefault("maxZ", flags.Tracking.ActiveConfig.maxZImpactSeed)
    kwargs.setdefault("minZ", -flags.Tracking.ActiveConfig.maxZImpactSeed)
    kwargs.setdefault("radMax", flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("etaMax", flags.Tracking.ActiveConfig.maxEta)

    kwargs.setdefault("usePixel",
                      flags.Tracking.ActiveConfig.useITkPixel and
                      flags.Tracking.ActiveConfig.useITkPixelSeeding)
    kwargs.setdefault("SpacePointsPixelName", 'ITkPixelSpacePoints')
    kwargs.setdefault("useStrip",
                      flags.Tracking.ActiveConfig.useITkStrip and
                      flags.Tracking.ActiveConfig.useITkStripSeeding)
    kwargs.setdefault("SpacePointsStripName", 'ITkStripSpacePoints')
    kwargs.setdefault("useOverlapSpCollection",
                      flags.Tracking.ActiveConfig.useITkStrip and
                      flags.Tracking.ActiveConfig.useITkStripSeeding)
    kwargs.setdefault("SpacePointsOverlapName", 'ITkOverlapSpacePoints')

    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", (
            'ITkPRDtoTrackMap' + flags.Tracking.ActiveConfig.extension))

    if flags.Beam.Type is not BeamType.Cosmics:
        kwargs.setdefault("maxRadius1",
                          0.75*flags.Tracking.ActiveConfig.radMax)
        kwargs.setdefault("maxRadius2",
                          flags.Tracking.ActiveConfig.radMax)
        kwargs.setdefault("maxRadius3",
                          flags.Tracking.ActiveConfig.radMax)

    if flags.ITk.Tracking.doFastTracking:
        kwargs.setdefault("useFastTracking", True)
        kwargs.setdefault("maxSeedsForSpacePoint", 3)

    if flags.Tracking.ActiveConfig.extension == "LargeD0":
        kwargs.setdefault("maxSeedsForSpacePoint", 5)
        kwargs.setdefault("isLRT", True)
        kwargs.setdefault("maxZPPP",
                          flags.Tracking.ActiveConfig.maxZSpacePointsPPPSeeds)
        kwargs.setdefault("maxZSSS",
                          flags.Tracking.ActiveConfig.maxZSpacePointsSSSSeeds)

    if flags.Tracking.writeSeedValNtuple:
        kwargs.setdefault("WriteNtuple", True)
        acc.addService(CompFactory.THistSvc(
            Output=["valNtuples DATAFILE='SeedMakerValidation.root' OPT='RECREATE'"]))

    acc.setPrivateTools(CompFactory.ITk.SiSpacePointsSeedMaker(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc
