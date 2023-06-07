#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
### Example of configuration of the Muon Momentum corrections tool
### https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/MuonID/MuonIDAnalysis/MuonMomentumCorrections/MuonMomentumCorrections/MuonCalibTool.h
### For the official MCP recommendations please consult https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MCPAnalysisGuidelinesR22
def setupMCastToolCfg(flags, name="MuonMomentumCorrections", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("IsRun3Geo", flags.GeoModel.Run >= LHCPeriod.Run3 )
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("MuonSelectionTool", acc.popToolsAndMerge(MuonSelectionToolCfg(flags,
                                                                                     MaxEta=2.7,
                                                                                     MuQuality=1)))
    the_tool = CompFactory.CP.MuonCalibTool(name, **kwargs)
    acc.setPrivateTools(the_tool)
    return acc

def setupCalibratedMuonProviderCfg(flags, name="CalibratedMuonProvider", calibMode = 1, **kwargs):
    acc = ComponentAccumulator()    
    ### prw tool configuration
    kwargs.setdefault("useRndRunNumber", flags.Input.isMC)
    useRndNumber =  kwargs["useRndRunNumber"] if "useRndRunNumber" in kwargs else False
    kwargs.setdefault("Tool", acc.popToolsAndMerge(setupMCastToolCfg(flags,
                                                                     calibMode = calibMode,
                                                                     useRandomRunNumber=useRndNumber)))    

    the_alg = CompFactory.CP.CalibratedMuonsProvider(name,**kwargs)
    acc.addEventAlgo(the_alg, primary = True)
    return acc
def setupCalibratedTracksProviderCfg(flags, name="CalibratedMuonTracksProvider",calibMode = 1, **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("useRndRunNumber", flags.Input.isMC)
    useRndNumber =  kwargs["useRndRunNumber"] if "useRndRunNumber" in kwargs else False
    kwargs.setdefault("Tool", acc.popToolsAndMerge(setupMCastToolCfg(flags,
                                                                     calibMode = calibMode,
                                                                     useRandomRunNumber=useRndNumber)))    

    the_alg = CompFactory.CP.CalibratedTracksProvider(name,**kwargs)
    acc.addEventAlgo(the_alg, primary = True)    
    return acc