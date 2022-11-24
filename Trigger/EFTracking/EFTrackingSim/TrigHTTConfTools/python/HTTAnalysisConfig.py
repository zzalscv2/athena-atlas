# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HTTAnalysisConfig():
    print('HTTAnalysisConfig')


def getNSubregions(path):
    import os

    from PyJobTransforms.trfUtils import findFile
    path = findFile(os.environ['DATAPATH'], path)

    with open(path, 'r') as f:
        fields = f.readline().split()
        assert(fields[0] == 'towers')
        return int(fields[1])


def HTTEventSelectionCfg():
    result=ComponentAccumulator()
    eventSelector = CompFactory.HTTEventSelectionSvc()
    eventSelector.OutputLevel=2
    eventSelector.regions = "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/HTT/TrigHTTMaps/V1/map_file/slices_v01_Jan21.txt"
    eventSelector.regionID = 0
    eventSelector.sampleType = 'singleMuons'
    eventSelector.withPU = False
    result.addService(eventSelector, create=True, primary=True)
    return result

def TrigHTTMappingCfg():
    result=ComponentAccumulator()
    pathMapping = '/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/HTT/TrigHTTMaps/V1/'
    HTTMapping = CompFactory.TrigHTTMappingSvc()
    HTTMapping.mappingType = "FILE"
    HTTMapping.rmap = f'{pathMapping}map_file/rmaps/eta0103phi0305_ATLAS-P2-ITK-22-02-00.rmap'
    HTTMapping.subrmap = f'{pathMapping}zslicemaps/ATLAS-P2-ITK-22-02-00/eta0103phi0305_KeyLayer-strip_barrel_2_extra03_trim_0_001_NSlices-6.rmap'
    HTTMapping.pmap = f'{pathMapping}map_file/ATLAS-P2-ITK-22-02-00.pmap'
    HTTMapping.modulemap = f'{pathMapping}map_file/ITk.global-to-local.moduleidmap'
    HTTMapping.NNmap = f'{pathMapping}map_file/NN_DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.json'
    HTTMapping.layerOverride = {}
    HTTMapping.OutputLevel=2 
    result.addService(HTTMapping, create=True, primary=True)
    return result

def HTTBankSvcCfg():
    result=ComponentAccumulator()
    HTTBankSvc = CompFactory.TrigHTTBankSvc()
    pathBankSvc = '/eos/atlas/atlascerngroupdisk/det-htt/HTTsim/ATLAS-P2-ITK-22-02-00/21.9.16/eta0103phi0305/SectorBanks/'
    HTTBankSvc.constantsNoGuess_1st = [
        f'{pathBankSvc}corrgen_raw_8L_skipPlane0.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane1.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane2.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane3.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane4.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane5.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane6.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane7.gcon']
    HTTBankSvc.constantsNoGuess_2nd = [
        f'{pathBankSvc}corrgen_raw_13L_skipPlane0.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane1.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane2.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane3.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane4.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane5.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane6.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane7.gcon']
    HTTBankSvc.constants_1st = f'{pathBankSvc}corrgen_raw_8L.gcon'
    HTTBankSvc.constants_2nd = f'{pathBankSvc}corrgen_raw_13L_reg0_checkGood1.gcon'
    HTTBankSvc.sectorBank_1st = f'{pathBankSvc}sectorsHW_raw_8L.patt'
    HTTBankSvc.sectorBank_2nd = f'{pathBankSvc}sectorsHW_raw_13L_reg0_checkGood1.patt'
    HTTBankSvc.sectorSlices = f'{pathBankSvc}slices_8L.root'
    result.addService(HTTBankSvc, create=True, primary=True)
    return result


