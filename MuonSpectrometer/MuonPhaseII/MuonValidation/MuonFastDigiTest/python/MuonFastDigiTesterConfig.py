# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MDTFastDigiTesterCfg(flags, name = "MDTFastDigiTester", **kwargs):
    result = ComponentAccumulator()
    theAlg = CompFactory.MuonValR4.MDTFastDigiTester(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result
