# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
## Algorithm sequence
from AthenaCommon.AppMgr import ToolSvc
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

## Output threshold (DEBUG, INFO, WARNING, ERROR, FATAL)
ServiceMgr.MessageSvc.OutputLevel = options['outLVL']

## Detector flags
from AthenaCommon.DetFlags import DetFlags
DetFlags.ID_setOn()
DetFlags.Calo_setOn()
DetFlags.Muon_setOn()
#DetFlags.Lucid_setOn()
DetFlags.Truth_setOn()

## Global conditions tag
from AthenaCommon.GlobalFlags import globalflags
print(options)
globalflags.ConditionsTag = options['condition']

## Simulation flags
from G4AtlasApps.SimFlags import simFlags
simFlags.load_atlas_flags()
simFlags.CalibrationRun.set_Off()

simFlags.RunNumber = options['runNumber']

if len(options['geometry']) > 0 :
    simFlags.SimLayout = options['geometry']
else :
    simFlags.SimLayout.set_On()

if len(options['physlist']) > 0 :
    simFlags.PhysicsList = options['physlist']

## AthenaCommon flags
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.EvtMax = options['nevents']
athenaCommonFlags.PoolHitsOutput.set_Off()

## Set the LAr parameterization
simFlags.LArParameterization = 1

from random import randint
simFlags.RandomSeedOffset = randint(1,443921180)

if options['input'] is not None:
    simFlags.RandomSeedOffset = options['skipevents']    
    athenaCommonFlags.PoolEvgenInput = options['input']
    athenaCommonFlags.SkipEvents = options['skipevents']
else:
    ## Use single particle generator
    import AthenaCommon.AtlasUnixGeneratorJob
    import ParticleGun as PG
    topSequence += PG.ParticleGun(randomStream = "SINGLE", randomSeed = simFlags.RandomSeedOffset.get_Value())
    topSequence.ParticleGun.sampler.pid = int(options["pid"])

    topSequence.ParticleGun.sampler.mom = PG.EEtaMPhiSampler(energy=[options['energyMin'],options['energyMax']], eta=[options['etaMin'],options['etaMax']])
    from EvgenJobTransforms.EvgenConfig import evgenConfig
    evgenConfig.generators += ["ParticleGun"]

    athenaCommonFlags.PoolEvgenInput.set_Off()


include("G4AtlasApps/G4Atlas.flat.configuration.py")

## Populate alg sequence
from AthenaCommon.CfgGetter import getAlgorithm
topSequence += getAlgorithm("G4AtlasAlg",tryDefaultConfigurable=True)
topSequence.G4AtlasAlg.InputTruthCollection = "GEN_EVENT"

ToolSvc.FastSimulationMasterTool.FastSimulations["EMBFastShower"].GeneratedStartingPointsFile  = "EMB."+options['output']
ToolSvc.FastSimulationMasterTool.FastSimulations["EMBFastShower"].GeneratedStartingPointsRatio = options['spratio']
ToolSvc.FastSimulationMasterTool.FastSimulations["EMECFastShower"].GeneratedStartingPointsFile ="EMEC."+options['output']
ToolSvc.FastSimulationMasterTool.FastSimulations["EMECFastShower"].GeneratedStartingPointsRatio=options['spratio']
ToolSvc.FastSimulationMasterTool.FastSimulations["FCALFastShower"].GeneratedStartingPointsFile ="FCAL1."+options['output']
ToolSvc.FastSimulationMasterTool.FastSimulations["FCALFastShower"].GeneratedStartingPointsRatio=options['spratio']
ToolSvc.FastSimulationMasterTool.FastSimulations["FCAL2FastShower"].GeneratedStartingPointsFile="FCAL2."+options['output']
ToolSvc.FastSimulationMasterTool.FastSimulations["FCAL2FastShower"].GeneratedStartingPointsRatio=options['spratio']

