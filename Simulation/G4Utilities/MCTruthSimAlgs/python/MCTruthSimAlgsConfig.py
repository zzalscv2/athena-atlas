"""ComponentAccumulator configuration for Monte Carlo Truth simulation algorithms

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from Digitization.PileUpToolsConfig import PileUpToolsCfg
from Digitization.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg

# Note: various experimentalDigi uses not migrated

def GenericMergeMcEventCollCfg(flags, name="MergeMcEventCollTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("TruthCollInputKey", "TruthEvent")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("TruthCollOutputKey", flags.Overlay.BkgPrefix + "TruthEvent")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent")
    kwargs.setdefault("LowTimeToKeep", -50.5)
    kwargs.setdefault("HighTimeToKeep", 50.5)
    kwargs.setdefault("KeepUnstable", False)
    kwargs.setdefault("AbsEtaMax", 5.0)
    kwargs.setdefault("OutOfTimeAbsEtaMax", 3.0)
    kwargs.setdefault("rRange", 20.0)
    kwargs.setdefault("zRange", 200.0)
    kwargs.setdefault("SaveCavernBackground", True)
    kwargs.setdefault("SaveInTimeMinBias", True)
    kwargs.setdefault("SaveOutOfTimeMinBias", True)
    kwargs.setdefault("SaveRestOfMinBias", False)
    kwargs.setdefault("AddBackgroundCollisionVertices", True)
    kwargs.setdefault("CompressOutputCollection", False)
    tool = CompFactory.MergeMcEventCollTool(name, **kwargs)
    acc.merge(PileUpToolsCfg(flags, PileUpTools=tool))
    return acc


def MergeMcEventCollCfg(flags, name="MergeMcEventCollTool", **kwargs):
    acc = ComponentAccumulator()
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", -30000)
        kwargs.setdefault("LastXing",   30000)
    kwargs.setdefault("DoSlimming", False)
    kwargs.setdefault("OnlySaveSignalTruth", False)
    acc.merge(GenericMergeMcEventCollCfg(flags, name, **kwargs))
    return acc


def SignalOnlyMcEventCollCfg(flags, name="SignalOnlyMcEventCollTool", **kwargs):
    acc = ComponentAccumulator()
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", 0)
        kwargs.setdefault("LastXing",  0)
    kwargs.setdefault("OnlySaveSignalTruth", True)
    acc.merge(GenericMergeMcEventCollCfg(flags, name, **kwargs))
    return acc


def InTimeOnlyMcEventCollCfg(flags, name="InTimeOnlyMcEventCollTool", **kwargs):
    acc = ComponentAccumulator()
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", 0)
        kwargs.setdefault("LastXing",  0)
    kwargs.setdefault("DoSlimming", False)
    kwargs.setdefault("OnlySaveSignalTruth", False)
    acc.merge(GenericMergeMcEventCollCfg(flags, name, **kwargs))
    return acc


def GenericSimpleMergeMcEventCollCfg(flags, name="MergeMcEventCollTool", **kwargs):
    acc = ComponentAccumulator()
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("PileUpMergeSvc", "")
    else: # 'Algorithm' approach (consider all bunch-crossings at once)
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags)).name)
    kwargs.setdefault("OnlySaveSignalTruth", False)
    kwargs.setdefault("OverrideEventNumbers", True)
    kwargs.setdefault("TruthCollInputKey", "TruthEvent")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("TruthCollOutputKey", flags.Overlay.BkgPrefix + "TruthEvent")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent")
    tool = CompFactory.SimpleMergeMcEventCollTool(name, **kwargs)
    acc.merge(PileUpToolsCfg(flags, PileUpTools=tool))
    return acc


def SimpleMergeMcEventCollCfg(flags, name="MergeMcEventCollTool", **kwargs):
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", -30000)
        kwargs.setdefault("LastXing",   30000)
    return GenericSimpleMergeMcEventCollCfg(flags, name, **kwargs)


def SignalOnlySimpleMergeMcEventCollCfg(flags, name="SignalOnlyMcEventCollTool", **kwargs):
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", 0)
        kwargs.setdefault("LastXing",  0)
    kwargs.setdefault("OnlySaveSignalTruth", True)
    return GenericSimpleMergeMcEventCollCfg(flags, name, **kwargs)


def InTimeOnlySimpleMergeMcEventCollCfg(flags, name="InTimeOnlyMcEventCollTool", **kwargs):
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", 0)
        kwargs.setdefault("LastXing",  0)
    return GenericSimpleMergeMcEventCollCfg(flags, name, **kwargs)


# The earliest bunch crossing time for which interactions will be sent
# to the Truth jet merging code. See discussions in ATLASSIM-3837.
def TruthJet_FirstXing():
    return -125


# The latest bunch crossing time for which interactions will be sent
# to the Truth jet merging code. See discussions in ATLASSIM-3837.
def TruthJet_LastXing():
    return 75


def TruthJetRangeCfg(flags, name="TruthJetRange", **kwargs):
    """Return a Truth-Jet configured PileUpXingFolder tool"""
    #this is the time of the xing in ns
    kwargs.setdefault("FirstXing", TruthJet_FirstXing())
    kwargs.setdefault("LastXing",  TruthJet_LastXing())
    itemList = ["xAOD::JetContainer#AntiKt4TruthJets",
                "xAOD::JetContainer#AntiKt6TruthJets"]
    kwargs.setdefault("ItemList", itemList)
    return PileUpXingFolderCfg(flags, name, **kwargs)


def MergeAntiKt4TruthJetsCfg(flags, name="MergeAntiKt4TruthJetsTool", **kwargs):
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(TruthJetRangeCfg(flags))
    acc.merge(PileUpMergeSvcCfg(flags, Intervals=rangetool))
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", TruthJet_FirstXing())
        kwargs.setdefault("LastXing",  TruthJet_LastXing())
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("InTimeOutputTruthJetCollKey", flags.Overlay.BkgPrefix + "InTimeAntiKt4TruthJets")
        kwargs.setdefault("OutOfTimeTruthJetCollKey", flags.Overlay.BkgPrefix + "OutOfTimeAntiKt4TruthJets")
    else:
        kwargs.setdefault("InTimeOutputTruthJetCollKey", "InTimeAntiKt4TruthJets")
        kwargs.setdefault("OutOfTimeTruthJetCollKey", "OutOfTimeAntiKt4TruthJets")
    tool = CompFactory.MergeTruthJetsTool(name, **kwargs)
    acc.merge(PileUpToolsCfg(flags, PileUpTools=tool))
    return acc


def MergeAntiKt6TruthJetsCfg(flags, name="MergeAntiKt6TruthJetsTool", **kwargs):
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(TruthJetRangeCfg(flags))
    acc.merge(PileUpMergeSvcCfg(flags, Intervals=rangetool))
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", TruthJet_FirstXing())
        kwargs.setdefault("LastXing",  TruthJet_LastXing())
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("InTimeOutputTruthJetCollKey", flags.Overlay.BkgPrefix + "InTimeAntiKt6TruthJets")
        kwargs.setdefault("OutOfTimeTruthJetCollKey", flags.Overlay.BkgPrefix + "OutOfTimeAntiKt6TruthJets")
    else:
        kwargs.setdefault("InTimeOutputTruthJetCollKey", "InTimeAntiKt6TruthJets")
        kwargs.setdefault("OutOfTimeTruthJetCollKey", "OutOfTimeAntiKt6TruthJets")
    tool = CompFactory.MergeTruthJetsTool(name, **kwargs)
    acc.merge(PileUpToolsCfg(flags, PileUpTools=tool))
    return acc


def MergeTruthJetsFilterCfg(flags, name="MergeTruthJetsFilterTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ActivateFilter", True)
    acc.merge(MergeAntiKt4TruthJetsCfg(flags, name, **kwargs))
    return acc


# The earliest bunch crossing time for which interactions will be sent
# to the Truth particle merging code.
def TruthParticle_FirstXing():
    return 0


# The latest bunch crossing time for which interactions will be sent
# to the Truth particle merging code.
def TruthParticle_LastXing():
    return 0


def TruthParticleRangeCfg(flags, name="TruthParticleRange", **kwargs):
    """Return a Truth-Particle configured PileUpXingFolder tool"""
    #this is the time of the xing in ns
    kwargs.setdefault("FirstXing", TruthParticle_FirstXing())
    kwargs.setdefault("LastXing",  TruthParticle_LastXing())
    kwargs.setdefault("ItemList", ["xAOD::TruthParticleContainer#TruthPileupParticles",
                                   "xAOD::TruthParticleAuxContainer#TruthPileupParticlesAux."])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def MergeTruthParticlesCfg(flags, name="MergeTruthParticlesTool", **kwargs):
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(TruthParticleRangeCfg(flags))
    acc.merge(PileUpMergeSvcCfg(flags, Intervals=rangetool))
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", TruthParticle_FirstXing())
        kwargs.setdefault("LastXing",  TruthParticle_LastXing())
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("InTimeOutputTruthParticleCollKey", flags.Overlay.BkgPrefix + "TruthPileupParticles")
    else:
        kwargs.setdefault("InTimeOutputTruthParticleCollKey", "TruthPileupParticles")
    tool = CompFactory.MergeTruthParticlesTool(name, **kwargs)
    acc.merge(PileUpToolsCfg(flags, PileUpTools=tool))
    return acc


# The earliest bunch crossing time for which interactions will be sent
# to the TrackRecordCollection merging code.
def TrackRecord_FirstXing():
    return -1


# The latest bunch crossing time for which interactions will be sent
# to the TrackRecordCollection merging code.
def TrackRecord_LastXing():
    return 1


def TrackRangeCfg(flags, name="TrackRange", **kwargs):
    """Return a Track configured PileUpXingFolder tool"""
    # this is the time of the xing in ns
    kwargs.setdefault("FirstXing", TrackRecord_FirstXing())
    kwargs.setdefault("LastXing",  TrackRecord_LastXing())
    kwargs.setdefault("ItemList", ["TrackRecordCollection#MuonExitLayer"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def MergeTrackRecordCollCfg(flags, name="MergeTrackRecordCollTool", **kwargs):
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(TrackRangeCfg(flags))
    acc.merge(PileUpMergeSvcCfg(flags, Intervals=rangetool))
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", TrackRecord_FirstXing())
        kwargs.setdefault("LastXing",  TrackRecord_LastXing())
    tool = CompFactory.MergeTrackRecordCollTool(name, **kwargs)
    acc.merge(PileUpToolsCfg(flags, PileUpTools=tool))
    return acc


def MergeCaloEntryLayerCfg(flags, name="MergeCaloEntryLayerTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("TrackRecordCollKey", "CaloEntryLayer")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("TrackRecordCollOutputKey", flags.Overlay.BkgPrefix + "CaloEntryLayer")
    else:
        kwargs.setdefault("TrackRecordCollOutputKey", "CaloEntryLayer")
    acc.merge(MergeTrackRecordCollCfg(flags, name, **kwargs))
    return acc


def MergeMuonEntryLayerCfg(flags, name="MergeMuonEntryLayerTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("TrackRecordCollKey", "MuonEntryLayer")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("TrackRecordCollOutputKey", flags.Overlay.BkgPrefix + "MuonEntryLayer")
    else:
        kwargs.setdefault("TrackRecordCollOutputKey", "MuonEntryLayer")
    acc.merge(MergeTrackRecordCollCfg(flags, name, **kwargs))
    return acc


def MergeMuonExitLayerCfg(flags, name="MergeMuonExitLayerTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("TrackRecordCollKey", "MuonExitLayer")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("TrackRecordCollOutputKey", flags.Overlay.BkgPrefix + "MuonExitLayer")
    else:
        kwargs.setdefault("TrackRecordCollOutputKey", "MuonExitLayer")
    acc.merge(MergeTrackRecordCollCfg(flags, name, **kwargs))
    return acc


def MergeHijingParsCfg(flags, name="MergeHijingParsTool", **kwargs):
    acc = ComponentAccumulator()
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", -1)
        kwargs.setdefault("LastXing",  +1)
    tool = CompFactory.MergeHijingParsTool(name, **kwargs)
    acc.merge(PileUpToolsCfg(flags, PileUpTools=tool))
    return acc


# The earliest bunch crossing time for which interactions will be sent
# to the CaloCalibrationHitContainer merging code.
def CalibHit_FirstXing():
    return -1

# The latest bunch crossing time for which interactions will be sent
# to the CaloCalibrationHitContainer merging code.
def CalibHit_LastXing():
    return 1


def CalibRangeCfg(flags, name="CalibRange", **kwargs):
    """Return a Calibration configured PileUpXingFolder tool"""
    # bunch crossing range in ns
    kwargs.setdefault("FirstXing", CalibHit_FirstXing())
    kwargs.setdefault("LastXing",  CalibHit_LastXing())
    ItemList = [
        "CaloCalibrationHitContainer#LArCalibrationHitActive",
        "CaloCalibrationHitContainer#LArCalibrationHitDeadMaterial",
        "CaloCalibrationHitContainer#LArCalibrationHitInactive",
        "CaloCalibrationHitContainer#TileCalibHitActiveCell",
        "CaloCalibrationHitContainer#TileCalibHitInactiveCell",
        "CaloCalibrationHitContainer#TileCalibHitDeadMaterial"
    ]
    kwargs.setdefault("ItemList", ItemList)
    return PileUpXingFolderCfg(flags, name, **kwargs)


def MergeCalibHitsCfg(flags, name="MergeCalibHitsTool", **kwargs):
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(CalibRangeCfg(flags))
    acc.merge(PileUpMergeSvcCfg(flags, Intervals=rangetool))
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", CalibHit_FirstXing())
        kwargs.setdefault("LastXing",  CalibHit_LastXing())
    tool = CompFactory.MergeCalibHitsTool(name, **kwargs)
    acc.merge(PileUpToolsCfg(flags, PileUpTools=tool))
    return acc


def MergeGenericMuonSimHitCollCfg(flags, name="MergeGenericMuonSimHitCollTool", **kwargs):
    acc = ComponentAccumulator()
    tool = CompFactory.MergeGenericMuonSimHitCollTool(name, **kwargs)
    acc.merge(PileUpToolsCfg(flags, PileUpTools=tool))
    return acc


def MergeMicromegasSimHitCollCfg(flags, name="MergeMicromegasSimHitCollTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SimHitContainerNames", ["MM_Hits"])
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", -250)
        kwargs.setdefault("LastXing",   200)
    tool = CompFactory.MergeGenericMuonSimHitCollTool(name, **kwargs)
    acc.merge(PileUpToolsCfg(flags, PileUpTools=tool))
    return acc


def MergeSTGCSimHitCollCfg(flags, name="MergeSTGCSimHitCollTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SimHitContainerNames", ["sTGC_Hits"])
    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", -50)
        kwargs.setdefault("LastXing",   75)
    tool = CompFactory.MergeGenericMuonSimHitCollTool(name, **kwargs)
    acc.merge(PileUpToolsCfg(flags, PileUpTools=tool))
    return acc
