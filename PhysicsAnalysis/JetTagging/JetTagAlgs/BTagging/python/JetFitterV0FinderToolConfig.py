# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

from BTagging.InDetJetFitterUtilsConfig import InDetJetFitterUtilsCfg
from BTagging.JetFitterMode3dTo1dFinderConfig import JetFitterMode3dTo1dFinderCfg


def JetFitterV0FinderToolCfg(flags, name, suffix="", useBTagFlagsDefaults = True, **options):
    """Sets up a JetFitterV0FinderTool tool and returns it. 

    The following options have defaults:
    
    revertFromPositiveToNegativeTags                               default: False
    cutTwoTrkVtxVtxProbForBFirstSelectCriteriumA                   default: 0.05
    cutTwoTrkVtxVtxProbForBFirstSelectCriteriumB                   default: 0.034
    cutCompatibilityPrimaryVertexSingleTrackForBFirstSelection     default: 1e-1
    cutCompatibilityPrimaryVertexBothTracksForBFirstSelection      default: 1e-2
    cutIPD0BothTracksForBFirstSelection                            default: 3.5
    cutIPZ0BothTracksForBFirstSelection                            default: 5.
    cutPtBothTracksForBFirstSelection                              default: 500.
    cutTwoTrkVtxLifeSignForBFirstSelectCriteriumA                  default: 1.
    cutTwoTrkVtxLifeSignForBFirstSelectCriteriumB                  default: 1.5
    cutCompToPrimarySingleTrackForMatInterac                       default: 1e-4
    cutCompToPrimaryBothTracksForMatInterac                        default: 1e-6
    firstLayer_min                                                 default: 34.0-2.5
    firstLayer_max                                                 default: 34.0+2.5
    secondLayer_min                                                default: 51.5-3
    secondLayer_max                                                default: 51.5+3
    cutCompPVSinglePosLifeTrackForBSecondSelect                    default: 5e-2 
    cutCompPVSingleNegLifeTrackForBSecondSelect                    default: 1e-2
    cutIPD0SigBoxSingleTrackForBSecondSelection                    default: 2.
    cutIPZ0SigBoxSingleTrackForBSecondSelection                    default: 5.
    cutIPD0SingleTrackForBSecondSelection                          default: 1.5
    cutIPZ0SingleTrackForBSecondSelection                          default: 3.
    cutPtSingleTrackForBSecondSelection                            default: 750
    
    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    acc = ComponentAccumulator()
    if useBTagFlagsDefaults:
        inDetJetFitterUtils = acc.popToolsAndMerge(InDetJetFitterUtilsCfg(flags,'InDetJFUtils'+suffix))
        jetFitterMode3dTo1dFinder = acc.popToolsAndMerge(JetFitterMode3dTo1dFinderCfg(flags, 'JFMode3dTo1dFinder'+suffix))
        defaults = { 'revertFromPositiveToNegativeTags' : True if (suffix=="FLIP_SIGN") else False,
                     'InDetJetFitterUtils' : inDetJetFitterUtils,
                     'Mode3dFinder' : jetFitterMode3dTo1dFinder,
                     'useITkMaterialRejection' : flags.GeoModel.Run >= LHCPeriod.Run4 }
        for option in defaults:
            options.setdefault(option, defaults[option])


    options['name'] = name
    acc.setPrivateTools(CompFactory.InDet.JetFitterV0FinderTool(**options))
    return acc



