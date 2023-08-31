# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = "Configure GSF track fitter building"

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def GSFExtrapolatorToolCfg(flags, **kwargs):
    '''dedicated Multi Component Extrapolator
    '''

    acc = ComponentAccumulator()

    if "Propagators" not in kwargs:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import (
            GSFRungeKuttaPropagatorCfg)
        gsfTrkPropagator = acc.popToolsAndMerge(
            GSFRungeKuttaPropagatorCfg(flags))
        kwargs["Propagator"] = gsfTrkPropagator

    if "Navigator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorToolsConfig import AtlasNavigatorCfg
        egTrkNavigator = AtlasNavigatorCfg(flags, name="egTrkNavigator")
        kwargs["Navigator"] = acc.popToolsAndMerge(egTrkNavigator)

    if "ElectronMaterialConvolution" not in kwargs:
        GsfMaterialUpdator = CompFactory.Trk.ElectronMaterialMixtureConvolution(
            name="ElectronMaterialUpdator",
            MaximumNumberOfComponents=12
        )
        kwargs["GsfMaterialConvolution"] = GsfMaterialUpdator

    acc.setPrivateTools(CompFactory.Trk.GsfExtrapolator(**kwargs))
    return acc


# default for refits with electron hypothesis
# for example see
# https://cds.cern.ch/record/1449796/files/ATLAS-CONF-2012-047.pdf
def GaussianSumFitterCfg(flags, name="GaussianSumFitter", **kwargs):
    ''' GSF Track Fitter Config
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault("RefitOnMeasurementBase", True)

    # Note that the ROT tool for calibration
    # has an effect if we fit on PrepRawData.
    # i.e not refitting Measurements
    RefitPRD = not kwargs["RefitOnMeasurementBase"]
    if RefitPRD and "ToolForROTCreation" not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import InDetRotCreatorCfg
        kwargs["ToolForROTCreation"] = acc.popToolsAndMerge(
            InDetRotCreatorCfg(flags)
        )

    if "ToolForExtrapolation" not in kwargs:
        gsfextrap = GSFExtrapolatorToolCfg(flags)
        kwargs["ToolForExtrapolation"] = acc.popToolsAndMerge(gsfextrap)

    kwargs.setdefault("ReintegrateOutliers", True)

    acc.setPrivateTools(CompFactory.Trk.GaussianSumFitter(**kwargs))

    return acc

def ITkGaussianSumFitterCfg(flags, name="ITkGaussianSumFitter", **kwargs):
    acc = ComponentAccumulator()

    if "ToolForROTCreation" not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkRotCreatorCfg
        ITkRotCreator = acc.popToolsAndMerge(ITkRotCreatorCfg(flags))
        kwargs.setdefault("ToolForROTCreation", ITkRotCreator)

    kwargs.setdefault("RefitOnMeasurementBase", True)

    GaussianSumFitter = acc.popToolsAndMerge(
            GaussianSumFitterCfg(flags, name=name, **kwargs)
    )

    acc.setPrivateTools(GaussianSumFitter)
    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.fillFromArgs()
    flags.lock()
    flags.dump()

    cfg = ComponentAccumulator()
    mlog = logging.getLogger("GSFTrackFitterConfigTest")
    mlog.info("Configuring GSFExtrapolatorTool :")
    printProperties(
        mlog,
        cfg.popToolsAndMerge(GSFExtrapolatorToolCfg(flags)),
        nestLevel=1,
        printDefaults=True,
    )
    mlog.info("Configuring EMGSFTrackFitter :")
    printProperties(
        mlog,
        cfg.popToolsAndMerge(GaussianSumFitterCfg(flags)),
        nestLevel=1,
        printDefaults=True,
    )

    f = open("gsftrackfitter.pkl", "wb")
    cfg.store(f)
    f.close()
