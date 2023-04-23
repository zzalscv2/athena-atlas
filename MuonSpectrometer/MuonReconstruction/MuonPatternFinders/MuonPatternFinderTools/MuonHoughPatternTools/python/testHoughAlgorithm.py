# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
         
def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("-o", "--output", dest="output", default='', help="Text file containing each cabling channel", metavar="FILE")
    parser.add_argument("--inputFile", "-i", default=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/WorkflowReferences/master/q443/v9/myESD.pool.root"], 
                        help="Input file to run on ", nargs="+")
    return parser
    

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    args = SetupArgParser().parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile    
    flags.lock()
    
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    cfg = setupServicesCfg(flags)
    from MuonConfig.MuonSegmentFindingConfig import MuonLayerHoughAlgCfg, MuonLayerHoughToolCfg
    cfg.merge(MuonLayerHoughAlgCfg(flags,
                                    MuonLayerScanTool=cfg.popToolsAndMerge(
                                                    MuonLayerHoughToolCfg(flags,
                                                                          DebugHough=False))))
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   
    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