def HTTRoadUnionToolCfg(configFlags):
    result=ComponentAccumulator()
    RF = CompFactory.HTTRoadUnionTool()
    
    xBins = configFlags.Trigger.HTT.ActivePass.xBins
    xBufferBins = configFlags.Trigger.HTT.ActivePass.xBufferBins
    yBins = configFlags.Trigger.HTT.ActivePass.yBins
    yBufferBins = configFlags.Trigger.HTT.ActivePass.yBufferBins
    xMin = configFlags.Trigger.HTT.ActivePass.phiMin
    xMax = configFlags.Trigger.HTT.ActivePass.phiMax
    xBuffer = (xMax - xMin) / xBins * xBufferBins
    xMin = xMin - xBuffer
    xMax = xMax +  xBuffer
    yMin = configFlags.Trigger.HTT.ActivePass.qptMin
    yMax = configFlags.Trigger.HTT.ActivePass.qptMax
    yBuffer = (yMax - yMin) / yBins * yBufferBins
    yMin -= yBuffer
    yMax += yBuffer
    tools = []
    HTTMapping = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")

    for number in range(getNSubregions(HTTMapping.subrmap)): 
        HoughTransform = CompFactory.HTTHoughTransformTool("HoughTransform_0_" + str(number))
        HoughTransform.HTTEventSelectionSvc = HTTEventSelectionCfg().getService("HTTEventSelectionSvc")
        HoughTransform.TrigHTTBankSvc = HTTBankSvcCfg().getService("TrigHTTBankSvc")
        HoughTransform.TrigHTTMappingSvc = HTTMapping 
        HoughTransform.combine_layers = configFlags.Trigger.HTT.ActivePass.combineLayers 
        HoughTransform.convSize_x = configFlags.Trigger.HTT.ActivePass.convSizeX 
        HoughTransform.convSize_y = configFlags.Trigger.HTT.ActivePass.convSizeY 
        HoughTransform.convolution = configFlags.Trigger.HTT.ActivePass.convolution 
        HoughTransform.d0_max = 0 
        HoughTransform.d0_min = 0 
        HoughTransform.fieldCorrection = configFlags.Trigger.HTT.ActivePass.fieldCorrection
        HoughTransform.hitExtend_x = configFlags.Trigger.HTT.ActivePass.hitExtendX
        HoughTransform.localMaxWindowSize = configFlags.Trigger.HTT.ActivePass.localMaxWindowSize        
        HoughTransform.nBins_x = xBins + 2 * xBufferBins
        HoughTransform.nBins_y = yBins + 2 * yBufferBins
        HoughTransform.phi_max = xMax
        HoughTransform.phi_min = xMin
        HoughTransform.qpT_max = yMax 
        HoughTransform.qpT_min = yMin 
        HoughTransform.scale = configFlags.Trigger.HTT.ActivePass.scale
        HoughTransform.subRegion = number
        HoughTransform.threshold = configFlags.Trigger.HTT.ActivePass.threshold 
        HoughTransform.traceHits = False
        tools.append(HoughTransform)

    RF.tools = tools
    result.addPublicTool(RF, primary=True)
    return result


def HTTRawLogicCfg(configFlags):
    result=ComponentAccumulator()
    HTTRawLogic = CompFactory.HTTRawToLogicalHitsTool()
    HTTRawLogic.SaveOptional = 2
    if (configFlags.Trigger.HTT.ActivePass.sampleType == 'skipTruth'): 
        HTTRawLogic.SaveOptional = 1
    HTTRawLogic.TowersToMap = [0] # TODO TODO why is this hardcoded?
    HTTRawLogic.HTTEventSelectionSvc = HTTEventSelectionCfg().getService("HTTEventSelectionSvc")
    HTTRawLogic.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    result.addPublicTool(HTTRawLogic, primary=True)
    return result

def HTTDataFlowToolCfg():
    result=ComponentAccumulator()
    DataFlowTool = CompFactory.HTTDataFlowTool()
    DataFlowTool.HTTEventSelectionSvc = HTTEventSelectionCfg().getService("HTTEventSelectionSvc")
    DataFlowTool.TrigHTTMappingSvc =TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    DataFlowTool.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(DataFlowTool)
    return result

def HTTSpacePointsToolCfg(configFlags):
    result=ComponentAccumulator()
    SpacePointTool = CompFactory.HTTSpacePointsTool_v2()
    SpacePointTool.Filtering = configFlags.Trigger.HTT.ActivePass.spacePointFiltering
    SpacePointTool.FilteringClosePoints = False
    SpacePointTool.PhiWindow = 0.008
    SpacePointTool.Duplication = True
    result.addPublicTool(SpacePointTool, primary=True)
    return result


def HTTHitFilteringToolCfg():
    result=ComponentAccumulator()
    HitFilteringTool = CompFactory.HTTHitFilteringTool()
    HitFilteringTool.barrelStubDphiCut = 3.0
    HitFilteringTool.doRandomRemoval = False
    HitFilteringTool.doStubs = False
    HitFilteringTool.endcapStubDphiCut = 1.5
    HitFilteringTool.pixelClusRmFrac = 0
    HitFilteringTool.pixelHitRmFrac = 0
    HitFilteringTool.stripClusRmFrac = 0
    HitFilteringTool.stripHitRmFrac = 0
    HitFilteringTool.useNstrips = False
    result.addPublicTool(HitFilteringTool, primary=True)
    return result


