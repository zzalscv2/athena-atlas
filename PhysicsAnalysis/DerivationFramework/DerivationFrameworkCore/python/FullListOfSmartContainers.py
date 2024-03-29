# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def FullListOfSmartContainers(flags=None):

   containers = [
      "EventInfo",
      "Electrons",
      "LRTElectrons",
      "Photons",
      "Muons",
      "MuonsLRT",
      "TauJets",
      "DiTauJets",
      "DiTauJetsLowPt",
      "TauJets_MuonRM",
      "TauJets_EleRM",
      "MET_Baseline_AntiKt4EMTopo",
      "MET_Baseline_AntiKt4EMPFlow",
      "AntiKt2TruthJets",
      "AntiKt4TruthJets",
      "AntiKt4TruthWZJets",
      "AntiKt4TruthDressedWZJets",
      "AntiKt10TruthJets",
      "AntiKt10TruthWZJets",
      "AntiKt10LCTopoJets",
      "AntiKt10TrackCaloClusterJets",
      "AntiKt10UFOCSSKJets",
      "AntiKt10UFOCHSJets",
      "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
      "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
      "AntiKt10TrackCaloClusterTrimmedPtFrac5SmallR20Jets",
      "AntiKt10TruthSoftDropBeta100Zcut10Jets",
      "AntiKt10TruthDressedWZSoftDropBeta100Zcut10Jets",
      "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
      "AntiKt10UFOCHSSoftDropBeta100Zcut10Jets",
      "AntiKt4LCTopoJets",
      "AntiKt4EMTopoJets",
      "AntiKt4EMPFlowJets",
      "AntiKt4EMPFlowByVertexJets",
      "AntiKt4UFOCSSKJets",
      "AntiKt4UFOCSSKLowPtJets",
      "AntiKt4EMTopoNoPtCutJets",
      "AntiKt4EMTopoLowPtJets",
      "AntiKt4EMPFlowLowPtJets",
      "AntiKt2LCTopoJets",
      "AntiKtVR30Rmax4Rmin02PV0TrackJets",
      "BTagging_AntiKt4EMPFlow",
      "BTagging_AntiKt4EMPFlow_expert",
      "BTagging_AntiKtVR30Rmax4Rmin02Track_expert",
      "BTagging_AntiKt4EMTopo",
      "BTagging_AntiKtVR30Rmax4Rmin02Track",
      "BTagging_AntiKt4HI",
      "BTagging_DFAntiKt4HI",
      "InDetTrackParticles",
      "InDetLargeD0TrackParticles",
      "PrimaryVertices",
      "HLT_xAOD__MuonContainer_MuonEFInfo",
      "HLT_xAOD__PhotonContainer_egamma_Photons",
      "HLT_xAOD__JetContainer_a4tcemsubjesFS",
      "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET",
      "HLT_xAOD__TauJetContainer_TrigTauRecMerged",
      "HLT_xAOD__BTaggingContainer_HLTBjetFex",
      "HLT_xAOD__TrigBphysContainer_EFBMuMuFex",
      "HLT_xAOD__TrigVertexCountsContainer_vertexcounts",
      "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf",
      "HLT_IDVertex_FS",
      "HLT_IDTrack_FS_FTF"
   ]

   if flags is not None and flags.Tracking.doPseudoTracking:
      containers += [
         "InDetPseudoTrackParticles",
         "InDetReplacedWithPseudoTrackParticles",
         "InDetReplacedWithPseudoFromBTrackParticles",
         "InDetReplacedWithPseudoNotFromBTrackParticles",
         "InDetPlusPseudoTrackParticles",
         "InDetPlusPseudoFromBTrackParticles",
         "InDetPlusPseudoNotFromBTrackParticles",
         "InDetNoFakesTrackParticles",
         "InDetNoFakesFromBTrackParticles",
         "InDetNoFakesNotFromBTrackParticles",
         "InDetSiSPSeededTracksParticles"
      ]

   return containers
