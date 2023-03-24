# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("-t", "--threads", dest="threads", type=int, help="number of threads", default=1)

    parser.add_argument("-o", "--output", dest="output", default='newESD.pool.root', help="write ESD to FILE", metavar="FILE")

    parser.add_argument("--run",
                        help="Run directly from the python. If false, just stop once the pickle is written.",
                        action="store_true",
                        default=True)

    parser.add_argument("--forceclone",
                        help="Override default cloneability of algorithms to force them to run in parallel",
                        action="store_true")
    parser.add_argument("-d", "--debug", default=None, help="attach debugger (gdb) before run, <stage>: conf, init, exec, fini")
    parser.add_argument("--inputFile", "-i", default=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data17_13TeV.00330470.physics_Main.daq.RAW._lb0310._SFO-1._0001.data"], 
                        help="Input file to run on ", nargs="+")
    return parser
    
def setupServicesCfg(flags):
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    result = MainServicesCfg(flags)
    ### Setup the file reading
    from AthenaConfiguration.Enums import Format
    if flags.Input.Format is Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        result.merge(PoolReadCfg(flags))
       

    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    
    return result

def MdtCablingTestAlgCfg(flags, name = "MdtCablingTestAlg"):
    from AthenaConfiguration.ComponentFactory import CompFactory
    result = setupServicesCfg(flags)
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    from MuonConfig.MuonCondAlgConfig import MdtCondDbAlgCfg
    result.merge(MdtCondDbAlgCfg(flags))
    result.merge(MDTCablingConfigCfg(flags,
                                #MezzanineJSON="MezzMapping.json",
                                #CablingJSON="MdtCabling.json"
                                ))
    event_algo = CompFactory.MdtCablingTestAlg(name)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    args = SetupArgParser().parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.Input.RunNumber = 310000
    flags.lock()   
    
    cfg = MdtCablingTestAlgCfg(flags)
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   
    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


