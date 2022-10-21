# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


#Content included in addition to the Smart Slimming Content

ExtraVariablesElectrons=[]

ExtraVariablesMuons=[
    "Muons.ptcone20",
    "Muons.ptcone30",
    "Muons.ptcone40",
    "Muons.etcone20",
    "Muons.etcone30",
    "Muons.etcone40",
# to be added when prompt lepton decoration is migrated to R22
#    "Muons.PromptLepton_TrackJetNTrack",
#    "Muons.PromptLepton_sv1_ntkv",
#    "Muons.PromptLepton_jf_ntrkv",
#    "Muons.PromptLepton_ip2",
#    "Muons.PromptLepton_ip2_cu",
#    "Muons.PromptLepton_ip3",
#    "Muons.PromptLepton_ip3_cu",
#    "Muons.PromptLepton_EtTopoCone20Rel",
#    "Muons.PromptLepton_TagWeight"
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

ExtraVariablesPrimaryVertices=["PrimaryVertices.x.y.sumPt2"]

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
# not yet in R22
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
                          "egammaClusters"]

