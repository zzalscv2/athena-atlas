# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetUsedInFitTrackDecoratorTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetUsedInFitTrackDecoratorToolCfg(flags, name, **kwargs):
    """Configure the InDetUsedInFitTrackDecoratorTool"""
    acc = ComponentAccumulator()
    acc.setPrivateTools(
        CompFactory.InDet.InDetUsedInFitTrackDecoratorTool(name, **kwargs))
    return acc
