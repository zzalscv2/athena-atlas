# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVertexTools package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from InDetConfig.VertexFindingFlags import VertexSortingSetup

def SumPt2VertexCollectionSortingToolCfg(flags, name="SumPt2VertexCollectionSortingTool", **kwargs):
    acc = ComponentAccumulator()

    if "VertexWeightCalculator" not in kwargs:
        from TrkConfig.TrkVertexWeightCalculatorsConfig import (
            SumPt2VertexWeightCalculatorCfg)
        kwargs.setdefault("VertexWeightCalculator", acc.popToolsAndMerge(
            SumPt2VertexWeightCalculatorCfg(flags)))

    kwargs.setdefault("decorationName", "sumPt2")

    acc.setPrivateTools(
        CompFactory.Trk.VertexCollectionSortingTool(name,**kwargs))
    return acc

def SumPtVertexCollectionSortingToolCfg(flags, name="SumPtVertexCollectionSortingTool", **kwargs):
    acc = ComponentAccumulator()
    
    if "VertexWeightCalculator" not in kwargs:
        from TrkConfig.TrkVertexWeightCalculatorsConfig import (
            SumPtVertexWeightCalculatorCfg)
        kwargs.setdefault("VertexWeightCalculator", acc.popToolsAndMerge(
            SumPtVertexWeightCalculatorCfg(flags)))

    kwargs.setdefault("decorationName", "sumPt")

    acc.setPrivateTools(
        CompFactory.Trk.VertexCollectionSortingTool(name,**kwargs))
    return acc

def VertexCollectionSortingToolCfg(flags, name="VertexCollectionSortingTool", **kwargs):
    vtxFlags = flags.ITk.PriVertex if flags.Detector.GeometryITk \
               else flags.InDet.PriVertex

    if vtxFlags.sortingSetup == VertexSortingSetup.SumPt2Sorting:
        return SumPt2VertexCollectionSortingToolCfg(flags)
    elif vtxFlags.sortingSetup == VertexSortingSetup.SumPtSorting:
        return SumPtVertexCollectionSortingToolCfg(flags)
