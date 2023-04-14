# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkAlignDBTool(flags, name="ITkAlignDBTool",setAlignmentFolderName="/Indet/AlignITk", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelManager","ITkPixel")
    kwargs.setdefault("SCT_Manager","ITkStrip")
    kwargs.setdefault("DBRoot",setAlignmentFolderName)
    acc.setPrivateTools(CompFactory.InDetAlignDBTool(name,**kwargs))
    return acc

def InDetAlignDBTool(flags, name="InDetAlignDBTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.InDetAlignDBTool(name,**kwargs))
    return acc