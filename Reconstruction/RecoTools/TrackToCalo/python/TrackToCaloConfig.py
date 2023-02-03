# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = "Tool configuration for the track to calo tools."

#---------------------------------------
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ParticleCaloExtensionToolCfg(flags, name='ParticleCaloExtensionTool', **kwargs):
    acc=ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs["Extrapolator"] = acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags))

    acc.setPrivateTools(
        CompFactory.Trk.ParticleCaloExtensionTool(name, **kwargs))
    return acc

def HLTPF_ParticleCaloExtensionToolCfg(flags, name='HLTPF_ParticleCaloExtension', **kwargs):
    acc=ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs["Extrapolator"] = acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags))

    acc.setPrivateTools(
        CompFactory.Trk.ParticleCaloExtensionTool(name, **kwargs))
    return acc

def ParticleCaloCellAssociationToolCfg(flags, name='ParticleCaloCellAssociationTool', **kwargs):
    acc=ComponentAccumulator()

    if "ParticleCaloExtensionTool" not in kwargs:
        kwargs["ParticleCaloExtensionTool"] = acc.popToolsAndMerge(
            ParticleCaloExtensionToolCfg(flags))

    # should this be a more global flag? It depends on whether you are in AOD
    kwargs.setdefault("CaloCellContainer", flags.Egamma.Keys.Input.CaloCells)

    acc.setPrivateTools(
        CompFactory.Rec.ParticleCaloCellAssociationTool(name, **kwargs))
    return acc

def HLTPF_ParticleCaloCellAssociationToolCfg(flags, name='HLTPF_ParticleCaloCellAssociationTool', **kwargs):
    acc=ComponentAccumulator()

    if "ParticleCaloExtensionTool" not in kwargs:
        kwargs["ParticleCaloExtensionTool"] = acc.popToolsAndMerge(
            HLTPF_ParticleCaloExtensionToolCfg(flags))

    kwargs.setdefault("CaloCellContainer", "")

    acc.setPrivateTools(
        CompFactory.Rec.ParticleCaloCellAssociationTool(name, **kwargs))
    return acc
