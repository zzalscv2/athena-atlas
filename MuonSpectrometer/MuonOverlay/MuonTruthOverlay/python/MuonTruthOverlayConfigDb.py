# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addAlgorithm
addAlgorithm("MuonTruthOverlay.MuonTruthOverlayConfigLegacy.getCscTruthOverlay", "CscTruthOverlay")
addAlgorithm("MuonTruthOverlay.MuonTruthOverlayConfigLegacy.getMdtTruthOverlay", "MdtTruthOverlay")
addAlgorithm("MuonTruthOverlay.MuonTruthOverlayConfigLegacy.getRpcTruthOverlay", "RpcTruthOverlay")
addAlgorithm("MuonTruthOverlay.MuonTruthOverlayConfigLegacy.getTgcTruthOverlay", "TgcTruthOverlay")
addAlgorithm("MuonTruthOverlay.MuonTruthOverlayConfigLegacy.getSTGC_TruthOverlay", "STGC_TruthOverlay")
addAlgorithm("MuonTruthOverlay.MuonTruthOverlayConfigLegacy.getMM_TruthOverlay", "MM_TruthOverlay")
