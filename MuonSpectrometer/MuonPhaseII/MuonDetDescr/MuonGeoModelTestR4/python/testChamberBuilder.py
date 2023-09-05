# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest, MuonChamberToolTestCfg
    parser = SetupArgParser()

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)    
    
    cfg.merge(MuonChamberToolTestCfg(flags))
    executeTest(cfg, num_events = args.nEvents)


