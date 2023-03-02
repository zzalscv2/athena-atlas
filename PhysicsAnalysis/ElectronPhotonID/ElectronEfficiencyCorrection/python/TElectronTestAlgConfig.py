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
    from argparse import ArgumentParser
    parser = ArgumentParser("TElectronTestAlg")
    parser.add_argument("-n", "--maxEvents",
                        default=10,
                        type=int,
                        help="The number of events to run. -1 (all).")
    parser.add_argument("-f", "--inputFile",
                        default=None,
                        type=str,
                        help="The input (D)AOD file")
    args = parser.parse_args()
    print(args)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = [args.inputFile]
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
    statusCode = cfg.run(args.maxEvents)

    assert statusCode is not None, "Issue while running"
    sys.exit(not statusCode.isSuccess())
