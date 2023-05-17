# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
__log = logging.getLogger('SubtractedCellGetterCA')

def SubtractedCellGetterCfgCA(flags, modulator):
    acc=ComponentAccumulator()
    
    cellCopyTool = acc.popToolsAndMerge(CaloCellFastCopyToolCfg(flags,name="HICellCopyTool"))
    
    cellSubtrTool = acc.popToolsAndMerge(HISubtractedCellMakerToolCfg(flags, name="HISubtractedCellMakerTool", Modulator=modulator))
    
    cellFinalizerTool=acc.popToolsAndMerge(CaloCellContainerFinalizerToolCfg(flags, name="HICaloCellFinalizerTool"))
    
    cellMakerTools=[cellCopyTool,cellSubtrTool,cellFinalizerTool]

    cellAlgo = CompFactory.CaloCellMaker("HICaloCellCopier",
                                          CaloCellMakerToolNames=cellMakerTools,
                                          CaloCellsOutputName=flags.HeavyIon.Egamma.SubtractedCells,
                                          OwnPolicy=0)
    
    acc.addEventAlgo(cellAlgo)
    return acc

def CaloCellFastCopyToolCfg(flags, name="HICellCopyTool", **kwargs):
    includeSamplings=["PreSamplerB", "EMB1", "EMB2", "EMB3",
                      "PreSamplerE", "EME1", "EME2", "EME3",
                      "HEC0", "HEC1", "HEC2", "HEC3",
                      "TileBar0", "TileBar1", "TileBar2",
                      "TileGap1", "TileGap2", "TileGap3",
                      "TileExt0", "TileExt1", "TileExt2",
                      "FCal1", "FCal2", "FCal3" ]
    kwargs.setdefault("IncludeSamplings",includeSamplings)
    kwargs.setdefault("InputName","AllCalo")

    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.CaloCellFastCopyTool(name, **kwargs))
    return acc

def HISubtractedCellMakerToolCfg(flags, name="HISubtractedCellMakerTool", **kwargs):
    acc = ComponentAccumulator()

    if "Modulator" not in kwargs:
        __log.warning("Modulator is None, will set it to NULL")
        from HIJetRec.HIJetRecConfigCA import GetNullModulator
        kwargs.setdefault("Modulator",GetNullModulator())
    if "EventShapeKey" not in kwargs:
        kwargs.setdefault("EventShapeKey", flags.HeavyIon.Egamma.EventShape)
    if "EventShapeMapTool" not in kwargs:
        from HIGlobal.HIGlobalConfig import HIEventShapeMapToolCfg
        eventShapeMapTool = acc.popToolsAndMerge(HIEventShapeMapToolCfg(flags, name="HIEventShapeMapTool"))
        kwargs.setdefault("EventShapeMapTool",eventShapeMapTool)

    acc.setPrivateTools(CompFactory.HISubtractedCellMakerTool(name, **kwargs))
    return acc

def CaloCellContainerFinalizerToolCfg(flags, name="HICaloCellFinalizerTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.CaloCellContainerFinalizerTool(name, **kwargs))
    return acc
