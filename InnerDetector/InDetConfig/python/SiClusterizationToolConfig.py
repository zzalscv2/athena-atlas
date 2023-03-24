# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SiClusterizationTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod
from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline


def ClusterMakerToolCfg(flags, name="InDetClusterMakerTool"):
    acc = ComponentAccumulator()
    kwargs = {}

    # This directly needs the following Conditions data:
    # PixelChargeCalibCondData & PixelOfflineCalibData
    from PixelConditionsAlgorithms.PixelConditionsConfig import (
        PixelChargeLUTCalibCondAlgCfg, PixelChargeCalibCondAlgCfg,
        PixelOfflineCalibCondAlgCfg)
    if flags.GeoModel.Run is LHCPeriod.Run3:
        acc.merge(PixelChargeLUTCalibCondAlgCfg(flags))
    else:
        acc.merge(PixelChargeCalibCondAlgCfg(flags))
    acc.merge(PixelOfflineCalibCondAlgCfg(flags))

    from PixelReadoutGeometry.PixelReadoutGeometryConfig import (
        PixelReadoutManagerCfg)
    acc.merge(PixelReadoutManagerCfg(flags))

    from SiLorentzAngleTool.PixelLorentzAngleConfig import (
        PixelLorentzAngleToolCfg)
    kwargs["PixelLorentzAngleTool"] = acc.popToolsAndMerge(
        PixelLorentzAngleToolCfg(flags))

    from SiLorentzAngleTool.SCT_LorentzAngleConfig import (
        SCT_LorentzAngleToolCfg)
    kwargs["SCTLorentzAngleTool"] = acc.popToolsAndMerge(
        SCT_LorentzAngleToolCfg(flags))

    acc.setPrivateTools(CompFactory.InDet.ClusterMakerTool(name, **kwargs))
    return acc


def ITkClusterMakerToolCfg(flags, name="ITkClusterMakerTool"):
    acc = ComponentAccumulator()
    kwargs = {}

    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import (
        ITkPixelChargeCalibCondAlgCfg)
    from PixelReadoutGeometry.PixelReadoutGeometryConfig import (
        ITkPixelReadoutManagerCfg)

    # This directly needs the following Conditions data:
    # PixelModuleData & PixelChargeCalibCondData
    acc.merge(ITkPixelChargeCalibCondAlgCfg(flags))
    acc.merge(ITkPixelReadoutManagerCfg(flags))
    kwargs["PixelReadoutManager"] = acc.getService("ITkPixelReadoutManager")

    from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import (
        ITkPixelLorentzAngleToolCfg)
    kwargs["PixelLorentzAngleTool"] = acc.popToolsAndMerge(
        ITkPixelLorentzAngleToolCfg(flags))

    from SiLorentzAngleTool.ITkStripLorentzAngleConfig import (
        ITkStripLorentzAngleToolCfg)
    kwargs["SCTLorentzAngleTool"] = acc.popToolsAndMerge(
        ITkStripLorentzAngleToolCfg(flags))

    kwargs["PixelChargeCalibCondData"] = "ITkPixelChargeCalibCondData"
    kwargs["PixelOfflineCalibData"] = ""

    acc.setPrivateTools(CompFactory.InDet.ClusterMakerTool(name, **kwargs))
    return acc


def InDetPixelRDOToolCfg(flags, name="InDetPixelRDOTool",
                         PixelDetElStatus="PixelDetectorElementStatus"):
    # To produce PixelDetectorElementCollection condition data
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    acc = PixelReadoutGeometryCfg(flags)
    kwargs = {}
    kwargs["PixelDetElStatus"] = PixelDetElStatus

    if PixelDetElStatus != "":
        from PixelConditionsAlgorithms.PixelConditionsConfig import (
            PixelDetectorElementStatusAlgCfg)
        acc.merge(PixelDetectorElementStatusAlgCfg(flags))

    from PixelConditionsTools.PixelConditionsSummaryConfig import (
        PixelConditionsSummaryCfg)
    kwargs["PixelConditionsSummaryTool"] = acc.popToolsAndMerge(
        PixelConditionsSummaryCfg(flags))

    # Enable duplicated RDO check for data15 because duplication mechanism
    # was used.
    kwargs["CheckDuplicatedRDO"] = (len(flags.Input.ProjectName) >= 6 and
                                    flags.Input.ProjectName[:6] == "data15")

    acc.setPrivateTools(CompFactory.InDet.PixelRDOTool(name, **kwargs))
    return acc


