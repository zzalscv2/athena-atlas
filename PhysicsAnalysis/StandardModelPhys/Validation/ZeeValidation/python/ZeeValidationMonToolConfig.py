#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file ZeeValidationMonToolConfig.py
@author N.Pettersson
@date 2022-07-08
@brief Main CA-based python configuration for ZeeValidationMonitoringTool
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhysValZeeCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    from AthenaCommon.Constants import WARNING
    kwargs.setdefault("IsData", flags.Input.isMC)
    kwargs.setdefault("EnableLumi", False)
    kwargs.setdefault("OutputLevel", WARNING)
    kwargs.setdefault("DetailLevel", 10)
    kwargs.setdefault("PhotonContainerName", "Photons")
    kwargs.setdefault("ElectronContainerName", "Electrons")
    kwargs.setdefault("ElectronContainerFrwdName", "ForwardElectrons")
    kwargs.setdefault("TruthParticleContainerName", "TruthParticles")

    tool = CompFactory.ZeeValidation.ZeeValidationMonitoringTool(**kwargs)
    acc.setPrivateTools(tool)
    return acc
