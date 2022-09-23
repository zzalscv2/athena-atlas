# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Content included in addition to the smart slimming content

# ==========================================================================================================================
# Extra content
# ==========================================================================================================================

ExtraContentTracks               = [    "InDetTrackParticles"
                                     + ".eProbabilityHT" 
                                     + ".truthOrigin"
                                     + ".truthType" 
                                     , "GSFTrackParticles" 
                                     + ".z0.d0.vz.definingParametersCovMatrix.truthOrigin.truthType"
                                     + ".charge"
                                     + ".parameterX"
                                     + ".parameterPX"
                                     + ".parameterPY"
                                     + ".parameterPZ"
                                     + ".eProbabilityHT"
                                     + ".parameterPosition" 
                                     ,"PrimaryVertices.covariance"
                                   ]

ExtraContentCaloClusters       = [    "CaloCalTopoClusters"
                                        + ".rawE"
                                        + ".rawEta"
                                        + ".rawPhi"
                                        + ".rawM"
                                        + ".calE"
                                        + ".calEta"
                                        + ".calPhi"
                                        + ".calM"
                                        + ".e_sampl" 
                                        + ".SECOND_R"
                                        + ".SECOND_LAMBDA"
                                        + ".CENTER_LAMBDA"
                                      ]

ExtraContentElectrons                = [ "Electrons" 
                                         + ".DFCommonElectronsLHTight" 
                                         + ".DFCommonElectronsLHMedium"
                                         + ".DFCommonElectronsLHLoose"
                                         + ".author"
                                         + ".charge" 
                                         + ".truthOrigin" 
                                         + ".truthType"
                                         + ".TruthLink"
                                         + ".ptvarcone20_TightTTVA_pt1000" 
                                         + ".ptvarcone20"
                                         + ".ptcone20_TightTTVALooseCone_pt500"
                                         + ".ptvarcone30_TightTTVA_pt1000"
                                         + ".ptcone30"
                                         + ".ptvarcone30_TightTTVALooseCone_pt500"
                                         + ".ptcone40"
                                         + ".phi"
                                         + ".eta" 
                                         + ".f1" 
                                         + ".OQ"
                                         + ".etcone20"
                                         + ".topoetcone20"
                                         + ".topoetcone20ptCorrection"
                                         + ".etcone30"
                                         + ".etcone40" 
                                         + ".Reta"
                                         + ".Rphi" 
                                         + ".Rhad1"
                                         + ".Rhad" 
                                         + ".wtots1"
                                         + ".weta2" 
                                         + ".f1" 
                                         + ".Eratio"
                                         + ".f3"
                                         + ".deltaEta1"
                                         + ".deltaPhiRescaled2" 
                                         + ".TruthClassifierFallback_truthType" 
                                         + ".TruthClassifierFallback_truthOrigin"
                                         + ".TruthClassifierFallback_dR"
                                   ]

ExtraContentMuons                 = [   "Muons" 
                                     + ".charge.quality.IDqlt"
                                     + ".etcone20.etcone30.etcone40.ptcone20.ptcone30.ptcone40"
                                     + ".Loose.Medium.Tight" 
                                     + ".clusterLink"
                                     + ".TruthLink"
                                    ]
 
ExtraContentMRTaus                  = [   "MuRmTauJets"
                                      + ".muOLdR"
                                      + ".isTauFlags"
                                      + ".RNNJetScore.RNNJetScoreSigTrans"
                                      + ".overlapMuonTrack"
                                      + ".overlapMuonCluster"
                                      + ".overlapMuonQuality"
                                      + ".IsTruthMatched"
                                      + ".nTrackNoClassify" 
                                      + ".tauTrackLinks"
                                      + ".nChargedTracks"
                                      + ".nIsolatedTracks"
                                      + ".truthParticleLink" 
                                      + ".truthOrigin"
                                      + ".truthType" 
                                      + ".truthJetLink"
                                      + ".clusterLinks"
                                      + ".centFrac"
                                      + ".etOverPtLeadTrk"
                                      + ".innerTrkAvgDist"
                                      + ".ipSigLeadTrk"
                                      + ".SumPtTrkFrac"
                                      + ".EMPOverTrkSysP"
                                      + ".ptRatioEflowApprox"
                                      + ".mEflowApprox"
                                      + ".dRmax"
                                      + ".trFlightPathSig"
                                      + ".massTrkSys"
                                      + ".ptDetectorAxis"
                                      + ".ptIntermediateAxis"
                                      + ".ptJetSeed"
                                      + ".BDTJetScoreSigTrans"
                                      + ".BDTEleScoreSigTrans_retuned"
                                    ]

ExtraContentTauTracksVertex       = [    "MuRmTauTracks" 
                                      + ".pt.eta.phi.flagSet.trackLinks.bdtScores" ,
                                         "MuRmTauSecondaryVertices"
                                      + ".x.y.z.covariance.trackParticleLinks"
                                    ]

