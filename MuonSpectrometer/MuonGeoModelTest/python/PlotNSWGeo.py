# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator   

def NSWGeoPlottingAlgCfg(flags, name = "NSWGeoPlottingAlg", **kwargs):
    result = ComponentAccumulator()
    event_algo = CompFactory.NSWGeoPlottingAlg(name, **kwargs)
    result.addEventAlgo(event_algo, primary = True)
    return result

def sTgcPadPlottingAlg(flags, name = "sTgcPadPlottingAlg", **kwargs):
    result = ComponentAccumulator()
    event_algo = CompFactory.sTgcPadPlottingAlg(name, **kwargs)
    result.addEventAlgo(event_algo, primary = True)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    parser = SetupArgParser()
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/EVGEN_ParticleGun_FourMuon_Pt10to500.root"])
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.Muon.applyMMPassivation = True
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.IOVDb.GlobalTag = "OFLCOND-MC23-SDR-RUN3-02"
    flags.lock()
    #### 
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    cfg = setupServicesCfg(flags)
    
    #cfg.merge(NSWGeoPlottingAlgCfg(flags))
    cfg.merge(sTgcPadPlottingAlg(flags))

    msgService = cfg.getService('MessageSvc')
    msgService.Format = "S:%s E:%e % F%128W%S%7W%R%T  %0W%M"

    cfg.printConfig(withDetails=True, summariseProps=True)

    flags.dump()
    
    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")
