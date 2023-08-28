#!/usr/bin/env python
"""Run tests on G4AtlasAlgConfig

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""


if __name__ == '__main__':

    import time
    a = time.time()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    # Set up logging
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)


    #import and set config flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Exec.MaxEvents = 4
    flags.Exec.SkipEvents = 0
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Simulation
    flags.Input.RunNumber = [284500] #Isn't updating - todo: investigate
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumber = [1]
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1'] #defaultTestFiles.EVNT
    flags.Output.HITSFileName = "myHITSnew.pool.root"

    #Sim flags
    #flags.Sim.WorldRRange = 15000
    #flags.Sim.WorldZRange = 27000 #change defaults?
    from SimulationConfig.SimEnums import BeamPipeSimMode, CalibrationRun, CavernBackground
    flags.Sim.CalibrationRun = CalibrationRun.Off
    flags.Sim.RecordStepInfo = False
    flags.Sim.CavernBackground = CavernBackground.Signal
    flags.Sim.ISFRun = False
    flags.Sim.BeamPipeSimMode = BeamPipeSimMode.FastSim

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.GeoModel.Align.Dynamic = False
    flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-14"

    # Finalize
    flags.lock()

    ## Initialize a new component accumulator
    cfg = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # add BeamEffectsAlg
    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    cfg.merge(BeamEffectsAlgCfg(flags))

    #add the G4AtlasAlg
    from G4AtlasAlg.G4AtlasAlgConfig import G4AtlasAlgCfg
    cfg.merge(G4AtlasAlgCfg(flags))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from SimuJobTransforms.SimOutputConfig import getStreamHITS_ItemList
    cfg.merge(OutputStreamCfg(flags, "HITS", ItemList=getStreamHITS_ItemList(flags), disableEventTag=True, AcceptAlgs=['G4AtlasAlg']))

    # FIXME hack to match to buggy behaviour in old style configuration
    OutputStreamHITS = cfg.getEventAlgo("OutputStreamHITS")
    OutputStreamHITS.ItemList.remove("xAOD::EventInfo#EventInfo")
    OutputStreamHITS.ItemList.remove("xAOD::EventAuxInfo#EventInfoAux.")

    # FIXME hack because deduplication is broken
    PoolAttributes = ["TREE_BRANCH_OFFSETTAB_LEN = '100'"]
    PoolAttributes += [f"DatabaseName = '{flags.Output.HITSFileName}'; ContainerName = 'TTree=CollectionTree'; TREE_AUTO_FLUSH = '1'"]
    cfg.getService("AthenaPoolCnvSvc").PoolAttributes += PoolAttributes

    # Dump config
    from AthenaConfiguration.ComponentFactory import CompFactory
    cfg.addEventAlgo(CompFactory.JobOptsDumperAlg(FileName="G4AtlasTestConfig.txt"))
    cfg.getService("StoreGateSvc").Dump = True
    cfg.getService("ConditionStore").Dump = True
    cfg.printConfig(withDetails=True, summariseProps = True)

    flags.dump()

    with open("test.pkl", "wb") as f:
        cfg.store(f)

    # Execute and finish
    sc = cfg.run()

    b = time.time()
    log.info("Run G4AtlasAlg in %s seconds", str(b-a))

    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())
