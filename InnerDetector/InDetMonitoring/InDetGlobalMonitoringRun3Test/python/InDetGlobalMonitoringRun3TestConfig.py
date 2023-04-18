#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

####################################################
#                                                  #
# InDetGlobalManager top algorithm                 #
#                                                  #
####################################################



def InDetGlobalMonitoringRun3TestConfig(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, "InDetGlobalMonitoringRun3Test")
        
    from AthenaConfiguration.ComponentFactory import CompFactory

    from AthenaMonitoring.FilledBunchFilterToolConfig import FilledBunchFilterToolCfg
    from AthenaMonitoring.AtlasReadyFilterConfig import AtlasReadyFilterCfg

    # run on RAW only
    if flags.DQ.Environment in ('online', 'tier0', 'tier0Raw'):
        
        ########### here begins InDetGlobalTrackMonAlg ###########
        from InDetGlobalMonitoringRun3Test.InDetGlobalTrackMonAlgCfg import InDetGlobalTrackMonAlgCfg 

        
        inDetGlobalTrackMonAlg = helper.addAlgorithm(CompFactory.InDetGlobalTrackMonAlg, 'InDetGlobalTrackMonAlg',addFilterTools = [FilledBunchFilterToolCfg(flags), AtlasReadyFilterCfg(flags)])

        from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg
        from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_Loose_Cfg
        TrackSelectionTool = acc.popToolsAndMerge(
            InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg(flags, name='TrackSelectionTool',
                                                                maxNPixelHoles = 1, # Default for TightPrimary is 0
                                                                minPt = 5000))
        Tight_TrackSelectionTool = acc.popToolsAndMerge(
            InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg(flags, name='TightTrackSelectionTool',
                                                                minPt = 5000))

        Loose_TrackSelectionTool = acc.popToolsAndMerge(
            InDetTrackSelectionTool_Loose_Cfg(flags, name='LooseTrackSelectionTool',
                                                                minPt = 1000))

        #special track selection for low energy and cosmics so plots are not empty
        LowECM_TrackSelectionTool = acc.popToolsAndMerge(
            InDetTrackSelectionTool_Loose_Cfg(flags, name='LowECMTrackSelectionTool',
                                                                minPt = 500))

        from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
        TrackToVertexIPEstimator = acc.popToolsAndMerge(
            TrackToVertexIPEstimatorCfg(flags, name='TrackToVertexIPEstimator'))
    
        from AthenaConfiguration.Enums import BeamType
        ### Change base selection to Loose and 500MeV for cosmics and <=900GeV collisions -- this should not change in Global Monitoring at P1 however
        ### as the job properties normally retrieved from metadata should be configured to defaults when reading direct from DCM 
        if flags.Beam.Type is BeamType.Cosmics or float(flags.Beam.Energy) < 500000:
            inDetGlobalTrackMonAlg.TrackSelectionTool = LowECM_TrackSelectionTool
        else:
            inDetGlobalTrackMonAlg.TrackSelectionTool = TrackSelectionTool

        inDetGlobalTrackMonAlg.Tight_TrackSelectionTool = Tight_TrackSelectionTool
        inDetGlobalTrackMonAlg.Loose_TrackSelectionTool = Loose_TrackSelectionTool
        inDetGlobalTrackMonAlg.TrackToVertexIPEstimator = TrackToVertexIPEstimator 

        InDetGlobalTrackMonAlgCfg(helper, inDetGlobalTrackMonAlg)
        ########### here ends InDetGlobalTrackMonAlg ###########


    if flags.DQ.Environment in ('online', 'tier0', 'tier0Raw') and (flags.Tracking.doLargeD0 or flags.Tracking.doLowPtLargeD0):
        ########### here begins InDetGlobalLRTMonAlg ###########
        from InDetGlobalMonitoringRun3Test.InDetGlobalLRTMonAlgCfg import InDetGlobalLRTMonAlgCfg
        inDetGlobalLRTMonAlg = helper.addAlgorithm(CompFactory.InDetGlobalLRTMonAlg, 'InDetGlobalLRTMonAlg',addFilterTools = [FilledBunchFilterToolCfg(flags), AtlasReadyFilterCfg(flags)])

        from InDetConfig.InDetTrackSelectionToolConfig import InDetGlobalLRTMonAlg_TrackSelectionToolCfg
        inDetGlobalLRTMonAlg.TrackSelectionTool = acc.popToolsAndMerge(InDetGlobalLRTMonAlg_TrackSelectionToolCfg(flags))

        InDetGlobalLRTMonAlgCfg(helper, inDetGlobalLRTMonAlg)
        ########### here ends InDetGlobalLRTMonAlg ###########







        
    # run on ESD
    if flags.DQ.Environment != 'tier0Raw':
        ########### here begins InDetGlobalPrimaryVertexMonAlg ###########
        from InDetGlobalMonitoringRun3Test.InDetGlobalPrimaryVertexMonAlgCfg import InDetGlobalPrimaryVertexMonAlgCfg 
        
        myInDetGlobalPrimaryVertexMonAlg = helper.addAlgorithm(CompFactory.InDetGlobalPrimaryVertexMonAlg,
                                                               'InDetGlobalPrimaryVertexMonAlg',addFilterTools = [FilledBunchFilterToolCfg(flags), AtlasReadyFilterCfg(flags)])
        
        kwargsInDetGlobalPrimaryVertexMonAlg = { 
            'vxContainerName'                      : 'PrimaryVertices', #InDetKeys.xAODVertexContainer(),
            'doEnhancedMonitoring'                 : True # InDetFlags.doMonitoringPrimaryVertexingEnhanced()
        }
        
        for k, v in kwargsInDetGlobalPrimaryVertexMonAlg.items():
            setattr(myInDetGlobalPrimaryVertexMonAlg, k, v)
            
        InDetGlobalPrimaryVertexMonAlgCfg(helper, myInDetGlobalPrimaryVertexMonAlg, **kwargsInDetGlobalPrimaryVertexMonAlg)

        ########### here ends InDetGlobalPrimaryVertexMonAlg ###########

        ########### here begins InDetGlobalBeamSpotMonAlg ###########
        from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
        acc.merge(BeamSpotCondAlgCfg(flags))
       
        from InDetGlobalMonitoringRun3Test.InDetGlobalBeamSpotMonAlgCfg import InDetGlobalBeamSpotMonAlgCfg 
        
        myInDetGlobalBeamSpotMonAlg = helper.addAlgorithm(CompFactory.InDetGlobalBeamSpotMonAlg,
                                                          'InDetGlobalBeamSpotMonAlg',addFilterTools = [FilledBunchFilterToolCfg(flags), AtlasReadyFilterCfg(flags)])
        
        kwargsInDetGlobalBeamSpotMonAlg = { 
            'BeamSpotKey'                      : 'BeamSpotData', #InDetKeys.BeamSpotData(),
            'vxContainerName'                  : 'PrimaryVertices', #InDetKeys.xAODVertexContainer(),
            'trackContainerName'               : 'InDetTrackParticles', #InDetKeys.xAODTrackParticleContainer(),
            'useBeamspot'                      : True, # InDetFlags.useBeamConstraint()
            'vxContainerWithBeamConstraint'    : False # InDetFlags.useBeamConstraint()
        }
        
        for k, v in kwargsInDetGlobalBeamSpotMonAlg.items():
            setattr(myInDetGlobalBeamSpotMonAlg, k, v)

        InDetGlobalBeamSpotMonAlgCfg(helper, myInDetGlobalBeamSpotMonAlg, **kwargsInDetGlobalBeamSpotMonAlg)

        ########### here ends InDetGlobalBeamSpotMonAlg ###########
        
    acc.merge(helper.result())
    return acc
