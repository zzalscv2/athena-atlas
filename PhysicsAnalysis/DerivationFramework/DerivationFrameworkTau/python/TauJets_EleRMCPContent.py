# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from DerivationFrameworkTau.TauJetsCPContent import TauJetsCPContent

TauJets_EleRMCPContent = []
for item in TauJetsCPContent:
    ermitem = item  .replace("TauJets", "TauJets_EleRM")\
                    .replace("TauTracks", "TauTracks_EleRM")\
                    .replace("TauSecondaryVertices", "TauSecondaryVertices_EleRM")\
                    .replace("TauNeutralParticleFlowObjects", "TauNeutralParticleFlowObjects_EleRM")
    TauJets_EleRMCPContent.append(ermitem)