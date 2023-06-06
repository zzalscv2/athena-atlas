# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from BTagging.InDetJetFitterUtilsConfig import InDetJetFitterUtilsCfg
from BTagging.ImprovedJetFitterInitializationHelperConfig import ImprovedJetFitterInitializationHelperCfg
from BTagging.ImprovedJetFitterRoutinesConfig import ImprovedJetFitterRoutinesCfg

def InDetJetFitterMultiStageFitCfg(flags, name, suffix="", useBTagFlagsDefaults = True, **options):
    """Sets up a JetFitterMultiStageFit  tool and returns it.

    The following options have defaults:
    
    MaxNumDeleteIterations                     default: 30
    VertexProbCut                              default: 0.001
    MaxClusteringIterations                    default: 30
    UseFastClustering                          default: False
    maxTracksForDetailedClustering             default: 25
    VertexClusteringProbabilityCutWithMasses   default: {0.002, 0.002, 0.050, 0.100, 0.200, 0.500, 0.700, 0.900, 0.900}

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""

    acc = ComponentAccumulator()
    if useBTagFlagsDefaults:
        improvedJetFitterRoutines = acc.popToolsAndMerge(ImprovedJetFitterRoutinesCfg(flags, 'ImprovedJFRoutines'+suffix))
        inDetJetFitterUtils = acc.popToolsAndMerge(InDetJetFitterUtilsCfg(flags,'InDetJFUtils'+suffix))
        improvedJetFitterInitializationHelper = acc.popToolsAndMerge(ImprovedJetFitterInitializationHelperCfg(flags, 'ImprovedJFInitHelper'+suffix))
        defaults = { 'JetFitterInitializationHelper' : improvedJetFitterInitializationHelper,
                     'JetFitterRoutines' : improvedJetFitterRoutines,
                     'InDetJetFitterUtils' : inDetJetFitterUtils }
        for option in defaults:
            options.setdefault(option, defaults[option])

    options['name'] = name
    acc.setPrivateTools( CompFactory.InDet.JetFitterMultiStageFit(**options) )
    return acc



