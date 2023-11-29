# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetUsedInFitTrackDecoratorTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetUsedInFitTrackDecoratorToolCfg(
        flags, name="InDetUsedInFitTrackDecoratorTool", **kwargs):
    """Configure the InDetUsedInFitTrackDecoratorTool"""
    acc = ComponentAccumulator()
    acc.setPrivateTools(
        CompFactory.InDet.InDetUsedInFitTrackDecoratorTool(name, **kwargs))
    return acc
