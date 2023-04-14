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
    elif flags.InDet.Tracking.ActiveConfig.isLowPt:
        SiSpacePointsSeedMaker = (
            CompFactory.InDet.SiSpacePointsSeedMaker_LowMomentum)
    elif flags.InDet.Tracking.ActiveConfig.extension == "BeamGas":
        SiSpacePointsSeedMaker = (
            CompFactory.InDet.SiSpacePointsSeedMaker_BeamGas)
    else:
        SiSpacePointsSeedMaker = CompFactory.InDet.SiSpacePointsSeedMaker_ATLxk
        if flags.Tracking.writeSeedValNtuple:
            kwargs.setdefault("WriteNtuple", True)
            acc.addService(CompFactory.THistSvc(
                Output=["valNtuples DATAFILE='SeedMakerValidation.root' OPT='RECREATE'"]))

    kwargs.setdefault("pTmin", flags.InDet.Tracking.ActiveConfig.minPT)
    kwargs.setdefault("maxdImpact",
                      flags.InDet.Tracking.ActiveConfig.maxPrimaryImpact)
    kwargs.setdefault("maxZ", flags.InDet.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("minZ", -flags.InDet.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("radMax", flags.InDet.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("RapidityCut",  flags.InDet.Tracking.ActiveConfig.maxEta)

    kwargs.setdefault("usePixel",
                      flags.InDet.Tracking.ActiveConfig.usePixel and
                      flags.InDet.Tracking.ActiveConfig.extension != "R3LargeD0")
    kwargs.setdefault("SpacePointsPixelName", 'PixelSpacePoints')
    kwargs.setdefault("useSCT", flags.InDet.Tracking.ActiveConfig.useSCT and
                      flags.InDet.Tracking.ActiveConfig.useSCTSeeding)
    kwargs.setdefault("SpacePointsSCTName", 'SCT_SpacePoints')
    kwargs.setdefault("useOverlapSpCollection",
                      flags.InDet.Tracking.ActiveConfig.useSCT and
                      flags.InDet.Tracking.ActiveConfig.useSCTSeeding)
    kwargs.setdefault("SpacePointsOverlapName", 'OverlapSpacePoints')

    if flags.InDet.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", 'InDetPRDtoTrackMap' +
                          flags.InDet.Tracking.ActiveConfig.extension)

    if not flags.Reco.EnableHI and (
            flags.InDet.Tracking.ActiveConfig.extension == "" or
            flags.InDet.Tracking.ActiveConfig.extension == "Forward" or
            flags.InDet.Tracking.ActiveConfig.extension == "BLS"):
        kwargs.setdefault("maxdImpactPPS",
                          flags.InDet.Tracking.ActiveConfig.maxdImpactPPSSeeds)
        kwargs.setdefault("maxdImpactSSS",
                          flags.InDet.Tracking.ActiveConfig.maxdImpactSSSSeeds)
        kwargs.setdefault("maxSeedsForSpacePointStrips",
                          flags.InDet.Tracking.ActiveConfig.maxSeedsPerSP_Strips)
        kwargs.setdefault("maxSeedsForSpacePointPixels",
                          flags.InDet.Tracking.ActiveConfig.maxSeedsPerSP_Pixels)
        kwargs.setdefault("alwaysKeepConfirmedStripSeeds",
                          flags.InDet.Tracking.ActiveConfig.keepAllConfirmedStripSeeds)
        kwargs.setdefault("alwaysKeepConfirmedPixelSeeds",
                          flags.InDet.Tracking.ActiveConfig.keepAllConfirmedPixelSeeds)
        kwargs.setdefault("mindRadius", 10)
        kwargs.setdefault("maxSizeSP", 200)
        kwargs.setdefault("dImpactCutSlopeUnconfirmedSSS", 1.25)
        kwargs.setdefault("dImpactCutSlopeUnconfirmedPPP", 2.0)

    if flags.Reco.EnableHI:
        kwargs.setdefault("maxdImpactPPS",
                          flags.InDet.Tracking.ActiveConfig.maxdImpactPPSSeeds)
        kwargs.setdefault("maxdImpactSSS",
                          flags.InDet.Tracking.ActiveConfig.maxdImpactSSSSeeds)

    if flags.Beam.Type is not BeamType.Cosmics:
        kwargs.setdefault("maxRadius1",
                          0.75*flags.InDet.Tracking.ActiveConfig.radMax)
        kwargs.setdefault("maxRadius2",
                          flags.InDet.Tracking.ActiveConfig.radMax)
        kwargs.setdefault("maxRadius3",
                          flags.InDet.Tracking.ActiveConfig.radMax)

    if flags.InDet.Tracking.ActiveConfig.isLowPt:
        kwargs.setdefault("pTmax", flags.InDet.Tracking.ActiveConfig.maxPT)
        kwargs.setdefault("mindRadius", 4.0)

    if flags.InDet.Tracking.ActiveConfig.extension == "R3LargeD0":
        kwargs.setdefault("optimisePhiBinning", False)
        kwargs.setdefault("etaMax", flags.InDet.Tracking.ActiveConfig.maxEta)
        kwargs.setdefault("maxSeedsForSpacePointStrips",
                          flags.InDet.Tracking.ActiveConfig.maxSeedsPerSP_Strips)
        kwargs.setdefault("alwaysKeepConfirmedStripSeeds",
                          flags.InDet.Tracking.ActiveConfig.keepAllConfirmedStripSeeds)
        kwargs.setdefault("maxdRadius", 150)
        kwargs.setdefault("seedScoreBonusConfirmationSeed", -2000)
    elif flags.InDet.Tracking.ActiveConfig.extension == "Forward":
        kwargs.setdefault("checkEta", True)
        kwargs.setdefault("etaMin", flags.InDet.Tracking.ActiveConfig.minEta)
        kwargs.setdefault("etaMax", flags.InDet.Tracking.ActiveConfig.maxEta)

    acc.setPrivateTools(SiSpacePointsSeedMaker(
        name=name+flags.InDet.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def ITkSiSpacePointsSeedMakerCfg(flags, name="ITkSpSeedsMaker", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("pTmin", flags.ITk.Tracking.ActiveConfig.minPTSeed)
    kwargs.setdefault("maxdImpact",
                      flags.ITk.Tracking.ActiveConfig.maxPrimaryImpactSeed)
    kwargs.setdefault("maxZ", flags.ITk.Tracking.ActiveConfig.maxZImpactSeed)
    kwargs.setdefault("minZ", -flags.ITk.Tracking.ActiveConfig.maxZImpactSeed)
    kwargs.setdefault("radMax", flags.ITk.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("etaMax", flags.ITk.Tracking.ActiveConfig.maxEta)

    kwargs.setdefault("usePixel",
                      flags.ITk.Tracking.ActiveConfig.useITkPixel and
                      flags.ITk.Tracking.ActiveConfig.useITkPixelSeeding)
    kwargs.setdefault("SpacePointsPixelName", 'ITkPixelSpacePoints')
    kwargs.setdefault("useStrip",
                      flags.ITk.Tracking.ActiveConfig.useITkStrip and
                      flags.ITk.Tracking.ActiveConfig.useITkStripSeeding)
    kwargs.setdefault("SpacePointsStripName", 'ITkStripSpacePoints')
    kwargs.setdefault("useOverlapSpCollection",
                      flags.ITk.Tracking.ActiveConfig.useITkStrip and
                      flags.ITk.Tracking.ActiveConfig.useITkStripSeeding)
    kwargs.setdefault("SpacePointsOverlapName", 'ITkOverlapSpacePoints')

    if flags.ITk.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", 'ITkPRDtoTrackMap' +
                          flags.ITk.Tracking.ActiveConfig.extension)

    if flags.Beam.Type is not BeamType.Cosmics:
        kwargs.setdefault("maxRadius1",
                          0.75*flags.ITk.Tracking.ActiveConfig.radMax)
        kwargs.setdefault("maxRadius2",
                          flags.ITk.Tracking.ActiveConfig.radMax)
        kwargs.setdefault("maxRadius3",
                          flags.ITk.Tracking.ActiveConfig.radMax)

    if flags.ITk.Tracking.doFastTracking:
        kwargs.setdefault("useFastTracking", True)
        kwargs.setdefault("maxSeedsForSpacePoint", 3)

    if flags.ITk.Tracking.ActiveConfig.extension == "LargeD0":
        kwargs.setdefault("maxSeedsForSpacePoint", 5)
        kwargs.setdefault("isLRT", True)
        kwargs.setdefault("maxZPPP",
                          flags.ITk.Tracking.ActiveConfig.maxZSpacePointsPPPSeeds)
        kwargs.setdefault("maxZSSS",
                          flags.ITk.Tracking.ActiveConfig.maxZSpacePointsSSSSeeds)

    if flags.Tracking.writeSeedValNtuple:
        kwargs.setdefault("WriteNtuple", True)
        acc.addService(CompFactory.THistSvc(
            Output=["valNtuples DATAFILE='SeedMakerValidation.root' OPT='RECREATE'"]))

    acc.setPrivateTools(CompFactory.ITk.SiSpacePointsSeedMaker(
        name=name+flags.ITk.Tracking.ActiveConfig.extension, **kwargs))
    return acc
