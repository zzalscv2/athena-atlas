# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# Headers and setup
#--------------------------------------------------------------
from AthenaCommon.AlgSequence import AlgSequence
theJob = AlgSequence()

from AthenaCommon.AppMgr import theApp, ToolSvc, ServiceMgr
from AthenaCommon.Constants import VERBOSE,DEBUG,INFO

theApp.EvtMax = runArgs.maxEvents

from GaudiSvc.GaudiSvcConf import THistSvc

from PerfMonComps.PerfMonFlags import jobproperties

#--------------------------------------------------------------
# FPGATrackSim Setup
#--------------------------------------------------------------

from FPGATrackSimConfTools.parseRunArgs import parseFPGATrackSimArgs
import FPGATrackSimConfTools.FPGATrackSimTagConfig as FPGATrackSimTagConfig
import FPGATrackSimMaps.FPGATrackSimMapConfig as FPGATrackSimMapConfig
import FPGATrackSimBanks.FPGATrackSimBankConfig as FPGATrackSimBankConfig
import FPGATrackSimAlgorithms.FPGATrackSimAlgorithmConfig as FPGATrackSimAlgorithmConfig

print("Input file:  ", runArgs.InFileName)
tags = FPGATrackSimTagConfig.getTags(stage='algo', options=parseFPGATrackSimArgs(runArgs))
map_tag = tags['map']
bank_tag = tags['bank']
algo_tag = tags['algo']

def defaultFilename():
    f = FPGATrackSimTagConfig.getDescription(map_tag, bank_tag, algo_tag, filename=True) # deliberately omitting hit filtering tag for now
    f = 'loghits__' +  map_tag['release'] + '-' + map_tag['geoTag'] + '__' + f + '.root'
    return f

OutFileName=getattr(runArgs, 'OutFileName', defaultFilename())
ServiceMgr += THistSvc()
ServiceMgr.THistSvc.Output += ["MONITOROUT DATAFILE='"+OutFileName+"' OPT='RECREATE'"]

MapSvc = FPGATrackSimMapConfig.addMapSvc(map_tag)

HitFilteringTool = FPGATrackSimMapConfig.addHitFilteringTool(map_tag)

BankSvc = FPGATrackSimBankConfig.addBankSvc(map_tag, bank_tag)


# Important to do hit tracing if we have physics samples or events with pileup.
doHitTracing = (map_tag['withPU'] or map_tag['sampleType']=='LLPs')

#--------------------------------------------------------------
# Make the algorithm
#--------------------------------------------------------------

from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmsConf import FPGATrackSimLogicalHitsProcessAlg
from FPGATrackSimHough.FPGATrackSimHoughConf import FPGATrackSimEtaPatternFilterTool, FPGATrackSimPhiRoadFilterTool

alg = FPGATrackSimLogicalHitsProcessAlg()
alg.HitFiltering = algo_tag['HitFiltering']
alg.writeOutputData = algo_tag['writeOutputData']
alg.Clustering = True
alg.tracking = algo_tag['doTracking']
alg.outputHitTxt = algo_tag['outputHitTxt']
alg.RunSecondStage = algo_tag['secondStage']
alg.DoMissingHitsChecks = algo_tag['DoMissingHitsChecks']

theJob += alg

#--------------------------------------------------------------
# Make hit tools
#--------------------------------------------------------------

from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimRawToLogicalHitsTool
FPGATrackSimRawLogic = FPGATrackSimRawToLogicalHitsTool()
FPGATrackSimRawLogic.SaveOptional = 2
if (map_tag['sampleType'] == 'skipTruth'):
    FPGATrackSimRawLogic.SaveOptional = 1
FPGATrackSimRawLogic.TowersToMap = [0] # TODO TODO why is this hardcoded?
ToolSvc += FPGATrackSimRawLogic
alg.RawToLogicalHitsTool = FPGATrackSimRawLogic

from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimInputHeaderTool
InputTool = FPGATrackSimInputHeaderTool("FPGATrackSimReadInput")
InputTool.InFileName = runArgs.InFileName
ToolSvc += InputTool
alg.InputTool = InputTool

from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimReadRawRandomHitsTool
InputTool2 = FPGATrackSimReadRawRandomHitsTool("FPGATrackSimReadRawRandomHitsTool")
InputTool2.InFileName = runArgs.InFileName[0]
ToolSvc += InputTool2
alg.InputTool2 = InputTool2


