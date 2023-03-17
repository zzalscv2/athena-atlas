# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrigBjetBtagHypoAlgCfg(flags, name: str = "TrigBjetBtagHypoAlg", **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("HypoTools", CompFactory.TrigBjetBtagHypoTool())
    kwargs.setdefault("MonTool", CompFactory.TrigBjetOnlineMonitoring())

    acc.addEventAlgo(CompFactory.TrigBjetBtagHypoAlg(name=name, **kwargs))
    return acc
