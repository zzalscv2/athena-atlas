# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep


def TrkEventCnvSuperToolCfg(flags):
    acc = ComponentAccumulator()
    Trk_EventCnvSuperTool = CompFactory.Trk.EventCnvSuperTool
    EventCnvSuperTool = Trk_EventCnvSuperTool(
        'EventCnvSuperTool',
        DoID=flags.Reco.EnableTracking,
        DoMuons=flags.Detector.EnableMuon,
    )
    if (flags.Common.ProductionStep == ProductionStep.Overlay or flags.Output.doWriteRDO) and flags.Overlay.doTrackOverlay:
        EventCnvSuperTool.DoTrackOverlay=True
    acc.addPublicTool(EventCnvSuperTool)
    return acc
