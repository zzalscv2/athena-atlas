# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def HiggsTruthCategoryToolCfg(flags, name="HiggsTruthCategoryTool", **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.HiggsTruthCategoryTool(name,**kwargs))
    return result