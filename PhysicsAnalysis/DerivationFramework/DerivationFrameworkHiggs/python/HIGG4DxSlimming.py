# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

#################################################
# Common code used for the HIGG4 slimming       #
# Z.Zinonos                                     #
# zenon@cern.ch                                 #
# Nov 2015                                      #
#################################################


def setup(HIGG4DxName, HIGG4DxStream, HIGG4DxSlimmingHelper):
    from AthenaCommon.GlobalFlags import globalflags
    DFisMC = (globalflags.DataSource()=='geant4')
    
    #smart slimming
    #main collections:
    if HIGG4DxName in ['HDBS1']:
        HIGG4DxSlimmingHelper.SmartCollections = ["Electrons",
                                              "Muons",
                                              "TauJets",
                                              "DiTauJetsLowPt",
                                              "InDetTrackParticles",
                                              "PrimaryVertices",
                                              "MET_Reference_AntiKt4EMPFlow",
                                              "AntiKt4EMPFlowJets",
                                              "AntiKt4EMPFlowJets_BTagging201810",
                                              "BTagging_AntiKt4EMPFlow_201810",
                                              "AntiKt4EMPFlowJets_BTagging201903",
                                              "BTagging_AntiKt4EMPFlow_201903"
                                              ]
    else:
        HIGG4DxSlimmingHelper.SmartCollections = ["Electrons",
                                              "Muons",
                                              "TauJets",
                                              "MET_Reference_AntiKt4EMTopo",
                                              "AntiKt4EMTopoJets",
                                              "AntiKt4EMTopoJets_BTagging201810",
                                              "BTagging_AntiKt4EMTopo_201810",
                                              "InDetTrackParticles",
                                              "PrimaryVertices",
                                              "MET_Reference_AntiKt4EMPFlow",
                                              "AntiKt4EMPFlowJets",
                                              "AntiKt4EMPFlowJets_BTagging201810",
                                              "BTagging_AntiKt4EMPFlow_201810",
                                              "AntiKt4EMPFlowJets_BTagging201903",
                                              "BTagging_AntiKt4EMPFlow_201903",
                                              ]
    
    # extra containers for some formats                                                  
    if HIGG4DxName in ['HIGG4D1', 'HIGG4D2', 'HIGG4D3', 'HIGG4D5', 'HIGG4D6', 'HIGG6D1', 'HIGG6D2', 'HDBS1']:
        HIGG4DxSlimmingHelper.SmartCollections += ["Photons"]

    if HIGG4DxName in ['HIGG4D2', 'HIGG4D3', 'HIGG6D1', 'HIGG6D2']:
        HIGG4DxSlimmingHelper.SmartCollections += ["AntiKt4LCTopoJets"]  # used as seeds for taus

    if HIGG4DxName in ['HIGG4D2', 'HIGG4D3', 'HIGG4D6']:
        HIGG4DxSlimmingHelper.SmartCollections += ["DiTauJets"]

    #extra variables added to the smart slimming content
    ExtraContentElectrons=[
        "Electrons."
        "LHLoose."
        "LHMedium."
        "LHTight."
        "LHValue"
        ]

    ExtraElectronsTruth=[
        "Electrons."
        "truthOrigin."
        "truthType."
        "truthParticleLink"
        ]

    ExtraContentMuons=[
        "Muons."
        "quality."
        "m"
        ]

    ExtraMuonsTruth=[
        "MuonTruthParticles."
        "truthOrigin."
        "truthType"
        ]

    ExtraContentTaus=[
        "TauJets."
        "pantau_CellBasedInput_isPanTauCandidate."
        "pantau_CellBasedInput_DecayMode."
        "ptPanTauCellBased."
        "etaPanTauCellBased."
        "phiPanTauCellBased."
        "mPanTauCellBased."
        "pantau_CellBasedInput_BDTValue_1p0n_vs_1p1n."
        "pantau_CellBasedInput_BDTValue_1p1n_vs_1pXn."
        "pantau_CellBasedInput_BDTValue_3p0n_vs_3pXn."
        "ele_match_lhscore."
        "ele_olr_pass."
        "electronLink."
        "seedTrackWidthPt500."
        "seedTrackWidthPt1000"
        ,
        "TauNeutralParticleFlowObjects."
        "pt."
        "eta."
        "phi."
        "m."
        "e."
        "rapidity."
        "bdtPi0Score"
        ,
        "TauChargedParticleFlowObjects."
        "pt."
        "eta."
        "phi."
        "m"
        ]
        
    # add tau-ID variables needed to rerun tau ID for HiggsCP analysis
    if HIGG4DxName == 'HIGG4D3':
        ExtraContentTaus[0] += ".centFrac.ChPiEMEOverCaloEME.dRmax.etOverPtLeadTrk.EMPOverTrkSysP.innerTrkAvgDist.ipSigLeadTrk.absipSigLeadTrk.massTrkSys.mEflowApprox.ptRatioEflowApprox.SumPtTrkFrac.trFlightPathSig"
        ExtraContentTaus += ["TauTracks.CaloSamplingEtaEM.CaloSamplingEtaHad.CaloSamplingPhiEM.CaloSamplingPhiHad"]

    if HIGG4DxName in ['HIGG4D2', 'HIGG4D3']:
        # further extra content for CP analysis
        ExtraContentTaus[0] += ".pt.eta.phi.m.tauTrackLinks.jetLink.vertexLink.secondaryVertexLink.hadronicPFOLinks.shotPFOLinks.chargedPFOLinks.neutralPFOLinks.pi0PFOLinks.protoChargedPFOLinks.protoNeutralPFOLinks.protoPi0PFOLinks.charge.isTauFlags.BDTJetScore.BDTEleScore.conversionTrackLinks.charged_PFOLinks.neutral_PFOLinks.pi0_PFOLinks.cellBased_Charged_PFOLinks.cellBased_Neutral_PFOLinks.cellBased_Pi0_PFOLinks.eflowRec_Charged_PFOLinks.eflowRec_Neutral_PFOLinks.eflowRec_Pi0_PFOLinks.shot_PFOLinks.ptFinalCalib.etaFinalCalib.phiFinalCalib.mFinalCalib.ele_match_lhscore.ele_olr_pass.electronLink.EleMatchLikelihoodScore.pt_combined.eta_combined.phi_combined.m_combined.BDTJetScoreSigTrans.BDTEleScoreSigTrans.PanTau_DecayMode.RNNJetScore.RNNJetScoreSigTrans.seedJetWidth.seedJetJvt.BDTEleScore_retuned.BDTEleScoreSigTrans_retuned.BDTEleLoose_retuned.BDTEleMedium_retuned.BDTEleTight_retuned.seedTrackWidthPt1000.truthParticleLink.pi0Links"
        ExtraContentTaus += [ "TauTruthParticles.IsHadronicTau.originalTruthParticle.numCharged.numChargedPions.numNeutral.numNeutralPions.pt_vis.eta_vis.phi_vis.m_vis.pt_prompt.eta_prompt.phi_prompt.m_prompt.pt_vis_charged.eta_vis_charged.phi_vis_charged.m_vis_charged.pt_vis_neutral.eta_vis_neutral.phi_vis_neutral.m_vis_neutral.DecayModeVector" ]


    ExtraTausTruth = [
        "TauJets.IsTruthMatched.truthParticleLink.truthJetLink"
        ]
    

    if HIGG4DxName == 'HIGG4D3':
        ExtraContentVtx=["PrimaryVertices.x.y.z.vertexType.TauRefittedPVLink.chiSquared.numberDoF"]
    else:
        ExtraContentVtx=["PrimaryVertices.x.y.z.vertexType"]

    if HIGG4DxName == 'HIGG4D6':
        ExtraContentElectrons[0] += ".asy1.barys1.f1core.pos.pos7.poscs1.poscs2.r33over37allcalo"
        ExtraContentMuons[0] += ".MeasEnergyLoss.ParamEnergyLoss.MeasEnergyLossSigma.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus"
        ExtraContentTaus[0] += ".etaIntermediateAxis.etEMAtEMScale.etHadAtEMScale.centFrac.ptDetectorAxis.trFlightPathSig.absipSigLeadTrk"
        ExtraContentJets = [
            "AntiKt4LCTopoJets."
            ,
            "CaloCalTopoClusters."
            ]

    if HIGG4DxName in ['HIGG6D1', 'HIGG6D2']:
      ExtraContentTaus[0] += ".chargedPFOLinks.neutralPFOLinks.pi0PFOLinks"

    if HIGG4DxName == 'HDBS1':
        ExtraContentElectrons[0] += ".asy1.barys1.f1core.pos.pos7.poscs1.poscs2.r33over37allcalo"
        ExtraContentMuons[0] += ".MeasEnergyLoss.ParamEnergyLoss.MeasEnergyLossSigma.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus"
        ExtraContentTaus[0] += ".etaIntermediateAxis.etEMAtEMScale.etHadAtEMScale.centFrac.ptDetectorAxis.trFlightPathSig.absipSigLeadTrk"


    HIGG4DxSlimmingHelper.ExtraVariables = ExtraContentElectrons + ExtraContentMuons + ExtraContentTaus + ExtraContentVtx

    if DFisMC:
        HIGG4DxSlimmingHelper.ExtraVariables += ExtraElectronsTruth + ExtraMuonsTruth + ExtraTausTruth

    if HIGG4DxName == 'HIGG4D6':
        HIGG4DxSlimmingHelper.ExtraVariables += ExtraContentJets

    if HIGG4DxName in ['HIGG4D1', 'HIGG4D2', 'HIGG6D1', 'HIGG6D2']:
        from HIGG4DxAugmentation import JetTagConfig
        HIGG4DxSlimmingHelper.ExtraVariables += JetTagConfig.GetExtraPromptVariablesForDxAOD()
        HIGG4DxSlimmingHelper.ExtraVariables += JetTagConfig.GetExtraPromptTauVariablesForDxAOD()
    
    #extra containers
    if HIGG4DxName in ['HIGG4D2', 'HIGG4D3', 'HIGG4D4', 'HIGG4D5', 'HIGG4D6']:
        HIGG4DxSlimmingHelper.AllVariables += ["LVL1JetRoIs"]

    from DerivationFrameworkJetEtMiss.JetCommon import *
    if HIGG4DxName in OutputJets:
        if HIGG4DxName == 'HDBS1':
            addJetOutputs(HIGG4DxSlimmingHelper, [HIGG4DxName], ['AntiKt4TruthJets', 'AntiKt4TruthWZJets'], ['AntiKt4PV0TrackJets','AntiKt2PV0TrackJets','AntiKt10LCTopoJets','AntiKt4EMTopoJets']) # last two arguments: smart slimming collection list, veto collection list 
        else:
            addJetOutputs(HIGG4DxSlimmingHelper, [HIGG4DxName], ['AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets','AntiKt4TruthJets', 'AntiKt4TruthWZJets'])
        
    if HIGG4DxName in ['HIGG4D2', 'HIGG4D3', 'HIGG4D6']:
        HIGG4DxSlimmingHelper.AppendToDictionary.update( {
              "AntiKtVR30Rmax4Rmin02TrackJets_BTagging201810"               :   "xAOD::JetContainer"        ,
              "AntiKtVR30Rmax4Rmin02TrackJets_BTagging201810Aux"            :   "xAOD::JetAuxContainer"     ,
              "BTagging_AntiKtVR30Rmax4Rmin02Track_201810"          :   "xAOD::BTaggingContainer"   ,
              "BTagging_AntiKtVR30Rmax4Rmin02Track_201810Aux"       :   "xAOD::BTaggingAuxContainer",
              } )
              
        HIGG4DxSlimmingHelper.AllVariables += ["AntiKtVR30Rmax4Rmin02TrackJets_BTagging201810", "BTagging_AntiKtVR30Rmax4Rmin02Track_201810"]
    
    # common for all formats
    HIGG4DxSlimmingHelper.AllVariables +=  [ "MET_Track" ]  # this is needed for the forward JVT
    
    
    #derivation truth
    if DFisMC:

        from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents
        addStandardTruthContents()

        HIGG4DxSlimmingHelper.AppendToDictionary.update( { 'TruthBoson'   :'xAOD::TruthParticleContainer',
                                                           'TruthBosonAux':'xAOD::TruthParticleAuxContainer' } )

        HIGG4DxSlimmingHelper.AllVariables += ["TruthEvents", 
                                               "TruthParticles", 
                                               "TruthVertices", 
                                               "TruthMuons", 
                                               "TruthElectrons", 
                                               "TruthNeutrinos", 
                                               "TruthTaus", 
                                               "TruthBoson",
                                               "TruthPhotons"
                                               ]

        if HIGG4DxName in ['HIGG4D5']:
            HIGG4DxSlimmingHelper.AllVariables += ["HLT_xAOD__CaloClusterContainer_TrigEFCaloCalibFex", "HLT_xAOD__BTaggingContainer_HLTBjetFex"]  

    #trigger content
    if HIGG4DxName == 'HIGG4D1':
        HIGG4DxSlimmingHelper.IncludeMuonTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeEGammaTriggerContent = True
    elif HIGG4DxName == 'HIGG4D2':
        HIGG4DxSlimmingHelper.IncludeMuonTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeEGammaTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeTauTriggerContent = True
    elif HIGG4DxName == 'HIGG4D3':
        HIGG4DxSlimmingHelper.IncludeTauTriggerContent = True
    elif HIGG4DxName == 'HIGG4D4':
        HIGG4DxSlimmingHelper.IncludeJetTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeTauTriggerContent = True
    elif HIGG4DxName == "HIGG4D5":
        HIGG4DxSlimmingHelper.IncludeJetTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeBJetTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeTriggerNavigation = True
        HIGG4DxSlimmingHelper.IncludeTauTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeEtMissTriggerContent = True
    elif HIGG4DxName == "HIGG4D6":
        pass
    elif HIGG4DxName == "HIGG6D1":
        HIGG4DxSlimmingHelper.IncludeEtMissTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeJetTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeTauTriggerContent = True
    elif HIGG4DxName == "HIGG6D2":
        HIGG4DxSlimmingHelper.IncludeEGammaTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeEtMissTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeMuonTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeTauTriggerContent = True
    elif HIGG4DxName == "HDBS1":
        HIGG4DxSlimmingHelper.IncludeMuonTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeEGammaTriggerContent = True
    else:
        assert False, "HIGG4DxSlimming: Unknown derivation stream '{}'".format(HIGG4DxName)

    # the very last line in job options
    HIGG4DxSlimmingHelper.AppendContentToStream(HIGG4DxStream)

    #add tau PV refitted results
    if HIGG4DxName == 'HIGG4D3':
        HIGG4DxStream.AddItem(['xAOD::VertexContainer#TauRefittedPrimaryVertices'])
        HIGG4DxStream.AddItem(['xAOD::VertexAuxContainer#TauRefittedPrimaryVerticesAux.-vxTrackAtVertex'])




