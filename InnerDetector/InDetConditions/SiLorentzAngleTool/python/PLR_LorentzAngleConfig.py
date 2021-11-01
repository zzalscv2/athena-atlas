# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
from PixelConditionsAlgorithms.PLR_ConditionsConfig import (
    PLR_DCSCondHVAlgCfg, PLR_DCSCondTempAlgCfg
)
from PLRGeoModelXml.PLR_GeoModelConfig import PLR_ReadoutGeometryCfg
from SiPropertiesTool.PLR_SiPropertiesConfig import PLR_SiPropertiesToolCfg


def PLR_LorentzAngleCondAlgCfg(flags, name="PLR_SiLorentzAngleCondAlg", **kwargs):
    acc = AtlasFieldCacheCondAlgCfg(flags)
    acc.merge(PLR_ReadoutGeometryCfg(flags)) # To produce PLR_DetectorElementCollection
    acc.merge(PLR_DCSCondHVAlgCfg(flags))
    acc.merge(PLR_DCSCondTempAlgCfg(flags))
    kwargs.setdefault("SiPropertiesTool", acc.popToolsAndMerge(PLR_SiPropertiesToolCfg(flags)))
    kwargs.setdefault("PixelIDName", "PLR_ID")
    kwargs.setdefault("UseMagFieldCache", True)
    kwargs.setdefault("UseMagFieldDcs", not flags.Common.isOnline)
    kwargs.setdefault("PixelModuleData", "PLR_ModuleData")
    kwargs.setdefault("ReadKeyeTemp", "PLR_DCSTempCondData")
    kwargs.setdefault("ReadKeyHV", "PLR_DCSHVCondData")
    kwargs.setdefault("PixelDetEleCollKey", "PLR_DetectorElementCollection")
    kwargs.setdefault("WriteKey", "PLR_SiLorentzAngleCondData")
    acc.addCondAlgo(CompFactory.PixelSiLorentzAngleCondAlg(name, **kwargs))
    return acc


def PLR_LorentzAngleToolCfg(flags, name="PLR_LorentzAngleTool", **kwargs):
    """Return a SiLorentzAngleTool configured for PLR"""
    acc = PLR_LorentzAngleCondAlgCfg(flags)
    kwargs.setdefault("DetectorName", "PLR")
    kwargs.setdefault("SiLorentzAngleCondData", "PLR_SiLorentzAngleCondData")
    kwargs.setdefault("DetEleCollKey", "PLR_DetectorElementCollection")
    kwargs.setdefault("UseMagFieldCache", True)
    acc.setPrivateTools(CompFactory.SiLorentzAngleTool(name, **kwargs))
    return acc
