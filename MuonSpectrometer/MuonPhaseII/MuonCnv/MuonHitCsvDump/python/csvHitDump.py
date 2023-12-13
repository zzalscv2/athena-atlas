# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(inputFile = ["myMuonSimTestStream.pool.root"])
    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    
    from xAODMuonSimHitCnv.MuonSimHitCnvCfg import xAODSimHitToMdtMeasCnvAlgCfg    
    cfg.merge(xAODSimHitToMdtMeasCnvAlgCfg(flags))
    
    from MuonHitCsvDump.MuonHitCsvDumpConfig import CsvMuonSimHitDumpCfg
    from MuonHitCsvDump.MuonHitCsvDumpConfig import CsvMdtDriftCircleDumpCfg
    cfg.merge(CsvMuonSimHitDumpCfg(flags,
                                   MuonSimHitKey ="xMdtSimHits")) 
    cfg.merge(CsvMdtDriftCircleDumpCfg(flags))
                                       
    executeTest(cfg, num_events = args.nEvents)


    
