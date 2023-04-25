# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_TrackExtensionAlg package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TRT_TrackExtensionAlgCfg(flags, name = 'InDetTRT_Extension', **kwargs):
    acc = ComponentAccumulator()

    if "TrackExtensionTool" not in kwargs:
        from InDetConfig.TRT_TrackExtensionToolConfig import TRT_TrackExtensionToolCfg
        kwargs.setdefault("TrackExtensionTool", acc.popToolsAndMerge(
            TRT_TrackExtensionToolCfg(flags)))

    acc.addEventAlgo(CompFactory.InDet.TRT_TrackExtensionAlg(
        name + flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc

def TRT_Phase_TrackExtensionAlgCfg(flags, name = 'InDetTRT_Phase_Extension', **kwargs):
    acc = ComponentAccumulator()

    if "TrackExtensionTool" not in kwargs:
        from InDetConfig.TRT_TrackExtensionToolConfig import TRT_TrackExtensionToolPhaseCfg
        kwargs.setdefault("TrackExtensionTool", acc.popToolsAndMerge(
            TRT_TrackExtensionToolPhaseCfg(flags)))

    acc.addEventAlgo(CompFactory.InDet.TRT_TrackExtensionAlg(name, **kwargs))
    return acc

def Trig_TRT_TrackExtensionAlgCfg(flags, name = 'InDetTrigMTTrackExtensionAlg', **kwargs):
    acc = ComponentAccumulator()

    if "TrackExtensionTool" not in kwargs:
        from InDetConfig.TRT_TrackExtensionToolConfig import Trig_TRT_TrackExtensionToolCfg
        kwargs.setdefault("TrackExtensionTool", acc.popToolsAndMerge(
            Trig_TRT_TrackExtensionToolCfg(flags)))

    kwargs.setdefault("InputTracksLocation",
                      flags.Tracking.ActiveConfig.trkTracks_IDTrig+"_Amb")
    kwargs.setdefault("ExtendedTracksLocation", "ExtendedTrackMap")

    acc.addEventAlgo(CompFactory.InDet.TRT_TrackExtensionAlg(
        f"{name}_{flags.Tracking.ActiveConfig.name}", **kwargs))
    return acc
