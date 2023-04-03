"""Define methods to configure SCT SiProperties

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_SiliconConditionsCfg
from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg


def SCT_SiPropertiesCondAlgCfg(flags, name="SCTSiPropertiesCondAlg", **kwargs):
    """Return configured ComponentAccumulator and tool for SCT_SiProperties

    SiConditionsTool and/or DCSConditionsTool may be provided in kwargs
    """
    acc = ComponentAccumulator()

    # Condition algorithm
    # SCTSiPropertiesCondAlg needs outputs of SCT_SiliconConditions algorithms
    if not kwargs.get("SiConditionsTool"):
        kwargs["SiConditionsTool"] = acc.popToolsAndMerge(SCT_SiliconConditionsCfg(flags))
    # For SCT_ID and SCT_DetectorElementCollection
    # used in SCTSiPropertiesCondAlg and SiPropertiesTool
    acc.merge(SCT_ReadoutGeometryCfg(flags))
    acc.addCondAlgo(CompFactory.SCTSiPropertiesCondAlg(name, **kwargs))
    return acc


def SCT_SiPropertiesToolCfg(flags, name="SCT_SiPropertiesTool", **kwargs):
    """Return configured ComponentAccumulator and tool for SCT_SiProperties

    SiConditionsTool and/or DCSConditionsTool may be provided in kwargs
    """
    SiConditionsTool = kwargs.pop("SiConditionsTool", None)
    acc = SCT_SiPropertiesCondAlgCfg(flags, SiConditionsTool=SiConditionsTool)
    kwargs.setdefault("DetectorName", "SCT")
    kwargs.setdefault("ReadKey", "SCTSiliconPropertiesVector")
    acc.setPrivateTools(CompFactory.SiPropertiesTool(name, **kwargs))
    return acc