def HTTHoughRootOutputToolCfg():
    result=ComponentAccumulator()
    HoughRootOutputTool = CompFactory.HTTHoughRootOutputTool()
    HoughRootOutputTool.HTTEventSelectionSvc = HTTEventSelectionCfg().getService("HTTEventSelectionSvc")
    HoughRootOutputTool.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    HoughRootOutputTool.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(HoughRootOutputTool)
    return result

def LRTRoadFinderCfg(configFlags):
    result=ComponentAccumulator()
    LRTRoadFinder =CompFactory.HTTHoughTransform_d0phi0_Tool()
    LRTRoadFinder.TrigHTTBankSvc = HTTBankSvcCfg().getService("TrigHTTBankSvc")
    LRTRoadFinder.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    LRTRoadFinder.combine_layers = configFlags.Trigger.HTT.ActivePass.lrtStraighttrackCombineLayers
    LRTRoadFinder.convolution = configFlags.Trigger.HTT.ActivePass.lrtStraighttrackConvolution
    LRTRoadFinder.hitExtend_x = configFlags.Trigger.HTT.ActivePass.lrtStraighttrackHitExtendX
    LRTRoadFinder.scale = configFlags.Trigger.HTT.ActivePass.scale
    LRTRoadFinder.threshold = configFlags.Trigger.HTT.ActivePass.lrtStraighttrackThreshold
    result.setPrivateTools(LRTRoadFinder)
    return result

def NNTrackToolCfg():
    result=ComponentAccumulator()
    NNTrackTool = CompFactory.HTTNNTrackTool()
    NNTrackTool.THistSvc = CompFactory.THistSvc()
    NNTrackTool.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    result.setPrivateTools(NNTrackTool)
    return result

def HTTWriteOutputCfg(configFlags):
    result=ComponentAccumulator()
    HTTWriteOutput = CompFactory.HTTOutputHeaderTool("HTTWriteOutput")
    HTTWriteOutput.InFileName = ["test"]
    HTTWriteOutput.RWstatus = "HEADER" # do not open file, use THistSvc
    HTTWriteOutput.RunSecondStage = configFlags.Trigger.HTT.ActivePass.secondStage
    result.addPublicTool(HTTWriteOutput, primary=True)
    return result

def HTTTrackFitterToolCfg(configFlags):
    result=ComponentAccumulator()
    TF_1st = CompFactory.HTTTrackFitterTool("HTTTrackFitterTool_1st")
    TF_1st.GuessHits = configFlags.Trigger.HTT.ActivePass.guessHits
    TF_1st.IdealCoordFitType = configFlags.Trigger.HTT.ActivePass.idealCoordFitType
    TF_1st.TrigHTTBankSvc = HTTBankSvcCfg().getService("TrigHTTBankSvc")
    TF_1st.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    TF_1st.chi2DofRecoveryMax = configFlags.Trigger.HTT.ActivePass.chi2DoFRecoveryMax
    TF_1st.chi2DofRecoveryMin = configFlags.Trigger.HTT.ActivePass.chi2DoFRecoveryMin
    TF_1st.doMajority = configFlags.Trigger.HTT.ActivePass.doMajority
    TF_1st.nHits_noRecovery = configFlags.Trigger.HTT.ActivePass.nHitsNoRecovery
    TF_1st.DoDeltaGPhis = configFlags.Trigger.HTT.ActivePass.doDeltaGPhis
    TF_1st.DoMissingHitsChecks = configFlags.Trigger.HTT.ActivePass.doMissingHitsChecks
    result.addPublicTool(TF_1st, primary=True)
    return result

def HTTOverlapRemovalToolCfg(configFlags):
    result=ComponentAccumulator()
    OR_1st = CompFactory.HTTOverlapRemovalTool("HTTOverlapRemovalTool_1st")
    OR_1st.ORAlgo = "Normal"
    OR_1st.doFastOR =configFlags.Trigger.HTT.ActivePass.doFastOR
    OR_1st.NumOfHitPerGrouping = 5
    OR_1st.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    if configFlags.Trigger.HTT.ActivePass.hough:
        OR_1st.nBins_x = configFlags.Trigger.HTT.ActivePass.xBins + 2 * configFlags.Trigger.HTT.ActivePass.xBufferBins
        OR_1st.nBins_y = configFlags.Trigger.HTT.ActivePass.yBins + 2 * configFlags.Trigger.HTT.ActivePass.yBufferBins
        OR_1st.localMaxWindowSize = configFlags.Trigger.HTT.ActivePass.localMaxWindowSize
        OR_1st.roadSliceOR = configFlags.Trigger.HTT.ActivePass.roadSliceOR
    
    result.addPublicTool(OR_1st, primary=True)
    return result


