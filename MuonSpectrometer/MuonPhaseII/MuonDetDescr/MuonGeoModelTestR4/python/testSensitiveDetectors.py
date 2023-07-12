# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser
    parser = SetupArgParser()
    parser.add_argument("--nEvents", help="Number of events to rum", type = int ,default = 100)

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    
    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    cfg.merge(BeamEffectsAlgCfg(flags))

    from G4AtlasAlg.G4AtlasAlgConfig import G4AtlasAlgCfg
    cfg.merge(G4AtlasAlgCfg(flags))
     ####    
    DetDescCnvSvc = cfg.getService("DetDescrCnvSvc")
    DetDescCnvSvc.IdDictFromRDB = False
    DetDescCnvSvc.MuonIDFileName="IdDictParser/IdDictMuonSpectrometer_R.10.00.xml"
    DetDescCnvSvc.MuonIDFileName="IdDictParser/IdDictMuonSpectrometer_R.09.03.xml"
    cfg.printConfig(withDetails=True, summariseProps=True)
    if not cfg.run(args.nEvents).isSuccess():
        import sys
        sys.exit("Execution failed")