def ITkPixelRDOToolCfg(flags, name="ITkPixelRDOTool"):
    # To produce ITkPixelDetectorElementCollection condition data
    from PixelGeoModelXml.ITkPixelGeoModelConfig import (
        ITkPixelReadoutGeometryCfg)
    acc = ITkPixelReadoutGeometryCfg(flags)
    kwargs = {}

    from PixelConditionsTools.ITkPixelConditionsSummaryConfig import (
        ITkPixelConditionsSummaryCfg)
    kwargs["PixelConditionsSummaryTool"] = acc.popToolsAndMerge(
        ITkPixelConditionsSummaryCfg(flags))

    kwargs["PixelDetEleCollKey"] = "ITkPixelDetectorElementCollection"
    kwargs["CheckGanged"] = False

    acc.setPrivateTools(CompFactory.InDet.PixelRDOTool(name, **kwargs))
    return acc


def TrigPixelRDOToolCfg(flags, name="InDetTrigPixelRDOTool"):
    return InDetPixelRDOToolCfg(flags, name, PixelDetElStatus="")


def MergedPixelsToolCfg(flags, name="InDetMergedPixelsTool"):
    acc = ComponentAccumulator()
    kwargs = {}

    kwargs["globalPosAlg"] = acc.popToolsAndMerge(ClusterMakerToolCfg(flags))
    kwargs["PixelRDOTool"] = acc.popToolsAndMerge(InDetPixelRDOToolCfg(flags))

    acc.setPrivateTools(CompFactory.InDet.MergedPixelsTool(name, **kwargs))
    return acc


def TrigMergedPixelsToolCfg(flags, name="InDetTrigMergedPixelsTool"):
    acc = ComponentAccumulator()
    kwargs = {}

    kwargs["globalPosAlg"] = acc.popToolsAndMerge(ClusterMakerToolCfg(flags))
    kwargs["PixelRDOTool"] = acc.popToolsAndMerge(TrigPixelRDOToolCfg(flags))

    acc.setPrivateTools(CompFactory.InDet.MergedPixelsTool(name, **kwargs))
    return acc


def ITkMergedPixelsToolCfg(flags, name="ITkMergedPixelsTool"):
    acc = ComponentAccumulator()
    kwargs = {}

    kwargs["globalPosAlg"] = acc.popToolsAndMerge(
        ITkClusterMakerToolCfg(flags))
    kwargs["PixelRDOTool"] = acc.popToolsAndMerge(ITkPixelRDOToolCfg(flags))

    acc.setPrivateTools(CompFactory.InDet.MergedPixelsTool(name, **kwargs))
    return acc


