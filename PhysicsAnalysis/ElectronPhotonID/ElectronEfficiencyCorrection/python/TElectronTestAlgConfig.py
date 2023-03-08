# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Simple script to run the TElectronTestAlg with CA


import sys
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TElectronTestAlgCfg():

    acc = ComponentAccumulator()
    TElectronTestAlg = CompFactory.getComp("CP::TElectronTestAlg")
    testAlg = TElectronTestAlg(name="CP_TElectronTestAlg",
                               IdKey="Medium")
    acc.addEventAlgo(testAlg)
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.fillFromArgs()
    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
    MessageSvc = CompFactory.MessageSvc
    cfg.addService(MessageSvc(infoLimit=0))
    cfg.merge(TElectronTestAlgCfg())

    print("Start running...")
    statusCode = None
    statusCode = cfg.run()

    assert statusCode is not None, "Issue while running"
    sys.exit(not statusCode.isSuccess())
