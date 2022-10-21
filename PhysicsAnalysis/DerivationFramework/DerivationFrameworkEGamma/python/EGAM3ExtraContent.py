# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#Content included in addition to the Smart Slimming Content

ExtraVariablesElectrons=[
    "Electrons.Loose",
    "Electrons.Medium",
    "Electrons.Tight"
    ]

# only if DoCellReweighting is ON
ExtraVariablesReweightedElectrons = ["NewSwElectrons.trackParticleLinks.pt.eta.phi.m.caloClusterLinks.author.OQ.ethad1.ethad.f1.f3.f3core.e233.e237.e277.weta1.weta2.e2tsts1.fracs1.wtots1.emins1.emaxs1.etcone20.ptcone30.deltaEta1.deltaPhi1.deltaPhi2.deltaPhiRescaled2.deltaPhiFromLastMeasurement.Loose.Medium.Tight.DFCommonElectronsLHVeryLoose.DFCommonElectronsLHLoose.DFCommonElectronsLHLooseBL.DFCommonElectronsLHMedium.DFCommonElectronsLHTight.ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.topoetcone20.topoetcone30.topoetcone40.charge.Reta.Rphi.Eratio.Rhad.Rhad1.DeltaE.topoetcone20ptCorrection.topoetcone30ptCorrection.topoetcone40ptCorrection.etcone20ptCorrection.etcone30ptCorrection.etcone40ptCorrection.ambiguityLink.truthParticleLink.truthOrigin.truthType.truthPdgId.firstEgMotherTruthType.firstEgMotherTruthOrigin.firstEgMotherTruthParticleLink.firstEgMotherPdgId.lastEgMotherTruthType.lastEgMotherTruthOrigin.lastEgMotherTruthParticleLink.lastEgMotherPdgId.ambiguityType.DFCommonAddAmbiguity"]

ExtraVariablesElectronsTruth=[
    "Electrons.truthOrigin",
    "Electrons.truthType",
    "Electrons.truthParticleLink"]

ExtraVariablesMuons=[
    "Muons.ptcone20",
    "Muons.ptcone30",
    "Muons.ptcone40",
    "Muons.etcone20",
    "Muons.etcone30",
    "Muons.etcone40"]

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
                        "egammaClusters",
                        "ForwardElectrons",
                        "ForwardElectronClusters"]