def PixelClusterNnCondAlgCfg(flags, name="PixelClusterNnCondAlg",
                             trackNetwork=False):
    acc = ComponentAccumulator()
    kwargs = {}

    nn_names = [
        "NumberParticles_NoTrack/",
        "ImpactPoints1P_NoTrack/",
        "ImpactPoints2P_NoTrack/",
        "ImpactPoints3P_NoTrack/",
        "ImpactPointErrorsX1_NoTrack/",
        "ImpactPointErrorsX2_NoTrack/",
        "ImpactPointErrorsX3_NoTrack/",
        "ImpactPointErrorsY1_NoTrack/",
        "ImpactPointErrorsY2_NoTrack/",
        "ImpactPointErrorsY3_NoTrack/"]

    if trackNetwork:
        nn_names = [elm.replace('_NoTrack', '') for elm in nn_names]

    kwargs["NetworkNames"] = nn_names
    kwargs["GetInputsInfo"] = True
    kwargs["WriteKey"] = ("PixelClusterNN" if not trackNetwork
                          else "PixelClusterNNWithTrack")

    acc.merge(addFoldersSplitOnline(
        flags, "PIXEL",
        "/PIXEL/Onl/PixelClustering/PixelClusNNCalib",
        "/PIXEL/PixelClustering/PixelClusNNCalib",
        className='CondAttrListCollection'))

    from TrkConfig.TrkNeuralNetworkUtilsConfig import (
        NeuralNetworkToHistoToolCfg)
    kwargs["NetworkToHistoTool"] = acc.popToolsAndMerge(
        NeuralNetworkToHistoToolCfg(flags))

    acc.addCondAlgo(CompFactory.InDet.TTrainedNetworkCondAlg(name, **kwargs))
    return acc


def PixelClusterNnWithTrackCondAlgCfg(
        flags, name="PixelClusterNnWithTrackCondAlg"):
    return PixelClusterNnCondAlgCfg(flags, name, trackNetwork=True)


def LWTNNCondAlgCfg(flags, name="LWTNNCondAlg"):
    acc = ComponentAccumulator()
    kwargs = {}

    acc.merge(addFoldersSplitOnline(
        flags, "PIXEL",
        "/PIXEL/Onl/PixelClustering/PixelNNCalibJSON",
        "/PIXEL/PixelClustering/PixelNNCalibJSON",
        className="CondAttrListCollection"))

    kwargs["WriteKey"] = "PixelClusterNNJSON"

    acc.addCondAlgo(CompFactory.InDet.LWTNNCondAlg(name, **kwargs))
    return acc


def NnClusterizationFactoryCfg(flags, name="NnClusterizationFactory"):
    acc = ComponentAccumulator()
    kwargs = {}

    from PixelConditionsAlgorithms.PixelConditionsConfig import (
        PixelChargeLUTCalibCondAlgCfg, PixelChargeCalibCondAlgCfg)
    if flags.GeoModel.Run is LHCPeriod.Run3:
        acc.merge(PixelChargeLUTCalibCondAlgCfg(flags))
    else:
        acc.merge(PixelChargeCalibCondAlgCfg(flags))

    if flags.GeoModel.Run is LHCPeriod.Run1:
        acc.merge(PixelClusterNnCondAlgCfg(flags))
        acc.merge(PixelClusterNnWithTrackCondAlgCfg(flags))
    else:
        acc.merge(LWTNNCondAlgCfg(flags))

    from SiLorentzAngleTool.PixelLorentzAngleConfig import (
        PixelLorentzAngleToolCfg)
    kwargs["PixelLorentzAngleTool"] = acc.popToolsAndMerge(
        PixelLorentzAngleToolCfg(flags))

    kwargs["doRunI"] = flags.GeoModel.Run is LHCPeriod.Run1
    kwargs["useToT"] = False
    kwargs["useRecenteringNNWithoutTracks"] = (
        flags.GeoModel.Run is LHCPeriod.Run1)
    kwargs["useRecenteringNNWithTracks"] = False
    kwargs["correctLorShiftBarrelWithoutTracks"] = 0
    kwargs["correctLorShiftBarrelWithTracks"] = (
        0.03 if flags.GeoModel.Run is LHCPeriod.Run1 else 0.)
    kwargs["useTTrainedNetworks"] = flags.GeoModel.Run is LHCPeriod.Run1
    kwargs["NnCollectionReadKey"] = (
        "PixelClusterNN" if flags.GeoModel.Run is LHCPeriod.Run1 else "")
    kwargs["NnCollectionWithTrackReadKey"] = (
        "PixelClusterNNWithTrack" if flags.GeoModel.Run is LHCPeriod.Run1
        else "")
    kwargs["NnCollectionJSONReadKey"] = (
        "" if flags.GeoModel.Run is LHCPeriod.Run1
        else "PixelClusterNNJSON")

    acc.setPrivateTools(
        CompFactory.InDet.NnClusterizationFactory(name, **kwargs))
    return acc


