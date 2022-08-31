# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def xAODtoHEPToolCfg(flags,name="xAODToHepMCTool", **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.xAODtoHepMCTool(name,**kwargs))
    return result