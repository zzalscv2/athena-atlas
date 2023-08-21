# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = "Instantiate egammaSelectedTrackCopy with default configuration"

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def egammaSelectedTrackCopyCfg(flags, name="egammaSelectedTrackCopy", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("doFwdTracks", flags.Detector.GeometryITk)

    if "egammaCaloClusterSelector" not in kwargs:
        from egammaCaloTools.egammaCaloToolsConfig import (
            egammaCaloClusterSelectorGSFCfg,
        )

        kwargs["egammaCaloClusterSelector"] = acc.popToolsAndMerge(
            egammaCaloClusterSelectorGSFCfg(flags)
        )

    if "ExtrapolationTool" not in kwargs:
        from egammaTrackTools.egammaTrackToolsConfig import (
            EMExtrapolationToolsCfg,
        )

        extraptool = EMExtrapolationToolsCfg(flags, name="EMExtrapolationTools")
        kwargs["ExtrapolationTool"] = acc.popToolsAndMerge(extraptool)

    kwargs.setdefault("ClusterContainerName", flags.Egamma.Keys.Internal.EgammaTopoClusters)
    kwargs.setdefault("FwdClusterContainerName", flags.Egamma.Keys.Internal.ForwardTopoClusters)
    kwargs.setdefault("TrackParticleContainerName", flags.Egamma.Keys.Input.TrackParticles)
    kwargs.setdefault("OutputTrkPartContainerName", flags.Egamma.Keys.Output.TrkPartContainerName)

    # P->T conversion extra dependencies
    if flags.Detector.GeometryITk:
        kwargs.setdefault(
            "ExtraInputs",
            [
                (
                    "InDetDD::SiDetectorElementCollection",
                    "ConditionStore+ITkPixelDetectorElementCollection",
                ),
                (
                    "InDetDD::SiDetectorElementCollection",
                    "ConditionStore+ITkStripDetectorElementCollection",
                ),
            ],
        )
    else:
        kwargs.setdefault(
            "ExtraInputs",
            [
                (
                    "InDetDD::SiDetectorElementCollection",
                    "ConditionStore+PixelDetectorElementCollection",
                ),
                (
                    "InDetDD::SiDetectorElementCollection",
                    "ConditionStore+SCT_DetectorElementCollection",
                ),
            ],
        )

    egseltrkcpAlg = CompFactory.egammaSelectedTrackCopy(name, **kwargs)

    acc.addEventAlgo(egseltrkcpAlg)
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(egammaSelectedTrackCopyCfg(flags))
    mlog = logging.getLogger("egammaSelectedTrackCopyConfigTest")
    mlog.info("Configuring  egammaSelectedTrackCopy: ")
    printProperties(
        mlog,
        acc.getEventAlgo("egammaSelectedTrackCopy"),
        nestLevel=1,
        printDefaults=True,
    )
    with open("egammaselectedtrackCopy.pkl", "wb") as f:
        acc.store(f)
