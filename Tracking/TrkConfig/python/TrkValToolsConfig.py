# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Configuration of TrkValTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrkObserverToolCfg(flags,
                       name="TrackObserverTool",
                       **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Trk.TrkObserverTool(name, **kwargs))
    return acc

def WriterTrkObserverToolCfg(flags,
                             name="WriterTrackObserverTool",
                             **kwargs):
    kwargs.setdefault("ObsTrackCollection", "ObservedTracksCollection")
    kwargs.setdefault("ObsTrackCollectionMap", "ObservedTracksCollectionMap")
    return TrkObserverToolCfg(flags, name, **kwargs)