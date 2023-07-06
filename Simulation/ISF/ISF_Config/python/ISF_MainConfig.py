"""Main ISF tools configuration with ComponentAccumulator

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SimulationConfig.SimulationMetadata import writeSimulationParametersMetadata, readSimulationParameters
from ISF_Services.ISF_ServicesCoreConfig import GeoIDSvcCfg, AFIIGeoIDSvcCfg
from ISF_Services.ISF_ServicesConfig import (
    InputConverterCfg, TruthServiceCfg,
    LongLivedInputConverterCfg, AFIIParticleBrokerSvcCfg
)
from ISF_Tools.ISF_ToolsConfig import (
    ParticleKillerToolCfg, EnergyParticleOrderingToolCfg,
    ParticleOrderingToolCfg, MemoryMonitorToolCfg
)
from ISF_SimulationSelectors.ISF_SimulationSelectorsConfig import (
    DefaultAFIIGeant4SelectorCfg,
    DefaultLegacyAFIIFastCaloSimSelectorCfg,
    DefaultParticleKillerSelectorCfg,
    EtaGreater5ParticleKillerSimSelectorCfg,
    FullGeant4SelectorCfg,
    MuonAFIIGeant4SelectorCfg,
    MuonAFII_QS_Geant4SelectorCfg,
    PassBackGeant4SelectorCfg,
    DefaultFastCaloSimV2SelectorCfg,
    PionATLFAST3Geant4SelectorCfg,
    PionATLFAST3_QS_Geant4SelectorCfg,
    ProtonATLFAST3Geant4SelectorCfg,
    ProtonATLFAST3_QS_Geant4SelectorCfg,
    NeutronATLFAST3Geant4SelectorCfg,
    NeutronATLFAST3_QS_Geant4SelectorCfg,
    ChargedKaonATLFAST3Geant4SelectorCfg,
    ChargedKaonATLFAST3_QS_Geant4SelectorCfg,
    KLongATLFAST3Geant4SelectorCfg,
    KLongATLFAST3_QS_Geant4SelectorCfg,
    DefaultFatrasSelectorCfg,
    MuonFatrasSelectorCfg,
    DefaultFastCaloSimSelectorCfg
)
from ISF_Geant4Tools.ISF_Geant4ToolsConfig import (
    AFIIGeant4ToolCfg,
    AFII_QS_Geant4ToolCfg,
    FullGeant4ToolCfg,
    LongLivedGeant4ToolCfg,
    PassBackGeant4ToolCfg,
)
from ISF_Geant4CommonTools.ISF_Geant4CommonToolsConfig import (
    EntryLayerToolMTCfg,
    AFIIEntryLayerToolMTCfg
)
AthSequencer=CompFactory.AthSequencer

# MT
def Kernel_GenericSimulatorMTCfg(flags, name="ISF_Kernel_GenericSimulatorMT", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UseShadowEvent", flags.Sim.UseShadowEvent)
    if flags.Sim.UseShadowEvent and "TruthPreselectionTool" not in kwargs:
        from ISF_HepMC_Tools.ISF_HepMC_ToolsConfig import TruthPreselectionToolCfg
        kwargs.setdefault( "TruthPreselectionTool", acc.popToolsAndMerge(TruthPreselectionToolCfg(flags)) )

    if "GeoIDSvc" not in kwargs:
        kwargs.setdefault("GeoIDSvc", acc.getPrimaryAndMerge(GeoIDSvcCfg(flags)).name)

    if "TruthRecordService" not in kwargs:
        kwargs.setdefault("TruthRecordService", acc.getPrimaryAndMerge(TruthServiceCfg(flags)).name)

    if "EntryLayerTool" not in kwargs:
        entryLayerTool  = acc.popToolsAndMerge(EntryLayerToolMTCfg(flags))
        acc.addPublicTool(entryLayerTool)
        kwargs.setdefault("EntryLayerTool", acc.getPublicTool(entryLayerTool.name))

    from AthenaConfiguration.Enums import ProductionStep
    if flags.Common.ProductionStep == ProductionStep.FastChain:
        if flags.Digitization.PileUp:
            OEsvc = CompFactory.StoreGateSvc("OriginalEvent_SG")
            acc.addService(OEsvc)
            kwargs.setdefault("EvtStore", OEsvc.name) # TODO check this is correct

    kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)
    kwargs.setdefault("InputEvgenCollection", "BeamTruthEvent")
    kwargs.setdefault("OutputTruthCollection", "TruthEvent")
    from SimulationConfig.SimEnums import CalibrationRun
    if flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile]:
        # Needed to ensure that DeadMaterialCalibrationHitsMerger is scheduled correctly.
        kwargs.setdefault("ExtraOutputs", [( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitActive_DEAD' ), ( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitDeadMaterial_DEAD' ), ( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitInactive_DEAD' )])

    if flags.Sim.ISF.Simulator.isQuasiStable():
        if "QuasiStablePatcher" not in kwargs:
            from BeamEffects.BeamEffectsAlgConfig import ZeroLifetimePositionerCfg
            kwargs.setdefault("QuasiStablePatcher", acc.getPrimaryAndMerge(ZeroLifetimePositionerCfg(flags)) )
        if "InputConverter" not in kwargs:
            kwargs.setdefault("InputConverter", acc.getPrimaryAndMerge(LongLivedInputConverterCfg(flags)).name)
    elif "InputConverter" not in kwargs:
        kwargs.setdefault("InputConverter", acc.getPrimaryAndMerge(InputConverterCfg(flags)).name)

    if flags.Sim.ISF.ReSimulation:
        acc.addSequence(AthSequencer('SimSequence'), parentName='AthAlgSeq') # TODO make the name configurable?
        acc.addEventAlgo(CompFactory.ISF.SimKernelMT(name, **kwargs), 'SimSequence') # TODO make the name configurable?
    else:
        acc.addEventAlgo(CompFactory.ISF.SimKernelMT(name, **kwargs))
    return acc


def Kernel_GenericSimulatorNoG4MTCfg(flags, name="ISF_Kernel_GenericSimulatorNoG4MT", **kwargs):
    return Kernel_GenericSimulatorMTCfg(flags, name, **kwargs)


def Kernel_GenericG4OnlyMTCfg(flags, name="ISF_Kernel_GenericG4OnlyMT", **kwargs):
    acc = ComponentAccumulator()

    defaultG4SelectorRegions = set(["BeamPipeSimulationSelectors", "IDSimulationSelectors", "CaloSimulationSelectors", "MSSimulationSelectors"])
    if flags.Detector.GeometryCavern:
        # If we are simulating the cavern then we want to use the FullGeant4Selector here too
        defaultG4SelectorRegions.add("CavernSimulationSelectors")
    if defaultG4SelectorRegions - kwargs.keys(): # i.e. if any of these have not been defined yet
        tool = acc.popToolsAndMerge(FullGeant4SelectorCfg(flags))
        acc.addPublicTool(tool)
        pubTool = acc.getPublicTool(tool.name)
        # SimulationSelectors are still public ToolHandleArrays currently
        for selectorRegion in defaultG4SelectorRegions:
            kwargs.setdefault(selectorRegion, [pubTool])

    if "CavernSimulationSelectors" not in kwargs:
        tool = acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags))
        acc.addPublicTool(tool)
        pubTool = acc.getPublicTool(tool.name)
        kwargs.setdefault("CavernSimulationSelectors", [pubTool])

    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs))
    return acc


def Kernel_FullG4MTCfg(flags, name="ISF_Kernel_FullG4MT", **kwargs):
    acc = ComponentAccumulator()

    if "SimulationTools" not in kwargs:
        kwargs.setdefault("SimulationTools", [
            acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
            acc.popToolsAndMerge(FullGeant4ToolCfg(flags))
        ]) #private ToolHandleArray

    acc.merge(Kernel_GenericG4OnlyMTCfg(flags, name, **kwargs))
    return acc


def Kernel_FullG4MT_QSCfg(flags, name="ISF_Kernel_FullG4MT_QS", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("SimulationTools", [
        acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
        acc.popToolsAndMerge(LongLivedGeant4ToolCfg(flags))
    ])

    acc.merge(Kernel_GenericG4OnlyMTCfg(flags, name, **kwargs))
    return acc


def Kernel_PassBackG4MTCfg(flags, name="ISF_Kernel_PassBackG4MT", **kwargs):
    acc = ComponentAccumulator()
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs)) # Workaround

    defaultG4SelectorRegions = set(["BeamPipeSimulationSelectors", "IDSimulationSelectors", "CaloSimulationSelectors", "MSSimulationSelectors"])
    if defaultG4SelectorRegions - kwargs.keys(): # i.e. if any of these have not been defined yet
        tool = acc.popToolsAndMerge(PassBackGeant4SelectorCfg(flags))
        acc.addPublicTool(tool)
        pubTool = acc.getPublicTool(tool.name)
        kwargs.setdefault("BeamPipeSimulationSelectors", [pubTool])
        kwargs.setdefault("IDSimulationSelectors", [pubTool])
        kwargs.setdefault("CaloSimulationSelectors", [pubTool])
        kwargs.setdefault("MSSimulationSelectors", [pubTool])

    if "CavernSimulationSelectors" not in kwargs:
        tool = acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags))
        acc.addPublicTool(tool)
        pubTool = acc.getPublicTool(tool.name)
        kwargs.setdefault("CavernSimulationSelectors", [pubTool])

    if "SimulationTools" not in kwargs:
        kwargs.setdefault("SimulationTools", [
            acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
            acc.popToolsAndMerge(PassBackGeant4ToolCfg(flags))
        ])

    if "ParticleOrderingTool" not in kwargs:
        kwargs.setdefault("ParticleOrderingTool", acc.popToolsAndMerge(EnergyParticleOrderingToolCfg(flags)))

    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs))
    return acc


def Kernel_ATLFASTIIMTCfg(flags, name="ISF_Kernel_ATLFASTIIMT", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("GeoIDSvc", acc.getPrimaryAndMerge(AFIIGeoIDSvcCfg(flags)).name)
    eltool = acc.popToolsAndMerge(AFIIEntryLayerToolMTCfg(flags))
    acc.addPublicTool(eltool)
    kwargs.setdefault("EntryLayerTool"             ,   acc.getPublicTool(eltool.name)) # public ToolHandle
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs)) # Workaround

    tool = acc.popToolsAndMerge(DefaultAFIIGeant4SelectorCfg(flags))
    acc.addPublicTool(tool)
    pubTool = acc.getPublicTool(tool.name)
    kwargs.setdefault("BeamPipeSimulationSelectors", [pubTool])
    kwargs.setdefault("IDSimulationSelectors", [pubTool])
    kwargs.setdefault("MSSimulationSelectors", [pubTool])

    caloSimSelectors = []
    tool = acc.popToolsAndMerge(MuonAFIIGeant4SelectorCfg(flags))
    acc.addPublicTool(tool)
    caloSimSelectors += [acc.getPublicTool(tool.name)]
    tool = acc.popToolsAndMerge(EtaGreater5ParticleKillerSimSelectorCfg(flags))
    acc.addPublicTool(tool)
    caloSimSelectors += [acc.getPublicTool(tool.name)]
    tool = acc.popToolsAndMerge(DefaultLegacyAFIIFastCaloSimSelectorCfg(flags))
    acc.addPublicTool(tool)
    caloSimSelectors += [acc.getPublicTool(tool.name)]
    kwargs.setdefault("CaloSimulationSelectors", caloSimSelectors)

    tool = acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags))
    acc.addPublicTool(tool)
    pubTool = acc.getPublicTool(tool.name)
    kwargs.setdefault("CavernSimulationSelectors", [pubTool])
    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import LegacyAFIIFastCaloToolCfg
    kwargs.setdefault("SimulationTools", [
        acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
        acc.popToolsAndMerge(LegacyAFIIFastCaloToolCfg(flags)),
        acc.popToolsAndMerge(AFIIGeant4ToolCfg(flags))
    ])

    kwargs.setdefault("ParticleOrderingTool", acc.popToolsAndMerge(EnergyParticleOrderingToolCfg(flags)))

    # not migrated simFlags.SimulationFlavour = "ATLFASTII"
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs))
    return acc


def Kernel_ATLFASTIIFMTCfg(flags, name="ISF_Kernel_ATLFASTIIFMT", **kwargs):
    acc = ComponentAccumulator()
    acc.merge(Kernel_GenericSimulatorNoG4MTCfg(flags, name, **kwargs)) # Workaround
    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloToolBaseCfg
    from ISF_FatrasServices.ISF_FatrasConfig import fatrasTransportToolCfg
    kwargs.setdefault("SimulationTools", [
        acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
        acc.popToolsAndMerge(FastCaloToolBaseCfg(flags)),
        acc.popToolsAndMerge(fatrasTransportToolCfg(flags))
    ])

    # not migrated 'simFlags.SimulationFlavour = "ATLFASTIIF"'
    acc.merge(Kernel_GenericSimulatorNoG4MTCfg(flags, name, **kwargs))
    return acc


def Kernel_ATLFAST3MTCfg(flags, name="ISF_Kernel_ATLFAST3MT", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ParticleOrderingTool"       ,   acc.popToolsAndMerge(ParticleOrderingToolCfg(flags)))

    eltool = acc.popToolsAndMerge(AFIIEntryLayerToolMTCfg(flags))
    acc.addPublicTool(eltool)
    kwargs.setdefault("EntryLayerTool"             ,   acc.getPublicTool(eltool.name)) # public ToolHandle
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs)) # Workaround

    # BeamPipe, ID, MS Simulation Selectors
    tool = acc.popToolsAndMerge(DefaultAFIIGeant4SelectorCfg(flags))
    acc.addPublicTool(tool)
    pubTool = acc.getPublicTool(tool.name)
    kwargs.setdefault("BeamPipeSimulationSelectors", [pubTool])
    kwargs.setdefault("IDSimulationSelectors"      , [pubTool])
    kwargs.setdefault("MSSimulationSelectors"      , [pubTool])

    # CaloSimulationSelectors
    acc.addPublicTool(acc.popToolsAndMerge(MuonAFIIGeant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(EtaGreater5ParticleKillerSimSelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(PionATLFAST3Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(ProtonATLFAST3Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(NeutronATLFAST3Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(ChargedKaonATLFAST3Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(KLongATLFAST3Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(DefaultFastCaloSimV2SelectorCfg(flags)))
    kwargs.setdefault("CaloSimulationSelectors"    , [ acc.getPublicTool("ISF_MuonAFIIGeant4Selector"),
                                                       acc.getPublicTool("ISF_EtaGreater5ParticleKillerSimSelector"),
                                                       acc.getPublicTool("ISF_PionATLFAST3Geant4Selector"),
                                                       acc.getPublicTool("ISF_ProtonATLFAST3Geant4Selector"),
                                                       acc.getPublicTool("ISF_NeutronATLFAST3Geant4Selector"),
                                                       acc.getPublicTool("ISF_ChargedKaonATLFAST3Geant4Selector"),
                                                       acc.getPublicTool("ISF_KLongATLFAST3Geant4Selector"),
                                                       acc.getPublicTool("ISF_DefaultFastCaloSimV2Selector") ])
    # CavernSimulationSelectors
    acc.addPublicTool(acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags)))
    kwargs.setdefault("CavernSimulationSelectors"  , [ acc.getPublicTool("ISF_DefaultParticleKillerSelector") ])
    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloSimV2ToolCfg
    kwargs.setdefault("SimulationTools"            , [ acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
                                                       acc.popToolsAndMerge(FastCaloSimV2ToolCfg(flags)),
                                                       acc.popToolsAndMerge(AFIIGeant4ToolCfg(flags)) ])
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs))
    return acc


def Kernel_ATLFAST3MT_QSCfg(flags, name="ISF_Kernel_ATLFAST3MT_QS", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ParticleOrderingTool"       ,   acc.popToolsAndMerge(ParticleOrderingToolCfg(flags)))

    tool = acc.popToolsAndMerge(AFIIEntryLayerToolMTCfg(flags))
    acc.addPublicTool(tool)
    kwargs.setdefault("EntryLayerTool"             ,   acc.getPublicTool(tool.name)) # public ToolHandle
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs)) # Workaround

    # BeamPipe, ID, MS Simulation Selectors
    tool = acc.popToolsAndMerge(DefaultAFIIGeant4SelectorCfg(flags))
    acc.addPublicTool(tool)
    pubTool = acc.getPublicTool(tool.name)
    kwargs.setdefault("BeamPipeSimulationSelectors", [pubTool])
    kwargs.setdefault("IDSimulationSelectors"      , [pubTool])
    kwargs.setdefault("MSSimulationSelectors"      , [pubTool])

    # CaloSimulationSelectors
    acc.addPublicTool(acc.popToolsAndMerge(MuonAFII_QS_Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(EtaGreater5ParticleKillerSimSelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(PionATLFAST3_QS_Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(ProtonATLFAST3_QS_Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(NeutronATLFAST3_QS_Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(ChargedKaonATLFAST3_QS_Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(KLongATLFAST3_QS_Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(DefaultFastCaloSimV2SelectorCfg(flags)))
    kwargs.setdefault("CaloSimulationSelectors"    , [ acc.getPublicTool("ISF_MuonAFII_QS_Geant4Selector"),
                                                       acc.getPublicTool("ISF_EtaGreater5ParticleKillerSimSelector"),
                                                       acc.getPublicTool("ISF_PionATLFAST3_QS_Geant4Selector"),
                                                       acc.getPublicTool("ISF_ProtonATLFAST3_QS_Geant4Selector"),
                                                       acc.getPublicTool("ISF_NeutronATLFAST3_QS_Geant4Selector"),
                                                       acc.getPublicTool("ISF_ChargedKaonATLFAST3_QS_Geant4Selector"),
                                                       acc.getPublicTool("ISF_KLongATLFAST3_QS_Geant4Selector"),
                                                       acc.getPublicTool("ISF_DefaultFastCaloSimV2Selector") ])
    # CavernSimulationSelectors
    acc.addPublicTool(acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags)))
    kwargs.setdefault("CavernSimulationSelectors"  , [ acc.getPublicTool("ISF_DefaultParticleKillerSelector") ])
    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloSimV2ToolCfg
    kwargs.setdefault("SimulationTools"            , [ acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
                                                       acc.popToolsAndMerge(FastCaloSimV2ToolCfg(flags)),
                                                       acc.popToolsAndMerge(AFII_QS_Geant4ToolCfg(flags)) ])
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs))
    return acc


def Kernel_GenericSimulatorCfg(flags, name="ISF_Kernel_GenericSimulator", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UseShadowEvent", flags.Sim.UseShadowEvent)
    if flags.Sim.UseShadowEvent and "TruthPreselectionTool" not in kwargs:
        from ISF_HepMC_Tools.ISF_HepMC_ToolsConfig import TruthPreselectionToolCfg
        kwargs.setdefault( "TruthPreselectionTool", acc.popToolsAndMerge(TruthPreselectionToolCfg(flags)) )

    if "TruthRecordService" not in kwargs:
        kwargs.setdefault("TruthRecordService", acc.getPrimaryAndMerge(TruthServiceCfg(flags)).name)

    if "MemoryMonitoringTool" not in kwargs:
        tool = acc.popToolsAndMerge(MemoryMonitorToolCfg(flags))
        acc.addPublicTool(tool)
        pubTool = acc.getPublicTool(tool.name)
        kwargs.setdefault("MemoryMonitoringTool", pubTool)

    if "ParticleBroker" not in kwargs:
        kwargs.setdefault("ParticleBroker", acc.getPrimaryAndMerge(AFIIParticleBrokerSvcCfg(flags)).name)

    if flags.Sim.ISF.Simulator.isQuasiStable():
        if "QuasiStablePatcher" not in kwargs:
            from BeamEffects.BeamEffectsAlgConfig import ZeroLifetimePositionerCfg
            kwargs.setdefault("QuasiStablePatcher", acc.getPrimaryAndMerge(ZeroLifetimePositionerCfg(flags)) )
        if "InputConverter" not in kwargs:
            kwargs.setdefault("InputConverter", acc.getPrimaryAndMerge(LongLivedInputConverterCfg(flags)).name)
    elif "InputConverter" not in kwargs:
        kwargs.setdefault("InputConverter", acc.getPrimaryAndMerge(InputConverterCfg(flags)).name)

    kwargs.setdefault("InputHardScatterCollection", "BeamTruthEvent")
    kwargs.setdefault("OutputHardScatterTruthCollection", "TruthEvent")
    kwargs.setdefault("DoCPUMonitoring", flags.Sim.ISF.DoTimeMonitoring)
    kwargs.setdefault("DoMemoryMonitoring", flags.Sim.ISF.DoMemoryMonitoring)

    if flags.Sim.ISF.ReSimulation:
        acc.addSequence(AthSequencer('SimSequence'), parentName='AthAlgSeq') # TODO make the name configurable?
        acc.addEventAlgo(CompFactory.ISF.SimKernel(name, **kwargs), 'SimSequence') # TODO make the name configurable?
    else:
        acc.addEventAlgo(CompFactory.ISF.SimKernel(name, **kwargs))
    return acc


def Kernel_ATLFASTIIF_G4MSCfg(flags, name="ISF_Kernel_ATLFASTIIF_G4MS", **kwargs):
    acc = ComponentAccumulator()
    acc.merge(Kernel_GenericSimulatorCfg(flags, name, **kwargs)) # Force the SimKernel to be before the CollectionMerger by adding it here
    acc.addPublicTool(acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(DefaultFatrasSelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(MuonFatrasSelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(EtaGreater5ParticleKillerSimSelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(DefaultFastCaloSimSelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(DefaultAFIIGeant4SelectorCfg(flags)))

    kwargs.setdefault("BeamPipeSimulationSelectors", [ acc.getPublicTool("ISF_DefaultParticleKillerSelector") ])
    kwargs.setdefault("IDSimulationSelectors"      , [ acc.getPublicTool("ISF_DefaultFatrasSelector") ])
    kwargs.setdefault("CaloSimulationSelectors"    , [ acc.getPublicTool("ISF_MuonFatrasSelector"),
                                                       acc.getPublicTool("ISF_EtaGreater5ParticleKillerSimSelector"),
                                                       acc.getPublicTool("ISF_DefaultFastCaloSimSelector") ])
    kwargs.setdefault("MSSimulationSelectors"      , [ acc.getPublicTool("ISF_DefaultAFIIGeant4Selector") ])
    kwargs.setdefault("CavernSimulationSelectors"  , [ acc.getPublicTool("ISF_DefaultParticleKillerSelector") ])
    # not migrated simFlags.SimulationFlavour = "ATLFASTIIF_MS"
    # simFlags.SimulationFlavour = "ATLFASTIIF_G4MS"

    acc.merge(Kernel_GenericSimulatorCfg(flags, name, **kwargs)) # Merge properly configured SimKernel here and let deduplication sort it out.
    return acc


def Kernel_ATLFAST3F_G4MSCfg(flags, name="ISF_Kernel_ATLFAST3F_G4MS", **kwargs):
    acc = ComponentAccumulator()
    acc.merge(Kernel_GenericSimulatorCfg(flags, name, **kwargs)) # Force the SimKernel to be before the CollectionMerger by adding it here
    acc.addPublicTool(acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(DefaultFatrasSelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(EtaGreater5ParticleKillerSimSelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(DefaultFastCaloSimV2SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(DefaultAFIIGeant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(MuonAFIIGeant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(PionATLFAST3Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(ProtonATLFAST3Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(NeutronATLFAST3Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(ChargedKaonATLFAST3Geant4SelectorCfg(flags)))
    acc.addPublicTool(acc.popToolsAndMerge(KLongATLFAST3Geant4SelectorCfg(flags)))

    kwargs.setdefault("BeamPipeSimulationSelectors", [ acc.getPublicTool("ISF_DefaultParticleKillerSelector") ])
    kwargs.setdefault("IDSimulationSelectors"      , [ acc.getPublicTool("ISF_DefaultFatrasSelector") ])
    kwargs.setdefault("CaloSimulationSelectors"    , [ acc.getPublicTool("ISF_MuonAFIIGeant4Selector"),
                                                       acc.getPublicTool("ISF_EtaGreater5ParticleKillerSimSelector"),
                                                       acc.getPublicTool("ISF_PionATLFAST3Geant4Selector"),
                                                       acc.getPublicTool("ISF_ProtonATLFAST3Geant4Selector"),
                                                       acc.getPublicTool("ISF_NeutronATLFAST3Geant4Selector"),
                                                       acc.getPublicTool("ISF_ChargedKaonATLFAST3Geant4Selector"),
                                                       acc.getPublicTool("ISF_KLongATLFAST3Geant4Selector"),
                                                       acc.getPublicTool("ISF_DefaultFastCaloSimV2Selector") ])
    kwargs.setdefault("MSSimulationSelectors"      , [ acc.getPublicTool("ISF_DefaultAFIIGeant4Selector") ])
    kwargs.setdefault("CavernSimulationSelectors"  , [ acc.getPublicTool("ISF_DefaultParticleKillerSelector") ])
    #simFlags.SimulationFlavour = "ATLFAST3F_G4MS" # not migrated

    acc.merge(Kernel_GenericSimulatorCfg(flags, name, **kwargs)) # Merge properly configured SimKernel here and let deduplication sort it out.
    return acc


def ISF_KernelCfg(flags):
    cfg = ComponentAccumulator()
    # Write MetaData container
    cfg.merge(writeSimulationParametersMetadata(flags))
    # Also allow reading it
    cfg.merge(readSimulationParameters(flags))  # for FileMetaData creation

    from SimulationConfig.SimEnums import SimulationFlavour
    if flags.Sim.ISF.Simulator is SimulationFlavour.FullG4MT:
        cfg.merge(Kernel_FullG4MTCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.FullG4MT_QS:
        cfg.merge(Kernel_FullG4MT_QSCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.PassBackG4MT:
        cfg.merge(Kernel_PassBackG4MTCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.ATLFASTIIMT:
        cfg.merge(Kernel_ATLFASTIIMTCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.ATLFASTIIFMT:
        cfg.merge(Kernel_ATLFASTIIFMTCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.ATLFASTIIF_G4MS:
        cfg.merge(Kernel_ATLFASTIIF_G4MSCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.ATLFAST3MT:
        cfg.merge(Kernel_ATLFAST3MTCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.ATLFAST3MT_QS:
        cfg.merge(Kernel_ATLFAST3MT_QSCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.ATLFAST3F_G4MS:
        cfg.merge(Kernel_ATLFAST3F_G4MSCfg(flags))
    else:
        raise ValueError('Unknown Simulator set, bailing out')
    
    return cfg
