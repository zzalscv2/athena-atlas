
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--inputFile", "-i", default=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/EVGEN_ParticleGun_FourMuon_Pt10to500.root"], 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--geoTag", default="ATLAS-R3S-2021-03-02-00", help="Geometry tag to use", choices=["ATLAS-R2-2016-01-02-01",
                                                                                     "ATLAS-R3S-2021-03-02-00"])
    parser.add_argument("--condTag", default="OFLCOND-MC23-SDR-RUN3-02", help="Conditions tag to use",
                                                                         choices=["OFLCOND-MC16-SDR-RUN2-11",
                                                                                  "OFLCOND-MC23-SDR-RUN3-02"])
    return parser

def setupServicesCfg(flags):
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    result = MainServicesCfg(flags)
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    return result

def GeoModelMdtTestCfg(flags, name = "GeoModelMdtTest", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonGM.GeoModelMdtTest(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result


if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    args = SetupArgParser().parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Input.Files = args.inputFile 
    flags.GeoModel.AtlasVersion = args.geoTag
    flags.IOVDb.GlobalTag = args.condTag
    flags.lock()
    
    cfg = setupServicesCfg(flags)
    cfg.merge(GeoModelMdtTestCfg(flags))
    
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   
    if not cfg.run(1).isSuccess():
        import sys
        sys.exit("Execution failed")
  