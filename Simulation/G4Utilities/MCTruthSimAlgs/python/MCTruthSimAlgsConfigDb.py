# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool

addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getTrackRange"                  , "TrackRange" )
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.MergeTrackRecordCollTool"       , "MergeTrackRecordCollTool")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.MergeCaloEntryLayerTool"        , "MergeCaloEntryLayerTool")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.MergeMuonEntryLayerTool"        , "MergeMuonEntryLayerTool")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.MergeMuonExitLayerTool"         , "MergeMuonExitLayerTool")

addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getCalibRange"                  , "CalibRange")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.MergeCalibHitsTool"             , "MergeCalibHitsTool")

addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getTruthJetRange"               , "TruthJetRange")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getMergeAntiKt4TruthJetsTool"          , "MergeAntiKt4TruthJetsTool")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getMergeAntiKt6TruthJetsTool"          , "MergeAntiKt6TruthJetsTool")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getMergeTruthJetsFilterTool"    , "MergeTruthJetsFilterTool")

addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getTruthParticleRange"          , "TruthParticleRange")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getMergeTruthParticlesTool"  , "MergeTruthParticlesTool")

addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getTimingObjRange"              , "TimingObjRange")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.MergeRecoTimingObjTool"         , "MergeRecoTimingObjTool")

addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.MergeHijingParsTool"            , "MergeHijingParsTool")

addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.MergeMcEventCollTool"           , "MergeMcEventCollTool")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getNewMergeMcEventCollTool_Signal"        , "NewMergeMcEventCollTool_Signal")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getNewMergeMcEventCollTool_MinBias"        , "NewMergeMcEventCollTool_MinBias")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getNewMergeMcEventCollTool_HighPtMinBias"        , "NewMergeMcEventCollTool_HighPtMinBias")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getNewMergeMcEventCollTool_Cavern"        , "NewMergeMcEventCollTool_Cavern")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.getNewMergeMcEventCollTool_HaloGas"        , "NewMergeMcEventCollTool_HaloGas")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.NewMergeMcEventCollTool"        , "InTimeOnlyNewMergeMcEventCollTool_MinBias")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.NewMergeMcEventCollTool"        , "InTimeOnlyNewMergeMcEventCollTool_HighPtMinBias")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.NewMergeMcEventCollTool"        , "InTimeOnlyNewMergeMcEventCollTool_Cavern")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.NewMergeMcEventCollTool"        , "InTimeOnlyNewMergeMcEventCollTool_HaloGas")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.SignalOnlyMcEventCollTool"      , "SignalOnlyMcEventCollTool")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.InTimeOnlyMcEventCollTool"      , "InTimeOnlyMcEventCollTool")

addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.MergeGenericMuonSimHitCollTool" , "MergeGenericMuonSimHitCollTool")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.MergeMicromegasSimHitCollTool"  , "MergeMicromegasSimHitCollTool")
addTool("MCTruthSimAlgs.MCTruthSimAlgsConfigLegacy.MergeSTGCSimHitCollTool"        , "MergeSTGCSimHitCollTool")
