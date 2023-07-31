#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


### Simple script to test the conversion of the xAOD -> legacy Mdt sim hit format
if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    from xAODMuonSimHitCnv.MuonSimHitCnvCfg import xAODtoMdtCnvAlgCfg
    cfg.merge(xAODtoMdtCnvAlgCfg(flags))

    executeTest(cfg, num_events = args.nEvents)
  