ExtraContentOrignalTAU            = [   "TauJets" 
                                      + ".overlapMuonTrack"
                                      + ".overlapMuonCluster"
                                      + ".overlapMuonQuality"
                                      + ".RNNJetScore.RNNJetScoreSigTrans"
                                      + ".IsTruthMatched"
                                      + ".tauTrackLinks"
                                      + ".nChargedTracks"
                                      + ".nIsolatedTracks"
                                      + ".truthParticleLink" 
                                      + ".truthOrigin"
                                      + ".truthType" 
                                      + ".truthJetLink"
                                      + ".clusterLinks"
                                      + ".centFrac"
                                      + ".etOverPtLeadTrk"
                                      + ".innerTrkAvgDist"
                                      + ".ipSigLeadTrk"
                                      + ".SumPtTrkFrac"
                                      + ".EMPOverTrkSysP"
                                      + ".ptRatioEflowApprox"
                                      + ".mEflowApprox"
                                      + ".dRmax"
                                      + ".trFlightPathSig"
                                      + ".massTrkSys"
                                      + ".ptDetectorAxis"
                                      + ".ptIntermediateAxis"
                                      + ".ptJetSeed"
                                      + ".BDTJetScoreSigTrans"
                                      + ".BDTEleScoreSigTrans_retuned" ,
                                        "TauTracks"
                                      + ".pt.eta.phi.flagSet.trackLinks.bdtScores" ,
                                      "METAssoc_AntiKt4EMPFlow", "METAssoc_AntiKt4EMPFlowAux.",
                                      "MET_Core_AntiKt4EMPFlow", "MET_Core_AntiKt4EMPFlowAux.",
                                      "METAssoc_AntiKt4EMTopo", "METAssoc_AntiKt4EMTopoAux.",
                                      "MET_Core_AntiKt4EMTopo", "MET_Core_AntiKt4EMTopoAux."
                                    ]

ExtraContentJets                = [  "AntiKt4EMTopoJets"
                                    + ".ActiveArea4vec_eta"
                                    + ".ActiveArea4vec_m" 
                                    + ".ActiveArea4vec_phi"
                                    + ".ActiveArea4vec_pt" 
                                    + ".DetectorEta"
                                    + ".DetectorPhi"
                                    , "AntiKt4EMPFlowJets"
                                    + ".ActiveArea4vec_eta" 
                                    + ".ActiveArea4vec_m" 
                                    + ".ActiveArea4vec_phi"
                                    + ".ActiveArea4vec_pt" 
                                    + ".DetectorEta"
                                    + ".DetectorPhi"
                                    , "BTagging_AntiKt4EMPFlow_201903"
                                    + ".ActiveArea4vec_eta"
                                    + ".ActiveArea4vec_m"
                                    + ".ActiveArea4vec_phi"
                                    + ".ActiveArea4vec_pt"
                                    + ".DetectorEta"
                                    + ".DetectorPhi"
                                    + ".MV1_discriminant"
                                    + ".MV1c_discriminant"
                                    , "BTagging_AntiKt4EMTopo_201810"
                                    + ".ActiveArea4vec_eta"
                                    + ".ActiveArea4vec_m"
                                    + ".ActiveArea4vec_phi"
                                    + ".ActiveArea4vec_pt"
                                    + ".DetectorEta"
                                    + ".DetectorPhi"
                                    + ".MV1_discriminant" 
                                    + ".MV1c_discriminant"
                                  ]

ExtraContentMET      = [ 
                         "METAssoc_AntiKt4EMPFlow_MuRmTauJets",
                         "METAssoc_AntiKt4EMPFlow_MuRmTauJetsAux.name.mpx.mpy.sumet.source",
                         "MET_Core_AntiKt4EMPFlow_MuRmTauJets",
                         "MET_Core_AntiKt4EMPFlow_MuRmTauJetsAux.name.mpx.mpy.sumet.source",
                         "MET_Reference_AntiKt4EMPFlow_MuRmTauJets",
                         "MET_Reference_AntiKt4EMPFlow_MuRmTauJetsAux.name.mpx.mpy.sumet.source",
                         "MET_Truth", "MET_TruthAux.name.mpx.mpy.sumet.source" 
                       ]

ExtraContentTruthEvents        = [   "TruthEvents", 
                                     "TruthParticles.barcode.prodVtxLink.decayVtxLink.status.pdgId.e.m.px.py.pz", 
                                     "TruthVertices.barcode.id.incomingParticleLinks.outgoingParticleLinks",
                                     "AntiKt4TruthJets.eta.m.phi.pt.TruthLabelDeltaR_B.TruthLabelDeltaR_C.TruthLabelDeltaR_T.TruthLabelID.ConeTruthLabelID.PartonTruthLabelID" ,
                                     "TruthTaus.originalTruthParticle.prodVtxLink.decayVtxLink.motherID.pt.eta.phi.m.pdgId.barcode.status.IsHadronicTau.DecayModeVector.numCharged.classifierParticleType.classifierParticleOrigin.numNeutralPion.numNeutral.numChargedPion.pt_vis.eta_vis.phi_vis.m_vis",
                                     "TruthMuons"
                                 ]


# ==========================================================================================================================

ExtraContentTAUP6              =   ExtraContentMuons                    \
                                 + ExtraContentElectrons                \
                                 + ExtraContentMRTaus                   \
                                 + ExtraContentTracks                   \
                                 + ExtraContentTauTracksVertex          \
                                 + ExtraContentMET

ExtraContentTruthTAUP6         =   ExtraContentTruthEvents

