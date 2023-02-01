# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ZDC_StripSDCfg(flags, name="ZDC_StripSD", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("LogicalVolumeNames", ["ZDC::Strip_Logical"])
    kwargs.setdefault("OutputCollectionNames", ["ZDC_SimStripHit_Collection"])
    result.setPrivateTools(CompFactory.ZDC_StripSDTool(name, **kwargs))
    return result


def ZDC_PixelSDCfg(flags, name="ZDC_PixelSD", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("LogicalVolumeNames", ["ZDC::Pixel_Logical"])
    kwargs.setdefault("OutputCollectionNames", ["ZDC_SimPixelHit_Collection"])
    result.setPrivateTools(CompFactory.ZDC_PixelSDTool(name, **kwargs))
    return result
