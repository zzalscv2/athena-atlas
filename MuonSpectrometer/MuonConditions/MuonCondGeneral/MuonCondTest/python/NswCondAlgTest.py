# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def NSWCondAlgTest(flags,alg_name="NSWCondTestAlg", **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonConfig.MuonCondAlgConfig import NswCalibDbAlgCfg
    result.merge(NswCalibDbAlgCfg(flags))
    the_alg = CompFactory.NswCondTestAlg(alg_name, **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result
    

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from .MdtCablingTester import SetupArgParser, setupServicesCfg
    
    parser = SetupArgParser()
    parser.add_argument("--LogName", default="LogFile", 
                        help="If the test is run multiple times to ensure reproducibility, then the dump of the test can be resteered")
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/UnitTestInput/Run3MC.ESD.pool.root"])
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.lock()
   
    cfg = setupServicesCfg(flags)
    msgService = cfg.getService('MessageSvc')
    msgService.Format = "S:%s E:%e % F%128W%S%7W%R%T  %0W%M"

    cfg.merge(NSWCondAlgTest(flags, LogName = args.LogName))
    cfg.printConfig(withDetails=True, summariseProps=True)

    flags.dump()

    with open("NSWCondAlgTest.pkl", "wb") as f:
        cfg.store(f)
        f.close()

    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")

