# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = "Instantiate egammaTopoClusterCopier with default configuration"

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def egammaTopoClusterCopierCfg(flags, name='', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault(
        "InputTopoCollection",
        flags.Egamma.Keys.Input.TopoClusters)

    egtopocluster = flags.Egamma.Keys.Internal.EgammaTopoClusters
    kwargs.setdefault(
        "OutputTopoCollection",
        egtopocluster)
    kwargs.setdefault(
        "OutputTopoCollectionShallow",
        "tmp_"+egtopocluster)

    kwargs.setdefault(
        "ECut",
        700 if not flags.Egamma.doLowMu else 300)
    if flags.Detector.GeometryITk: 
        kwargs.setdefault(
            "EtaCut",
            4.0)

    if name=='':
        name = kwargs["OutputTopoCollection"]+'Copier'

    egcopierAlg = CompFactory.egammaTopoClusterCopier(name, **kwargs)

    acc.addEventAlgo(egcopierAlg)

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()
    acc = MainServicesCfg(flags)
    mlog = logging.getLogger("egammaTopoClusterCopierConfigTest")
    mlog.info("Configuring  egammaTopoClusterCopier: ")
    acc.merge(egammaTopoClusterCopierCfg(flags))
    printProperties(mlog,
                    acc.getEventAlgo("egammaTopoClustersCopier"),
                    nestLevel=1,
                    printDefaults=True)
    with open("egammatopoclustercopier.pkl", "wb") as f:
        acc.store(f)
