# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVertexWeightCalculators package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SumPt2VertexWeightCalculatorCfg(flags, name="SumPt2VertexWeightCalculator",
                                    **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("DoSumPt2Selection", True)
    acc.setPrivateTools(
        CompFactory.Trk.SumPtVertexWeightCalculator(name, **kwargs))
    return acc

def SumPtVertexWeightCalculatorCfg(flags, name="SumPtVertexWeightCalculator",
                                   **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("DoSumPt2Selection", False)
    acc.setPrivateTools(
        CompFactory.Trk.SumPtVertexWeightCalculator(name, **kwargs))
    return acc
