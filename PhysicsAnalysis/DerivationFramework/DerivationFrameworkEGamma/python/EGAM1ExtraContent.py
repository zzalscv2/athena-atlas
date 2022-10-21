# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


#Content included in addition to the Smart Slimming Content

ExtraVariablesElectrons=[]

ExtraVariablesMuons=[
    "Muons.ptcone20",
    "Muons.ptcone30",
    "Muons.ptcone40",
    "Muons.etcone20",
    "Muons.etcone30",
    "Muons.etcone40"
]   

ExtraVariablesMuonsTruth=[
    "MuonTruthParticles.e",
    "MuonTruthParticles.px",
    "MuonTruthParticles.py",
    "MuonTruthParticles.pz",
    "MuonTruthParticles.status",
    "MuonTruthParticles.pdgId",
    "MuonTruthParticles.truthOrigin",
    "MuonTruthParticles.truthType"
    ]

ExtraVariablesPhotons=[
    ]

ExtraVariablesPrimaryVertices=["PrimaryVertices.sumPt2"]

ExtraVariablesPhotonsTruth=[
    "Photons.truthOrigin",
    "Photons.truthType",
    "Photons.truthParticleLink"
    ]

ExtraVariablesGSFConversionVertices=[
        "GSFConversionVertices.x",
        "GSFConversionVertices.y",
        "GSFConversionVertices.z",
        "GSFConversionVertices.px",
        "GSFConversionVertices.py",
        "GSFConversionVertices.pz",
        "GSFConversionVertices.pt1",
        "GSFConversionVertices.pt2",
        "GSFConversionVertices.etaAtCalo",
        "GSFConversionVertices.phiAtCalo",
        "GSFConversionVertices.trackParticleLinks"
        ]

ExtraVariablesTrackJets=["AntiKt4PV0TrackJets.pt.eta.phi.e.m.btaggingLink.constituentLinks"]

ExtraVariablesBtagging=[]


from DerivationFrameworkCalo.DerivationFrameworkCaloFactories import GainDecorator, getGainDecorations
GainDecoratorTool = GainDecorator()
ExtraVariablesPhotons.extend( getGainDecorations(GainDecoratorTool) )
ExtraVariablesElectrons.extend( getGainDecorations(GainDecoratorTool) )

ExtraVariables=ExtraVariablesElectrons+ExtraVariablesMuons+ExtraVariablesPhotons+ExtraVariablesGSFConversionVertices+ExtraVariablesPrimaryVertices+ExtraVariablesTrackJets+ExtraVariablesBtagging

ExtraVariablesTruth=ExtraVariablesMuonsTruth+ExtraVariablesPhotonsTruth

ExtraContainersTruth=["TruthEvents", 
                      "TruthParticles",
                      "TruthVertices",
                      "egammaTruthParticles"
                      ]

ExtraContainersElectrons=["Electrons",
                          "GSFTrackParticles",
                          "egammaClusters",
                          "CaloCalTopoClusters"]

ExtraContainersReweightedElectrons=[
                          "NewSwElectrons",    # only if DoCellReweighting is ON
                          "MaxVarSwElectrons", # if variations are ON
                          "MinVarSwElectrons"  # if variations are ON
                          ]

# for trigger studies
ExtraContainersTrigger=[
        "HLT_xAOD__ElectronContainer_egamma_Electrons",
        "HLT_xAOD__ElectronContainer_egamma_ElectronsAux.",
        "HLT_xAOD__PhotonContainer_egamma_Photons",
        "HLT_xAOD__PhotonContainer_egamma_PhotonsAux.",
        "HLT_xAOD__TrigRingerRingsContainer_TrigT2CaloEgamma",
        "HLT_xAOD__TrigRingerRingsContainer_TrigT2CaloEgammaAux.",
        "HLT_xAOD__TrigEMClusterContainer_TrigT2CaloEgamma",
        "HLT_xAOD__TrigEMClusterContainer_TrigT2CaloEgammaAux.",
        "HLT_xAOD__CaloClusterContainer_TrigEFCaloCalibFex",
        "HLT_xAOD__CaloClusterContainer_TrigEFCaloCalibFexAux.",
        "HLT_xAOD__TrigRNNOutputContainer_TrigRingerNeuralFex",
        "HLT_xAOD__TrigRNNOutputContainer_TrigRingerNeuralFexAux.",
        "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Electron_IDTrig",
        "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Electron_IDTrigAux.",
        "HLT_xAOD__TrigPassBitsContainer_passbits",
        "HLT_xAOD__TrigPassBitsContainer_passbitsAux.",
        "LVL1EmTauRoIs",
        "LVL1EmTauRoIsAux.",
        "HLT_TrigRoiDescriptorCollection_initialRoI",
        "HLT_TrigRoiDescriptorCollection_initialRoIAux.",
        "HLT_xAOD__RoiDescriptorStore_initialRoI",
        "HLT_xAOD__RoiDescriptorStore_initialRoIAux.",
        "HLT_xAOD__TrigElectronContainer_L2ElectronFex",
        "HLT_xAOD__TrigElectronContainer_L2ElectronFexAux."
        ]

ExtraContainersTriggerDataOnly=[]

ExtraVariablesEventShape=[
    "TopoClusterIsoCentralEventShape.Density",
    "TopoClusterIsoForwardEventShape.Density",
]
