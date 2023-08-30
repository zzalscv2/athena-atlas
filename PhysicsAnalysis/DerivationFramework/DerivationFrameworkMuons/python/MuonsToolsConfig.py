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
### Algorithm that decorates the calorimeter deposits in form of 3 vectors to the
### muon. The deposits are used to identify the track as CT muon
def MuonCaloDepositAlgCfg(ConfigFlags, name= "MuonCaloDepositAlg", **kwargs):
    acc = ComponentAccumulator()
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import TrackDepositInCaloToolCfg
    kwargs.setdefault("TrackDepositInCaloTool", acc.popToolsAndMerge(TrackDepositInCaloToolCfg(ConfigFlags)))
    the_alg = CompFactory.DerivationFramework.IDTrackCaloDepositsDecoratorAlg(name, **kwargs)
    acc.addEventAlgo(the_alg, primary = True)
    return acc

### Algorithm used to thin bad muons from the analysis stream
def AnalysisMuonThinningAlgCfg(ConfigFlags, name="AnalysisMuonThinningAlg", **kwargs):
    acc = ComponentAccumulator()
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("SelectionTool", acc.popToolsAndMerge(MuonSelectionToolCfg(ConfigFlags,
                                                            name="MuonSelThinningTool")))
    the_alg = CompFactory.DerivationFramework.AnalysisMuonThinningAlg(name, **kwargs)
    acc.addEventAlgo(the_alg, primary = True)
    return acc

### Di-muon tagging tool, for T&P studies
def DiMuonTaggingAlgCfg(ConfigFlags, name="DiMuonTaggingTool", **kwargs):
    acc = ComponentAccumulator()
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    kwargs.setdefault("TrigDecisionTool",  acc.getPrimaryAndMerge(TrigDecisionToolCfg(ConfigFlags)))
    kwargs.setdefault("isMC", ConfigFlags.Input.isMC)
    the_alg = CompFactory.DerivationFramework.DiMuonTaggingAlg(name, **kwargs)
    acc.addEventAlgo(the_alg, primary = True)
    return acc

