# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonJetDrToolCfg(ConfigFlags, name):
    acc = ComponentAccumulator()
    muonJetDrTool = CompFactory.DerivationFramework.MuonJetDrTool(name)
    acc.addPublicTool(muonJetDrTool, primary=True)
    return acc

### Configuration for the MuonTPExtrapolation tool
def MuonTPExtrapolationToolCfg(ConfigFlags, name = "MuonTPExtrapolationTool", **kwargs):
    acc= ComponentAccumulator()
    from TrkConfig.AtlasExtrapolatorConfig import MuonExtrapolatorCfg
    kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(MuonExtrapolatorCfg(ConfigFlags)))
    the_tool = CompFactory.MuonTPExtrapolationTool(name = name,
                                                    **kwargs)
    acc.setPrivateTools(the_tool)
    return acc
def MuonCaloDepositAlgCfg(ConfigFlags, name= "MuonCaloDepositAlg", **kwargs):
    acc = ComponentAccumulator()
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import TrackDepositInCaloToolCfg
    kwargs.setdefault("TrackDepositInCaloTool", acc.popToolsAndMerge(TrackDepositInCaloToolCfg(ConfigFlags)))
    the_alg = CompFactory.DerivationFramework.IDTrackCaloDepositsDecoratorAlg(name, **kwargs)
    acc.addEventAlgo(the_alg, primary = True)
    return acc