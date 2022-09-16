"""Define methods to configure SiPropertiesTool

Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from PixelConditionsAlgorithms.ITkPixelConditionsConfig import (
    ITkPixelDCSCondHVAlgCfg, ITkPixelDCSCondTempAlgCfg
)

def ITkPixelSiPropertiesCondAlgCfg(flags, name="ITkPixelSiPropertiesCondAlg", **kwargs):
    """Return configured ComponentAccumulator and tool for ITkPixelSiPropertiesCondAlg

    SiPropertiesTool may be provided in kwargs
    """
    acc = ComponentAccumulator()
    acc.merge(ITkPixelDCSCondHVAlgCfg(flags))
    acc.merge(ITkPixelDCSCondTempAlgCfg(flags))
    kwargs.setdefault("PixelDetEleCollKey", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("ReadKeyeTemp", "ITkPixelDCSTempCondData")
    kwargs.setdefault("ReadKeyHV", "ITkPixelDCSHVCondData")
    kwargs.setdefault("WriteKey", "ITkPixelSiliconPropertiesVector")
    acc.addCondAlgo(CompFactory.PixelSiPropertiesCondAlg(name, **kwargs))
    return acc


def ITkPixelSiPropertiesToolCfg(flags, name="ITkPixelSiPropertiesTool", **kwargs):
    """Return a SiPropertiesTool configured for ITk Pixel"""
    acc = ITkPixelSiPropertiesCondAlgCfg(flags)
    kwargs.setdefault("DetectorName", "Pixel")
    kwargs.setdefault("ReadKey", "ITkPixelSiliconPropertiesVector")
    acc.setPrivateTools(CompFactory.SiPropertiesTool(name, **kwargs))
    return acc
