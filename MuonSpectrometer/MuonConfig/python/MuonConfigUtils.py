# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# This file is just for shared functions etc used by this package.
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SetupMuonStandaloneArguments():
    from argparse import ArgumentParser
    
    parser = ArgumentParser()
    parser.add_argument("-t", "--threads", dest="threads", type=int,
                        help="number of threads", default=1)
                        
    parser.add_argument("-o", "--output", dest="output", default='newESD.pool.root',
                        help="write ESD to FILE", metavar="FILE")
                        
    parser.add_argument("--run", help="Run directly from the python. If false, just stop once the pickle is written.",
                        action="store_true")
                        
    parser.add_argument("--forceclone", help="Override default cloneability of algorithms to force them to run in parallel",
                        action="store_true")
    parser.add_argument("-d","--debug", default=None, help="attach debugger (gdb) before run, <stage>: conf, init, exec, fini")
    parser.add_argument("--input", "-i", help="Input file to run the config", nargs="+",
                                        default= ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/ESD.16747874._000011_100events.pool.root'])
    args = parser.parse_args()
    
    return args
    
def SetupMuonStandaloneConfigFlags(args):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    # Keeping this commented out so we can easily switch to the default for testing against that.
    # from AthenaConfiguration.TestDefaults import defaultTestFiles
    # flags.Input.Files = defaultTestFiles.ESD
    flags.Input.Files = args.input
    
    flags.Concurrency.NumThreads=args.threads
    flags.Concurrency.NumConcurrentEvents=args.threads # Might change this later, but good enough for the moment.

    flags.Detector.GeometryMDT   = True 
    flags.Detector.GeometryTGC   = True
    flags.Detector.GeometryCSC   = True     
    flags.Detector.GeometryRPC   = True
    # TODO: disable these for now, to be determined if needed
    flags.Detector.GeometryCalo  = False
    flags.Detector.GeometryID    = False

    # FIXME This is temporary. I think it can be removed with some other refactoring
    flags.Muon.makePRDs          = False

    flags.Output.ESDFileName=args.output
    
    flags.Input.isMC = True
    flags.lock()
    flags.dump()
    return flags
    
def SetupMuonStandaloneCA(args,flags):
    # When running from a pickled file, athena inserts some services automatically. So only use this if running now.
    if args.run:
        from AthenaConfiguration.MainServicesConfig import MainServicesCfg
        cfg = MainServicesCfg(flags)
        msgService = cfg.getService('MessageSvc')
        msgService.Format = "S:%s E:%e % F%128W%S%7W%R%T  %0W%M"
    else:
        cfg=ComponentAccumulator()

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        cfg.merge(GEN_AOD2xAODCfg(flags))

    return cfg
    
def SetupMuonStandaloneOutput(cfg, flags, itemsToRecord):
    # Set up output
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

    cfg.merge( OutputStreamCfg( flags, 'ESD', ItemList=itemsToRecord) )
    outstream = cfg.getEventAlgo("OutputStreamESD")
    outstream.ForceRead = True

    # Fix for ATLASRECTS-5151
    Trk__EventCnvSuperTool = CompFactory.Trk.EventCnvSuperTool
    cnvTool = Trk__EventCnvSuperTool(name = 'EventCnvSuperTool')
    cnvTool.MuonCnvTool.FixTGCs = True 
    cfg.addPublicTool(cnvTool)
