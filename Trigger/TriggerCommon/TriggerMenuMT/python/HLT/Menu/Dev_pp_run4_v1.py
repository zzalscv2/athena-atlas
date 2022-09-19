# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_run4_v1.py menu for Phase-II developments
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],

import TriggerMenuMT.HLT.Menu.MC_pp_run4_v1 as mc_menu
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp

# this is not the best option, due to flake violation, this list has to be changed when some groups are removed
from TriggerMenuMT.HLT.Menu.Physics_pp_run4_v1 import (#PhysicsStream,
                                                       SingleMuonGroup,
                                                       #MultiMuonGroup,
                                                       SingleElectronGroup,
                                                       #MultiElectronGroup,
                                                       #SinglePhotonGroup,
                                                       METGroup,
                                                       #SingleJetGroup,
                                                       MultiJetGroup,
                                                       SingleBjetGroup,
                                                       #MultiBjetGroup,
                                                       #SingleTauGroup,
                                                       #MultiTauGroup,
                                                       #BphysicsGroup,
                                                       #BphysElectronGroup,
                                                       #EgammaMETGroup,
                                                       #EgammaMuonGroup,
                                                       #EgammaBjetGroup,
                                                       #MuonJetGroup,
                                                       #MuonMETGroup,
                                                       #EgammaJetGroup,
                                                       #MinBiasGroup,
                                                       #PrimaryLegGroup,
                                                       #PrimaryPhIGroup,
                                                       #SupportGroup,
                                                       #SupportLegGroup,
                                                       #SupportPhIGroup,
                                                       #TagAndProbeLegGroup,
                                                       #UnconvTrkGroup,
                                                       #METPhaseIStreamersGroup,
                                                       #EOFTLALegGroup,
                                                       #LegacyTopoGroup,
                                                       #Topo2Group,
                                                       #Topo3Group,
                                                       #EOFL1MuGroup,
)

DevGroup = ['Development']

def setupMenu():

    chains = mc_menu.setupMenu()

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the Dev menu chains now')

    chains['Muon'] += [
        ChainProp(name='HLT_mu6_ivarmedium_L1MU5VF', groups=DevGroup+SingleMuonGroup),
    ]
    
    chains['Egamma'] += [
        ChainProp(name='HLT_e26_lhtight_L1EM22VHI', groups=DevGroup+SingleElectronGroup,monGroups=['egammaMon:t0_tp']),
    ]

    chains['MET'] += [
        ChainProp(name='HLT_xe30_mht_L1XE30',        l1SeedThresholds=['FSNOSEED'], groups=METGroup+DevGroup),
    ]


    chains['Jet'] += [
        ChainProp(name='HLT_6j25c_L14J15', l1SeedThresholds=['FSNOSEED'],            groups=MultiJetGroup+DevGroup),
    ]


    chains['Bjet'] += [
        ChainProp(name="HLT_j225_0eta290_bdl1r70_pf_ftf_L1J100", l1SeedThresholds=['FSNOSEED'], groups=SingleBjetGroup + DevGroup),
    ]

    chains['Tau'] += [
    ]

    chains['Bphysics'] += [
    ]

    chains['UnconventionalTracking'] += [
    ]

    chains['Combined'] += [
    ]

    chains['Beamspot'] += [
    ]

    chains['MinBias'] += [

    ]

    chains['Calib'] += [
    ]

    chains['Streaming'] += [
    ]

    chains['Monitor'] += [
    ]


    return chains
