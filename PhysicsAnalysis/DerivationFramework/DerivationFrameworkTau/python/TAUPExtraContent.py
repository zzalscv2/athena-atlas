# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# Content included in addition to the smart slimming content

# ==========================================================================================================================
# Extra content
# ==========================================================================================================================

ExtraContentPhotons                              = ["Photons.Loose.Medium.Tight.author.OQ"]
ExtraContentPhotonsTruth                         = ["Photons.truthOrigin.truthParticleLink.truthType"]

ExtraContentElectrons                            = ["Electrons.etcone20.etcone30.etcone40.ptcone20.ptcone30.ptcone40.Loose.Medium.Tight.DFCommonElectronsLHLoose.DFCommonElectronsLHMedium.DFCommonElectronsLHTight.DFCommonElectronsML.author.OQ"]
ExtraContentElectronsTruth                       = ["Electrons.truthOrigin.truthType.truthParticleLink"]

ExtraContentMuons                                = ["Muons.DFCommonGoodMuon.ptcone20.ptcone30.ptcone40.etcone20.etcone30.etcone40"]
ExtraContentMuonsTruth                           = ["MuonTruthParticles.truthOrigin.truthType"]

ExtraContentTaus                                 = [   "TauJets.pt.eta.phi.m.charge.jetLink.isTauFlags.BDTEleScore.BDTJetScore."
                                                     + "caloIso."
                                                     + "isolFrac."
                                                     + "IsTruthMatched."
                                                     + "truthJetLink."
                                                     + "truthParticleLink."
                                                     + "ptDetectorAxis."
                                                     + "etaDetectorAxis."
                                                     + "phiDetectorAxis."
                                                     + "mDetectorAxis."
                                                     + "ptIntermediateAxis."
                                                     + "etaIntermediateAxis."
                                                     + "phiIntermediateAxis."
                                                     + "mIntermediateAxis."
                                                     + "leadTrkPt."
                                                     + "massTrkSys."
                                                     + "trFlightPathSig."
                                                     + "centFrac."
                                                     + "centFracCorrected."
                                                     + "ChPiEMEOverCaloEME."
                                                     + "ChPiEMEOverCaloEMECorrected."
                                                     + "dRmax."
                                                     + "dRmaxCorrected."
                                                     + "etOverPtLeadTrk."
                                                     + "etOverPtLeadTrkCorrected."
                                                     + "EMPOverTrkSysP."
                                                     + "EMPOverTrkSysPCorrected."
                                                     + "innerTrkAvgDist."
                                                     + "innerTrkAvgDistCorrected."
                                                     + "ipSigLeadTrk."
                                                     + "ipSigLeadTrkCorrected."
                                                     + "massTrkSys."
                                                     + "massTrkSysCorrected."
                                                     + "mEflowApprox."
                                                     + "mEflowApproxCorrected."
                                                     + "ptRatioEflowApprox."
                                                     + "ptRatioEflowApproxCorrected."
                                                     + "SumPtTrkFrac."
                                                     + "SumPtTrkFracCorrected."
                                                     + "trFlightPathSig."
                                                     + "trFlightPathSigCorrected."
                                                     + "ptPanTauCellBased."
                                                     + "ptPanTauCellBasedProto."
                                                     + "etaPanTauCellBased."
                                                     + "etaPanTauCellBasedProto."
                                                     + "phiPanTauCellBased."
                                                     + "phiPanTauCellBasedProto."
                                                     + "mPanTauCellBased."
                                                     + "mPanTauCellBasedProto."
                                                     + "pantau_CellBasedInput_BDTValue_1p0n_vs_1p1n."
                                                     + "pantau_CellBasedInput_BDTValue_1p1n_vs_1pXn."
                                                     + "pantau_CellBasedInput_BDTValue_3p0n_vs_3pXn."
                                                     + "pantau_CellBasedInput_BDTVar_Basic_NNeutralConsts."
                                                     + "pantau_CellBasedInput_BDTVar_Charged_HLV_SumM."
                                                     + "pantau_CellBasedInput_BDTVar_Charged_JetMoment_EtDRxTotalEt."
                                                     + "pantau_CellBasedInput_BDTVar_Charged_StdDev_Et_WrtEtAllConsts."
                                                     + "pantau_CellBasedInput_BDTVar_Combined_DeltaR1stNeutralTo1stCharged."
                                                     + "pantau_CellBasedInput_BDTVar_Neutral_HLV_SumM."
                                                     + "pantau_CellBasedInput_BDTVar_Neutral_PID_BDTValues_BDTSort_1."
                                                     + "pantau_CellBasedInput_BDTVar_Neutral_PID_BDTValues_BDTSort_2."
                                                     + "pantau_CellBasedInput_BDTVar_Neutral_Ratio_1stBDTEtOverEtAllConsts."
                                                     + "pantau_CellBasedInput_BDTVar_Neutral_Ratio_EtOverEtAllConsts."
                                                     + "pantau_CellBasedInput_BDTVar_Neutral_Shots_NPhotonsInSeed."
                                                     + "pantau_CellBasedInput_DecayMode."
                                                     + "pantau_CellBasedInput_DecayModeProto."
                                                     + "pantau_CellBasedInput_isPanTauCandidate",
                                                       "TauChargedParticleFlowObjects",
                                                       "TauNeutralParticleFlowObjects",
                                                       "TauHadronicParticleFlowObjects",
                                                       "TauShotParticleFlowObjects",
                                                       "TauPi0Clusters"   ]
