# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--geoTag", default="ATLAS-R3S-2021-03-02-00", help="Geometry tag to use", choices=["ATLAS-R3S-2021-03-02-00",
                                                                                                            "ATLAS-P2-RUN4-01-00-00"])
    parser.add_argument("--condTag", default="OFLCOND-MC23-SDR-RUN3-02", help="Conditions tag to use",
                                                                         choices= ["OFLCOND-MC23-SDR-RUN3-02"])
    parser.add_argument("--inputFile", "-i", default=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/EVGEN_ParticleGun_FourMuon_Pt10to500.root"], 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--geoModelFile", default ="root://eoshome.cern.ch:1094//eos/user/c/cimuonsw/GeometryFiles/CompleteATLAS.db", help="GeoModel SqLite file containing the muon geometry.")
    parser.add_argument("--chambers", default=["all"], nargs="+", help="Chambers to check. If string is all, all chambers will be checked")
    parser.add_argument("--outRootFile", default="MdtGeoDump.root", help="Output ROOT file to dump the geomerty")
    parser.add_argument("--outTxtFile", default ="MdtGeoDump.txt", help="Output txt file to dump the geometry")
    parser.add_argument("--nEvents", help="Number of events to rum", type = int ,default = 1)
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

def setupHistSvcCfg(flags, out_file="MdtGeoDump.root"):
    result = ComponentAccumulator()
    if len(out_file) == 0: return result
    histSvc = CompFactory.THistSvc(Output=["GEOMODELTESTER DATAFILE='{out_file}', OPT='RECREATE'".format(out_file = out_file)])
    result.addService(histSvc, primary=True)
    return result


def GeoModelMdtTestCfg(flags, name = "GeoModelMdtTest", **kwargs):
    result = ComponentAccumulator()
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags)))
    the_alg = CompFactory.MuonGMR4.GeoModelMdtTest(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result

def setupGeoR4TestCfg(args):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads
    flags.Input.Files = args.inputFile 
    from os import path, system
    if args.geoModelFile.startswith("root://"):
        if not path.exists("MuonGeometryDB.db"):
            system("xrdcp {source} MuonGeometryDB.db".format(source = args.geoModelFile))
        args.geoModelFile = "MuonGeometryDB.db"
    
    flags.GeoModel.AtlasVersion = args.geoTag
    flags.IOVDb.GlobalTag = args.condTag
    flags.GeoModel.SQLiteDB = args.geoModelFile
    
    flags.Detector.GeometryBpipe = False
    ### Inner detector
    flags.Detector.GeometryBCM = False
    flags.Detector.GeometryPixel = False
    flags.Detector.GeometrySCT = False
    flags.Detector.GeometryTRT = False
    ### ITK
    flags.Detector.GeometryPLR = False
    flags.Detector.GeometryBCMPrime = False
    flags.Detector.GeometryITkPixel = False
    flags.Detector.GeometryITkStrip = False
    ### HGTD
    flags.Detector.GeometryHGTD = False
    ### Calorimeter
    flags.Detector.GeometryLAr = False
    flags.Detector.GeometryTile = False
    flags.Detector.GeometryMBTS = False
    flags.Detector.GeometryCalo = False
    ### Muon spectrometer
    flags.Detector.GeometryCSC = False
    flags.Detector.GeometrysTGC = False
    flags.Detector.GeometryMM = False
    flags.Detector.GeometryTGC = False
    flags.Detector.GeometryRPC = False
    flags.Detector.GeometryMDT = True

    flags.Muon.setupGeoModelXML = True

    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.EnableVerboseViews = True
    flags.Scheduler.AutoLoadUnmetDependencies = True

    flags.lock()
    flags.dump()


    cfg = setupServicesCfg(flags)
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    geoModelSvc = cfg.getPrimaryAndMerge(GeoModelCfg(flags))
    from MuonGeoModelR4.MuonGeoModelConfig import MuonDetectorToolCfg
    geoModelSvc.DetectorTools =[cfg.popToolsAndMerge(MuonDetectorToolCfg(flags))]    

    return flags, cfg

def executeTest(cfg, num_events = 1):
    DetDescCnvSvc = cfg.getService("DetDescrCnvSvc")
    DetDescCnvSvc.IdDictFromRDB = False
    DetDescCnvSvc.MuonIDFileName="IdDictParser/IdDictMuonSpectrometer_R.10.00.xml"
    DetDescCnvSvc.MuonIDFileName="IdDictParser/IdDictMuonSpectrometer_R.09.03.xml"
    
    cfg.printConfig(withDetails=True, summariseProps=True)
    if not cfg.run(num_events).isSuccess():
        import sys
        sys.exit("Execution failed")
if __name__=="__main__":
    args = SetupArgParser().parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    cfg.merge(GeoModelMdtTestCfg(flags))    
    #### 
    cfg.merge(setupHistSvcCfg(flags, out_file = args.outRootFile))
    cfg.merge(GeoModelMdtTestCfg(flags, DumpTxtFile = args.outTxtFile,
                                        TestStations = args.chambers if len([x for x in args.chambers if x =="all"]) ==0 else []))
    executeTest(cfg, num_events = args.nEvents)
    
   
  