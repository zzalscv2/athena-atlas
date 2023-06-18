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

# Input
FPGATrackSimInputFile = []
if runArgs.InFileName:
	for ex in runArgs.InFileName.split(','):
		files = glob.glob(ex)
		if files:
			FPGATrackSimInputFile += files
		else:
			FPGATrackSimInputFile += [ex]
else:
    FPGATrackSimInputFile = ["httsim_input.root"]
print "Input file:  ", FPGATrackSimInputFile

tags = FPGATrackSimTagConfig.getTags(stage='algo', options=parseFPGATrackSimArgs(runArgs))
map_tag = tags['map']
bank_tag = tags['bank']
algo_tag = tags['algo']

def defaultFilename():
    f = FPGATrackSimTagConfig.getDescription(map_tag, bank_tag, algo_tag, filename=True)
    f = 'output__' +  map_tag['release'] + '-' + map_tag['geoTag'] + '__' + f + '.root'
    return f

OutFileName=getattr(runArgs, 'OutFileName', defaultFilename())
ServiceMgr += THistSvc()
ServiceMgr.THistSvc.Output += ["MONITOROUT DATAFILE='"+OutFileName+"' OPT='RECREATE'"]

ES = FPGATrackSimConfig.addEvtSelSvc(map_tag)
ES.OutputLevel = FPGATrackSim_OutputLevel

MapSvc = FPGATrackSimMapConfig.addMapSvc(map_tag)
MapSvc.OutputLevel = FPGATrackSim_OutputLevel

#--------------------------------------------------------------
# Make the algorithm
#--------------------------------------------------------------

from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmsConf import FPGATrackSimOutputMonitorAlg

alg = FPGATrackSimOutputMonitorAlg()
alg.OutputLevel = FPGATrackSim_OutputLevel
alg.RunSecondStage = algo_tag['secondStage']
alg.histoPrintDetail = 2

theJob += alg

#--------------------------------------------------------------
# Make Read Output Tool
#--------------------------------------------------------------

from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimOutputHeaderTool

OutputReader = FPGATrackSimOutputHeaderTool()
OutputReader.OutputLevel = FPGATrackSim_OutputLevel
OutputReader.InFileName = FPGATrackSimInputFile
OutputReader.RWstatus="READ"
OutputReader.RunSecondStage = alg.RunSecondStage

ToolSvc += OutputReader
alg.ReadOutputTool = OutputReader

#--------------------------------------------------------------
# Make Monitor Tools and Data Flow Tool
#--------------------------------------------------------------

from FPGATrackSimMonitor.FPGATrackSimMonitorConf import FPGATrackSimHitMonitorTool, FPGATrackSimClusterMonitorTool, FPGATrackSimRoadMonitorTool, FPGATrackSimTrackMonitorTool, FPGATrackSimPerformanceMonitorTool, FPGATrackSimEventMonitorTool, FPGATrackSimSecondStageMonitorTool, FPGATrackSimMonitorUnionTool
from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmsConf import FPGATrackSimDataFlowTool

HitMonitor = FPGATrackSimHitMonitorTool()
HitMonitor.OutputLevel = FPGATrackSim_OutputLevel
HitMonitor.fastMon = algo_tag['fastMon']
HitMonitor.OutputMon = True
HitMonitor.RunSecondStage = alg.RunSecondStage
HitMonitor.canExtendHistRanges = algo_tag['canExtendHistRanges']

ClusterMonitor = FPGATrackSimClusterMonitorTool()
ClusterMonitor.OutputLevel = FPGATrackSim_OutputLevel
ClusterMonitor.Clustering = False
ClusterMonitor.Spacepoints = False
ClusterMonitor.RunSecondStage = alg.RunSecondStage
ClusterMonitor.canExtendHistRanges = algo_tag['canExtendHistRanges']

RoadMonitor = FPGATrackSimRoadMonitorTool()
RoadMonitor.OutputLevel = FPGATrackSim_OutputLevel
RoadMonitor.fastMon = algo_tag['fastMon']
RoadMonitor.RunSecondStage = alg.RunSecondStage
RoadMonitor.canExtendHistRanges = algo_tag['canExtendHistRanges']
RoadMonitor.BarcodeFracCut = algo_tag['barcodeFracMatch']

TrackMonitor = FPGATrackSimTrackMonitorTool()
TrackMonitor.OutputLevel = FPGATrackSim_OutputLevel
TrackMonitor.fastMon = algo_tag['fastMon']
TrackMonitor.OutputMon = True
TrackMonitor.DoMissingHitsChecks = False
TrackMonitor.RunSecondStage = alg.RunSecondStage

PerfMonitor = FPGATrackSimPerformanceMonitorTool()
PerfMonitor.OutputLevel = FPGATrackSim_OutputLevel
PerfMonitor.fastMon = algo_tag['fastMon']
PerfMonitor.RunSecondStage = alg.RunSecondStage
PerfMonitor.BarcodeFracCut = algo_tag['barcodeFracMatch']

EventMonitor = FPGATrackSimEventMonitorTool()
EventMonitor.OutputLevel = FPGATrackSim_OutputLevel
EventMonitor.fastMon = algo_tag['fastMon']
EventMonitor.OutputMon = True
EventMonitor.Clustering = False
EventMonitor.Spacepoints = False
EventMonitor.RunSecondStage = alg.RunSecondStage
EventMonitor.BarcodeFracCut = algo_tag['barcodeFracMatch']

SecondMonitor = FPGATrackSimSecondStageMonitorTool()
SecondMonitor.OutputLevel = FPGATrackSim_OutputLevel
SecondMonitor.fastMon = algo_tag['fastMon']
SecondMonitor.BarcodeFracCut = algo_tag['barcodeFracMatch']
SecondMonitor.Chi2ndofCut = 40.

MonitorUnion = FPGATrackSimMonitorUnionTool()
MonitorUnion.OutputLevel = FPGATrackSim_OutputLevel
MonitorTools = []
MonitorTools.append(HitMonitor)
MonitorTools.append(ClusterMonitor)
MonitorTools.append(RoadMonitor)
MonitorTools.append(TrackMonitor)
MonitorTools.append(PerfMonitor)
MonitorTools.append(EventMonitor)
if alg.RunSecondStage:
	MonitorTools.append(SecondMonitor)
MonitorUnion.MonitorTools = MonitorTools

DataFlowTool = FPGATrackSimDataFlowTool()
DataFlowTool.OutputLevel = FPGATrackSim_OutputLevel
DataFlowTool.RunSecondStage = alg.RunSecondStage

ToolSvc += MonitorUnion
ToolSvc += DataFlowTool

alg.MonitorUnionTool = MonitorUnion
alg.DataFlowTool = DataFlowTool
