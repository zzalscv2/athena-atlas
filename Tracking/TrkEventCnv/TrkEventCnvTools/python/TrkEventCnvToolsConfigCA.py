# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonCnvToolFixTGCsCfg(flag, name='MuonCnvToolFixTGCs', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("FixTGCs", True)
    acc.setPrivateTools(CompFactory.Muon.MuonEventCnvTool(name, **kwargs))
    return acc

def TrkEventCnvSuperToolCfg(flags, name='EventCnvSuperTool', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("DoID", flags.Reco.EnableTracking)
    kwargs.setdefault("DoMuons", flags.Detector.EnableMuon)
    kwargs.setdefault("DoTrackOverlay",
                      (flags.Common.isOverlay or flags.Output.doWriteRDO) and \
                      flags.Overlay.doTrackOverlay)

    acc.addPublicTool(CompFactory.Trk.EventCnvSuperTool(name, **kwargs))
    return acc

def MuonTrkEventCnvSuperToolCfg(flags, name='MuonEventCnvSuperTool', **kwargs):
    acc = ComponentAccumulator()

    if "MuonCnvTool" not in kwargs:
        kwargs.setdefault("MuonCnvTool", acc.popToolsAndMerge(
            MuonCnvToolFixTGCsCfg(flags)))

    acc.addPublicTool(CompFactory.Trk.EventCnvSuperTool(name, **kwargs))
    return acc