def TrigNnClusterizationFactoryCfg(flags, name="TrigNnClusterizationFactory"):
    acc = ComponentAccumulator()
    kwargs = {}

    from PixelConditionsAlgorithms.PixelConditionsConfig import (
        PixelChargeLUTCalibCondAlgCfg, PixelChargeCalibCondAlgCfg)
    if flags.GeoModel.Run is LHCPeriod.Run3:
        acc.merge(PixelChargeLUTCalibCondAlgCfg(flags))
    else:
        acc.merge(PixelChargeCalibCondAlgCfg(flags))

    acc.merge(PixelClusterNnCondAlgCfg(flags))
    acc.merge(PixelClusterNnWithTrackCondAlgCfg(flags))

    from SiLorentzAngleTool.PixelLorentzAngleConfig import (
        PixelLorentzAngleToolCfg)
    kwargs["PixelLorentzAngleTool"] = acc.popToolsAndMerge(
        PixelLorentzAngleToolCfg(flags))

    kwargs["useToT"] = False
    kwargs["NnCollectionReadKey"] = "PixelClusterNN"
    kwargs["NnCollectionWithTrackReadKey"] = "PixelClusterNNWithTrack"

    acc.setPrivateTools(
        CompFactory.InDet.NnClusterizationFactory(name, **kwargs))
    return acc


def NnPixelClusterSplitProbToolCfg(flags, name="NnPixelClusterSplitProbTool"):
    acc = ComponentAccumulator()
    kwargs = {}

    kwargs["NnClusterizationFactory"] = acc.popToolsAndMerge(
        NnClusterizationFactoryCfg(flags))
    kwargs["PriorMultiplicityContent"] = [1, 1, 1]
    kwargs["useBeamSpotInfo"] = flags.InDet.Tracking.useBeamSpotInfoNN

    acc.setPrivateTools(
        CompFactory.InDet.NnPixelClusterSplitProbTool(name, **kwargs))
    return acc


def PixelGangedAmbiguitiesFinderCfg(
        flags, name="InDetPixelGangedAmbiguitiesFinder"):
    # To produce PixelDetectorElementCollection condition data
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    acc = PixelReadoutGeometryCfg(flags)
    acc.setPrivateTools(CompFactory.InDet.PixelGangedAmbiguitiesFinder(name))
    return acc


def ITkPixelGangedAmbiguitiesFinderCfg(
        flags, name="ITkPixelGangedAmbiguitiesFinder"):
    # To produce ITkPixelDetectorElementCollection condition data
    from PixelGeoModelXml.ITkPixelGeoModelConfig import (
        ITkPixelReadoutGeometryCfg)
    acc = ITkPixelReadoutGeometryCfg(flags)
    kwargs = {}
    kwargs["PixelDetEleCollKey"] = "ITkPixelDetectorElementCollection"

    acc.setPrivateTools(
        CompFactory.InDet.PixelGangedAmbiguitiesFinder(name, **kwargs))
    return acc


