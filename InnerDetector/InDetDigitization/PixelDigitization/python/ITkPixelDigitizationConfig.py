"""Define methods to construct configured ITk Pixel Digitization tools and algorithms

Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from Digitization.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg
from Digitization.PileUpToolsConfig import PileUpToolsCfg
from Digitization.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from PixelConditionsAlgorithms.ITkPixelConditionsConfig import (
    ITkPixelModuleConfigCondAlgCfg, ITkPixelChargeCalibCondAlgCfg,
    ITkPixelDistortionAlgCfg
)
from PixelConditionsTools.ITkPixelConditionsSummaryConfig import ITkPixelConditionsSummaryCfg
from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
from SiPropertiesTool.ITkPixelSiPropertiesConfig import ITkPixelSiPropertiesToolCfg
from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType


# The earliest and last bunch crossing times for which interactions will be sent
# to the ITk Pixel Digitization code
def ITkPixel_FirstXing(flags):
    return -25


def ITkPixel_LastXing(flags):
    if flags.Beam.BunchSpacing > 50:
        return 75
    else:
        return 25


def ITkEnergyDepositionToolCfg(flags, name="ITkEnergyDepositionTool", **kwargs):
    """Return a configured EnergyDepositionTool"""
    acc = ITkPixelDistortionAlgCfg(flags)
    kwargs.setdefault("DeltaRayCut", 80.7687)
    kwargs.setdefault("nCols", 5)
    kwargs.setdefault("LoopLimit", 100000)
    kwargs.setdefault("doBichsel", True)
    kwargs.setdefault("doBichselBetaGammaCut", 0.7) # dEdx not quite consistent below this
    kwargs.setdefault("doDeltaRay", False)          # needs validation
    kwargs.setdefault("doPU", False)
    kwargs.setdefault("PixelDistortionData", "ITkPixelDistortionData")
    EnergyDepositionTool = CompFactory.EnergyDepositionTool
    acc.setPrivateTools(EnergyDepositionTool(name, **kwargs))
    return acc


def BarrelRD53SimToolCfg(flags, name="BarrelRD53SimTool", **kwargs):
    """Return a RD53SimTool configured for Barrel"""
    acc = ITkPixelReadoutManagerCfg(flags)
    acc.merge(ITkPixelModuleConfigCondAlgCfg(flags))
    acc.merge(ITkPixelChargeCalibCondAlgCfg(flags))
    kwargs.setdefault("BarrelEC", 0)
    kwargs.setdefault("DoNoise", flags.Digitization.DoInnerDetectorNoise)
    kwargs.setdefault("PixelReadoutManager", acc.getPrimary())
    kwargs.setdefault("PixelModuleData", "ITkPixelModuleData")
    kwargs.setdefault("PixelChargeCalibCondData", "ITkPixelChargeCalibCondData")
    kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(ITkPixelConditionsSummaryCfg(flags)))
    kwargs.setdefault("DoTimeWalk", False) #Set this to true to enable Timewalk effects
    acc.setPrivateTools(CompFactory.RD53SimTool(name, **kwargs))
    return acc


def EndcapRD53SimToolCfg(flags, name="EndcapRD53SimTool", **kwargs):
    """Return a RD53SimTool configured for Endcap"""
    acc = ITkPixelReadoutManagerCfg(flags)
    acc.merge(ITkPixelModuleConfigCondAlgCfg(flags))
    acc.merge(ITkPixelChargeCalibCondAlgCfg(flags))
    kwargs.setdefault("BarrelEC", 2)
    kwargs.setdefault("DoNoise", flags.Digitization.DoInnerDetectorNoise)
    kwargs.setdefault("PixelReadoutManager", acc.getPrimary())
    kwargs.setdefault("PixelModuleData", "ITkPixelModuleData")
    kwargs.setdefault("PixelChargeCalibCondData", "ITkPixelChargeCalibCondData")
    kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(ITkPixelConditionsSummaryCfg(flags)))
    kwargs.setdefault("DoTimeWalk", False) #Set this to true to enable Timewalk effects
    RD53SimTool = CompFactory.RD53SimTool
    acc.setPrivateTools(RD53SimTool(name, **kwargs))
    return acc


def ITkSensorSimPlanarToolCfg(flags, name="ITkSensorSimPlanarTool", **kwargs):
    """Return ComponentAccumulator with configured SensorSimPlanarTool for ITk"""
    acc = ITkPixelModuleConfigCondAlgCfg(flags)
    kwargs.setdefault("SiPropertiesTool", acc.popToolsAndMerge(ITkPixelSiPropertiesToolCfg(flags)))
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags)))
    kwargs.setdefault("PixelModuleData", "ITkPixelModuleData")
    kwargs.setdefault("RadiationDamageSimulationType", flags.Digitization.PixelPlanarRadiationDamageSimulationType.value)
    kwargs.setdefault("IsITk", True)
    if flags.Digitization.PixelPlanarRadiationDamageSimulationType is not PixelRadiationDamageSimulationType.NoRadiationDamage:
        # acc.merge(ITkPixelRadSimFluenceMapAlgCfg(flags))  # TODO: not supported yet
        pass
    acc.setPrivateTools(CompFactory.SensorSimPlanarTool(name, **kwargs))
    return acc


def ITkSensorSim3DToolCfg(flags, name="ITkSensorSim3DTool", **kwargs):
    """Return ComponentAccumulator with configured SensorSim3DTool for ITk"""
    acc = ITkPixelModuleConfigCondAlgCfg(flags)
    kwargs.setdefault("SiPropertiesTool", acc.popToolsAndMerge(ITkPixelSiPropertiesToolCfg(flags)))
    kwargs.setdefault("PixelModuleData", "ITkPixelModuleData")
    kwargs.setdefault("RadiationDamageSimulationType", flags.Digitization.Pixel3DRadiationDamageSimulationType.value)
    kwargs.setdefault("IsITk", True)
    if flags.Digitization.Pixel3DRadiationDamageSimulationType is not PixelRadiationDamageSimulationType.NoRadiationDamage:
        # acc.merge(ITkPixelRadSimFluenceMapAlgCfg(flags))  # TODO: not supported yet
        pass
    acc.setPrivateTools(CompFactory.SensorSim3DTool(name, **kwargs))
    return acc


def ITkPixelDigitizationBasicToolCfg(flags, name="ITkPixelDigitizationBasicTool", **kwargs):
    """Return ComponentAccumulator with configured PixelDigitizationTool for ITk"""
    acc = ITkPixelReadoutGeometryCfg(flags)
    # set up tool handle lists
    chargeTools = []
    feSimTools = []
    chargeTools.append(acc.popToolsAndMerge(ITkSensorSimPlanarToolCfg(flags)))
    chargeTools.append(acc.popToolsAndMerge(ITkSensorSim3DToolCfg(flags)))
    feSimTools.append(acc.popToolsAndMerge(BarrelRD53SimToolCfg(flags)))
    feSimTools.append(acc.popToolsAndMerge(EndcapRD53SimToolCfg(flags)))
    kwargs.setdefault("ChargeTools", chargeTools)
    kwargs.setdefault("EnergyDepositionTool", acc.popToolsAndMerge(ITkEnergyDepositionToolCfg(flags)))
    kwargs.setdefault("FrontEndSimTools", feSimTools)
    kwargs.setdefault("InputObjectName", "ITkPixelHits")
    kwargs.setdefault("PixelDetEleCollKey", "ITkPixelDetectorElementCollection")
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", ITkPixel_FirstXing(flags))
        kwargs.setdefault("LastXing", ITkPixel_LastXing(flags))
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    acc.setPrivateTools(CompFactory.PixelDigitizationTool(name, **kwargs))
    return acc


def ITkPixelDigitizationToolCfg(flags, name="ITkPixelDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured ITkPixelDigitizationBasicTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(ITkPixelRangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("PileUpMergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    kwargs.setdefault("HardScatterSplittingMode", 0)
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("RDOCollName", flags.Overlay.BkgPrefix + "ITkPixelRDOs")
        kwargs.setdefault("SDOCollName", flags.Overlay.BkgPrefix + "ITkPixelSDO_Map")
    else:
        kwargs.setdefault("RDOCollName", "ITkPixelRDOs")
        kwargs.setdefault("SDOCollName", "ITkPixelSDO_Map")
    tool = acc.popToolsAndMerge(ITkPixelDigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def ITkPixelDigitizationHSToolCfg(flags, name="ITkPixelDigitizationHSTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for Hard Scatter ITk"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(ITkPixelRangeCfg(flags))
    kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("HardScatterSplittingMode", 1)
    tool = acc.popToolsAndMerge(ITkPixelDigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def ITkPixelDigitizationPUToolCfg(flags, name="ITkPixelDigitizationPUTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for PileUp ITk"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(ITkPixelRangeCfg(flags))
    kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("HardScatterSplittingMode", 2)
    kwargs.setdefault("RDOCollName", "ITkPixel_PU_RDOs")
    kwargs.setdefault("SDOCollName", "ITkPixel_PU_SDO_Map")
    tool = acc.popToolsAndMerge(ITkPixelDigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def ITkPixelDigitizationSplitNoMergePUToolCfg(flags, name="ITkPixelDigitizationSplitNoMergePUTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for PileUpITkPixelHits"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(ITkPixelRangeCfg(flags))
    kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("InputObjectName", "PileupITkPixelHits")
    kwargs.setdefault("RDOCollName", "ITkPixel_PU_RDOs")
    kwargs.setdefault("SDOCollName", "ITkPixel_PU_SDO_Map")
    tool = acc.popToolsAndMerge(ITkPixelDigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def ITkPixelOverlayDigitizationToolCfg(flags, name="ITkPixelOverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for overlay"""
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("RDOCollName", flags.Overlay.SigPrefix + "ITkPixelRDOs")
    kwargs.setdefault("SDOCollName", flags.Overlay.SigPrefix + "ITkPixelSDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    return ITkPixelDigitizationBasicToolCfg(flags, name, **kwargs)


def ITkPixelRangeCfg(flags, name="ITkPixelRange", **kwargs):
    """Return a configured PileUpXingFolder tool"""
    kwargs.setdefault("FirstXing", ITkPixel_FirstXing(flags))
    kwargs.setdefault("LastXing", ITkPixel_LastXing(flags))
    kwargs.setdefault("CacheRefreshFrequency", 1.0) # default 0 no dataproxy reset
    kwargs.setdefault("ItemList", ["SiHitCollection#ITkPixelHits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def ITkPixelOutputCfg(flags):
    """Return ComponentAccumulator with Output for ITk Pixel. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["PixelRDO_Container#*"]
        if flags.Digitization.EnableTruth:
            ItemList += ["InDetSimDataCollection#*"]
            acc.merge(TruthDigitizationOutputCfg(flags))
        if flags.ITk.savePixelSiHits:
            ItemList += ["SiHitCollection#ITkPixelHits"]
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def ITkPixelDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for ITk Pixel digitization"""
    acc = ComponentAccumulator()
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(ITkPixelDigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def ITkPixelOverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with ITk Pixel Overlay digitization"""
    acc = ComponentAccumulator()
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["SiHitCollection#ITkPixelHits"]))

    if "DigitizationTool" not in kwargs:
        tool = acc.popToolsAndMerge(ITkPixelOverlayDigitizationToolCfg(flags))
        kwargs["DigitizationTool"] = tool

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)
    acc.addEventAlgo(CompFactory.PixelDigitization(name="ITkPixelOverlayDigitization", **kwargs))
    return acc


# with output defaults
def ITkPixelDigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for ITk Pixel digitization and Output"""
    acc = ITkPixelDigitizationBasicCfg(flags, **kwargs)
    acc.merge(ITkPixelOutputCfg(flags))
    return acc


# additional specialisations
def ITkPixelDigitizationHSCfg(flags, **kwargs):
    """Return ComponentAccumulator for Hard-Scatter-only ITk Pixel digitization and Output"""
    acc = ComponentAccumulator()
    tool = acc.popToolsAndMerge(ITkPixelDigitizationHSToolCfg(flags))
    kwargs["PileUpTools"] = tool
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    acc.merge(ITkPixelOutputCfg(flags))
    return acc


def ITkPixelDigitizationPUCfg(flags, **kwargs):
    """Return ComponentAccumulator with Pile-up-only ITk Pixel digitization and Output"""
    acc = ComponentAccumulator()
    tool = acc.popToolsAndMerge(ITkPixelDigitizationPUToolCfg(flags))
    kwargs["PileUpTools"] = tool
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    acc.merge(ITkPixelOutputCfg(flags))
    return acc
