"""Define methods to configure ITkStrip SiProperties

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripSiliconConditionsCfg
from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg


def ITkStripSiPropertiesCondAlgCfg(flags, name="ITkStripSiPropertiesCondAlg", **kwargs):
    """Return configured ComponentAccumulator and tool for ITkStripSiProperties

    SiConditionsTool and/or DCSConditionsTool may be provided in kwargs
    """
    acc = ComponentAccumulator()

    # Condition algorithm
    # SCTSiPropertiesCondAlg needs outputs of SCT_SiliconConditions algorithms
    if not kwargs.get("SiConditionsTool"):
        kwargs["SiConditionsTool"] = acc.popToolsAndMerge(ITkStripSiliconConditionsCfg(flags))

    kwargs.setdefault("SCTDetEleCollKey", "ITkStripDetectorElementCollection")
    kwargs.setdefault("WriteKey", "ITkStripSiliconPropertiesVector")

    acc.merge(ITkStripReadoutGeometryCfg(flags))
    acc.addCondAlgo(CompFactory.SCTSiPropertiesCondAlg(name, **kwargs))
    return acc


def ITkStripSiPropertiesToolCfg(flags, name="ITkStripSiPropertiesTool", **kwargs):
    """Return a SiPropertiesTool configured for ITk Strip"""
    SiConditionsTool = kwargs.pop("SiConditionsTool", None)
    acc = ITkStripSiPropertiesCondAlgCfg(flags, SiConditionsTool=SiConditionsTool)
    kwargs.setdefault("DetectorName", "SCT")
    kwargs.setdefault("ReadKey", "ITkStripSiliconPropertiesVector")
    acc.setPrivateTools(CompFactory.SiPropertiesTool(name, **kwargs))
    return acc
