# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# Headers and setup
#--------------------------------------------------------------
import glob

from AthenaCommon.AlgSequence import AlgSequence
theJob = AlgSequence()

from AthenaCommon.AppMgr import theApp, ToolSvc, ServiceMgr
from AthenaCommon.Constants import VERBOSE,DEBUG,INFO

theApp.EvtMax = runArgs.maxEvents

from GaudiSvc.GaudiSvcConf import THistSvc

#--------------------------------------------------------------
# FPGATrackSim Setup
#--------------------------------------------------------------

from FPGATrackSimConfig.parseRunArgs import parseFPGATrackSimArgs
from FPGATrackSimConfig.formatMessageSvc import FPGATrackSim_OutputLevel, formatMessageSvc
import FPGATrackSimConfig.FPGATrackSimTagConfig as FPGATrackSimTagConfig
import FPGATrackSimConfig.FPGATrackSimConfigCompInit as FPGATrackSimConfig
import FPGATrackSimMaps.FPGATrackSimMapConfig as FPGATrackSimMapConfig

formatMessageSvc()

FPGATrackSim_OutputLevel=getattr(runArgs, 'OutputLevel', FPGATrackSim_OutputLevel)
#FPGATrackSim_OutputLevel = DEBUG

FPGATrackSimInputFile = runArgs.InFileName
print "Input file:  ", FPGATrackSimInputFile

tags = FPGATrackSimTagConfig.getTags(stage='algo', options=parseFPGATrackSimArgs(runArgs))
map_tag = tags['map']

def defaultFilename():
	return "MapMakerOutput"

OutFileName = getattr(runArgs, 'OutFileName', defaultFilename())
ServiceMgr += THistSvc()
ServiceMgr.THistSvc.Output += ["MONITOROUT DATAFILE='"+OutFileName+"' OPT='RECREATE'"]

ES = FPGATrackSimConfig.addEvtSelSvc(map_tag)
ES.OutputLevel = FPGATrackSim_OutputLevel
MapSvc = FPGATrackSimMapConfig.addMapSvc(map_tag)
MapSvc.OutputLevel = FPGATrackSim_OutputLevel

#--------------------------------------------------------------
# Make the algorithm
#--------------------------------------------------------------

from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmsConf import FPGATrackSimMapMakerAlg

alg = FPGATrackSimMapMakerAlg()
alg.OutputLevel = FPGATrackSim_OutputLevel
alg.OutFileName = OutFileName
alg.region = getattr(runArgs, 'region', 0)
alg.KeyString = getattr(runArgs, 'KeyString', "strip,barrel,2")
alg.KeyString2 = getattr(runArgs, 'KeyString2', "")
alg.nSlices = getattr(runArgs, 'nSlices', -1)
alg.trim = getattr(runArgs, 'trim', 0.1)
alg.maxEvents = getattr(runArgs, 'maxEvents', 1000)

theJob += alg

#--------------------------------------------------------------
# Make hit tools
#--------------------------------------------------------------

from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimInputHeaderTool

InputTool = FPGATrackSimInputHeaderTool("FPGATrackSimReadInput")
InputTool.OutputLevel = FPGATrackSim_OutputLevel
InputTool.InFileName = runArgs.InFileName

ToolSvc += InputTool
alg.InputTool = InputTool

# END TEST