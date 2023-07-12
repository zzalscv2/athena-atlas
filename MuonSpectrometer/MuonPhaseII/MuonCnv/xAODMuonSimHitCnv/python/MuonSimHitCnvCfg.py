#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MdtToxAODConvAlgCfg(flags,name="MdtToxAODConvAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MdtSimHitToxAODCnvAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
