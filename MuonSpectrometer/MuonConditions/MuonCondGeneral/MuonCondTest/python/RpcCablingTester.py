# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def RpcCablingTestAlgCfg(flags, name = "RpcCablingTestAlg"):
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    result = setupServicesCfg(flags)
    from MuonConfig.MuonCablingConfig import NRPCCablingConfigCfg
    from AthenaCommon.Constants import DEBUG
    result.merge(NRPCCablingConfigCfg(flags, JSONFile = "CablingFile.json", OutputLevel = DEBUG ))
    event_algo = CompFactory.RpcCablingTestAlg(name, OutputLevel = DEBUG)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    parser = SetupArgParser()
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/WorkflowReferences/22.0/q445/v20/myESD.pool.root"])
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Muon.enableNRPC = True
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.lock()   
    
    cfg = RpcCablingTestAlgCfg(flags)  
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   
    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


