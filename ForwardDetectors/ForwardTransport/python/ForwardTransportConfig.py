# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ForwardTransportModelCfg(flags, name="ForwardTransportModel", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ForwardTransportSvcName", "ForwardTransportSvc")
    kwargs.setdefault("RegionNames" , ["FWDBeamLine"] )
    result.setPrivateTools(CompFactory.ForwardTransportModelTool(name, **kwargs))
    return result
