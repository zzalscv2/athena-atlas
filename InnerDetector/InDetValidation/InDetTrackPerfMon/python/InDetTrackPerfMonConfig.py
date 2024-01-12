#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file InDetTrackPerfMonConfig.py
@author M. Aparo
@date 2023-02-17
@brief Main CA-based python configuration for InDetTrackPerfMonTool
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging


def TrackAnalysisDefinitionSvcCfg( flags, name="TrkAnaDefSvc", **kwargs ):
    '''
    CA-based configuration for the TrackAnalysisDefinition Service
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "SubFolder", flags.PhysVal.IDTPM.currentTrkAna.SubFolder )
    kwargs.setdefault( "TrkAnaTag", flags.PhysVal.IDTPM.currentTrkAna.anaTag )

    kwargs.setdefault( "TestType", flags.PhysVal.IDTPM.currentTrkAna.TestType )
    kwargs.setdefault( "RefType",  flags.PhysVal.IDTPM.currentTrkAna.RefType )

    ## TODO - to be uncommented in future MRs
    #kwargs.setdefault( "TestTag", getTrkTag(flags.PhysVal.IDTPM.currentTrkAna.TestType))
    #kwargs.setdefault( "RefTag",  getTrkTag(flags.PhysVal.IDTPM.currentTrkAna.RefType))

    kwargs.setdefault( "MatchingType", flags.PhysVal.IDTPM.currentTrkAna.MatchingType )

    testToRefDecoName = "testToRefLink"
    refToTestDecoName = "refToTestLinks"
    if flags.PhysVal.IDTPM.currentTrkAna.MatchingType == "DeltaRMatch":
        testToRefDecoName = "testToRefDRLink"
        refToTestDecoName = "refToTestDRLinks"
    if flags.PhysVal.IDTPM.currentTrkAna.MatchingType == "TruthMatch":
        testToRefDecoName = "trackToTruthLink"
        refToTestDecoName = "truthToTrackLinks"

    kwargs.setdefault( "TestToRefDecoName",
                       testToRefDecoName + flags.PhysVal.IDTPM.currentTrkAna.anaTag )
    kwargs.setdefault( "RefToTestDecoName",
                       refToTestDecoName + flags.PhysVal.IDTPM.currentTrkAna.anaTag )

    if flags.PhysVal.IDTPM.currentTrkAna.TestType == "Trigger":
        ## get configured list of chains from regex
        from InDetTrackPerfMon.ConfigUtils import getChainList
        kwargs.setdefault( "ChainNames", getChainList( flags ) )

    ## TODO - to be uncommented in future MRs
    kwargs.setdefault( "doTrackParameters", flags.PhysVal.IDTPM.currentTrkAna.doTrackParameters )
    kwargs.setdefault( "doEfficiencies", flags.PhysVal.IDTPM.currentTrkAna.doEfficiencies )
    kwargs.setdefault( "doOfflineElectrons", flags.PhysVal.IDTPM.currentTrkAna.doOfflineElectrons )

    trkAnaSvc = CompFactory.TrackAnalysisDefinitionSvc( name, **kwargs )
    acc.addService( trkAnaSvc )
    return acc


def InDetTrackPerfMonToolCfg( flags, name="InDetTrackPerfMonTool", **kwargs ):
    '''
    Main IDTPM tool instance CA-based configuration
    '''
    acc = ComponentAccumulator()

    ## TODO - to be uncommented in future MRs
    #acc.merge(HistogramDefinitionSvcCfg(flags, name="HistoDefSvc"+
    #                                flags.PhysVal.IDTPM.currentTrkAna.anaTag))

    kwargs.setdefault( "OfflineTrkParticleContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.OfflineTrkKey )
    kwargs.setdefault( "TruthParticleContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.TruthPartKey )

    kwargs.setdefault( "DirName", flags.PhysVal.IDTPM.DirName )
    kwargs.setdefault( "AnaTag", flags.PhysVal.IDTPM.currentTrkAna.anaTag )

    acc.merge( TrackAnalysisDefinitionSvcCfg( flags,
                   name="TrkAnaDefSvc"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) )

    if "TrackQualitySelectionTool" not in kwargs:
        from InDetTrackPerfMon.InDetSelectionConfig import TrackQualitySelectionToolCfg
        kwargs.setdefault( "TrackQualitySelectionTool", acc.popToolsAndMerge(
            TrackQualitySelectionToolCfg( flags,
                name="TrackQualitySelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    if "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.TestType :

        kwargs.setdefault( "TriggerTrkParticleContainerName",
                           flags.PhysVal.IDTPM.currentTrkAna.TrigTrkKey )

        if "TrigDecisionTool" not in kwargs:
            from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
            kwargs.setdefault( "TrigDecisionTool", 
                               acc.getPrimaryAndMerge( TrigDecisionToolCfg(flags) ) )

        if "RoiSelectionTool" not in kwargs:
            from InDetTrackPerfMon.InDetSelectionConfig import RoiSelectionToolCfg
            kwargs.setdefault( "RoiSelectionTool", acc.popToolsAndMerge(
                RoiSelectionToolCfg( flags,
                    name="RoiSelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

        if "TrackRoiSelectionTool" not in kwargs:
            from InDetTrackPerfMon.InDetSelectionConfig import TrackRoiSelectionToolCfg
            kwargs.setdefault( "TrackRoiSelectionTool", acc.popToolsAndMerge(
                TrackRoiSelectionToolCfg( flags,
                    name="TrackRoiSelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    ## TODO - to be uncommented in future MRs
    #if "TrackMatchingTool" not in kwargs:
    #    kwargs.setdefault("TrackMatchingTool", acc.popToolsAndMerge(
    #        TrackMatchingToolCfg(flags)))

    acc.setPrivateTools( CompFactory.InDetTrackPerfMonTool( name, **kwargs ) )
    return acc


def InDetTrackPerfMonCfg( flags ):
    '''
    CA-based configuration of all tool instances (= TrackAnalyses)
    '''
    log = logging.getLogger( "InDetTrackPerfMonCfg" )
    acc = ComponentAccumulator()

    ## Configuring IDTPM tool instances
    tools = [] 

    ## TODO - to be uncommented in future MRs
    #useTruth = False
    #for trkAnaName in flags.PhysVal.IDTPM.trkAnaNames :
    #    if "Truth" in getattr( flags.PhysVal.IDTPM, trkAnaName+".RefType" ) :
    #        useTruth = True
    #if useTruth:
    #    acc.merge( InDetTruthDecoratorAlgCfg(flags) )

    ## TODO - to be uncommented in future MRs
    ## true only if offline objects are requested in any scheduled trkAnalysis
    #if getObjectStrList(flags):
    #    from InDetTrackPerfMon.InDetOfflineObjectDecoratorAlgConfig import InDetOfflineObjectDecoratorAlgCfg
    #    acc.merge( InDetOfflineObjectDecoratorAlgCfg(flags) )

    for trkAnaName in flags.PhysVal.IDTPM.trkAnaNames:
        ## cloning flags of current TrackAnalysis to PhysVal.IDTPM.currentTrkAna
        flags_thisTrkAna = flags.cloneAndReplace( "PhysVal.IDTPM.currentTrkAna",
                                                  "PhysVal.IDTPM."+trkAnaName )

        ## further cloning into different sets of flags if unpackTrigChains = True
        from InDetTrackPerfMon.ConfigUtils import getFlagsList
        for iflags in getFlagsList( flags_thisTrkAna ):
            if iflags.PhysVal.IDTPM.currentTrkAna.enabled:
                log.debug( "Scheduling TrackAnalysis: %s",
                           iflags.PhysVal.IDTPM.currentTrkAna.anaTag )
                tools.append(
                    acc.popToolsAndMerge( InDetTrackPerfMonToolCfg( iflags,
                            name="InDetTrackPerfMonTool"+
                                 iflags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg
    acc.merge( PhysValMonitoringCfg( flags, tools=tools ) )
    return acc
