# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SolenoidalIntersectorCfg(flags, name='SolenoidalIntersector', **kwargs):
    result = ComponentAccumulator()

    cls = CompFactory.Trk.SolenoidParametrizationCondAlg # TrkExSolenoidalIntersector
    condalg = cls (name='SolenoidParametrizationCondAlg', AtlasFieldCacheCondObj = 'fieldCondObj',
                   WriteKey = 'SolenoidParametrization')
    result.addCondAlgo (condalg)

    kwargs.setdefault ('SolenoidParameterizationKey', 'SolenoidParametrization')
    cls = CompFactory.Trk.SolenoidalIntersector # TrkExSolenoidalIntersector
    tool = cls (name, **kwargs)
    result.setPrivateTools (tool)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    acc = SolenoidalIntersectorCfg (flags)
    print ('private tools:', acc.popPrivateTools())
    acc.printConfig (summariseProps=True)
    acc.wasMerged()
