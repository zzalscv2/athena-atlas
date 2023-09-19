# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

__doc__ = """
Tool configuration to instantiate
all egammaCaloTools with default configuration.
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def CaloFillRectangularClusterCfg(flags, **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("eta_size", 5)
    kwargs.setdefault("phi_size", 7)
    kwargs.setdefault("cells_name", flags.Egamma.Keys.Input.CaloCells)

    result.setPrivateTools(CompFactory.CaloFillRectangularCluster(**kwargs))
    return result


def egammaCheckEnergyDepositToolcfg(
    flags, name="egammaCheckEnergyDepositTool", **kwargs
):
    result = ComponentAccumulator()
    kwargs.setdefault("ThrE2min", 50 if not flags.Egamma.doLowMu else 20)
    kwargs.setdefault("ThrF0max", 0.9)
    kwargs.setdefault("ThrF1max", 0.9 if not flags.Egamma.doLowMu else 0.95)
    kwargs.setdefault("ThrF2max", 0.999)
    kwargs.setdefault("ThrF3max", 0.8)
    result.setPrivateTools(CompFactory.egammaCheckEnergyDepositTool(name, **kwargs))
    return result


def egammaCaloClusterSelectorGSFCfg(flags, name="caloClusterGSFSelector", **kwargs):
    result = ComponentAccumulator()

    if "egammaCheckEnergyDepositTool" not in kwargs:
        kwargs["egammaCheckEnergyDepositTool"] = result.popToolsAndMerge(
            egammaCheckEnergyDepositToolcfg(flags)
        )

    kwargs.setdefault("EMEtCut", 2250.0 if not flags.Egamma.doLowMu else 400.0)
    kwargs.setdefault("EMEtSplittingFraction", 0.7)
    kwargs.setdefault("EMFCut", 0.5)
    kwargs.setdefault("CellContainerName", flags.Egamma.Keys.Input.CaloCells)
    result.setPrivateTools(CompFactory.egammaCaloClusterSelector(name, **kwargs))
    return result


def egammaCaloClusterSelectorCfg(flags, name="caloClusterROISelector", **kwargs):
    result = ComponentAccumulator()

    if "egammaCheckEnergyDepositTool" not in kwargs:
        kwargs["egammaCheckEnergyDepositTool"] = result.popToolsAndMerge(
            egammaCheckEnergyDepositToolcfg(flags)
        )
    kwargs.setdefault("EMEtCut", 2250.0 if not flags.Egamma.doLowMu else 400.0)
    kwargs.setdefault("EMEtSplittingFraction", 0.7)
    kwargs.setdefault("EMFCut", 0.7)
    kwargs.setdefault("RetaCut", 0.65 if not flags.Egamma.doLowMu else 0.0)
    kwargs.setdefault("HadLeakCut", 0.15)
    result.setPrivateTools(CompFactory.egammaCaloClusterSelector(name, **kwargs))
    return result


def egammaHadCaloClusterSelectorCfg(
    ConfigFlags, name="caloClusterHadROISelector", **kwargs
):
    result = ComponentAccumulator()
    kwargs.setdefault("egammaCheckEnergyDepositTool", "")
    kwargs.setdefault("ClusterEtCut", 150e3)
    result.setPrivateTools(CompFactory.egammaCaloClusterSelector(name, **kwargs))
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import logging

    ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
    ConfigFlags.fillFromArgs()
    ConfigFlags.lock()
    ConfigFlags.dump()

    cfg = ComponentAccumulator()
    mlog = logging.getLogger("egammaCaloToolsConfigTest")
    mlog.info("Configuring egammaCaloClusterSelector : ")
    printProperties(
        mlog,
        cfg.popToolsAndMerge(egammaCaloClusterSelectorCfg(ConfigFlags)),
        nestLevel=1,
        printDefaults=True,
    )
    mlog.info("Configuring egammaCaloClusterSelectorGSF :")
    printProperties(
        mlog,
        cfg.popToolsAndMerge(egammaCaloClusterSelectorGSFCfg(ConfigFlags)),
        nestLevel=1,
        printDefaults=True,
    )
    mlog.info("Configuring egammaHadCaloClusterSelector :")
    printProperties(
        mlog,
        cfg.popToolsAndMerge(egammaHadCaloClusterSelectorCfg(ConfigFlags)),
        nestLevel=1,
        printDefaults=True,
    )
    f = open("egtracktools.pkl", "wb")
    cfg.store(f)
    f.close()
