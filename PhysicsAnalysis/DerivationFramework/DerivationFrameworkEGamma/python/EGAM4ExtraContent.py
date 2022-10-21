# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#Content included in addition to the Smart Slimming Content

ExtraVariablesElectrons=[
    "Electrons.Loose",
    "Electrons.Medium",
    "Electrons.Tight",
    ]

ExtraVariablesElectronsTruth=[
    "Electrons.truthOrigin",
    "Electrons.truthType",
    "Electrons.truthParticleLink"
    ]

ExtraVariablesMuons=[
    "Muons.ptcone20",
    "Muons.ptcone30",
    "Muons.ptcone40",
    "Muons.etcone20",
    "Muons.etcone30",
    "Muons.etcone40"
    ]

ExtraVariablesMuonsTruth=[
    ]

ExtraVariablesPhotons=[
]

ExtraVariablesPrimaryVertices=["PrimaryVertices.x.y.sumPt2"]

ExtraVariablesPhotonsTruth=[
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
ExtraVariablesTruth=ExtraVariablesElectronsTruth+ExtraVariablesMuonsTruth+ExtraVariablesPhotonsTruth

ExtraContainersTruth=["TruthEvents", 
                      "TruthParticles",
                      "TruthVertices",
                      "egammaTruthParticles",
                      "MuonTruthParticles"
                      ]

ExtraContainersPhotons=["Photons",
                        "GSFTrackParticles",
                        "egammaClusters"]


