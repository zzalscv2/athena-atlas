"""Define methods to configure SiPropertiesTool

Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from PixelConditionsAlgorithms.PixelConditionsConfig import (
    PixelDCSCondHVAlgCfg, PixelDCSCondTempAlgCfg
)


def PixelSiPropertiesCondAlgCfg(flags, name="PixelSiPropertiesCondAlg", **kwargs):
    """Return configured ComponentAccumulator and tool for PixelSiPropertiesCondAlg

    SiPropertiesTool may be provided in kwargs
    """
    acc = ComponentAccumulator()
    acc.merge(PixelDCSCondHVAlgCfg(flags))
    acc.merge(PixelDCSCondTempAlgCfg(flags))
    acc.addCondAlgo(CompFactory.PixelSiPropertiesCondAlg(name, **kwargs))
    return acc


def PixelSiPropertiesToolCfg(flags, name="PixelSiPropertiesTool", **kwargs):
    """Return a SiPropertiesTool configured for Pixel"""
    acc = PixelSiPropertiesCondAlgCfg(flags)
    kwargs.setdefault("DetectorName", "Pixel")
    kwargs.setdefault("ReadKey", "PixelSiliconPropertiesVector")
    acc.setPrivateTools(CompFactory.SiPropertiesTool(name, **kwargs))
    return acc
