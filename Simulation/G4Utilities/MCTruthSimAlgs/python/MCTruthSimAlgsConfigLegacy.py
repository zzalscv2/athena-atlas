# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from Digitization.DigitizationFlags import digitizationFlags
from AthenaCommon import CfgMgr

############################################################################

def HepMCVersion():
    try:
        from AthenaPython.PyAthena import HepMC3 # noqa: F401
        HepMCVersion=3
    except ImportError:
        HepMCVersion=2
    return HepMCVersion


def genericMergeMcEventCollTool(name="MergeMcEventCollTool", **kwargs):
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TruthCollOutputKey", overlayFlags.bkgPrefix() + "TruthEvent")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent")
    kwargs.setdefault("TruthCollInputKey", "TruthEvent")
    if 'NewMerge' in digitizationFlags.experimentalDigi():
        if not digitizationFlags.doXingByXingPileUp(): # Algorithm approach
            kwargs.setdefault("PileUpMergeSvc", "PileUpMergeSvc")
        if digitizationFlags.doLowPtMinBias:
            kwargs.setdefault("ExpectLowPtMinBiasBackgroundCollection", True)
        if digitizationFlags.doHighPtMinBias:
            kwargs.setdefault("ExpectHighPtMinBiasBackgroundCollection", True)
        # Default `PileUpType` to "Unknown"
        kwargs.setdefault("PileUpType", -1)
        return CfgMgr.NewMergeMcEventCollTool(name, **kwargs)
    elif HepMCVersion() == 3 or 'SimpleMerge' in digitizationFlags.experimentalDigi():
        kwargs.setdefault("OverrideEventNumbers", True)
        if not digitizationFlags.doXingByXingPileUp(): # Algorithm approach
            kwargs.setdefault("PileUpMergeSvc", "PileUpMergeSvc")
        return CfgMgr.SimpleMergeMcEventCollTool(name, **kwargs)
    else:
        kwargs.setdefault("LowTimeToKeep", -50.5)
        kwargs.setdefault("HighTimeToKeep", 50.5)
        kwargs.setdefault("KeepUnstable", False)
        kwargs.setdefault("AbsEtaMax", 5.0)
        kwargs.setdefault("OutOfTimeAbsEtaMax", 3.0)
        kwargs.setdefault("rRange", 20.0)
        kwargs.setdefault("zRange", 200.0)
        #kwargs.setdefault("ptMin", 0.4*GeV)
        #kwargs.setdefault("EKinMin", 1.0*MeV)
        kwargs.setdefault("SaveCavernBackground", True)
        kwargs.setdefault("SaveInTimeMinBias", True)
        kwargs.setdefault("SaveOutOfTimeMinBias", True)
        kwargs.setdefault("SaveRestOfMinBias", False)
        kwargs.setdefault("AddBackgroundCollisionVertices", True)
        kwargs.setdefault("CompressOutputCollection", False)
        #kwargs.setdefault("CopyCompleteGenEvents", True)
        return CfgMgr.MergeMcEventCollTool(name, **kwargs)