if algo_tag['EtaPatternRoadFilter']:
    print("Setting Up EtaPatternFilter")
    RoadFilter = FPGATrackSimEtaPatternFilterTool()
    alg.FilterRoads=True
    if len(algo_tag['threshold']) != 1:
        raise NotImplementedError("EtaPatternRoadFilter does not support multi-value/neighboring bin treshold")
    RoadFilter.threshold=algo_tag['threshold'][0]
    RoadFilter.EtaPatterns=algo_tag['EtaPatternRoadFilter']
    ToolSvc += RoadFilter

if  algo_tag['PhiRoadFilter']!=None:
    print("Setting Up PhiRoadFilter")
    PhiRoadFilter = FPGATrackSimPhiRoadFilterTool()
    alg.FilterRoads2=True
    if len(algo_tag['threshold'])!=1:
        raise NotImplementedError("PhiRoadFilter does not support multi-value/neighboring bin treshold")
    PhiRoadFilter.threshold=algo_tag['threshold'][0]
    PhiRoadFilter.window=FPGATrackSimAlgorithmConfig.floatList(algo_tag['PhiRoadFilter'])
    ToolSvc += PhiRoadFilter

# writing down the output file
from FPGATrackSimInput.FPGATrackSimInputConf import FPGATrackSimOutputHeaderTool
FPGATrackSimWriteOutput = FPGATrackSimOutputHeaderTool("FPGATrackSimWriteOutput")
FPGATrackSimWriteOutput.InFileName = ["test"]
FPGATrackSimWriteOutput.RWstatus = "HEADER" # do not open file, use THistSvc
FPGATrackSimWriteOutput.RunSecondStage = alg.RunSecondStage
ToolSvc += FPGATrackSimWriteOutput
alg.OutputTool= FPGATrackSimWriteOutput

#--------------------------------------------------------------
# Spacepoint tool
#--------------------------------------------------------------

from FPGATrackSimMaps.FPGATrackSimMapsConf import FPGATrackSimSpacePointsTool, FPGATrackSimSpacePointsTool_v2

spacepoints_version = 2
if spacepoints_version == 1:
    SPT = FPGATrackSimSpacePointsTool()
    SPT.FilteringClosePoints = True
else:
    SPT = FPGATrackSimSpacePointsTool_v2()
    SPT.FilteringClosePoints = False
    SPT.PhiWindow=0.008

SPT.Duplication = True
SPT.Filtering = algo_tag['SpacePointFiltering']
ToolSvc += SPT
alg.SpacePointTool = SPT

#--------------------------------------------------------------
# Make RF/TF tools
#--------------------------------------------------------------

from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmsConf import FPGATrackSimNNTrackTool, FPGATrackSimOverlapRemovalTool, FPGATrackSimTrackFitterTool 
from FPGATrackSimHough.FPGATrackSimHoughConf import FPGATrackSimHoughRootOutputTool

if algo_tag['hough']:
    RF = FPGATrackSimAlgorithmConfig.addHoughTool(map_tag, algo_tag,doHitTracing)
    alg.DoNNTrack = algo_tag['TrackNNAnalysis']
    if algo_tag['TrackNNAnalysis']:
        NNTrackTool = FPGATrackSimNNTrackTool()
        ToolSvc += NNTrackTool
    alg.DoHoughRootOutput = algo_tag['hough_rootoutput']
    if algo_tag['hough_rootoutput']:
        rootOutTool = FPGATrackSimHoughRootOutputTool()
        ToolSvc += rootOutTool
        HoughRootOutputName="FPGATrackSimHoughOutput.root"
        ServiceMgr.THistSvc.Output += ["TRIGFPGATrackSimHOUGHOUTPUT DATAFILE='"+HoughRootOutputName+"' OPT='RECREATE'"]
elif algo_tag['hough_1d']:
    RF = FPGATrackSimHoughConfig.addHough1DShiftTool(map_tag, algo_tag)

TF_1st= FPGATrackSimTrackFitterTool("FPGATrackSimTrackFitterTool_1st")
FPGATrackSimAlgorithmConfig.applyTag(TF_1st, algo_tag)
ToolSvc += TF_1st

alg.RoadFinder = RF
alg.TrackFitter_1st = TF_1st

