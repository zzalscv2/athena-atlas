# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of SiSpacePointsSeedTool_xk package
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType

def SiSpacePointsSeedMakerCfg(flags, name="InDetSpSeedsMaker", **kwargs) :
    acc = ComponentAccumulator()
    #
    # --- Space points seeds maker, use different ones for cosmics and collisions
    #
    if flags.Beam.Type is BeamType.Cosmics:
        SiSpacePointsSeedMaker = CompFactory.InDet.SiSpacePointsSeedMaker_Cosmic
    elif flags.Reco.EnableHI:
        SiSpacePointsSeedMaker = CompFactory.InDet.SiSpacePointsSeedMaker_HeavyIon
    elif flags.InDet.Tracking.ActivePass.isLowPt:
        SiSpacePointsSeedMaker = CompFactory.InDet.SiSpacePointsSeedMaker_LowMomentum
    elif flags.InDet.Tracking.ActivePass.extension == "BeamGas":
        SiSpacePointsSeedMaker = CompFactory.InDet.SiSpacePointsSeedMaker_BeamGas
    else:
        SiSpacePointsSeedMaker = CompFactory.InDet.SiSpacePointsSeedMaker_ATLxk
        if flags.InDet.Tracking.writeSeedValNtuple:
            kwargs.setdefault("WriteNtuple", True)
            HistService = CompFactory.THistSvc(Output = ["valNtuples DATAFILE='SeedMakerValidation.root' OPT='RECREATE'"])
            acc.addService(HistService)

    kwargs.setdefault("pTmin", flags.InDet.Tracking.ActivePass.minPT)
    kwargs.setdefault("maxdImpact", flags.InDet.Tracking.ActivePass.maxPrimaryImpact)
    kwargs.setdefault("maxZ", flags.InDet.Tracking.ActivePass.maxZImpact)
    kwargs.setdefault("minZ", -flags.InDet.Tracking.ActivePass.maxZImpact)
    kwargs.setdefault("radMax", flags.InDet.Tracking.ActivePass.radMax)
    kwargs.setdefault("RapidityCut",  flags.InDet.Tracking.ActivePass.maxEta)

    kwargs.setdefault("usePixel", flags.InDet.Tracking.ActivePass.usePixel
                      and flags.InDet.Tracking.ActivePass.extension != "R3LargeD0")
    kwargs.setdefault("SpacePointsPixelName", 'PixelSpacePoints')
    kwargs.setdefault("useSCT", flags.InDet.Tracking.ActivePass.useSCT
                      and flags.InDet.Tracking.ActivePass.useSCTSeeding)
    kwargs.setdefault("SpacePointsSCTName", 'SCT_SpacePoints')
    kwargs.setdefault("useOverlapSpCollection", flags.InDet.Tracking.ActivePass.useSCT \
                      and flags.InDet.Tracking.ActivePass.useSCTSeeding)
    kwargs.setdefault("SpacePointsOverlapName", 'OverlapSpacePoints')

    if flags.InDet.Tracking.ActivePass.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", 'InDetPRDtoTrackMap'+ flags.InDet.Tracking.ActivePass.extension)

    if not flags.Reco.EnableHI \
       and (flags.InDet.Tracking.ActivePass.extension=="" \
            or flags.InDet.Tracking.ActivePass.extension=="Forward" \
            or flags.InDet.Tracking.ActivePass.extension=="BLS"):
        kwargs.setdefault("maxdImpactPPS", flags.InDet.Tracking.ActivePass.maxdImpactPPSSeeds)
        kwargs.setdefault("maxdImpactSSS", flags.InDet.Tracking.ActivePass.maxdImpactSSSSeeds)
        kwargs.setdefault("maxSeedsForSpacePointStrips", flags.InDet.Tracking.ActivePass.maxSeedsPerSP_Strips)
        kwargs.setdefault("maxSeedsForSpacePointPixels", flags.InDet.Tracking.ActivePass.maxSeedsPerSP_Pixels)
        kwargs.setdefault("alwaysKeepConfirmedStripSeeds", flags.InDet.Tracking.ActivePass.keepAllConfirmedStripSeeds)
        kwargs.setdefault("alwaysKeepConfirmedPixelSeeds", flags.InDet.Tracking.ActivePass.keepAllConfirmedPixelSeeds)
        kwargs.setdefault("mindRadius", 10)
        kwargs.setdefault("maxSizeSP", 200)
        kwargs.setdefault("dImpactCutSlopeUnconfirmedSSS", 1.25)
        kwargs.setdefault("dImpactCutSlopeUnconfirmedPPP", 2.0)

    if flags.Reco.EnableHI:
        kwargs.setdefault("maxdImpactPPS", flags.InDet.Tracking.ActivePass.maxdImpactPPSSeeds)
        kwargs.setdefault("maxdImpactSSS", flags.InDet.Tracking.ActivePass.maxdImpactSSSSeeds)

    if flags.Beam.Type is not BeamType.Cosmics:
        kwargs.setdefault("maxRadius1", 0.75*flags.InDet.Tracking.ActivePass.radMax)
        kwargs.setdefault("maxRadius2", flags.InDet.Tracking.ActivePass.radMax)
        kwargs.setdefault("maxRadius3", flags.InDet.Tracking.ActivePass.radMax)

    if flags.InDet.Tracking.ActivePass.isLowPt:
        kwargs.setdefault("pTmax", flags.InDet.Tracking.ActivePass.maxPT)
        kwargs.setdefault("mindRadius", 4.0)

    if flags.InDet.Tracking.ActivePass.extension == "R3LargeD0":
        kwargs.setdefault("optimisePhiBinning", False)
        kwargs.setdefault("etaMax", flags.InDet.Tracking.ActivePass.maxEta)
        kwargs.setdefault("maxSeedsForSpacePointStrips", flags.InDet.Tracking.ActivePass.maxSeedsPerSP_Strips)
        kwargs.setdefault("alwaysKeepConfirmedStripSeeds", flags.InDet.Tracking.ActivePass.keepAllConfirmedStripSeeds)
        kwargs.setdefault("maxdRadius", 150)
        kwargs.setdefault("seedScoreBonusConfirmationSeed", -2000)
    elif flags.InDet.Tracking.ActivePass.extension == "Forward":
        kwargs.setdefault("checkEta", True)
        kwargs.setdefault("etaMin", flags.InDet.Tracking.ActivePass.minEta)
        kwargs.setdefault("etaMax", flags.InDet.Tracking.ActivePass.maxEta)

    InDetSiSpacePointsSeedMaker = SiSpacePointsSeedMaker (name = name+flags.InDet.Tracking.ActivePass.extension, **kwargs)
    acc.setPrivateTools(InDetSiSpacePointsSeedMaker)
    return acc


