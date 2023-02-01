#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetHardScatterSelectionTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetHardScatterSelectionToolCfg(flags, name="InDetHardScatterSelectionTool", **kwargs):
    """Configure the InDet hard scatter selection tool"""
    acc = ComponentAccumulator()
    acc.setPrivateTools(
        CompFactory.InDet.InDetHardScatterSelectionTool(name, **kwargs))
    return acc
