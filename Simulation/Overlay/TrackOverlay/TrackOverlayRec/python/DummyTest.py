# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaPython.PyAthenaComps import Alg, StatusCode

class TestAlg (Alg):
    def __init__(self, name):
        super(TestAlg, self).__init__(name)
    def execute(self):
        self.msg.info("======Running "+self.name+"=========")
        return StatusCode.Success

def DummyCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    doTrackOverlay = getattr(flags.TrackOverlay, "ActiveConfig.doTrackOverlay", None)
    from pprint import pprint
    pprint(vars(flags))
    acc.addEventAlgo(TestAlg(name=('Sig_' if doTrackOverlay else '')+'Dummy'))
    return acc

