# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ElectronVariableCorrectionToolCfg(
        flags, name="ElectronVariableCorrectionTool", **kwargs):
    """Configure the e/gamma variable correction tool"""
    acc = ComponentAccumulator()
    # Can ultimately be configured differently between Run 2 and Run 3 configs
    kwargs.setdefault("ConfigFile", "EGammaVariableCorrection/TUNE26/ElPhVariableNominalCorrection.conf")
    acc.setPrivateTools(
        CompFactory.ElectronPhotonVariableCorrectionTool(name, **kwargs))
    return acc

def PhotonVariableCorrectionToolCfg(
        flags, name="PhotonVariableCorrectionTool", **kwargs):
    """Configure the e/gamma variable correction tool"""
    acc = ComponentAccumulator()
    # Keep using TUNE 23 for now for photons
    kwargs.setdefault("ConfigFile", "EGammaVariableCorrection/TUNE23/ElPhVariableNominalCorrection.conf")
    acc.setPrivateTools(
        CompFactory.ElectronPhotonVariableCorrectionTool(name, **kwargs))
    return acc


