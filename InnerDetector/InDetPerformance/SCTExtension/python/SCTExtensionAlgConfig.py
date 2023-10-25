# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def SCTExtensionAlgCfg(flags):
    """Add the SCTExtensionAlg to the list of algorithms.
    """
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(CompFactory.SCTExtensionAlg('SCTExtensionAlg'))

    return cfg
