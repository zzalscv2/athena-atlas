# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HTTAnalysisConfig():
    print('HTTAnalysisConfig')


def HTTLogicalHistProcessAlgCfg(configFlags):
   
    result=ComponentAccumulator()

    theHTTLogicalHistProcessAlg=CompFactory.HTTLogicalHitsProcessAlg()

    theHTTLogicalHistProcessAlg.HitFiltering = configFlags.Trigger.HTT.hitFiltering
    theHTTLogicalHistProcessAlg.writeOutputData = configFlags.Trigger.HTT.writeOutputData
    theHTTLogicalHistProcessAlg.Clustering = True
    theHTTLogicalHistProcessAlg.tracking = configFlags.Trigger.HTT.doTracking
    theHTTLogicalHistProcessAlg.outputHitTxt = configFlags.Trigger.HTT.outputHitTxt
    theHTTLogicalHistProcessAlg.RunSecondStage = configFlags.Trigger.HTT.secondStage
    theHTTLogicalHistProcessAlg.DoMissingHitsChecks = configFlags.Trigger.HTT.doMissingHitsChecks

    evSelectionSvc = CompFactory.HTTEventSelectionSvc()
    evSelectionSvc.OutputLevel=2
    evSelectionSvc.regions = "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/HTT/TrigHTTMaps/V1/map_file/slices_v01_Jan21.txt"
    result.addService(evSelectionSvc)

    HTTMapping = CompFactory.TrigHTTMappingSvc()
    HTTMapping.mappingType = "FILE"
    HTTMapping.rmap = '/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/HTT/TrigHTTMaps/V1/map_file/rmaps/eta0103phi0305_ATLAS-P2-ITK-22-02-00.rmap'
    HTTMapping.subrmap = '/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/HTT/TrigHTTMaps/V1/zslicemaps/ATLAS-P2-ITK-22-02-00/eta0103phi0305_KeyLayer-strip_barrel_2_extra03_trim_0_001_NSlices-6.rmap'
    HTTMapping.pmap = '/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/HTT/TrigHTTMaps/V1/map_file/ATLAS-P2-ITK-22-02-00.pmap'
    HTTMapping.modulemap = '/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/HTT/TrigHTTMaps/V1/map_file/ITk.global-to-local.moduleidmap'
    HTTMapping.NNmap = '/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/HTT/TrigHTTMaps/V1/map_file/NN_DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.json'
    HTTMapping.layerOverride = {}
    HTTMapping.OutputLevel=2 

    result.addService(HTTMapping)
    theHTTLogicalHistProcessAlg.HTTMapping = HTTMapping

    RF = CompFactory.HTTHoughTransform_d0phi0_Tool()
    theHTTLogicalHistProcessAlg.RoadFinder = RF



    HTTRawLogic = CompFactory.HTTRawToLogicalHitsTool()
    HTTRawLogic.SaveOptional = 2
    if (configFlags.Trigger.HTT.sampleType == 'skipTruth'): 
        HTTRawLogic.SaveOptional = 1
    HTTRawLogic.TowersToMap = [0] # TODO TODO why is this hardcoded?
    theHTTLogicalHistProcessAlg.RawToLogicalHitsTool = HTTRawLogic

    InputTool = CompFactory.HTTInputHeaderTool("HTTReadInput")
    InputTool.InFileName = configFlags.Input.Files
    theHTTLogicalHistProcessAlg.InputTool = InputTool

    InputTool2 = CompFactory.HTTReadRawRandomHitsTool("HTTReadRawRandomHitsTool")
    InputTool2.InFileName = configFlags.Input.Files[0]
    theHTTLogicalHistProcessAlg.InputTool2 = InputTool2

    HTTWriteOutput = CompFactory.HTTOutputHeaderTool("HTTWriteOutput")
    HTTWriteOutput.InFileName = ["test"]
    HTTWriteOutput.RWstatus = "HEADER" # do not open file, use THistSvc
    HTTWriteOutput.RunSecondStage = configFlags.Trigger.HTT.secondStage
    theHTTLogicalHistProcessAlg.OutputTool= HTTWriteOutput

    TF_1st= CompFactory.HTTTrackFitterTool("HTTTrackFitterTool_1st")
    theHTTLogicalHistProcessAlg.TrackFitter_1st = TF_1st

    OR_1st = CompFactory.HTTOverlapRemovalTool("HTTOverlapRemovalTool_1st")
    OR_1st.ORAlgo = "Normal"
    OR_1st.doFastOR =configFlags.Trigger.HTT.doFastOR
    OR_1st.NumOfHitPerGrouping = 5

    if configFlags.Trigger.HTT.hough:
        OR_1st.nBins_x = configFlags.Trigger.HTT.xBins + 2 * configFlags.Trigger.HTT.xBufferBins
        OR_1st.nBins_y = configFlags.Trigger.HTT.yBins + 2 * configFlags.Trigger.HTT.yBufferBins
        OR_1st.localMaxWindowSize = configFlags.Trigger.HTT.localMaxWindowSize
        OR_1st.roadSliceOR = configFlags.Trigger.HTT.roadSliceOR

    if configFlags.Trigger.HTT.secondStage:
        TF_2nd = CompFactory.HTTTrackFitterTool("HTTTrackFitterTool_2nd")
        TF_2nd.Do2ndStageTrackFit = True
        theHTTLogicalHistProcessAlg.TrackFitter_2nd = TF_2nd

        OR_2nd = CompFactory.HTTOverlapRemovalTool("HTTOverlapRemovalTool_2nd")
        OR_2nd.DoSecondStage = True
        OR_2nd.ORAlgo = "Normal"
        OR_2nd.doFastOR = configFlags.Trigger.HTT.doFastOR
        OR_2nd.NumOfHitPerGrouping = 5
        theHTTLogicalHistProcessAlg.OverlapRemoval_2nd = OR_2nd

        HTTExtrapolatorTool = CompFactory.HTTExtrapolator()
        HTTExtrapolatorTool.Ncombinations = 16
        theHTTLogicalHistProcessAlg.Extrapolator = HTTExtrapolatorTool

    thellpfilter = CompFactory.HTTLLPRoadFilterTool()

    if configFlags.Trigger.HTT.lrt:

        assert configFlags.Trigger.HTT.lrtUseBasicHitFilter != configFlags.Trigger.HTT.lrtUseMlHitFilter, 'Inconsistent LRT hit filtering setup, need either ML of Basic filtering enabled'
        assert configFlags.Trigger.HTT.lrtUseStraightTrackHT != configFlags.Trigger.HTT.lrtUseDoubletHT, 'Inconsistent LRT HT setup, need either double or strightTrack enabled'

        theHTTLogicalHistProcessAlg.doLRT = True
        theHTTLogicalHistProcessAlg.LRTHitFiltering = (not configFlags.Trigger.HTT.lrtSkipHitFiltering)

        if configFlags.Trigger.HTT.lrtUseBasicHitFilter:
            theHTTLogicalHistProcessAlg.LRTRoadFilter = thellpfilter
    
    theHTTLogicalHistProcessAlg.OverlapRemoval_1st = OR_1st
    result.addEventAlgo(theHTTLogicalHistProcessAlg)

    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    ConfigFlags.Input.Files = ['/ATLAS/tbold/DATA/HTTWrapper.singlemu_Pt10.root']

    acc=MainServicesCfg(ConfigFlags)
    acc.merge(HTTLogicalHistProcessAlgCfg(ConfigFlags)) 
    acc.run(10)

