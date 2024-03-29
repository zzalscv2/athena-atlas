# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of SCT/Strip tools of SiClusterOnTrackTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetSCT_ClusterOnTrackToolCfg(flags, name='InDetSCT_ClusterOnTrackTool', **kwargs):
    from TrkConfig.TrkRIO_OnTrackCreatorConfig import RIO_OnTrackErrorScalingCondAlgCfg
    acc = RIO_OnTrackErrorScalingCondAlgCfg(flags) # To produce RIO_OnTrackErrorScaling

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.SCT_LorentzAngleConfig import SCT_LorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(
            SCT_LorentzAngleToolCfg(flags)))
        
    kwargs.setdefault("CorrectionStrategy", 0 ) # do correct position bias
    kwargs.setdefault("ErrorStrategy", 2 ) # do use phi dependent errors

    acc.setPrivateTools(CompFactory.InDet.SCT_ClusterOnTrackTool(name, **kwargs))
    return acc

def InDetBroadSCT_ClusterOnTrackToolCfg(flags, name='InDetBroadSCT_ClusterOnTrackTool', **kwargs):
    kwargs.setdefault("ErrorStrategy", 0)
    return InDetSCT_ClusterOnTrackToolCfg(flags, name=name, **kwargs)

def ITkStripClusterOnTrackToolCfg(flags, name='ITkStrip_ClusterOnTrackTool', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("ErrorStrategy", 0 ) # use width / sqrt(12)
    kwargs.setdefault("ErrorScalingKey", "")

    acc.setPrivateTools(CompFactory.ITk.StripClusterOnTrackTool(name, **kwargs))
    return acc

def ITkBroadStripClusterOnTrackToolCfg(flags, name='ITkBroadStripClusterOnTrackTool', **kwargs):
    kwargs.setdefault("ErrorStrategy", 0)
    return ITkStripClusterOnTrackToolCfg(flags, name=name, **kwargs)
