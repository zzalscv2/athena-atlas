# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 
def MdtCablMezzAlgCfg(flags, name = "MdtCablMezzAlg"):
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    result = setupServicesCfg(flags)
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    result.merge(MDTCablingConfigCfg(flags))
    event_algo = CompFactory.MdtCablMezzAlg(name)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTester.MdtCablingTester import SetupArgParser
    args = SetupArgParser().parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.lock()   
    
    cfg = MdtCablMezzAlgCfg(flags)
    cfg.printConfig(withDetails=True, summariseProps=True)

    flags.dump()

    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