def SCT_ClusteringToolCfg(
        flags, name="InDetSCT_ClusteringTool",
        conditionsTool=None,
        SCTDetElStatus="SCTDetectorElementStatusWithoutFlagged"):
    # To produce SCT_DetectorElementCollection
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc = SCT_ReadoutGeometryCfg(flags)
    kwargs = {}
    kwargs["SCTDetElStatus"] = SCTDetElStatus

    if conditionsTool is None:
        from SCT_ConditionsTools.SCT_ConditionsToolsConfig import (
            SCT_ConditionsSummaryToolCfg)
        conditionsTool = acc.popToolsAndMerge(
            SCT_ConditionsSummaryToolCfg(flags, withFlaggedCondTool=False))
    kwargs["conditionsTool"] = conditionsTool

    from SiLorentzAngleTool.SCT_LorentzAngleConfig import (
        SCT_LorentzAngleToolCfg)
    kwargs["LorentzAngleTool"] = acc.popToolsAndMerge(
        SCT_LorentzAngleToolCfg(flags))

    kwargs["globalPosAlg"] = acc.popToolsAndMerge(ClusterMakerToolCfg(flags))

    if flags.InDet.selectSCTIntimeHits:
        coll_25ns = (flags.Beam.BunchSpacing <= 25 and
                     flags.Beam.Type is BeamType.Collisions)
        kwargs["timeBins"] = "01X" if coll_25ns else "X1X"

    acc.setPrivateTools(CompFactory.InDet.SCT_ClusteringTool(name, **kwargs))
    return acc


def Trig_SCT_ClusteringToolCfg(flags, name="Trig_SCT_ClusteringTool"):
    acc = ComponentAccumulator()

    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import (
        SCT_ConditionsSummaryToolCfg)
    conditionsTool = acc.popToolsAndMerge(SCT_ConditionsSummaryToolCfg(
        flags, withFlaggedCondTool=False, withTdaqTool=False))

    acc.setPrivateTools(acc.popToolsAndMerge(SCT_ClusteringToolCfg(
        flags, name,
        conditionsTool=conditionsTool,
        SCTDetElStatus=""
    )))
    return acc


def ITKStrip_SCT_ClusteringToolCfg(flags, name="ITkStripClusteringTool"):
    # To produce ITkStripDetectorElementCollection
    from StripGeoModelXml.ITkStripGeoModelConfig import (
        ITkStripReadoutGeometryCfg)
    acc = ITkStripReadoutGeometryCfg(flags)
    kwargs = {}
    kwargs["SCTDetEleCollKey"] = "ITkStripDetectorElementCollection"
    kwargs["useRowInformation"] = True  # ITk-specific clustering

    from SCT_ConditionsTools.ITkStripConditionsToolsConfig import (
        ITkStripConditionsSummaryToolCfg)
    kwargs["conditionsTool"] = acc.popToolsAndMerge(
        ITkStripConditionsSummaryToolCfg(flags))

    from SiLorentzAngleTool.ITkStripLorentzAngleConfig import (
        ITkStripLorentzAngleToolCfg)
    kwargs["LorentzAngleTool"] = acc.popToolsAndMerge(
        ITkStripLorentzAngleToolCfg(flags))

    kwargs["globalPosAlg"] = acc.popToolsAndMerge(
        ITkClusterMakerToolCfg(flags))

    if flags.ITk.selectStripIntimeHits:
        coll_25ns = (flags.Beam.BunchSpacing <= 25 and
                     flags.Beam.Type is BeamType.Collisions)
        kwargs["timeBins"] = "01X" if coll_25ns else "X1X"

    acc.setPrivateTools(CompFactory.InDet.SCT_ClusteringTool(name, **kwargs))
    return acc


def ITkTruthClusterizationFactoryCfg(
        flags, name='ITkTruthClusterizationFactory'):
    acc = ComponentAccumulator()
    kwargs = {}
    kwargs["InputSDOMap"] = "ITkPixelSDO_Map"

    acc.setPrivateTools(
        CompFactory.InDet.TruthClusterizationFactory(name, **kwargs))
    return acc


def ITkTruthPixelClusterSplitProbToolCfg(
        flags, name="ITkTruthPixelClusterSplitProbTool"):
    acc = ComponentAccumulator()
    kwargs = {}
    kwargs["PriorMultiplicityContent"] = [1, 1, 1]
    kwargs["NnClusterizationFactory"] = acc.popToolsAndMerge(
        ITkTruthClusterizationFactoryCfg(flags))

    # Truth-based for ITk for now
    acc.setPrivateTools(
        CompFactory.InDet.TruthPixelClusterSplitProbTool(name, **kwargs))
    return acc
