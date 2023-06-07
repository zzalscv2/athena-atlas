"""ComponentAccumulator tool configuration for ISF

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import MeV


def ParticleHelperCfg(flags, name="ISF_ParticleHelper", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ISF.ParticleHelper(name, **kwargs))
    return acc


def MemoryMonitorToolCfg(flags, name="ISF_MemoryMonitor", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ISF.MemoryMonitoringTool(name, **kwargs))
    return acc


def EntryLayerFilterCfg(flags, **kwargs):
    """Return the MCxEntryLayerFilterCfg config flagged by Sim.TruthStrategy"""
    from SimulationConfig.SimEnums import TruthStrategy
    stratmap = {
        TruthStrategy.MC12: MC12EntryLayerFilterCfg,
        TruthStrategy.MC12LLP: MC12LLPEntryLayerFilterCfg,
        TruthStrategy.MC12Plus: MC12PlusEntryLayerFilterCfg,
        TruthStrategy.MC15: MC15EntryLayerFilterCfg,
        TruthStrategy.MC15a: MC15aEntryLayerFilterCfg,
        TruthStrategy.MC15aPlus: MC15aPlusEntryLayerFilterCfg,
        TruthStrategy.MC15aPlusLLP: MC15aPlusLLPEntryLayerFilterCfg,
        TruthStrategy.MC16: MC16EntryLayerFilterCfg,
        TruthStrategy.MC16LLP: MC16LLPEntryLayerFilterCfg,
        TruthStrategy.MC18: MC18EntryLayerFilterCfg,
        TruthStrategy.MC18LLP: MC18LLPEntryLayerFilterCfg,
        TruthStrategy.Validation: ValidationEntryLayerFilterCfg,
        # TruthStrategy.PhysicsProcess: PhysicsProcessTruthServiceCfg,
        # TruthStrategy.Global: GlobalTruthServiceCfg,
        TruthStrategy.Cosmic: CosmicEventFilterToolCfg,
    }
    MCxCfg = stratmap[flags.Sim.TruthStrategy]
    return MCxCfg(flags, **kwargs)


def MC12EntryLayerFilterCfg(flags, name="ISF_MC12EntryLayerFilter", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("AllowOnlyDefinedBarcodes", True)
    kwargs.setdefault("AllowOnlyLegacyPrimaries", False)
    acc.setPrivateTools(CompFactory.ISF.GenericBarcodeFilter(name, **kwargs))
    return acc


def MC12LLPEntryLayerFilterCfg(flags, name="ISF_MC12LLPEntryLayerFilter", **kwargs):
    return MC12EntryLayerFilterCfg(flags, name, **kwargs)


def MC12PlusEntryLayerFilterCfg(flags, name="ISF_MC12PlusEntryLayerFilter", **kwargs):
    return MC12EntryLayerFilterCfg(flags, name, **kwargs)


def MC15EntryLayerFilterCfg(flags, name="ISF_MC15EntryLayerFilter", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("MinEkinCharged", 100.*MeV)
    kwargs.setdefault("MinEkinNeutral", -1.)
    acc.setPrivateTools(CompFactory.ISF.EntryLayerFilter(name, **kwargs))
    return acc


def MC15aEntryLayerFilterCfg(flags, name="ISF_MC15aEntryLayerFilter", **kwargs):
    return MC15EntryLayerFilterCfg(flags, name, **kwargs)


def MC15aPlusEntryLayerFilterCfg(flags, name="ISF_MC15aPlusEntryLayerFilter", **kwargs):
    return MC15EntryLayerFilterCfg(flags, name, **kwargs)


def MC15aPlusLLPEntryLayerFilterCfg(flags, name="ISF_MC15aPlusLLPEntryLayerFilter", **kwargs):
    return MC15aPlusEntryLayerFilterCfg(flags, name, **kwargs)


def MC16EntryLayerFilterCfg(flags, name="ISF_MC16EntryLayerFilter", **kwargs):
    return MC15aPlusEntryLayerFilterCfg(flags, name, **kwargs)


def MC16LLPEntryLayerFilterCfg(flags, name="ISF_MC16LLPEntryLayerFilter", **kwargs):
    return MC15aPlusLLPEntryLayerFilterCfg(flags, name, **kwargs)


def MC18EntryLayerFilterCfg(flags, name="ISF_MC18EntryLayerFilter", **kwargs):
    return MC15aPlusEntryLayerFilterCfg(flags, name, **kwargs)


def MC18LLPEntryLayerFilterCfg(flags, name="ISF_MC18LLPEntryLayerFilter", **kwargs):
    return MC15aPlusLLPEntryLayerFilterCfg(flags, name, **kwargs)


def ValidationEntryLayerFilterCfg(flags, name="ISF_ValidationEntryLayerFilter", **kwargs):
    return MC12EntryLayerFilterCfg(flags, name, **kwargs)


def CosmicEventFilterToolCfg(flags, name="ISF_CosmicEventFilter", **kwargs):
    from G4CosmicFilter.G4CosmicFilterConfig import configCosmicFilterVolumeNames
    acc = ComponentAccumulator()
    volumeNames = configCosmicFilterVolumeNames(flags)
    kwargs.setdefault("UseAndFilter", len(volumeNames)<3 )
    kwargs.setdefault("VolumeNames", volumeNames)
    if flags.Sim.CosmicFilterID:
        kwargs.setdefault("PDG_ID", flags.Sim.CosmicFilterID)
    if flags.Sim.CosmicFilterPTmin:
        kwargs.setdefault("ptMin", flags.Sim.CosmicFilterPTmin)
    if flags.Sim.CosmicFilterPTmax:
        kwargs.setdefault("ptMax", flags.Sim.CosmicFilterPTmax)
    acc.setPrivateTools(CompFactory.ISF.CosmicEventFilterTool(name, **kwargs))
    return acc


def StoppedParticleFilterToolCfg(flags, name="ISF_StoppedParticleFilter", **kwargs):
    kwargs.setdefault("VolumeNames", ["StoppingPositions"])
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ISF.CosmicEventFilterTool(name, **kwargs))
    return acc


def InToOutSubDetOrderingToolCfg(flags, name="ISF_InToOutSubDetOrderingTool", **kwargs):
    # higher ordered particles will be simulated first
    kwargs.setdefault("OrderID"      , 100000000)
    kwargs.setdefault("OrderBeamPipe", 1000000  )
    kwargs.setdefault("OrderCalo"    , 10000    )
    kwargs.setdefault("OrderMS"      , 100      )
    kwargs.setdefault("OrderCavern"  , 1        )
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ISF.GenericParticleOrderingTool(name, **kwargs))
    return acc


def ParticleOrderingToolCfg(flags, name="ISF_ParticleOrderingTool", **kwargs):
    kwargs.setdefault("OrderID"      , 1)
    kwargs.setdefault("OrderBeamPipe", 1)
    kwargs.setdefault("OrderCalo"    , 1)
    kwargs.setdefault("OrderMS"      , 1)
    kwargs.setdefault("OrderCavern"  , 1)
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ISF.GenericParticleOrderingTool(name, **kwargs))
    return acc


def EnergyParticleOrderingToolCfg(flags, name="ISF_EnergyParticleOrderingTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ISF.EnergyParticleOrderingTool(name, **kwargs))
    return acc


def ParticleKillerToolCfg(flags, name="ISF_ParticleKillerTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ISF.ParticleKillerSimTool(name, **kwargs))
    return acc