def ITkSiSpacePointsSeedMakerCfg(flags, name="ITkSpSeedsMaker", **kwargs) :
    acc = ComponentAccumulator()

    kwargs.setdefault("pTmin", flags.ITk.Tracking.ActivePass.minPTSeed)
    kwargs.setdefault("maxdImpact", flags.ITk.Tracking.ActivePass.maxPrimaryImpactSeed)
    kwargs.setdefault("maxZ", flags.ITk.Tracking.ActivePass.maxZImpactSeed)
    kwargs.setdefault("minZ", -flags.ITk.Tracking.ActivePass.maxZImpactSeed)
    kwargs.setdefault("radMax", flags.ITk.Tracking.ActivePass.radMax)
    kwargs.setdefault("etaMax", flags.ITk.Tracking.ActivePass.maxEta )

    kwargs.setdefault("usePixel", flags.ITk.Tracking.ActivePass.useITkPixel
                      and flags.ITk.Tracking.ActivePass.useITkPixelSeeding)
    kwargs.setdefault("SpacePointsPixelName", 'ITkPixelSpacePoints')
    kwargs.setdefault("useStrip", flags.ITk.Tracking.ActivePass.useITkStrip
                      and flags.ITk.Tracking.ActivePass.useITkStripSeeding)
    kwargs.setdefault("SpacePointsStripName", 'ITkStripSpacePoints')
    kwargs.setdefault("useOverlapSpCollection", flags.ITk.Tracking.ActivePass.useITkStrip
                      and flags.ITk.Tracking.ActivePass.useITkStripSeeding )
    kwargs.setdefault("SpacePointsOverlapName", 'ITkOverlapSpacePoints')

    if flags.ITk.Tracking.ActivePass.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", 'ITkPRDtoTrackMap'+ flags.ITk.Tracking.ActivePass.extension)

    if flags.Beam.Type is not BeamType.Cosmics:
        kwargs.setdefault("maxRadius1", 0.75*flags.ITk.Tracking.ActivePass.radMax)
        kwargs.setdefault("maxRadius2", flags.ITk.Tracking.ActivePass.radMax)
        kwargs.setdefault("maxRadius3", flags.ITk.Tracking.ActivePass.radMax)

    if flags.ITk.Tracking.doFastTracking :
        kwargs.setdefault("useFastTracking", True)
        kwargs.setdefault("maxSeedsForSpacePoint", 3)

    if flags.ITk.Tracking.ActivePass.extension == "LargeD0":
        kwargs.setdefault("maxSeedsForSpacePoint", 5)
        kwargs.setdefault("isLRT", True)
        kwargs.setdefault("maxZPPP", flags.ITk.Tracking.ActivePass.maxZSpacePointsPPPSeeds)
        kwargs.setdefault("maxZSSS", flags.ITk.Tracking.ActivePass.maxZSpacePointsSSSSeeds)

    if flags.ITk.Tracking.writeSeedValNtuple:
        kwargs.setdefault("WriteNtuple", True)
        HistService = CompFactory.THistSvc(Output = ["valNtuples DATAFILE='SeedMakerValidation.root' OPT='RECREATE'"])
        acc.addService(HistService)

    acc.setPrivateTools(CompFactory.ITk.SiSpacePointsSeedMaker(name = name+flags.ITk.Tracking.ActivePass.extension, **kwargs))
    return acc
