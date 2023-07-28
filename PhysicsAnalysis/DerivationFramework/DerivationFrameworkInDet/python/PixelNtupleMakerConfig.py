# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# ********************************************************************
# PixelNtupleConfig.py
# Configures PixelNtupleMaker
# Component accumulator version
# ********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PixelNtupleMakerCfg(flags, name="PixelMonitoringTool", **kwargs):
    acc= ComponentAccumulator()
    InDetTrackSelectionTool = CompFactory.InDet.InDetTrackSelectionTool(name="InDetTrackSelectionTool", CutLevel="Loose")
    acc.addPublicTool(InDetTrackSelectionTool, primary=False)
    kwargs["TrackSelectionTool"] = InDetTrackSelectionTool
    the_tool = CompFactory.DerivationFramework.PixelNtupleMaker(name,**kwargs)
    acc.addPublicTool(the_tool, primary = True)
    return acc

def EventInfoPixelModuleStatusMonitoringCfg(flags, name="EventInfoPixelModuleStatusMonitoring", **kwargs):
    acc= ComponentAccumulator()
    the_tool = CompFactory.DerivationFramework.EventInfoPixelModuleStatusMonitoring(name,**kwargs)
    acc.addPublicTool(the_tool, primary = True)
    return acc

