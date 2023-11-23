# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# ********************************************************************
# PixelNtupleConfig.py
# Configures PixelNtupleMaker
# Component accumulator version
# ********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from PixelConditionsTools.PixelConditionsSummaryConfig import PixelConditionsSummaryCfg

def PixelNtupleMakerCfg(flags, name="PixelMonitoringTool", **kwargs):
    acc= ComponentAccumulator()
    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_Loose_Cfg
    InDetTrackSelectionTool = acc.popToolsAndMerge(
        InDetTrackSelectionTool_Loose_Cfg(flags))
    acc.addPublicTool(InDetTrackSelectionTool, primary=False)
    kwargs["TrackSelectionTool"] = InDetTrackSelectionTool
    the_tool = CompFactory.DerivationFramework.PixelNtupleMaker(name,**kwargs)
    acc.addPublicTool(the_tool, primary = True)
    return acc

def EventInfoPixelModuleStatusMonitoringCfg(flags, name="EventInfoPixelModuleStatusMonitoring", **kwargs):
    acc= ComponentAccumulator()
    kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(PixelConditionsSummaryCfg(flags)))
    the_tool = CompFactory.DerivationFramework.EventInfoPixelModuleStatusMonitoring(name,**kwargs)
    acc.addPublicTool(the_tool, primary = True)
    return acc

