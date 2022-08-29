#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def EgammaCalibToolCfg(flags, name ="EgammaCalibrationAndSmearingTool", **kwargs ):
    acc = ComponentAccumulator()
    kwargs.setdefault("decorrelationModel", "1NP_v1" )
    kwargs.setdefault("useAFII", flags.Input.isMC and  not flags.Sim.ISF.Simulator.isFullSim() )
    kwargs.setdefault("ESModel", "es2018_R21_v1")
    the_tool = CompFactory.CP.EgammaCalibrationAndSmearingTool(name, **kwargs)
    acc.setPrivateTools(the_tool)    
    return acc

def setupEgammaCalibProviderCfg(flags, name="EgammaCalibProvider", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("Tool", acc.popToolsAndMerge(EgammaCalibToolCfg(flags)))
    the_alg = CompFactory.CP.CalibratedEgammaProvider(name, **kwargs)
    acc.addEventAlgo(the_alg, primary =True)
    return acc
