"""Define methods to configure SiPropertiesTool for PLR

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from PixelConditionsAlgorithms.PLR_ConditionsConfig import (
    PLR_DCSCondHVAlgCfg, PLR_DCSCondTempAlgCfg
)


def PLR_SiPropertiesCondAlgCfg(flags, name="PLR_SiPropertiesCondAlg", **kwargs):
    """Return configured ComponentAccumulator and tool for PLR_SiPropertiesCondAlg

    SiPropertiesTool may be provided in kwargs
    """
    acc = ComponentAccumulator()
    acc.merge(PLR_DCSCondHVAlgCfg(flags))
    acc.merge(PLR_DCSCondTempAlgCfg(flags))
    kwargs.setdefault("PixelIDName", "PLR_ID")
    kwargs.setdefault("PixelDetEleCollKey", "PLR_DetectorElementCollection")
    kwargs.setdefault("ReadKeyeTemp", "PLR_DCSTempCondData")
    kwargs.setdefault("ReadKeyHV", "PLR_DCSHVCondData")
    kwargs.setdefault("WriteKey", "PLR_SiliconPropertiesVector")
    acc.addCondAlgo(CompFactory.PixelSiPropertiesCondAlg(name, **kwargs))
    return acc


def PLR_SiPropertiesToolCfg(flags, name="PLR_SiPropertiesTool", **kwargs):
    """Return a SiPropertiesTool configured for PLR"""
    acc = PLR_SiPropertiesCondAlgCfg(flags)
    kwargs.setdefault("DetectorName", "PLR")
    kwargs.setdefault("ReadKey", "PLR_SiliconPropertiesVector")
    acc.setPrivateTools(CompFactory.SiPropertiesTool(name, **kwargs))
    return acc
