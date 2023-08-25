#!/usr/bin/env python
"""FastCaloSimServices test helpers

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""

from argparse import ArgumentParser
from AthenaConfiguration.Enums import ProductionStep
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def JobOptsDumperCfg(flags):
    """Configure event loop for FCSServices"""
    JobOptsDumperAlg = CompFactory.JobOptsDumperAlg
    acc = ComponentAccumulator()
    acc.addEventAlgo(JobOptsDumperAlg(FileName="FCSServicesTestConfig.txt"))
    return acc


def TestMessageSvcCfg(flags):
    """MessageSvc for FCSServices"""
    MessageSvc = CompFactory.MessageSvc
    acc = ComponentAccumulator()
    acc.addService(MessageSvc(setError=["HepMcParticleLink"]))
    return acc


def CommonTestArgumentParser():
    """FCSServices test argument parser"""
    parser = ArgumentParser()
    parser.add_argument("-n", "--maxEvents", default=3, type=int,
                        help="The number of events to run. 0 skips execution")
    parser.add_argument("-t", "--threads", default=1, type=int,
                        help="The number of concurrent threads to run. 0 uses serial Athena.")
    parser.add_argument("-V", "--verboseAccumulators", default=False, action="store_true",
                        help="Print full details of the AlgSequence for each accumulator")
    parser.add_argument("-S", "--verboseStoreGate", default=False, action="store_true",
                        help="Dump the StoreGate(s) each event iteration")
    parser.add_argument("-o", "--output", default='', type=str,
                        help="Output RDO file")
    parser.add_argument("-s", "--outputSig", default='', type=str,
                        help="Output RDO_SGNL file")
    return parser


def defaultTestFlags(configFlags, args):
    """Fill default FCSServices flags for testing"""

    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    configFlags.Input.RunNumber = [284500]
    configFlags.Input.OverrideRunNumber = True
    configFlags.Input.LumiBlockNumber = [1]
    configFlags.Input.Files = defaultTestFiles.EVNT # ["root://eosuser.cern.ch///eos/atlas/atlascerngroupdisk/proj-simul/OutputSamples/rel21/mc16_13TeV.photon.E65536.eta20_25.EVNT.merged.pool.root"]
    configFlags.Output.HITSFileName = "myHITSnew.pool.root"
    configFlags.Common.ProductionStep = ProductionStep.Simulation

    # Sim configFlags
    from SimulationConfig.SimEnums import BeamPipeSimMode, CalibrationRun, LArParameterization, SimulationFlavour, TruthStrategy
    configFlags.Sim.TruthStrategy = TruthStrategy.MC15aPlus
    configFlags.Sim.PhysicsList = "FTFP_BERT_ATL"
    configFlags.Sim.CalibrationRun = CalibrationRun.Off 
    configFlags.Sim.RecordStepInfo = False
    configFlags.Sim.BeamPipeSimMode = BeamPipeSimMode.FastSim
    configFlags.Sim.ISFRun = True
    configFlags.Sim.ISF.Simulator = SimulationFlavour.ATLFAST3MT
    configFlags.Sim.FastCalo.ParamsInputFilename = "FastCaloSim/MC16/TFCSparam_run2_reprocessing.root"
    configFlags.Sim.FastCalo.CaloCellsName = "AllCalo"

    configFlags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-14"
    configFlags.GeoModel.Align.Dynamic = False
    configFlags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2

    detectors = ['Bpipe', 'BCM', 'Pixel', 'SCT', 'TRT', 'LAr', 'Tile', 'MBTS', 'CSC', 'MDT', 'RPC', 'TGC']
    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(configFlags, detectors, toggle_geometry=True)

    # Frozen showers OFF = 0
    configFlags.Sim.LArParameterization = LArParameterization.NoFrozenShowers

    if args.output:
        if args.output == 'None':
            configFlags.Output.RDOFileName = ''
        else:
            configFlags.Output.RDOFileName = args.output

    if args.outputSig:
        configFlags.Output.RDO_SGNLFileName = args.outputSig


def postprocessAndLockFlags(configFlags, args):
    """Postprocess and lock config flags for FCSServices"""
    # Flags relating to multithreaded execution
    configFlags.Concurrency.NumThreads = args.threads
    if args.threads > 0:
        configFlags.Scheduler.ShowDataDeps = True
        configFlags.Scheduler.ShowDataFlow = True
        configFlags.Scheduler.ShowControlFlow = True
        configFlags.Concurrency.NumConcurrentEvents = args.threads

    configFlags.lock()


def printAndRun(accessor, configFlags, args):
    """Common debugging and execution for FCSServices tests"""
    # Dump config
    if args.verboseAccumulators:
        accessor.printConfig(withDetails=True)
    if args.verboseStoreGate:
        accessor.getService("StoreGateSvc").Dump = True
    configFlags.dump()

    # Dump config summary
    accessor.printConfig(withDetails=False)

    # Execute and finish
    sc = accessor.run(maxEvents=args.maxEvents)


    # Success should be 0
    return not sc.isSuccess()
