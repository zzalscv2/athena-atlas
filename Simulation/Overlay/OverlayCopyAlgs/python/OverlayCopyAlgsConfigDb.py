# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addAlgorithm

addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyCaloCalibrationHitContainer", "CopyCaloCalibrationHitContainer")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyInTimeAntiKt4JetTruthInfo", "CopyInTimeAntiKt4JetTruthInfo")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyOutOfTimeAntiKt4JetTruthInfo", "CopyOutOfTimeAntiKt4JetTruthInfo")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyInTimeAntiKt6JetTruthInfo", "CopyInTimeAntiKt6JetTruthInfo")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyOutOfTimeAntiKt6JetTruthInfo", "CopyOutOfTimeAntiKt6JetTruthInfo")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyPileupParticleTruthInfo", "CopyPileupParticleTruthInfo")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyMcEventCollection", "CopyMcEventCollection")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyTrackRecordCollection", "CopyTrackRecordCollection")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyTrackCollection", "CopyTrackCollection")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyDetailedTrackTruthCollection", "CopyDetailedTrackTruthCollection")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyPRD_MultiTruthCollection", "CopyPRD_MultiTruthCollection")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyPixelClusterContainer", "CopyPixelClusterContainer")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopySCT_ClusterContainer", "CopySCT_ClusterContainer")
addAlgorithm("OverlayCopyAlgs.OverlayCopyAlgsConfigLegacy.getCopyTRT_DriftCircleContainer", "CopyTRT_DriftCircleContainer")
