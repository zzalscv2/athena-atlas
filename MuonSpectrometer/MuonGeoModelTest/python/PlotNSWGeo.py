# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
    
def setupServicesCfg(flags):
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    result = MainServicesCfg(flags)
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    
    return result

def NSWGeoPlottingAlgCfg(flags, name = "NSWGeoPlottingAlg"):
    from AthenaConfiguration.ComponentFactory import CompFactory
    result = setupServicesCfg(flags)   
    event_algo = CompFactory.NSWGeoPlottingAlg(name)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    parser = SetupArgParser()
    parser.set_defaults(inputFile=["../group.det-muon.DiMuonGenerator_EtaGtr_1p2_Pt10to100.HITS.Run3_2NSW_100822_EXT0/group.det-muon.30014436.EXT0._000047.HITS.pool.root"])
    args = parser.parse_args()

    ConfigFlags.Concurrency.NumThreads = args.threads
    ConfigFlags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    ConfigFlags.Output.ESDFileName = args.output
    ConfigFlags.Input.Files = args.inputFile
    ConfigFlags.Muon.applyMMPassivation = True
    ConfigFlags.lock()

   
    
    cfg = NSWGeoPlottingAlgCfg(ConfigFlags)
    msgService = cfg.getService('MessageSvc')
    msgService.Format = "S:%s E:%e % F%128W%S%7W%R%T  %0W%M"

    cfg.printConfig(withDetails=True, summariseProps=True)

    ConfigFlags.dump()

    f = open("MdtCablingTester.pkl", "wb")
    cfg.store(f)
    f.close()

    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")




