# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# List of containers that are made on-the-fly by basically all DAOD types and
# can therefore be reasonably added to the NameAndTypes dictionary centrally
# rather than asking each DAOD to do it themselves


def ContainersOnTheFly(flags=None):

    containers = [
        ['TruthEvents','xAOD::TruthEventContainer'],
        ['TruthEventsAux','xAOD::TruthEventAuxContainer'],
        ['MET_Truth','xAOD::MissingETContainer'],
        ['MET_TruthAux','xAOD::MissingETAuxContainer'],
        ['MET_TruthRegions','xAOD::MissingETContainer'],
        ['MET_TruthRegionsAux','xAOD::MissingETAuxContainer'],
        ['TruthBSM','xAOD::TruthParticleContainer'],
        ['TruthBSMAux','xAOD::TruthParticleAuxContainer'],
        ['TruthBoson','xAOD::TruthParticleContainer'],
        ['TruthBosonAux','xAOD::TruthParticleAuxContainer'],
        ['TruthBottom','xAOD::TruthParticleContainer'],
        ['TruthBottomAux','xAOD::TruthParticleAuxContainer'],
        ['TruthTop','xAOD::TruthParticleContainer'],
        ['TruthTopAux','xAOD::TruthParticleAuxContainer'],
        ['TruthTopQuarkWithDecayParticles','xAOD::TruthParticleContainer'],
        ['TruthTopQuarkWithDecayParticlesAux','xAOD::TruthParticleAuxContainer'],
        ['TruthTopQuarkWithDecayVertices','xAOD::TruthVertexContainer'],
        ['TruthTopQuarkWithDecayVerticesAux','xAOD::TruthVertexAuxContainer'],
        ["TruthMuons","xAOD::TruthParticleContainer"],
        ["TruthMuonsAux","xAOD::TruthParticleAuxContainer"],
        ["TruthElectrons","xAOD::TruthParticleContainer"],
        ["TruthElectronsAux","xAOD::TruthParticleAuxContainer"],
        ["TruthPhotons","xAOD::TruthParticleContainer"],
        ["TruthPhotonsAux","xAOD::TruthParticleAuxContainer"],
        ["TruthNeutrinos","xAOD::TruthParticleContainer"],
        ["TruthNeutrinosAux","xAOD::TruthParticleAuxContainer"],
        ["TruthTaus","xAOD::TruthParticleContainer"],
        ["TruthTausAux","xAOD::TruthParticleAuxContainer"],
        ['TruthTausWithDecayParticles','xAOD::TruthParticleContainer'],
        ["TruthTausWithDecayParticlesAux",'xAOD::TruthParticleAuxContainer'],
        ['TruthTausWithDecayVertices','xAOD::TruthVertexContainer'],
        ['TruthTausWithDecayVerticesAux','xAOD::TruthVertexAuxContainer'],
        ['TruthWbosonWithDecayParticles','xAOD::TruthParticleContainer'],
        ["TruthWbosonWithDecayParticlesAux",'xAOD::TruthParticleAuxContainer'],
        ['TruthWbosonWithDecayVertices','xAOD::TruthVertexContainer'],
        ['TruthWbosonWithDecayVerticesAux','xAOD::TruthVertexAuxContainer'],
        ['TruthForwardProtons','xAOD::TruthParticleContainer'],
        ['TruthForwardProtonsAux','xAOD::TruthParticleAuxContainer'],
        ['BornLeptons','xAOD::TruthParticleContainer'],
        ['BornLeptonsAux','xAOD::TruthParticleAuxContainer'],
        ['TruthBosonsWithDecayParticles','xAOD::TruthParticleContainer'],
        ['TruthBosonsWithDecayParticlesAux','xAOD::TruthParticleAuxContainer'],
        ['TruthBosonsWithDecayVertices','xAOD::TruthVertexContainer'],
        ['TruthBosonsWithDecayVerticesAux','xAOD::TruthVertexAuxContainer'],
        ['TruthBSMWithDecayParticles','xAOD::TruthParticleContainer'],
        ['TruthBSMWithDecayParticlesAux','xAOD::TruthParticleAuxContainer'],
        ['TruthBSMWithDecayVertices','xAOD::TruthVertexContainer'],
        ['TruthBSMWithDecayVerticesAux','xAOD::TruthVertexAuxContainer'],
        ['HardScatterParticles','xAOD::TruthParticleContainer'],
        ['HardScatterParticlesAux','xAOD::TruthParticleAuxContainer'],
        ['HardScatterVertices','xAOD::TruthVertexContainer'],
        ['HardScatterVerticesAux','xAOD::TruthVertexAuxContainer'],
        ['TruthHFWithDecayParticles','xAOD::TruthParticleContainer'],
        ['TruthHFWithDecayParticlesAux','xAOD::TruthParticleAuxContainer'],
        ['TruthHFWithDecayVertices','xAOD::TruthVertexContainer'],
        ['TruthHFWithDecayVerticesAux','xAOD::TruthVertexAuxContainer'],
        ['TruthCharm','xAOD::TruthParticleContainer'],
        ['TruthCharmAux','xAOD::TruthParticleAuxContainer'],
        ['TruthPrimaryVertices','xAOD::TruthVertexContainer'],
        ['TruthPrimaryVerticesAux','xAOD::TruthVertexAuxContainer'],
        ["AntiKt2TruthJets","xAOD::JetContainer"],
        ["AntiKt2TruthJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt4TruthJets","xAOD::JetContainer"],
        ["AntiKt4TruthJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt4TruthWZJets","xAOD::JetContainer"],
        ["AntiKt4TruthWZJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt4TruthDressedWZJets","xAOD::JetContainer"],
        ["AntiKt4TruthDressedWZJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt2PV0TrackJets","xAOD::JetContainer"],
        ["AntiKt2PV0TrackJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt4PV0TrackJets","xAOD::JetContainer"],
        ["AntiKt4PV0TrackJetsAux","xAOD::JetAuxContainer"],
        ["AntiKtVR30Rmax4Rmin02PV0TrackJets","xAOD::JetContainer"],
        ["AntiKtVR30Rmax4Rmin02PV0TrackJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt2LCTopoJets","xAOD::JetContainer"],
        ["AntiKt2LCTopoJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt4EMTopoJets","xAOD::JetContainer"],
        ["AntiKt4EMTopoJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt4LCTopoJets","xAOD::JetContainer"],
        ["AntiKt4LCTopoJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt4EMPFlowJets","xAOD::JetContainer"],
        ["AntiKt4EMPFlowJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt4EMTopoCSSKJets","xAOD::JetContainer"],
        ["AntiKt4EMTopoCSSKJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt4EMPFlowCSSKJets","xAOD::JetContainer"],
        ["AntiKt4EMPFlowCSSKJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt4UFOCSSKJets","xAOD::JetContainer"],
        ["AntiKt4UFOCSSKJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt4UFOCSSKLowPtJets","xAOD::JetContainer"],
        ["AntiKt4UFOCSSKLowPtJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt10PV0TrackJets","xAOD::JetContainer"],
        ["AntiKt10PV0TrackJetsAux","xAOD::JetAuxContainer"],

        ["AntiKt10LCTopoJets","xAOD::JetContainer"],
        ["AntiKt10LCTopoJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets","xAOD::JetContainer"],
        ["AntiKt10LCTopoTrimmedPtFrac5SmallR20JetsAux","xAOD::JetAuxContainer"],
        ["AntiKt10TruthTrimmedPtFrac5SmallR20Jets","xAOD::JetContainer"],
        ["AntiKt10TruthTrimmedPtFrac5SmallR20JetsAux","xAOD::JetAuxContainer"],
        ["AntiKt10TrackCaloClusterJets","xAOD::JetContainer"],
        ["AntiKt10TrackCaloClusterJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt10UFOCSSKJets","xAOD::JetContainer"],
        ["AntiKt10UFOCSSKJetsAux","xAOD::JetAuxContainer"],
        ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets","xAOD::JetContainer"],
        ["AntiKt10UFOCSSKSoftDropBeta100Zcut10JetsAux","xAOD::JetAuxContainer"],
        ["AntiKt10TruthSoftDropBeta100Zcut10Jets","xAOD::JetContainer"],
        ["AntiKt10TruthSoftDropBeta100Zcut10JetsAux","xAOD::JetAuxContainer"],
        ["AntiKt10TruthJets","xAOD::JetContainer"],
        ["AntiKt10TruthJetsAux","xAOD::JetAuxContainer"],

        ["Kt4EMPFlowEventShape","xAOD::EventShape"],
        ["Kt4EMPFlowEventShapeAux","xAOD::EventShapeAuxInfo"],
        ["Kt4EMPFlowPUSBEventShape","xAOD::EventShape"],
        ["Kt4EMPFlowPUSBEventShapeAux","xAOD::EventShapeAuxInfo"],
        ["Kt4EMPFlowNeutEventShape","xAOD::EventShape"],
        ["Kt4EMPFlowNeutEventShapeAux","xAOD::EventShapeAuxInfo"],
        ["Kt4EMTopoOriginEventShape","xAOD::EventShape"],
        ["Kt4EMTopoOriginEventShapeAux","xAOD::EventShapeAuxInfo"],

        ["TrackCaloClustersCombinedAndNeutral","xAOD::TrackCaloClusterContainer"],
        ["TrackCaloClustersCombinedAndNeutralAux","xAOD::TrackCaloClusterAuxContainer"],
        ["BTagging_AntiKt4EMPFlow","xAOD::BTaggingContainer"],
        ["BTagging_AntiKt4EMPFlowAux","xAOD::BTaggingAuxContainer"],
        ["BTagging_AntiKt4EMTopo","xAOD::BTaggingContainer"],
        ["BTagging_AntiKt4EMTopoAux","xAOD::BTaggingAuxContainer"],
        ["BTagging_AntiKtVR30Rmax4Rmin02Track","xAOD::BTaggingContainer"],
        ["BTagging_AntiKtVR30Rmax4Rmin02TrackAux","xAOD::BTaggingAuxContainer"],
        ["DiTauJetsLowPt","xAOD::DiTauJetContainer"],
        ["DiTauJetsLowPtAux","xAOD::DiTauJetAuxContainer"],
        ["MET_Reference_AntiKt4EMTopo",'xAOD::MissingETContainer'],
        ["MET_Reference_AntiKt4EMTopoAux",'xAOD::MissingETAuxContainer'],
        ["MET_Core_AntiKt4EMTopo",'xAOD::MissingETContainer'],
        ["MET_Core_AntiKt4EMTopoAux",'xAOD::MissingETAuxContainer'],
        ["METAssoc_AntiKt4EMTopo",'xAOD::MissingETAssociationMap'],
        ["METAssoc_AntiKt4EMTopoAux",'xAOD::MissingETAuxAssociationMap'],
        ["MET_Core_AntiKt4EMTopo_LRT",'xAOD::MissingETContainer'],
        ["MET_Core_AntiKt4EMTopo_LRTAux",'xAOD::MissingETAuxContainer'],
        ["METAssoc_AntiKt4EMTopo_LRT",'xAOD::MissingETAssociationMap'],
        ["METAssoc_AntiKt4EMTopo_LRTAux",'xAOD::MissingETAuxAssociationMap'],
        ["MET_Reference_AntiKt4EMPFlow",'xAOD::MissingETContainer'],
        ["MET_Reference_AntiKt4EMPFlowAux",'xAOD::MissingETAuxContainer'],
        ["MET_Core_AntiKt4EMPFlow",'xAOD::MissingETContainer'],
        ["MET_Core_AntiKt4EMPFlowAux",'xAOD::MissingETAuxContainer'],
        ["METAssoc_AntiKt4EMPFlow",'xAOD::MissingETAssociationMap'],
        ["METAssoc_AntiKt4EMPFlowAux",'xAOD::MissingETAuxAssociationMap'],
        ["MET_Core_AntiKt4EMPFlow_LRT",'xAOD::MissingETContainer'],
        ["MET_Core_AntiKt4EMPFlow_LRTAux",'xAOD::MissingETAuxContainer'],
        ["METAssoc_AntiKt4EMPFlow_LRT",'xAOD::MissingETAssociationMap'],
        ["METAssoc_AntiKt4EMPFlow_LRTAux",'xAOD::MissingETAuxAssociationMap'],

        ["EMOriginTopoClusters","xAOD::CaloClusterContainer"],
        ["EMOriginTopoClustersAux","xAOD::ShallowAuxContainer"],
        ["LCOriginTopoClusters","xAOD::CaloClusterContainer"],
        ["LCOriginTopoClustersAux","xAOD::ShallowAuxContainer"],

        ["BTagging_AntiKt4EMPFlowJFVtx","xAOD::BTagVertexContainer"],
        ["BTagging_AntiKt4EMPFlowJFVtxAux","xAOD::BTagVertexAuxContainer"],
        ["BTagging_AntiKt4EMPFlowSecVtx","xAOD::VertexContainer"],
        ["BTagging_AntiKt4EMPFlowSecVtxAux","xAOD::VertexAuxContainer"],

        ["GlobalChargedParticleFlowObjects","xAOD::FlowElementContainer"],
        ["GlobalChargedParticleFlowObjectsAux","xAOD::FlowElementAuxContainer"],
        ["GlobalNeutralParticleFlowObjects","xAOD::FlowElementContainer"],
        ["GlobalNeutralParticleFlowObjectsAux","xAOD::FlowElementAuxContainer"],

        ["CHSGChargedParticleFlowObjects","xAOD::FlowElementContainer"],
        ["CHSGChargedParticleFlowObjectsAux","xAOD::ShallowAuxContainer"],
        ["CHSGNeutralParticleFlowObjects","xAOD::FlowElementContainer"],
        ["CHSGNeutralParticleFlowObjectsAux","xAOD::ShallowAuxContainer"],

        ['UFOCSSK','xAOD::FlowElementContainer'],
        ['UFOCSSKAux','xAOD::FlowElementAuxContainer'],

    ]

    if flags is not None and flags.Tracking.doPseudoTracking:
        containers += [
            ["InDetReplacedWithPseudoTrackParticles","xAOD::TrackParticleContainer"],
            ["InDetReplacedWithPseudoTrackParticlesAux","xAOD::TrackParticleAuxContainer"],
            ["InDetReplacedWithPseudoFromBTrackParticles","xAOD::TrackParticleContainer"],
            ["InDetReplacedWithPseudoFromBTrackParticlesAux","xAOD::TrackParticleAuxContainer"],
            ["InDetReplacedWithPseudoNotFromBTrackParticles","xAOD::TrackParticleContainer"],
            ["InDetReplacedWithPseudoNotFromBTrackParticlesAux","xAOD::TrackParticleAuxContainer"],
            ["InDetPlusPseudoTrackParticles","xAOD::TrackParticleContainer"],
            ["InDetPlusPseudoTrackParticlesAux","xAOD::TrackParticleAuxContainer"],
            ["InDetPlusPseudoFromBTrackParticles","xAOD::TrackParticleContainer"],
            ["InDetPlusPseudoFromBTrackParticlesAux","xAOD::TrackParticleAuxContainer"],
            ["InDetPlusPseudoNotFromBTrackParticles","xAOD::TrackParticleContainer"],
            ["InDetPlusPseudoNotFromBTrackParticlesAux","xAOD::TrackParticleAuxContainer"],
            ["InDetNoFakesTrackParticles","xAOD::TrackParticleContainer"],
            ["InDetNoFakesTrackParticlesAux","xAOD::TrackParticleAuxContainer"],
            ["InDetNoFakesFromBTrackParticles","xAOD::TrackParticleContainer"],
            ["InDetNoFakesFromBTrackParticlesAux","xAOD::TrackParticleAuxContainer"],
            ["InDetNoFakesNotFromBTrackParticles","xAOD::TrackParticleContainer"],
            ["InDetNoFakesNotFromBTrackParticlesAux","xAOD::TrackParticleAuxContainer"],
            ["InDetSiSPSeededTracksParticles","xAOD::TrackParticleContainer"],
            ["InDetSiSPSeededTracksParticlesAux","xAOD::TrackParticleAuxContainer"]
        ]

    return containers
