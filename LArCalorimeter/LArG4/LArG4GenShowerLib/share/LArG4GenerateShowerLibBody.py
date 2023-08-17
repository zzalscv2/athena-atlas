# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

## Algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
topSeq = AlgSequence()

## Output threshold (DEBUG, INFO, WARNING, ERROR, FATAL)
ServiceMgr.MessageSvc.OutputLevel = INFO

## Detector flags
from AthenaCommon.DetFlags import DetFlags
DetFlags.ID_setOn()
DetFlags.Calo_setOn()
DetFlags.Muon_setOn()
DetFlags.Truth_setOn()

## Global conditions tag
from AthenaCommon.GlobalFlags import globalflags
globalflags.ConditionsTag = options['condition']

## Simulation flags
from G4AtlasApps.SimFlags import simFlags
simFlags.load_atlas_flags()
simFlags.CalibrationRun.set_Off()

## Layout tags: see simFlags.SimLayout for allowed values
## Use the default layout:
#simFlags.SimLayout.set_On()
## Set a specific layout tag:
if len(options['geometry']) > 0 :
    simFlags.SimLayout = options['geometry']
else :
    simFlags.SimLayout.set_On()

if len(options['physlist']) > 0 :
    simFlags.PhysicsList = options['physlist']

simFlags.RunNumber = options['runNumber']

## AthenaCommon flags
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.EvtMax = options['nevents']
athenaCommonFlags.SkipEvents = options['skipevents']
athenaCommonFlags.PoolHitsOutput.set_Off()

athenaCommonFlags.PoolEvgenInput=options['inputevt']

## Set the LAr parameterization
simFlags.LArParameterization = options['parametrization']

if options['parametrization'] == 2:
    ## this is used when printing out showers taken from lib for debugging purposes
    if len(options['fsLibs']) > 0 :
        printfunc ("Setting up ShowerLib Service")
        #from LArG4ShowerLibSvc.LArG4ShowerLibSvcConf import LArG4ShowerLibSvc
        from LArG4ShowerLibSvc.LArG4ShowerLibSvcConfig import getLArG4ShowerLibSvc
        if not hasattr( ServiceMgr, 'LArG4ShowerLibSvc' ):
            ServiceMgr += getLArG4ShowerLibSvc()
            ServiceMgr.LArG4ShowerLibSvc.FileNameList = options['fsLibs']

# get service manager
from AthenaCommon.AppMgr import ServiceMgr

include("G4AtlasApps/G4Atlas.flat.configuration.py")

## Populate alg sequence
from random import randint
simFlags.RandomSeedOffset = randint(1,443921180)

## Adding G4 TestAaction, which creates showers.
from G4AtlasApps.SimFlags import simFlags
simFlags.OptionalUserActionList.addAction('G4UA::TestActionShowerLibTool')

## Specifying event collection for the TestActionShowerLibTool.
from AthenaCommon.CfgGetter import getAlgorithm
topSeq += getAlgorithm("G4AtlasAlg",tryDefaultConfigurable=True)
topSeq.G4AtlasAlg.InputTruthCollection = "GEN_EVENT"

## Adding algorithm, which performs shower post-processing,
## namely affine transformation, clusterisation and truncation.
from AthenaCommon.CfgGetter import getAlgorithm
topSeq += getAlgorithm("LArG4GenShowerLib")
topSeq.LArG4GenShowerLib.LibStructFiles = options['inputstruct']