ExtraContentTausTruth                            = []

ExtraContentJetsTruth                            = ["AntiKt4TruthJets.TruthLabelDeltaR_B.TruthLabelDeltaR_C.TruthLabelDeltaR_T.TruthLabelID"]

# ==========================================================================================================================

ExtraContentTAUP5                                =   ExtraContentPhotons                  \
                                                     + ExtraContentElectrons                \
                                                     + ExtraContentMuons

ExtraContentTruthTAUP5                           =   ExtraContentPhotonsTruth             \
                                                     + ExtraContentElectronsTruth           \
                                                     + ExtraContentMuonsTruth               \
                                                     + ExtraContentTausTruth                \
                                                     + ExtraContentJetsTruth

# ==========================================================================================================================
# Extra containers
# ==========================================================================================================================

ExtraContainersElectrons                         = ["ForwardElectrons"]

ExtraContainersJets                              = ["CaloCalTopoClusters"]

ExtraContainersTrigger                           = ["LVL1EmTauRoIs",
#
                                                    "HLT_TrigRoiDescriptorCollection_forID",
                                                    "HLT_TrigRoiDescriptorCollection_forID1",
                                                    "HLT_TrigRoiDescriptorCollection_forID2",
                                                    "HLT_TrigRoiDescriptorCollection_forID3",
                                                    "HLT_TrigRoiDescriptorCollection_forMS",
                                                    "HLT_TrigRoiDescriptorCollection_initialRoI",
                                                    "HLT_TrigRoiDescriptorCollection_secondaryRoI_EF",
                                                    "HLT_TrigRoiDescriptorCollection_secondaryRoI_HLT",
                                                    "HLT_TrigRoiDescriptorCollection_secondaryRoI_L2",
#
                                                    "HLT_xAOD__EmTauRoIContainer_L1TopoEM",
                                                    "HLT_xAOD__EmTauRoIContainer_L1TopoTau",
#
                                                    "HLT_xAOD__JetContainer_TrigTauJet",
#
                                                    "HLT_xAOD__TauJetContainer_TrigTauRecPreselection",
#
                                                    "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Tau_EFID",
                                                    "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Tau_FTF",
                                                    "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Tau_IDTrig",
                                                    "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_TauCore_FTF",
                                                    "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_TauIso_FTF"]

# ==========================================================================================================================

ExtraContainersTAUP5                             =   ExtraContainersElectrons

ExtraContainersTruthTAUP5                        = ["TruthEvents",
                                                    "TruthParticles",
                                                    "TruthVertices",
                                                    "AntiKt4TruthJets",
                                                    #"BTagging_AntiKt4Truth", JRC TEMPORARILY COMMENTED
                                                    "AntiKt4TruthWZJets"
                                                ]

