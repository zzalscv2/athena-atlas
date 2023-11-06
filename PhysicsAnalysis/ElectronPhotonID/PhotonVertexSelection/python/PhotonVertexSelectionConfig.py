# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhotonPointingToolCfg(flags, name="PhotonPointingTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("isSimulation", flags.Input.isMC)
    acc.setPrivateTools(CompFactory.CP.PhotonPointingTool(name, **kwargs))
    return acc

def PhotonVertexSelectionToolCfg(
        flags, name="PhotonVertexSelectionTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools( CompFactory.CP.PhotonVertexSelectionTool( **kwargs ) )
    return acc
