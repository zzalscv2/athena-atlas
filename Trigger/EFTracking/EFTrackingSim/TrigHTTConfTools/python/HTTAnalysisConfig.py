# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

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


def HTTRoadUnionToolCfg(flags):
    result=ComponentAccumulator()
    RF = CompFactory.HTTRoadUnionTool()
    
    xBins = flags.Trigger.HTT.ActiveConfig.xBins
    xBufferBins = flags.Trigger.HTT.ActiveConfig.xBufferBins
    yBins = flags.Trigger.HTT.ActiveConfig.yBins
    yBufferBins = flags.Trigger.HTT.ActiveConfig.yBufferBins
    xMin = flags.Trigger.HTT.ActiveConfig.phiMin
    xMax = flags.Trigger.HTT.ActiveConfig.phiMax
    xBuffer = (xMax - xMin) / xBins * xBufferBins
    xMin = xMin - xBuffer
    xMax = xMax +  xBuffer
    yMin = flags.Trigger.HTT.ActiveConfig.qptMin
    yMax = flags.Trigger.HTT.ActiveConfig.qptMax
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
        HoughTransform.combine_layers = flags.Trigger.HTT.ActiveConfig.combineLayers 
        HoughTransform.convSize_x = flags.Trigger.HTT.ActiveConfig.convSizeX 
        HoughTransform.convSize_y = flags.Trigger.HTT.ActiveConfig.convSizeY 
        HoughTransform.convolution = flags.Trigger.HTT.ActiveConfig.convolution 
        HoughTransform.d0_max = 0 
        HoughTransform.d0_min = 0 
        HoughTransform.fieldCorrection = flags.Trigger.HTT.ActiveConfig.fieldCorrection
        HoughTransform.hitExtend_x = flags.Trigger.HTT.ActiveConfig.hitExtendX
        HoughTransform.localMaxWindowSize = flags.Trigger.HTT.ActiveConfig.localMaxWindowSize        
        HoughTransform.nBins_x = xBins + 2 * xBufferBins
        HoughTransform.nBins_y = yBins + 2 * yBufferBins
        HoughTransform.phi_max = xMax
        HoughTransform.phi_min = xMin
        HoughTransform.qpT_max = yMax 
        HoughTransform.qpT_min = yMin 
        HoughTransform.scale = flags.Trigger.HTT.ActiveConfig.scale
        HoughTransform.subRegion = number
        HoughTransform.threshold = flags.Trigger.HTT.ActiveConfig.threshold 
        HoughTransform.traceHits = False
        tools.append(HoughTransform)

    RF.tools = tools
    result.addPublicTool(RF, primary=True)
    return result


def HTTRawLogicCfg(flags):
    result=ComponentAccumulator()
    HTTRawLogic = CompFactory.HTTRawToLogicalHitsTool()
    HTTRawLogic.SaveOptional = 2
    if (flags.Trigger.HTT.ActiveConfig.sampleType == 'skipTruth'): 
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

def HTTSpacePointsToolCfg(flags):
    result=ComponentAccumulator()
    SpacePointTool = CompFactory.HTTSpacePointsTool_v2()
    SpacePointTool.Filtering = flags.Trigger.HTT.ActiveConfig.spacePointFiltering
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

def LRTRoadFinderCfg(flags):
    result=ComponentAccumulator()
    LRTRoadFinder =CompFactory.HTTHoughTransform_d0phi0_Tool()
    LRTRoadFinder.TrigHTTBankSvc = HTTBankSvcCfg().getService("TrigHTTBankSvc")
    LRTRoadFinder.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    LRTRoadFinder.combine_layers = flags.Trigger.HTT.ActiveConfig.lrtStraighttrackCombineLayers
    LRTRoadFinder.convolution = flags.Trigger.HTT.ActiveConfig.lrtStraighttrackConvolution
    LRTRoadFinder.hitExtend_x = flags.Trigger.HTT.ActiveConfig.lrtStraighttrackHitExtendX
    LRTRoadFinder.scale = flags.Trigger.HTT.ActiveConfig.scale
    LRTRoadFinder.threshold = flags.Trigger.HTT.ActiveConfig.lrtStraighttrackThreshold
    result.setPrivateTools(LRTRoadFinder)
    return result

