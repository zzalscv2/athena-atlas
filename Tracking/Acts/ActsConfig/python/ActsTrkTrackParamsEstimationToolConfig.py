#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsInterop import UnitConstants

def TrackParamsEstimationToolCfg(ConfigFlags,
                                 name: str = "TrackParamsEstimationTool",
                                 **kwargs) -> ComponentAccumulator:
    assert isinstance(name, str)

    acc = ComponentAccumulator()

    kwargs.setdefault('bFieldMin', 0.1 * UnitConstants.T)
    kwargs.setdefault('sigmaLoc0', 25 * UnitConstants.um)
    kwargs.setdefault('sigmaLoc1', 100 * UnitConstants.um)
    kwargs.setdefault('sigmaPhi', 0.02 * UnitConstants.degree)
    kwargs.setdefault('sigmaTheta', 0.02 * UnitConstants.degree)
    kwargs.setdefault('sigmaQOverP', 0.1 / UnitConstants.GeV)
    kwargs.setdefault('sigmaT0', 1400 * UnitConstants.s)
    # eBoundLoc0, eBoundLoc1, eBoundPhi, eBoundTheta, eBoundQOverP, eBoundTime
    kwargs.setdefault('initialVarInflation', [1., 1., 1., 1., 1., 1.])

    acc.setPrivateTools(CompFactory.ActsTrk.TrackParamsEstimationTool(name=name, **kwargs))
    return acc