def MergeMcEventCollTool(name="MergeMcEventCollTool", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", -30000)
        kwargs.setdefault("LastXing",   30000)
    keys = ['SimpleMerge', 'NewMerge']
    if HepMCVersion() == 2 and set(keys).isdisjoint(set(digitizationFlags.experimentalDigi())):
        kwargs.setdefault("DoSlimming", False)
    keys = ['NewMerge']
    if set(keys).isdisjoint(set(digitizationFlags.experimentalDigi())):
        kwargs.setdefault("OnlySaveSignalTruth", False)
    return genericMergeMcEventCollTool(name, **kwargs)

def SignalOnlyMcEventCollTool(name="SignalOnlyMcEventCollTool", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", 0)
        kwargs.setdefault("LastXing",  0)
    keys = ['NewMerge']
    if set(keys).isdisjoint(set(digitizationFlags.experimentalDigi())):
        kwargs.setdefault("OnlySaveSignalTruth", True)
    return genericMergeMcEventCollTool(name, **kwargs)

def InTimeOnlyMcEventCollTool(name="InTimeOnlyMcEventCollTool", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", 0)
        kwargs.setdefault("LastXing",  0)
    keys = ['SimpleMerge', 'NewMerge']
    if HepMCVersion() == 2 and set(keys).isdisjoint(set(digitizationFlags.experimentalDigi())):
        kwargs.setdefault("DoSlimming", False)
    keys = ['NewMerge']
    if set(keys).isdisjoint(set(digitizationFlags.experimentalDigi())):
        kwargs.setdefault("OnlySaveSignalTruth", False)
    return genericMergeMcEventCollTool(name, **kwargs)

############################################################################

# The earliest bunch crossing time for which interactions will be sent
# to the Truth jet merging code. See discussions in ATLASSIM-3837.
def TruthJet_FirstXing():
    return -125

# The latest bunch crossing time for which interactions will be sent
# to the Truth jet merging code. See discussions in ATLASSIM-3837.
def TruthJet_LastXing():
    return 75

def getTruthJetRange(name="TruthJetRange", **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', TruthJet_FirstXing() )
    kwargs.setdefault('LastXing',  TruthJet_LastXing() )
    itemList = []
    if 'PileUpAntiKt4TruthJets' in digitizationFlags.experimentalDigi():
        itemList += ["xAOD::JetContainer#AntiKt4TruthJets"]
    if 'PileUpAntiKt6TruthJets' in digitizationFlags.experimentalDigi():
        itemList += ["xAOD::JetContainer#AntiKt6TruthJets"]
    kwargs.setdefault('ItemList',  itemList)
    return CfgMgr.PileUpXingFolder(name, **kwargs)


def getMergeAntiKt4TruthJetsTool(name="MergeAntiKt4TruthJetsTool", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", TruthJet_FirstXing() )
        kwargs.setdefault("LastXing",  TruthJet_LastXing() )

    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("InTimeOutputTruthJetCollKey", overlayFlags.bkgPrefix() + "InTimeAntiKt4TruthJets")
        kwargs.setdefault("OutOfTimeTruthJetCollKey", overlayFlags.bkgPrefix() + "OutOfTimeAntiKt4TruthJets")
    else:
        kwargs.setdefault("InTimeOutputTruthJetCollKey", "InTimeAntiKt4TruthJets")
        kwargs.setdefault("OutOfTimeTruthJetCollKey", "OutOfTimeAntiKt4TruthJets")

    return CfgMgr.MergeTruthJetsTool(name, **kwargs)

def getMergeAntiKt6TruthJetsTool(name="MergeAntiKt6TruthJetsTool", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", TruthJet_FirstXing() )
        kwargs.setdefault("LastXing",  TruthJet_LastXing() )

    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("InTimeOutputTruthJetCollKey", overlayFlags.bkgPrefix() + "InTimeAntiKt6TruthJets")
        kwargs.setdefault("OutOfTimeTruthJetCollKey", overlayFlags.bkgPrefix() + "OutOfTimeAntiKt6TruthJets")
    else:
        kwargs.setdefault("InTimeOutputTruthJetCollKey", "InTimeAntiKt6TruthJets")
        kwargs.setdefault("OutOfTimeTruthJetCollKey", "OutOfTimeAntiKt6TruthJets")

    return CfgMgr.MergeTruthJetsTool(name, **kwargs)

def getMergeTruthJetsFilterTool(name="MergeTruthJetsFilterTool", **kwargs):
    kwargs.setdefault("ActivateFilter", True )
    return getMergeAntiKt4TruthJetsTool(name, **kwargs)

############################################################################

# The earliest bunch crossing time for which interactions will be sent
# to the Truth particle merging code.
def TruthParticle_FirstXing():
    return 0

# The latest bunch crossing time for which interactions will be sent
# to the Truth particle merging code.
def TruthParticle_LastXing():
    return 0

def getTruthParticleRange(name="TruthParticleRange", **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', TruthParticle_FirstXing() )
    kwargs.setdefault('LastXing',  TruthParticle_LastXing() )
    kwargs.setdefault("ItemList", ["xAOD::TruthParticleContainer#TruthPileupParticles",
                                   "xAOD::TruthParticleAuxContainer#TruthPileupParticlesAux."])
    return CfgMgr.PileUpXingFolder(name, **kwargs)


def getMergeTruthParticlesTool(name="MergeTruthParticlesTool", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", TruthParticle_FirstXing() )
        kwargs.setdefault("LastXing",  TruthParticle_LastXing() )

    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("InTimeOutputTruthParticleCollKey", overlayFlags.bkgPrefix() + "TruthPileupParticles")
    else:
        kwargs.setdefault("InTimeOutputTruthParticleCollKey", "TruthPileupParticles")

    return CfgMgr.MergeTruthParticlesTool(name, **kwargs)


############################################################################

def getNewMergeMcEventCollTool_Base(name="NewMergeMcEventCollTool_Base", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", -30000)
        kwargs.setdefault("LastXing",   30000)
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TruthCollOutputKey", overlayFlags.bkgPrefix() + "TruthEvent")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent")
    kwargs.setdefault("TruthCollInputKey", "TruthEvent")
    if not digitizationFlags.doXingByXingPileUp(): # Algorithm approach
        kwargs.setdefault("PileUpMergeSvc", "PileUpMergeSvc")
    # Default `PileUpType` to "Unknown"
    kwargs.setdefault("PileUpType", -1)
    return CfgMgr.NewMergeMcEventCollTool(name, **kwargs)

def getNewMergeMcEventCollTool_Signal(name="NewMergeMcEventCollTool_Signal", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", 0)
        kwargs.setdefault("LastXing",  0)
    from Digitization import PileUpEventType
    kwargs.setdefault("PileUpType", PileUpEventType.Signal)
    return getNewMergeMcEventCollTool_Base(name, **kwargs)

### All bunch-crossing pileup `NewMergeMcEventCollTool`s
def getNewMergeMcEventCollTool_MinBias(name="NewMergeMcEventCollTool_MinBias", **kwargs):
    from Digitization import PileUpEventType
    kwargs.setdefault("PileUpType", PileUpEventType.MinimumBias)
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TruthCollOutputKey", overlayFlags.bkgPrefix() + "TruthEvent_PU")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent_PU")
    return getNewMergeMcEventCollTool_Base(name, **kwargs)

def getNewMergeMcEventCollTool_HighPtMinBias(name="NewMergeMcEventCollTool_HighPtMinBias", **kwargs):
    from Digitization import PileUpEventType
    kwargs.setdefault("PileUpType", PileUpEventType.HighPtMinimumBias)
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TruthCollOutputKey", overlayFlags.bkgPrefix() + "TruthEvent_HighPtPU")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent_HighPtPU")
    return getNewMergeMcEventCollTool_Base(name, **kwargs)

def getNewMergeMcEventCollTool_Cavern(name="NewMergeMcEventCollTool_Cavern", **kwargs):
    from Digitization import PileUpEventType
    kwargs.setdefault("PileUpType", PileUpEventType.Cavern)
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TruthCollOutputKey", overlayFlags.bkgPrefix() + "TruthEvent_Cavern")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent_Cavern")
    return getNewMergeMcEventCollTool_Base(name, **kwargs)

def getNewMergeMcEventCollTool_HaloGas(name="NewMergeMcEventCollTool_HaloGas", **kwargs):
    from Digitization import PileUpEventType
    kwargs.setdefault("PileUpType", PileUpEventType.HaloGas)
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TruthCollOutputKey", overlayFlags.bkgPrefix() + "TruthEvent_HaloGas")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent_HaloGas")
    return getNewMergeMcEventCollTool_Base(name, **kwargs)

### InTimeOnly pileup `NewMergeMcEventCollTool`s
def getInTimeOnlyNewMergeMcEventCollTool_Base(name="InTimeOnlyNewMergeMcEventCollTool_Base", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", 0)
        kwargs.setdefault("LastXing",  0)
    return getNewMergeMcEventCollTool_Base(name, **kwargs)

def getInTimeOnlyNewMergeMcEventCollTool_MinBias(name="InTimeOnlyNewMergeMcEventCollTool_MinBias", **kwargs):
    from Digitization import PileUpEventType
    kwargs.setdefault("PileUpType", PileUpEventType.MinimumBias)
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TruthCollOutputKey", overlayFlags.bkgPrefix() + "TruthEvent_PU")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent_PU")
    return getInTimeOnlyNewMergeMcEventCollTool_Base(name, **kwargs)

def getInTimeOnlyNewMergeMcEventCollTool_HighPtMinBias(name="InTimeOnlyNewMergeMcEventCollTool_HighPtMinBias", **kwargs):
    from Digitization import PileUpEventType
    kwargs.setdefault("PileUpType", PileUpEventType.HighPtMinimumBias)
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TruthCollOutputKey", overlayFlags.bkgPrefix() + "TruthEvent_HighPtPU")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent_HighPtPU")
    return getInTimeOnlyNewMergeMcEventCollTool_Base(name, **kwargs)

def getInTimeOnlyNewMergeMcEventCollTool_Cavern(name="InTimeOnlyNewMergeMcEventCollTool_Cavern", **kwargs):
    from Digitization import PileUpEventType
    kwargs.setdefault("PileUpType", PileUpEventType.Cavern)
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TruthCollOutputKey", overlayFlags.bkgPrefix() + "TruthEvent_Cavern")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent_Cavern")
    return getInTimeOnlyNewMergeMcEventCollTool_Base(name, **kwargs)

def getInTimeOnlyNewMergeMcEventCollTool_HaloGas(name="InTimeOnlyNewMergeMcEventCollTool_HaloGas", **kwargs):
    from Digitization import PileUpEventType
    kwargs.setdefault("PileUpType", PileUpEventType.HaloGas)
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TruthCollOutputKey", overlayFlags.bkgPrefix() + "TruthEvent_HaloGas")
    else:
        kwargs.setdefault("TruthCollOutputKey", "TruthEvent_HaloGas")
    return getInTimeOnlyNewMergeMcEventCollTool_Base(name, **kwargs)

############################################################################

# The earliest bunch crossing time for which interactions will be sent
# to the TrackRecordCollection merging code.
def TrackRecord_FirstXing():
    return -1

# The latest bunch crossing time for which interactions will be sent
# to the TrackRecordCollection merging code.
def TrackRecord_LastXing():
    return 1

def getTrackRange(name="TrackRange" , **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', TrackRecord_FirstXing() )
    kwargs.setdefault('LastXing',  TrackRecord_LastXing() )
    kwargs.setdefault('ItemList', ["TrackRecordCollection#MuonExitLayer"] )
    #"TrackRecordCollection#MuonEntryLayer",
    #"TrackRecordCollection#CaloEntryLayer"
    return CfgMgr.PileUpXingFolder(name, **kwargs)

def MergeTrackRecordCollTool(name="MergeTrackRecordCollTool", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", TrackRecord_FirstXing() )
        kwargs.setdefault("LastXing",  TrackRecord_LastXing() )

    return CfgMgr.MergeTrackRecordCollTool(name, **kwargs)

def MergeCaloEntryLayerTool(name="MergeCaloEntryLayerTool", **kwargs):
    kwargs.setdefault("TrackRecordCollKey", "CaloEntryLayer" )
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TrackRecordCollOutputKey", overlayFlags.bkgPrefix() + "CaloEntryLayer")
    else:
        kwargs.setdefault("TrackRecordCollOutputKey", "CaloEntryLayer" )
    return MergeTrackRecordCollTool(name, **kwargs)

def MergeMuonEntryLayerTool(name="MergeMuonEntryLayerTool", **kwargs):
    kwargs.setdefault("TrackRecordCollKey", "MuonEntryLayer" )
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TrackRecordCollOutputKey", overlayFlags.bkgPrefix() + "MuonEntryLayer")
    else:
        kwargs.setdefault("TrackRecordCollOutputKey", "MuonEntryLayer" )
    return MergeTrackRecordCollTool(name, **kwargs)

def MergeMuonExitLayerTool(name="MergeMuonExitLayerTool", **kwargs):
    kwargs.setdefault("TrackRecordCollKey", "MuonExitLayer" )
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("TrackRecordCollOutputKey", overlayFlags.bkgPrefix() + "MuonExitLayer")
    else:
        kwargs.setdefault("TrackRecordCollOutputKey", "MuonExitLayer" )
    return MergeTrackRecordCollTool(name, **kwargs)


############################################################################
def MergeHijingParsTool(name="MergeHijingParsTool", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", -1)
        kwargs.setdefault("LastXing",  +1)

    return CfgMgr.MergeHijingParsTool(name, **kwargs)


############################################################################

# The earliest bunch crossing time for which interactions will be sent
# to the CaloCalibrationHitContainer merging code.
def CalibHit_FirstXing():
    return -1

# The latest bunch crossing time for which interactions will be sent
# to the Truth jet merging code.
def CalibHit_LastXing():
    return 1

def getCalibRange(name="CalibRange", **kwargs):
    # bunch crossing range in ns
    kwargs.setdefault('FirstXing', CalibHit_FirstXing() )
    kwargs.setdefault('LastXing',  CalibHit_LastXing() )
    ItemList = []
    ItemList += ["CaloCalibrationHitContainer#LArCalibrationHitActive",
                 "CaloCalibrationHitContainer#LArCalibrationHitDeadMaterial",
                 "CaloCalibrationHitContainer#LArCalibrationHitInactive" ]
    from Digitization.DigitizationFlags import digitizationFlags
    if 'OldTileCalibHitContainers' in digitizationFlags.experimentalDigi():
        ItemList += ["CaloCalibrationHitContainer#TileCalibrationCellHitCnt",
                     "CaloCalibrationHitContainer#TileCalibrationDMHitCnt" ]
    else:
        ItemList += ["CaloCalibrationHitContainer#TileCalibHitActiveCell",
                     "CaloCalibrationHitContainer#TileCalibHitInactiveCell",
                     "CaloCalibrationHitContainer#TileCalibHitDeadMaterial" ]
    kwargs.setdefault('ItemList', ItemList)
    return CfgMgr.PileUpXingFolder(name, **kwargs)

def MergeCalibHitsTool(name="MergeCalibHitsTool", **kwargs):
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", CalibHit_FirstXing() )
        kwargs.setdefault("LastXing",  CalibHit_LastXing() )
    if 'OldTileCalibHitContainers' in digitizationFlags.experimentalDigi():
        kwargs.setdefault("OldFormat", True)

    return CfgMgr.MergeCalibHitsTool(name, **kwargs)


############################################################################
def MergeGenericMuonSimHitCollTool(name="MergeGenericMuonSimHitCollTool", **kwargs):
    return CfgMgr.MergeGenericMuonSimHitCollTool(name, **kwargs)

def MergeMicromegasSimHitCollTool(name="MergeMicromegasSimHitCollTool", **kwargs):
    kwargs.setdefault("SimHitContainerNames", ["MM_Hits"])
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", -250)
        kwargs.setdefault("LastXing",   200)
    return CfgMgr.MergeGenericMuonSimHitCollTool(name, **kwargs)

def MergeSTGCSimHitCollTool(name="MergeSTGCSimHitCollTool", **kwargs):
    kwargs.setdefault("SimHitContainerNames", ["sTGC_Hits"])
    if digitizationFlags.doXingByXingPileUp(): # PileUpTool approach
        kwargs.setdefault("FirstXing", -50)
        kwargs.setdefault("LastXing",   75)
    return CfgMgr.MergeGenericMuonSimHitCollTool(name, **kwargs)

############################################################################