def NNTrackToolCfg():
    result=ComponentAccumulator()
    NNTrackTool = CompFactory.HTTNNTrackTool()
    NNTrackTool.THistSvc = CompFactory.THistSvc()
    NNTrackTool.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    result.setPrivateTools(NNTrackTool)
    return result

def HTTWriteOutputCfg(flags):
    result=ComponentAccumulator()
    HTTWriteOutput = CompFactory.HTTOutputHeaderTool("HTTWriteOutput")
    HTTWriteOutput.InFileName = ["test"]
    HTTWriteOutput.RWstatus = "HEADER" # do not open file, use THistSvc
    HTTWriteOutput.RunSecondStage = flags.Trigger.HTT.ActiveConfig.secondStage
    result.addPublicTool(HTTWriteOutput, primary=True)
    return result

def HTTTrackFitterToolCfg(flags):
    result=ComponentAccumulator()
    TF_1st = CompFactory.HTTTrackFitterTool("HTTTrackFitterTool_1st")
    TF_1st.GuessHits = flags.Trigger.HTT.ActiveConfig.guessHits
    TF_1st.IdealCoordFitType = flags.Trigger.HTT.ActiveConfig.idealCoordFitType
    TF_1st.TrigHTTBankSvc = HTTBankSvcCfg().getService("TrigHTTBankSvc")
    TF_1st.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    TF_1st.chi2DofRecoveryMax = flags.Trigger.HTT.ActiveConfig.chi2DoFRecoveryMax
    TF_1st.chi2DofRecoveryMin = flags.Trigger.HTT.ActiveConfig.chi2DoFRecoveryMin
    TF_1st.doMajority = flags.Trigger.HTT.ActiveConfig.doMajority
    TF_1st.nHits_noRecovery = flags.Trigger.HTT.ActiveConfig.nHitsNoRecovery
    TF_1st.DoDeltaGPhis = flags.Trigger.HTT.ActiveConfig.doDeltaGPhis
    TF_1st.DoMissingHitsChecks = flags.Trigger.HTT.ActiveConfig.doMissingHitsChecks
    result.addPublicTool(TF_1st, primary=True)
    return result

def HTTOverlapRemovalToolCfg(flags):
    result=ComponentAccumulator()
    OR_1st = CompFactory.HTTOverlapRemovalTool("HTTOverlapRemovalTool_1st")
    OR_1st.ORAlgo = "Normal"
    OR_1st.doFastOR =flags.Trigger.HTT.ActiveConfig.doFastOR
    OR_1st.NumOfHitPerGrouping = 5
    OR_1st.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    if flags.Trigger.HTT.ActiveConfig.hough:
        OR_1st.nBins_x = flags.Trigger.HTT.ActiveConfig.xBins + 2 * flags.Trigger.HTT.ActiveConfig.xBufferBins
        OR_1st.nBins_y = flags.Trigger.HTT.ActiveConfig.yBins + 2 * flags.Trigger.HTT.ActiveConfig.yBufferBins
        OR_1st.localMaxWindowSize = flags.Trigger.HTT.ActiveConfig.localMaxWindowSize
        OR_1st.roadSliceOR = flags.Trigger.HTT.ActiveConfig.roadSliceOR
    
    result.addPublicTool(OR_1st, primary=True)
    return result


def HTTOverlapRemovalTool_2ndCfg(flags):
    result=ComponentAccumulator()
    OR_2nd = CompFactory.HTTOverlapRemovalTool("HTTOverlapRemovalTool_2nd")
    OR_2nd.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    if flags.Trigger.HTT.ActiveConfig.secondStage:
        OR_2nd.DoSecondStage = True
        OR_2nd.ORAlgo = "Normal"
        OR_2nd.doFastOR = flags.Trigger.HTT.ActiveConfig.doFastOR
        OR_2nd.NumOfHitPerGrouping = 5
    result.setPrivateTools(OR_2nd)
    return result


