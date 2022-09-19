# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def PixelReadoutManagerCfg(flags, name="PixelReadoutManager", **kwargs):
    """Return a ComponentAccumulator with configured PixelReadoutManager"""
    acc = ComponentAccumulator()
    acc.addService(CompFactory.InDetDD.PixelReadoutManager(name, **kwargs), primary=True)
    return acc


def ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager", **kwargs):
    """Return a ComponentAccumulator with configured ITkPixelReadoutManager"""
    acc = ComponentAccumulator()
    acc.addService(CompFactory.InDetDD.ITk.PixelReadoutManager(name, **kwargs), primary=True)
    return acc

def PLR_ReadoutManagerCfg(flags, name="PLR_ReadoutManager", **kwargs):
    """Return a ComponentAccumulator with a PLR configured ITkPixelReadoutManager"""
    acc = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "PLR")
    kwargs.setdefault("PixelIDName", "PLR_ID")
    acc.addService(CompFactory.InDetDD.ITk.PixelReadoutManager(name, **kwargs), primary=True)
    return acc
