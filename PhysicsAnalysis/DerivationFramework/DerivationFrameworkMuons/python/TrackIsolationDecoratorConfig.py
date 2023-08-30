# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Import the xAOD isolation parameters.
from xAODPrimitives.xAODIso import xAODIso as isoPar
deco_ptcones = [isoPar.ptcone40, isoPar.ptcone30]
deco_topoetcones = [isoPar.topoetcone40, isoPar.topoetcone20]
deco_prefix = 'MUON_'

def TrackIsolationToolCfg(ConfigFlags,name= "TrackIsolationTool", **kwargs):

    acc = ComponentAccumulator()
    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_TrackTools_Cfg
    kwargs.setdefault("TrackSelectionTool",acc.popToolsAndMerge(InDetTrackSelectionTool_TrackTools_Cfg(ConfigFlags,
                                                                                     maxZ0SinTheta = 3.,
                                                                                     minPt         = 1000.,
                                                                                     CutLevel      = "Loose")))
       
    TrackIsoTool = CompFactory.xAOD.TrackIsolationTool(name, **kwargs)
    acc.setPrivateTools(TrackIsoTool)
    return acc

def MUON1IDTrackDecoratorCfg(ConfigFlags, name, **kwargs):

    acc = ComponentAccumulator()
    kwargs.setdefault('TrackIsolationTool',  acc.getPrimaryAndMerge(TrackIsolationToolCfg(ConfigFlags)))
    from IsolationAlgs.IsoToolsConfig import MuonCaloIsolationToolCfg
    kwargs.setdefault('CaloIsolationTool',  acc.getPrimaryAndMerge(MuonCaloIsolationToolCfg(ConfigFlags)))
    kwargs.setdefault('TargetContainer', "InDetTrackParticles")
    kwargs.setdefault('SelectionString', "")
    kwargs.setdefault('SelectionFlag', "MUON1DIMU_Status")
    kwargs.setdefault('SelectionFlagValue', 1000)
    kwargs.setdefault('ptcones', deco_ptcones)
    kwargs.setdefault('topoetcones', deco_topoetcones)
    kwargs.setdefault('Prefix', deco_prefix)

    MUON1IDTrackDecorator = CompFactory.DerivationFramework.isolationDecorator(name = "MUON1IDTrackDecorator", **kwargs)

    acc.addPublicTool(MUON1IDTrackDecorator, primary=True)
    return acc


def MUON1MSTrackDecoratorCfg(ConfigFlags, name, **kwargs):

    acc = ComponentAccumulator()
    kwargs.setdefault('TrackIsolationTool',  acc.getPrimaryAndMerge(TrackIsolationToolCfg(ConfigFlags)))
    from IsolationAlgs.IsoToolsConfig import MuonCaloIsolationToolCfg
    kwargs.setdefault('CaloIsolationTool',  acc.getPrimaryAndMerge(MuonCaloIsolationToolCfg(ConfigFlags)))
    kwargs.setdefault('TargetContainer', "ExtrapolatedMuonTrackParticles")
    kwargs.setdefault('SelectionString', "")
    kwargs.setdefault('SelectionFlag', "")
    kwargs.setdefault('SelectionFlagValue', 1000)
    kwargs.setdefault('ptcones', deco_ptcones)
    kwargs.setdefault('topoetcones', deco_topoetcones)
    kwargs.setdefault('Prefix', deco_prefix)

    MUON1MSTrackDecorator = CompFactory.DerivationFramework.isolationDecorator(name = "MUON1MSTrackDecorator", **kwargs)

    acc.addPublicTool(MUON1MSTrackDecorator, primary = True)
    return acc
