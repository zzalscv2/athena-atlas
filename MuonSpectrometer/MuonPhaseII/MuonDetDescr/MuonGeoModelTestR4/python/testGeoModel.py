
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--inputFile", "-i", default=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/EVGEN_ParticleGun_FourMuon_Pt10to500.root"], 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--geoModelFile", default ="root://eoshome.cern.ch:1094//eos/user/c/cimuonsw/GeometryFiles/muonsOnlyR4WMDT.db", help="GeoModel SqLite file containing the muon geometry.")
    return parser

def setupServicesCfg(flags):
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    result = MainServicesCfg(flags)
    ### Setup the file reading
    from AthenaConfiguration.Enums import Format
    if flags.Input.Format is Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        result.merge(PoolReadCfg(flags))

    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    result.merge(MuonIdHelperSvcCfg(flags))
    return result


def GeoModelMdtTestCfg(flags, name = "GeoModelMdtTest", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonGMR4.GeoModelMdtTest(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result


if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    args = SetupArgParser().parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Input.Files = args.inputFile 
    from os import path, system
    if args.geoModelFile.startswith("root://"):
        if not path.exists("MuonGeometryDB.db"):
            system("xrdcp {source} MuonGeometryDB.db".format(source = args.geoModelFile))
        args.geoModelFile = "MuonGeometryDB.db"

    flags.GeoModel.SQLiteDB = args.geoModelFile
    #flags.GeoModel.AtlasVersion ="ATLAS-P2-RUN4-01-00-00"
    
    flags.Detector.GeometryCSC = False
    flags.Detector.GeometrysTGC = False
    flags.Detector.GeometryMM = False
    flags.Detector.GeometryTGC = False
    flags.Detector.GeometryRPC = False
   

    flags.lock()
    
    cfg = setupServicesCfg(flags)
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    geoModelSvc = cfg.getPrimaryAndMerge(GeoModelCfg(flags))
    from MuonGeoModelR4.MuonGeoModelConfig import MuonDetectorToolCfg
    #from AthenaCommon.Constants import VERBOSE
    geoModelSvc.DetectorTools =[cfg.popToolsAndMerge(MuonDetectorToolCfg(flags,#OutputLevel= VERBOSE
    ))]
    ####    
    DetDescCnvSvc = cfg.getService("DetDescrCnvSvc")
    DetDescCnvSvc.IdDictFromRDB = False
    DetDescCnvSvc.MuonIDFileName="IdDictParser/IdDictMuonSpectrometer_R.10.00.xml"
    DetDescCnvSvc.MuonIDFileName="IdDictParser/IdDictMuonSpectrometer_R.09.03.xml"
    
    cfg.merge(GeoModelMdtTestCfg(flags))
    
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   
    if not cfg.run(1).isSuccess():
        import sys
        sys.exit("Execution failed")
  