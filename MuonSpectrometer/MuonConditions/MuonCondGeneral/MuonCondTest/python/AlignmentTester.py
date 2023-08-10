# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def ALineInjectorAlgCfg(flags, name="AlineInjectorAlg", **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    result = ComponentAccumulator()
    the_alg = CompFactory.ALineInjectTestAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result