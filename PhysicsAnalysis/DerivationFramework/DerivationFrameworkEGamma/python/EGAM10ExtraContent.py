# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#Content included in addition to the Smart Slimming Content
ExtraVariablesElectrons=["Electrons.topoetcone20.topoetcone30.topoetcone40.ptcone20.ptcone30.ptcone40.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z"] + ["egammaClusters.PHI2CALOFRAME.ETA2CALOFRAME.phi_sampl"]

ExtraVariablesPhotons=["Photons.core57cellsEnergyCorrection.topoetcone20.topoetcone30.topoetcone40.ptcone20.ptcone30.ptcone40.f3.f3core.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z"] + \
    ["Photons.ptcone20_Nonprompt_All_MaxWeightTTVA_pt500.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500"]

ExtraVariablesVtx=["PrimaryVertices.covariance.trackWeights.sumPt2.EGAM10_sumPt.EGAM10_sumPt2.EGAM10_pt.EGAM10_eta.EGAM10_phi"]

ExtraVariablesTrk=["InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights"]

ExtraVariablesJets=[]

ExtraVariablesEventShape=[
    "TopoClusterIsoCentralEventShape.Density",
    "TopoClusterIsoForwardEventShape.Density",
]

ExtraVariables=ExtraVariablesElectrons+ExtraVariablesPhotons+ExtraVariablesVtx+ExtraVariablesTrk+ExtraVariablesJets+ExtraVariablesEventShape

# These are only added if running on MC
ExtraVariablesElectronsTruth=[".".join(["Electrons", 
    "truthOrigin", 
    "truthType", 
    "truthParticleLink", 
    "truthPdgId", 
    "lastEgMotherTruthType",
    "lastEgMotherTruthOrigin",
    "lastEgMotherTruthParticleLink",
    "lastEgMotherPdgId",
    "firstEgMotherTruthType", 
    "firstEgMotherTruthOrigin", 
    "firstEgMotherTruthParticleLink", 
    "firstEgMotherPdgId"
    ]) ]

ExtraVariablesPhotonsTruth=["Photons.truthOrigin.truthParticleLink.truthType"]

ExtraVariablesTruthEventShape=[
    "TruthIsoCentralEventShape.DensitySigma.Density.DensityArea",
    "TruthIsoForwardEventShape.DensitySigma.Density.DensityArea"
    ]

ExtraVariablesTruth=ExtraVariablesElectronsTruth+ExtraVariablesPhotonsTruth+ExtraVariablesTruthEventShape


# Extra containers
ExtraContainers=["CaloCalTopoClusters"]

ExtraContainersTruth=["TruthEvents",
                      "TruthParticles",
                      "TruthVertices",
                      "TruthMuons",
                      "TruthElectrons",
                      "TruthPhotons",
                      "TruthNeutrinos",
                      "TruthTaus",
                      "AntiKt4TruthJets",
                      "AntiKt4TruthDressedWZJets"]

ExtraContainersTruthPhotons=["egammaTruthParticles"]


ExtraDictionary={"TruthIsoCentralEventShape":"xAOD::EventShape",
                 "TruthIsoCentralEventShapeAux":"xAOD::EventShapeAuxInfo",
                 "TruthIsoForwardEventShape":"xAOD::EventShape",
                 "TruthIsoForwardEventShapeAux":"xAOD::EventShapeAuxInfo"}


