# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#Content included in addition to the Smart Slimming Content

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

ExtraVariablesElectrons=[
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

from DerivationFrameworkCalo.DerivationFrameworkCaloFactories import GainDecorator, getGainDecorations
GainDecoratorTool = GainDecorator()
ExtraVariablesPhotons.extend( getGainDecorations(GainDecoratorTool) )
ExtraVariablesElectrons.extend( getGainDecorations(GainDecoratorTool) )

ExtraVariables=ExtraVariablesElectrons+ExtraVariablesMuons+ExtraVariablesPhotons+ExtraVariablesGSFConversionVertices+ExtraVariablesPrimaryVertices
ExtraVariablesTruth=ExtraVariablesMuonsTruth+ExtraVariablesPhotonsTruth

ExtraContainersTruth=["TruthEvents", 
                      "TruthParticles",
                      "TruthVertices",
                      "egammaTruthParticles"
                      ]

ExtraContainersElectrons=["Electrons",
                          "GSFTrackParticles",
                          "egammaClusters"
                          ]

