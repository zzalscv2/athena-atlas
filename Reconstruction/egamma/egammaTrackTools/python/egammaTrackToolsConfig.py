# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = """Tool configuration to instantiate all
 egammaCaloTools with default configuration"""

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
from TrackToCalo.TrackToCaloConfig import EMParticleCaloExtensionToolCfg


def EMExtrapolationToolsCfg(flags, **kwargs):

    mlog = logging.getLogger('EMExtrapolationTools')
    mlog.debug('Start configuration')

    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        extrapAcc = AtlasExtrapolatorCfg(flags)
        kwargs["Extrapolator"] = acc.popToolsAndMerge(extrapAcc)

    if "CaloExtensionTool" not in kwargs:
        kwargs["CaloExtensionTool"] = acc.popToolsAndMerge(
            EMParticleCaloExtensionToolCfg(flags))

    kwargs["EnableTRT"] = flags.Detector.GeometryTRT

    emExtrapolationTools = CompFactory.EMExtrapolationTools(**kwargs)
    acc.setPrivateTools(emExtrapolationTools)
    return acc


def egammaTrkRefitterToolCfg(flags,
                             name='GSFRefitterTool',
                             **kwargs):
    acc = ComponentAccumulator()
    if "FitterTool" not in kwargs:
        if flags.Acts.useActsGsfInEgamma:
            from ActsConfig.ActsGaussianSumFitterConfig import ActsGaussianSumFitterCfg
            kwargs["FitterTool"] = acc.popToolsAndMerge(
                ActsGaussianSumFitterCfg(flags, name="ActsGSFTrackFitter"))
        else:
            from TrkConfig.TrkGaussianSumFilterConfig import GaussianSumFitterCfg
            kwargs["FitterTool"] = acc.popToolsAndMerge(
                GaussianSumFitterCfg(flags, name="GSFTrackFitter"))

    tool = CompFactory.egammaTrkRefitterTool(name, **kwargs)
    acc.setPrivateTools(tool)
    return acc


def CaloCluster_OnTrackBuilderCfg(flags,
                                  name='CaloCluster_OnTrackBuilder',
                                  **kwargs):
    acc = ComponentAccumulator()
    if "CaloSurfaceBuilder" not in kwargs:
        from CaloTrackingGeometry.CaloTrackingGeometryConfig import (
            CaloSurfaceBuilderMiddleCfg)
        kwargs["CaloSurfaceBuilder"] = acc.popToolsAndMerge(
            CaloSurfaceBuilderMiddleCfg(flags))
    tool = CompFactory.CaloCluster_OnTrackBuilder(name, **kwargs)
    acc.setPrivateTools(tool)
    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
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
    mlog.info("Configuring egammaTrkRefitterTool :")
    printProperties(mlog, cfg.popToolsAndMerge(
        egammaTrkRefitterToolCfg(ConfigFlags)),
        nestLevel=1,
        printDefaults=True)
    mlog.info("Configuring CaloCluster_OnTrackBuilder :")
    printProperties(mlog, cfg.popToolsAndMerge(
        CaloCluster_OnTrackBuilderCfg(ConfigFlags)),
        nestLevel=1,
        printDefaults=True)

    f = open("egtracktools.pkl", "wb")
    cfg.store(f)
    f.close()