def HTTTrackFitterTool_2ndCfg(flags):
    result=ComponentAccumulator()
    TF_2nd = CompFactory.HTTTrackFitterTool("HTTTrackFitterTool_2nd")
    TF_2nd.TrigHTTBankSvc = HTTBankSvcCfg().getService("TrigHTTBankSvc")
    TF_2nd.TrigHTTMappingSvc = TrigHTTMappingCfg().getService("TrigHTTMappingSvc")
    if flags.Trigger.HTT.ActiveConfig.secondStage:
        TF_2nd.Do2ndStageTrackFit = True 
    result.setPrivateTools(TF_2nd)
    return result


def checkIfAlgoTagExist(flags, tag):
    try:
        getattr(flags, tag)
    except RuntimeError:
        raise Exception(f'{tag} does not appear to be flag category')

def HTTReadInputCfg(flags):
    result=ComponentAccumulator()
    InputTool = CompFactory.HTTInputHeaderTool("HTTReadInput",
                                               InFileName = flags.Input.Files)
    result.addPublicTool(InputTool, primary=True)
    return result


def HTTLogicalHistProcessAlgCfg(flags):
   
    result=ComponentAccumulator()

   
    theHTTLogicalHistProcessAlg=CompFactory.HTTLogicalHitsProcessAlg()
    theHTTLogicalHistProcessAlg.HitFiltering = flags.Trigger.HTT.ActiveConfig.hitFiltering
    theHTTLogicalHistProcessAlg.writeOutputData = flags.Trigger.HTT.ActiveConfig.writeOutputData
    theHTTLogicalHistProcessAlg.Clustering = True
    theHTTLogicalHistProcessAlg.tracking = flags.Trigger.HTT.ActiveConfig.doTracking
    theHTTLogicalHistProcessAlg.outputHitTxt = flags.Trigger.HTT.ActiveConfig.outputHitTxt
    theHTTLogicalHistProcessAlg.RunSecondStage = flags.Trigger.HTT.ActiveConfig.secondStage
    theHTTLogicalHistProcessAlg.DoMissingHitsChecks = flags.Trigger.HTT.ActiveConfig.doMissingHitsChecks
    theHTTLogicalHistProcessAlg.DoHoughRootOutput = False
    theHTTLogicalHistProcessAlg.DoNNTrack = False
    theHTTLogicalHistProcessAlg.eventSelector = result.getPrimaryAndMerge(HTTEventSelectionCfg())

    HTTMaping = result.getPrimaryAndMerge(TrigHTTMappingCfg())
    theHTTLogicalHistProcessAlg.HTTMapping = HTTMaping

    result.getPrimaryAndMerge(HTTBankSvcCfg())

    theHTTLogicalHistProcessAlg.RoadFinder = result.getPrimaryAndMerge(HTTRoadUnionToolCfg(flags))
    theHTTLogicalHistProcessAlg.RawToLogicalHitsTool = result.getPrimaryAndMerge(HTTRawLogicCfg(flags))

    theHTTLogicalHistProcessAlg.InputTool = result.getPrimaryAndMerge(HTTReadInputCfg(flags))

    InputTool2 = CompFactory.HTTReadRawRandomHitsTool("HTTReadRawRandomHitsTool")
    InputTool2.InFileName = flags.Input.Files[0]
    result.addPublicTool(InputTool2)
    theHTTLogicalHistProcessAlg.InputTool2 = InputTool2

    theHTTLogicalHistProcessAlg.DataFlowTool = result.getPrimaryAndMerge(HTTDataFlowToolCfg())
    theHTTLogicalHistProcessAlg.SpacePointTool = result.getPrimaryAndMerge(HTTSpacePointsToolCfg(flags))

    RoadFilter = CompFactory.HTTEtaPatternFilterTool()
    RoadFilter.TrigHTTMappingSvc = HTTMaping
    theHTTLogicalHistProcessAlg.RoadFilter = RoadFilter

    theHTTLogicalHistProcessAlg.HitFilteringTool = result.getPrimaryAndMerge(HTTHitFilteringToolCfg())
    theHTTLogicalHistProcessAlg.HoughRootOutputTool = result.getPrimaryAndMerge(HTTHoughRootOutputToolCfg())

    LRTRoadFilter = CompFactory.HTTLLPRoadFilterTool()
    result.addPublicTool(LRTRoadFilter)
    theHTTLogicalHistProcessAlg.LRTRoadFilter = LRTRoadFilter

    theHTTLogicalHistProcessAlg.LRTRoadFinder = result.getPrimaryAndMerge(LRTRoadFinderCfg(flags))
    theHTTLogicalHistProcessAlg.NNTrackTool = result.getPrimaryAndMerge(NNTrackToolCfg())

    RoadFilter2 = CompFactory.HTTPhiRoadFilterTool()
    RoadFilter2.TrigHTTMappingSvc = HTTMaping
    RoadFilter2.window = []
    theHTTLogicalHistProcessAlg.RoadFilter2 = RoadFilter2

    theHTTLogicalHistProcessAlg.ClusteringTool = CompFactory.HTTClusteringTool()
    theHTTLogicalHistProcessAlg.OutputTool = result.getPrimaryAndMerge(HTTWriteOutputCfg(flags))
    theHTTLogicalHistProcessAlg.TrackFitter_1st = result.getPrimaryAndMerge(HTTTrackFitterToolCfg(flags))
    theHTTLogicalHistProcessAlg.OverlapRemoval_1st = result.getPrimaryAndMerge(HTTOverlapRemovalToolCfg(flags))
    theHTTLogicalHistProcessAlg.OverlapRemoval_2nd = result.getPrimaryAndMerge(HTTOverlapRemovalTool_2ndCfg(flags))
    theHTTLogicalHistProcessAlg.TrackFitter_2nd = result.getPrimaryAndMerge(HTTTrackFitterTool_2ndCfg(flags))


    if flags.Trigger.HTT.ActiveConfig.secondStage:
        HTTExtrapolatorTool = CompFactory.HTTExtrapolator()
        HTTExtrapolatorTool.Ncombinations = 16
        theHTTLogicalHistProcessAlg.Extrapolator = HTTExtrapolatorTool


    if flags.Trigger.HTT.ActiveConfig.lrt:
        assert flags.Trigger.HTT.ActiveConfig.lrtUseBasicHitFilter != flags.Trigger.HTT.ActiveConfig.lrtUseMlHitFilter, 'Inconsistent LRT hit filtering setup, need either ML of Basic filtering enabled'
        assert flags.Trigger.HTT.ActiveConfig.lrtUseStraightTrackHT != flags.Trigger.HTT.ActiveConfig.lrtUseDoubletHT, 'Inconsistent LRT HT setup, need either double or strightTrack enabled'
        theHTTLogicalHistProcessAlg.doLRT = True
        theHTTLogicalHistProcessAlg.LRTHitFiltering = (not flags.Trigger.HTT.ActiveConfig.lrtSkipHitFiltering)

    from TrigHTTAlgorithms.HTTAlgorithmConfig import HTTLogicalHitsProcessAlgMonitoringCfg
    theHTTLogicalHistProcessAlg.MonTool = result.getPrimaryAndMerge(HTTLogicalHitsProcessAlgMonitoringCfg(flags))

    result.addEventAlgo(theHTTLogicalHistProcessAlg)
    return result


def prepareFlagsForHTTLogicalHistProcessAlg(flags):
    flags.Trigger.HTT.algoTag="Hough"
    checkIfAlgoTagExist(flags.Trigger.HTT, flags.Trigger.HTT.algoTag)
    newFlags = flags.cloneAndReplace("Trigger.HTT.ActiveConfig", "Trigger.HTT." + flags.Trigger.HTT.algoTag)
    return newFlags


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.Input.Files = ['HTTWrapper.singlemu_Pt10.root']

    newFlags = prepareFlagsForHTTLogicalHistProcessAlg(flags)
    del flags

    acc=MainServicesCfg(newFlags)
    acc.addService(CompFactory.THistSvc(Output = ["EXPERT DATAFILE='monitoring.root', OPT='RECREATE'"]))
    acc.merge(HTTLogicalHistProcessAlgCfg(newFlags)) 
    acc.store(open('AnalysisConfig.pkl','wb'))
    
    statusCode = acc.run()
    assert statusCode.isSuccess() is True, "Application execution did not succeed"