def HTTOverlapRemovalTool_2ndCfg(configFlags):
    result=ComponentAccumulator()
    OR_2nd = CompFactory.HTTOverlapRemovalTool("HTTOverlapRemovalTool_2nd")
    OR_2nd.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    if configFlags.Trigger.HTT.ActivePass.secondStage:
        OR_2nd.DoSecondStage = True
        OR_2nd.ORAlgo = "Normal"
        OR_2nd.doFastOR = configFlags.Trigger.HTT.ActivePass.doFastOR
        OR_2nd.NumOfHitPerGrouping = 5
    result.setPrivateTools(OR_2nd)
    return result


def HTTTrackFitterTool_2ndCfg(configFlags):
    result=ComponentAccumulator()
    TF_2nd = CompFactory.HTTTrackFitterTool("HTTTrackFitterTool_2nd")
    TF_2nd.TrigHTTBankSvc = HTTBankSvcCfg().getService("TrigHTTBankSvc")
    TF_2nd.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    if configFlags.Trigger.HTT.ActivePass.secondStage:
        TF_2nd.Do2ndStageTrackFit = True 
    result.setPrivateTools(TF_2nd)
    return result


def checkIfAlgoTagExist(flags, tag):
    try:
        getattr(flags, tag)
    except RuntimeError:
        raise Exception(f'{tag} does not appear to be flag category')


    

