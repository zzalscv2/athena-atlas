# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

__doc__ = """Tool configuration to instantiate all
 egammaCaloTools with default configuration"""

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TrkConfig.AtlasExtrapolatorConfig import (
    AtlasExtrapolatorCfg, egammaCaloExtrapolatorCfg)
from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg


def egCaloDepthCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("name", "egCaloDepthToolmiddle")
    kwargs.setdefault("DepthChoice", "middle")
    acc.setPrivateTools(
        CompFactory.CaloDepthTool(**kwargs))
    return acc


def egCaloSurfaceBuilderCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault(
        "CaloDepthTool",
        acc.popToolsAndMerge(egCaloDepthCfg(flags)))

    acc.setPrivateTools(
        CompFactory.CaloSurfaceBuilder(**kwargs)
    )
    return acc


def EMLastCaloExtensionToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("name", "EMLastCaloExtensionTool")
    kwargs.setdefault("ParticleType", "electron")

    if "CaloSurfaceBuilder" not in kwargs:
        kwargs["CaloSurfaceBuilder"] = acc.popToolsAndMerge(
            egCaloSurfaceBuilderCfg(flags))

    if "Extrapolator" not in kwargs:
        extrapAcc = egammaCaloExtrapolatorCfg(flags)
        kwargs["Extrapolator"] = acc.popToolsAndMerge(extrapAcc)
    return ParticleCaloExtensionToolCfg(flags, **kwargs)


def EMParticleCaloExtensionToolCfg(flags, **kwargs):
    kwargs.setdefault("name", "EMParticleCaloExtensionTool")
    kwargs.setdefault("StartFromPerigee", True)
    return EMLastCaloExtensionToolCfg(flags, **kwargs)


def EMExtrapolationToolsCfg(flags, **kwargs):

    mlog = logging.getLogger('EMExtrapolationTools')
    mlog.debug('Start configuration')

    acc = ComponentAccumulator()

    # in the std config, it is egammaExtrapolator
    # (called AtlasExtrapolator now). Should this be egammaCaloExtrapolator ?
    if "Extrapolator" not in kwargs:
        extrapAcc = AtlasExtrapolatorCfg(flags)
        kwargs["Extrapolator"] = acc.popToolsAndMerge(extrapAcc)

    if "PerigeeCaloExtensionTool" not in kwargs:
        kwargs["PerigeeCaloExtensionTool"] = acc.popToolsAndMerge(
            EMParticleCaloExtensionToolCfg(flags))

    if "LastCaloExtensionTool" not in kwargs:
        kwargs["LastCaloExtensionTool"] = acc.popToolsAndMerge(
            EMLastCaloExtensionToolCfg(flags))

    emExtrapolationTools = CompFactory.EMExtrapolationTools(**kwargs)
    acc.setPrivateTools(emExtrapolationTools)
    return acc


def EMExtrapolationToolsCacheCfg(flags, **kwargs):
    kwargs.setdefault("name", "EMExtrapolationToolsCache")
    kwargs.setdefault("PerigeeCache", "GSFPerigeeCaloExtension")
    kwargs.setdefault("LastCache", "GSFLastCaloExtension")
    kwargs.setdefault("useCaching", True)
    kwargs.setdefault("useLastCaching", True)
    return EMExtrapolationToolsCfg(flags, **kwargs)


def EMExtrapolationToolsCommonCacheCfg(flags, **kwargs):
    kwargs.setdefault("name", "EMExtrapolationToolsCommonCache")
    kwargs.setdefault("LastCache", "ParticleCaloExtension")
    kwargs.setdefault("useCaching", False)
    kwargs.setdefault("useLastCaching", True)
    return EMExtrapolationToolsCfg(flags, **kwargs)


def EMExtrapolationToolsLRTCacheCfg(flags, **kwargs):
    kwargs.setdefault("name", "EMExtrapolationToolsLRTCache")
    kwargs.setdefault("PerigeeCache", "LRTGSFPerigeeCaloExtension")
    kwargs.setdefault("LastCache", "LRTGSFLastCaloExtension")
    return EMExtrapolationToolsCacheCfg(flags, **kwargs)


def EMExtrapolationToolsLRTCommonCacheCfg(flags, **kwargs):
    kwargs.setdefault("name", "EMExtrapolationToolsLRTCommonCache")
    kwargs.setdefault("LastCache", "ParticleCaloExtension_LRT")
    kwargs.setdefault("useCaching", False)
    kwargs.setdefault("useLastCaching", True)
    return EMExtrapolationToolsCfg(flags, **kwargs)


def egammaTrkRefitterToolCfg(flags,
                             name='GSFRefitterTool',
                             **kwargs):
    acc = ComponentAccumulator()
    if "FitterTool" not in kwargs:
        from egammaTrackTools.GSFTrackFitterConfig import EMGSFTrackFitterCfg
        kwargs["FitterTool"] = acc.popToolsAndMerge(
            EMGSFTrackFitterCfg(flags, name="GSFTrackFitter"), **kwargs)
    kwargs.setdefault("useBeamSpot", False)
    kwargs.setdefault("ReintegrateOutliers", True)
    if "Extrapolator" not in kwargs:
        kwargs["Extrapolator"] = acc.getPrimaryAndMerge(
            AtlasExtrapolatorCfg(flags))
    tool = CompFactory.egammaTrkRefitterTool(name, **kwargs)
    acc.setPrivateTools(tool)
    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaCommon.Configurable import Configurable
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    Configurable.configurableRun3Behavior = True

    ConfigFlags.Input.Files = defaultTestFiles.RDO
    ConfigFlags.fillFromArgs()
    ConfigFlags.lock()
    ConfigFlags.dump()

    cfg = ComponentAccumulator()
    mlog = logging.getLogger("egammaTrackToolsConfigTest")
    mlog.info("Configuring EMExtrapolationTools : ")
    printProperties(mlog, cfg.popToolsAndMerge(
        EMExtrapolationToolsCfg(ConfigFlags)),
        nestLevel=1,
        printDefaults=True)
    mlog.info("Configuring EMExtrapolationTools with cache : ")
    printProperties(mlog, cfg.popToolsAndMerge(
        EMExtrapolationToolsCacheCfg(ConfigFlags)),
        nestLevel=1,
        printDefaults=True)
    mlog.info("Configuring egammaTrkRefitterToolCfg :")
    printProperties(mlog, cfg.popToolsAndMerge(
        egammaTrkRefitterToolCfg(ConfigFlags)),
        nestLevel=1,
        printDefaults=True)

    f = open("egtracktools.pkl", "wb")
    cfg.store(f)
    f.close()