OR_1st = FPGATrackSimOverlapRemovalTool("FPGATrackSimOverlapRemovalTool_1st")
OR_1st.ORAlgo = "Normal"
OR_1st.doFastOR = algo_tag['doFastOR']
OR_1st.NumOfHitPerGrouping = 5
# For invert grouping use the below setup
# OR1st.ORAlgo = "Invert"
# OR1st.NumOfHitPerGrouping = 3
if algo_tag['hough']:
    OR_1st.nBins_x = algo_tag['xBins'] + 2 * algo_tag['xBufferBins']
    OR_1st.nBins_y = algo_tag['yBins'] + 2 * algo_tag['yBufferBins']
    OR_1st.localMaxWindowSize = algo_tag['localMaxWindowSize']
    OR_1st.roadSliceOR = algo_tag['roadSliceOR']

#--------------------------------------------------------------
# Second stage fitting
#--------------------------------------------------------------

if algo_tag['secondStage']:

    TF_2nd = FPGATrackSimTrackFitterTool("FPGATrackSimTrackFitterTool_2nd")
    FPGATrackSimAlgorithmConfig.applyTag(TF_2nd, algo_tag)
    TF_2nd.Do2ndStageTrackFit = True
    ToolSvc += TF_2nd
    alg.TrackFitter_2nd = TF_2nd

    OR_2nd = FPGATrackSimOverlapRemovalTool("FPGATrackSimOverlapRemovalTool_2nd")
    OR_2nd.DoSecondStage = True
    OR_2nd.ORAlgo = "Normal"
    OR_2nd.doFastOR = algo_tag['doFastOR']
    OR_2nd.NumOfHitPerGrouping = 5
    # For invert grouping use the below setup
    # OR2nd.ORAlgo = "Invert"
    # OR2nd.NumOfHitPerGrouping = 3
    ToolSvc += OR_2nd
    alg.OverlapRemoval_2nd = OR_2nd

    from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmsConf import FPGATrackSimExtrapolator
    FPGATrackSimExtrapolatorTool = FPGATrackSimExtrapolator()
    FPGATrackSimExtrapolatorTool.Ncombinations = 16
    ToolSvc += FPGATrackSimExtrapolatorTool
    alg.Extrapolator = FPGATrackSimExtrapolatorTool

#--------------------------------------------------------------
# Configure LRT
#--------------------------------------------------------------

# We'll use this in other tools if LRT is requested
from FPGATrackSimConfTools import FPGATrackSimConfigCompInit
evtSelLRT = FPGATrackSimConfigCompInit.addEvtSelSvc(map_tag,"EvtSelLRTSvc")

from FPGATrackSimLRT.FPGATrackSimLRTConf import FPGATrackSimLLPRoadFilterTool
thellpfilter = FPGATrackSimLLPRoadFilterTool()
ToolSvc += thellpfilter

if algo_tag['lrt']:

    # Finish setting up that event selector
    evtSelLRT.doLRT = True
    evtSelLRT.minLRTpT = algo_tag['lrt_ptmin']
    evtSelLRT.OutputLevel=DEBUG

    # consistency checks
    assert algo_tag['lrt_use_basicHitFilter'] != algo_tag['lrt_use_mlHitFilter'], 'Inconsistent LRT hit filtering setup, need either ML of Basic filtering enabled'
    assert algo_tag['lrt_use_straightTrackHT'] != algo_tag['lrt_use_doubletHT'], 'Inconsistent LRT HT setup, need either double or strightTrack enabled'
    alg.doLRT = True
    alg.LRTHitFiltering = (not algo_tag['lrt_skip_hit_filtering'])

    # now set up the rest
    from FPGATrackSimLRT.FPGATrackSimLRTConf import FPGATrackSimLLPRoadFilterTool
    if algo_tag['lrt_use_basicHitFilter']:
        alg.LRTRoadFilter = thellpfilter
    if algo_tag['lrt_use_doubletHT']:
        from FPGATrackSimLRT.FPGATrackSimLRTConf import addLRTDoubletFPGATrackSimool
        doubletTool = addLRTDoubletFPGATrackSimool(algo_tag)
        alg.LRTRoadFinder  = doubletTool
    elif algo_tag['lrt_use_straightTrackHT'] :
        alg.LRTRoadFinder = FPGATrackSimAlgorithmConfig.addHough_d0phi0_Tool(map_tag, algo_tag, doHitTracing)

    # Use our event selector in the algorithm
    alg.eventSelector = evtSelLRT


ToolSvc += OR_1st
alg.OverlapRemoval_1st = OR_1st
