# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

def PFTauRemaps():
    from SGComps.AddressRemappingConfig import InputRenameCfg
    
    #If we rerun the tau reco, then remap things
    list_remaps=[    
        InputRenameCfg ('xAOD::TauJetContainer','TauJets','TauJets_renamed'),
        InputRenameCfg ('xAOD::TauJetAuxContainer','TauJetsAux.','TauJets_renamedAux.'),
        InputRenameCfg ('xAOD::TauJetContainer','DiTauJets','DiTauJets_renamed'),
        InputRenameCfg ('xAOD::TauJetAuxContainer','DiTauJetsAux.','DiTauJets_renamedAux.')
    ]

    return list_remaps