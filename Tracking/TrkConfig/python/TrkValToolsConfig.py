# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Configuration of TrkValTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrkObserverToolCfg(flags,
                       name="TrackObserverTool",
                       **kwargs):
    acc = ComponentAccumulator()
    if "Fitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetTrackFitterCfg
        ObserverFitter = acc.popToolsAndMerge(InDetTrackFitterCfg(flags))
        acc.addPublicTool(ObserverFitter)
        kwargs.setdefault("Fitter", ObserverFitter)
    kwargs.setdefault("HadROIPhiRZEtContainer", "InDetHadCaloClusterROIPhiRZEt")
    acc.setPrivateTools(CompFactory.Trk.TrkObserverTool(name, **kwargs))
    return acc

def WriterTrkObserverToolCfg(flags,
                             name="WriterTrackObserverTool",
                             **kwargs):
    kwargs.setdefault("ObsTrackCollection", "ObservedTracksCollection")
    kwargs.setdefault("ObsTrackCollectionMap", "ObservedTracksCollectionMap")
    return TrkObserverToolCfg(flags, name, **kwargs)