def HTTLogicalHistProcessAlgCfg(configFlags):
   
    result=ComponentAccumulator()

   
    theHTTLogicalHistProcessAlg=CompFactory.HTTLogicalHitsProcessAlg()
    theHTTLogicalHistProcessAlg.HitFiltering = configFlags.Trigger.HTT.ActivePass.hitFiltering
    theHTTLogicalHistProcessAlg.writeOutputData = configFlags.Trigger.HTT.ActivePass.writeOutputData
    theHTTLogicalHistProcessAlg.Clustering = True
    theHTTLogicalHistProcessAlg.tracking = configFlags.Trigger.HTT.ActivePass.doTracking
    theHTTLogicalHistProcessAlg.outputHitTxt = configFlags.Trigger.HTT.ActivePass.outputHitTxt
    theHTTLogicalHistProcessAlg.RunSecondStage = configFlags.Trigger.HTT.ActivePass.secondStage
    theHTTLogicalHistProcessAlg.DoMissingHitsChecks = configFlags.Trigger.HTT.ActivePass.doMissingHitsChecks
    theHTTLogicalHistProcessAlg.DoHoughRootOutput = False
    theHTTLogicalHistProcessAlg.DoNNTrack = False
    theHTTLogicalHistProcessAlg.eventSelector = result.getPrimaryAndMerge(HTTEventSelectionCfg())

    HTTMaping = result.getPrimaryAndMerge(TrigHTTMappingCfg())
    theHTTLogicalHistProcessAlg.HTTMapping = HTTMaping

    result.getPrimaryAndMerge(HTTBankSvcCfg())

    theHTTLogicalHistProcessAlg.RoadFinder = result.getPrimaryAndMerge(HTTRoadUnionToolCfg(configFlags))
    theHTTLogicalHistProcessAlg.RawToLogicalHitsTool = result.getPrimaryAndMerge(HTTRawLogicCfg(configFlags))

    InputTool = CompFactory.HTTInputHeaderTool("HTTReadInput")
    InputTool.InFileName = configFlags.Input.Files
    result.addPublicTool(InputTool)
    theHTTLogicalHistProcessAlg.InputTool = InputTool

    InputTool2 = CompFactory.HTTReadRawRandomHitsTool("HTTReadRawRandomHitsTool")
    InputTool2.InFileName = configFlags.Input.Files[0]
    result.addPublicTool(InputTool2)
    theHTTLogicalHistProcessAlg.InputTool2 = InputTool2

    theHTTLogicalHistProcessAlg.DataFlowTool = result.getPrimaryAndMerge(HTTDataFlowToolCfg())
    theHTTLogicalHistProcessAlg.SpacePointTool = result.getPrimaryAndMerge(HTTSpacePointsToolCfg(configFlags))

    RoadFilter = CompFactory.HTTEtaPatternFilterTool()
    RoadFilter.TrigHTTMappingSvc = HTTMaping
    theHTTLogicalHistProcessAlg.RoadFilter = RoadFilter

    theHTTLogicalHistProcessAlg.HitFilteringTool = result.getPrimaryAndMerge(HTTHitFilteringToolCfg())
    theHTTLogicalHistProcessAlg.HoughRootOutputTool = result.getPrimaryAndMerge(HTTHoughRootOutputToolCfg())

    LRTRoadFilter = CompFactory.HTTLLPRoadFilterTool()
    result.addPublicTool(LRTRoadFilter)
    theHTTLogicalHistProcessAlg.LRTRoadFilter = LRTRoadFilter

    theHTTLogicalHistProcessAlg.LRTRoadFinder = result.getPrimaryAndMerge(LRTRoadFinderCfg(configFlags))
    theHTTLogicalHistProcessAlg.NNTrackTool = result.getPrimaryAndMerge(NNTrackToolCfg())

    RoadFilter2 = CompFactory.HTTPhiRoadFilterTool()
    RoadFilter2.TrigHTTMappingSvc = HTTMaping
    RoadFilter2.window = []
    theHTTLogicalHistProcessAlg.RoadFilter2 = RoadFilter2

    theHTTLogicalHistProcessAlg.ClusteringTool = CompFactory.HTTClusteringTool()
    theHTTLogicalHistProcessAlg.OutputTool = result.getPrimaryAndMerge(HTTWriteOutputCfg(configFlags))
    theHTTLogicalHistProcessAlg.TrackFitter_1st = result.getPrimaryAndMerge(HTTTrackFitterToolCfg(configFlags))
    theHTTLogicalHistProcessAlg.OverlapRemoval_1st = result.getPrimaryAndMerge(HTTOverlapRemovalToolCfg(configFlags))
    theHTTLogicalHistProcessAlg.OverlapRemoval_2nd = result.getPrimaryAndMerge(HTTOverlapRemovalTool_2ndCfg(configFlags))
    theHTTLogicalHistProcessAlg.TrackFitter_2nd = result.getPrimaryAndMerge(HTTTrackFitterTool_2ndCfg(configFlags))


    if configFlags.Trigger.HTT.ActivePass.secondStage:
        HTTExtrapolatorTool = CompFactory.HTTExtrapolator()
        HTTExtrapolatorTool.Ncombinations = 16
        theHTTLogicalHistProcessAlg.Extrapolator = HTTExtrapolatorTool


    if configFlags.Trigger.HTT.ActivePass.lrt:
        assert configFlags.Trigger.HTT.ActivePass.lrtUseBasicHitFilter != configFlags.Trigger.HTT.ActivePass.lrtUseMlHitFilter, 'Inconsistent LRT hit filtering setup, need either ML of Basic filtering enabled'
        assert configFlags.Trigger.HTT.ActivePass.lrtUseStraightTrackHT != configFlags.Trigger.HTT.ActivePass.lrtUseDoubletHT, 'Inconsistent LRT HT setup, need either double or strightTrack enabled'
        theHTTLogicalHistProcessAlg.doLRT = True
        theHTTLogicalHistProcessAlg.LRTHitFiltering = (not configFlags.Trigger.HTT.ActivePass.lrtSkipHitFiltering)

    
    result.addEventAlgo(theHTTLogicalHistProcessAlg)
    return result


def prepareFlagsForHTTLogicalHistProcessAlg(flags):
    flags.Trigger.HTT.algoTag="Hough"
    checkIfAlgoTagExist(flags.Trigger.HTT, flags.Trigger.HTT.algoTag)
    newFlags = flags.cloneAndReplace("Trigger.HTT.ActivePass", "Trigger.HTT." + flags.Trigger.HTT.algoTag)
    return newFlags


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    ConfigFlags.Input.Files = ['/ATLAS/tbold/DATA/HTTWrapper.singlemu_Pt10.root']

    newFlags = prepareFlagsForHTTLogicalHistProcessAlg(ConfigFlags)

    acc=MainServicesCfg(newFlags)
    acc.merge(HTTLogicalHistProcessAlgCfg(newFlags)) 
    acc.store(open('AnalysisConfig.pkl','wb'))

