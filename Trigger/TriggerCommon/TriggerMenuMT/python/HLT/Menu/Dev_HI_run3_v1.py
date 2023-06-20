# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_HI_run3_v1.py menu for Run 3 development
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],


from ..Config.Utility.ChainDefInMenu import ChainProp

from .Physics_pp_run3_v1 import (
    SingleMuonGroup,
    MinBiasGroup,
    MultiMuonGroup,
    SinglePhotonGroup,
    SingleElectronGroup,
    MultiElectronGroup,
    PrimaryLegGroup,
    PrimaryPhIGroup,
    PrimaryL1MuGroup,
    SupportGroup,
    SupportLegGroup,
    SupportPhIGroup,
    SingleJetGroup,
    SingleBjetGroup,
    TagAndProbeGroup
)

from .SignatureDicts import ChainStore
from . import PhysicsP1_HI_run3_v1 as HIp1_menu
from .PhysicsP1_HI_run3_v1 import HardProbesStream,MinBiasStream,UPCStream, MinBiasOverlayStream



def getDevHISignatures():

    chains = ChainStore()
    chains['Muon'] += [
        #-- 1 mu
        ChainProp(name='HLT_mu6_L1MU3V',   stream=[HardProbesStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu6_L1MU5VF',  stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu8_L1MU5VF',  stream=[HardProbesStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu10_L1MU8F',  stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu10_L1MU5VF', stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
            
        #-- 2 mu
        ChainProp(name='HLT_2mu4_L12MU3V', stream=[HardProbesStream, 'express'], groups=MultiMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V', stream=[HardProbesStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),

        #-- tag-and-probe 
        ChainProp(name='HLT_mu8_mu4_probe_L1MU5VF', l1SeedThresholds=['MU5VF','PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu6_mu4_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu4_mu4_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu8_mu6_probe_L1MU5VF', l1SeedThresholds=['MU5VF','PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu6_mu6_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),
        ChainProp(name='HLT_mu4_mu6_probe_L1MU3V',  l1SeedThresholds=['MU3V', 'PROBEMU3V'], groups=SingleMuonGroup+TagAndProbeGroup),

        #-- mu_idperf for ID monitoring
        ChainProp(name='HLT_mu4_idperf_L1MU3V',  stream=[HardProbesStream,'express'], groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu6_idperf_L1MU5VF', stream=[HardProbesStream,'express'], groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu8_idperf_L1MU5VF', stream=[HardProbesStream,'express'], groups=SupportGroup+SingleMuonGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu4_mu4_idperf_1invmAB5_L12MU3V',      l1SeedThresholds=['MU3V', 'MU3V'],  stream=[HardProbesStream,'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:t0']),
        ChainProp(name='HLT_mu10_mu10_idperf_50invmAB130_L12MU5VF',l1SeedThresholds=['MU5VF','MU5VF'], stream=[HardProbesStream,'express'], groups=MultiMuonGroup+SupportGroup, monGroups=['idMon:shifter','idMon:t0']),

        #-- UPC
        ChainProp(name='HLT_mu4_L1MU3V_VTE50',        stream=[UPCStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu6_L1MU3V_VTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu8_L1MU5VF_VTE50',       stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_2mu4_L12MU3V_VTE50',      stream=[UPCStream], groups=MultiMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V_VTE50',stream=[UPCStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup),

        #-- UPC - phase-1
        ChainProp(name='HLT_mu4_L1MU3V_VjTE50',        stream=[UPCStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu4_L1MU3V_VjTE50_GAP_AANDC',        stream=[UPCStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu6_L1MU3V_VjTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu8_L1MU5VF_VjTE50',       stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2mu4_L12MU3V_VjTE50',      stream=[UPCStream], groups=MultiMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V_VjTE50',stream=[UPCStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup+PrimaryPhIGroup),
     ]

    chains['Egamma'] += [
        # ElectronChains----------
        #--------- phase-1 supporting electron chains
        # replace L1eEM9 with L1eEM15 and L1eEM15 with eEM18, ATR-26366
        ChainProp(name='HLT_e13_etcut_ion_L1eEM12L', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e15_etcut_ion_L1eEM15',    stream=[HardProbesStream], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e20_etcut_ion_L1eEM18',   stream=[HardProbesStream], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e30_etcut_ion_L1eEM26', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e50_etcut_ion_L1eEM26', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e20_idperf_loose_nogsf_ion_L1eEM18', stream=[HardProbesStream, 'express'], groups=SingleElectronGroup+SupportPhIGroup, monGroups=['idMon:t0']),

        #--------- phase-1 physics electron chains
        ChainProp(name='HLT_e15_lhloose_nogsf_ion_L1eEM15',  stream=[HardProbesStream, 'express'], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e15_loose_nogsf_ion_L1eEM15',    stream=[HardProbesStream, 'express'], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e15_lhmedium_nogsf_ion_L1eEM15', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e15_medium_nogsf_ion_L1eEM15',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_e20_lhloose_nogsf_ion_L1eEM18', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_lhmedium_nogsf_ion_L1eEM18',stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_nogsf_ion_L1eEM18',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_nogsf_ion_L1eEM18L',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_medium_nogsf_ion_L1eEM18',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2e20_loose_nogsf_ion_L12eEM18L',stream=[HardProbesStream], groups=MultiElectronGroup+PrimaryPhIGroup),
        
        
        # PhotonChains----------
        #----------- phase-1 support photon chains
        ChainProp(name='HLT_g13_etcut_ion_L1eEM12L', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g18_etcut_ion_L1eEM12L', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g28_etcut_ion_L1eEM18', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g15_etcut_ion_L1eEM15', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),        
        ChainProp(name='HLT_g20_etcut_ion_L1eEM15', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g18_etcut_L1eEM12L',     stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g20_loose_L1eEM15',     stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_2g15_loose_L12eEM12L',   stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        
        #----------- phase-1 primary photon chains
        ChainProp(name='HLT_g15_loose_ion_L1eEM12L',  stream=[HardProbesStream, 'express'], groups=SinglePhotonGroup+PrimaryPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),
        ChainProp(name='HLT_g15_loose_ion_L1eEM15', stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g20_loose_ion_L1eEM15', stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g30_loose_ion_L1eEM18',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g50_loose_ion_L1eEM26',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2g15_loose_ion_L12eEM12L',stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
    ]

    chains['Jet'] += [
        ChainProp(name='HLT_j40_ion_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j50_ion_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j50_ion_L1TE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j60_ion_L1TE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j30f_ion_L1TE20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j50f_ion_L1TE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j50f_ion_L1J15p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+SupportLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j60f_ion_L1J15p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+SupportLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j70f_ion_L1J30p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),

        ChainProp(name='HLT_j60_ion_L1J15', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j75_ion_L1J20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j75_ion_L1J30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j85_ion_L1J15', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j85_ion_L1J30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j100_ion_L1J30',l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j120_ion_L1J30',l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j150_ion_L1J50',l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),

        #--- phase-1 HI jets
        ChainProp(name='HLT_j60_ion_L1jJ20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j75_ion_L1jJ20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j75_ion_L1jJ30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j85_ion_L1jJ20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j85_ion_L1jJ30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j100_ion_L1jJ30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j120_ion_L1jJ30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j150_ion_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        
        ChainProp(name='HLT_j50f_ion_L1jJ15p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j60f_ion_L1jJ15p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream, 'express'], groups=SingleJetGroup+SupportPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j70f_ion_L1jJ20p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),

         # TE-seeded jets, phase-1
        ChainProp(name='HLT_j50_ion_L1jTE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j60_ion_L1jTE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j30f_ion_L1jTE20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j50f_ion_L1jTE50', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportPhIGroup),

        #--- UPC jets
        ChainProp(name='HLT_j10a_L1VTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j15a_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j20a_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j30a_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j40a_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j30a_L1TE20_VTE200',l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j40a_L1TE20_VTE200',l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),


        ChainProp(name='HLT_j10a_pf_ftf_L1VTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j10a_pf_ftf_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j15a_pf_ftf_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j20a_pf_ftf_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j30a_pf_ftf_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j30a_pf_ftf_L1TE20_VTE200',l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryLegGroup, monGroups=['jetMon:t0']),
        ChainProp(name='HLT_j40a_pf_ftf_L1TE20_VTE200',l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),

        #--- UPC jets - phase-1
        ChainProp(name='HLT_j10a_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j15a_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j20a_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream, 'express'], groups=SingleJetGroup+PrimaryPhIGroup, monGroups=['jetMon:t0','jetMon:online']),
        ChainProp(name='HLT_j30a_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j40a_L1jTE5_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryPhIGroup),


    ]


    chains['Combined'] += [
        #----------- mu+j PrimaryLeg
        ChainProp(name='HLT_mu4_j50_ion_dRAB04_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream, 'express'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu4_j50_ion_dRAB04_L1MU3V_J12', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu4_j60_ion_dRAB04_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j40_ion_dRAB04_L1MU5VF',     l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[HardProbesStream, 'express'], groups=PrimaryLegGroup+SingleBjetGroup, monGroups=['bJetMon:t0','muonMon:online','bJetMon:online']),
        ChainProp(name='HLT_mu6_j50_ion_dRAB04_L1MU5VF',     l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryLegGroup+SingleBjetGroup),
        #----------- mu+j SupportLeg
        ChainProp(name='HLT_mu4_j20_L1MU3V_J12',            l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j40_ion_dRAB04_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j30_ion_dRAB04_L1MU5VF',     l1SeedThresholds=['MU5VF','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j40_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j50_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j60_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j30_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j40_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu6_j50_ion_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),

        #----------- mu+j with new calo
        ChainProp(name='HLT_mu4_j60_ion_dRAB04_L1MU3V_jJ20', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryPhIGroup+SingleBjetGroup),

        #----------- mu + UPC Fgap
        ChainProp(name='HLT_mu4_hi_FgapAC3_L1MU3V_VTE50', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[UPCStream], groups=SingleMuonGroup+PrimaryLegGroup),

        #----------- UPC HMT
        ChainProp(name='HLT_mb_sptrk_hi_FgapC5_L1VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sptrk_hi_FgapA5_L1VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapA5_L1MBTS_1_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_hi_FgapC5_L1MBTS_1_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapA5_L1TE3_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_hi_FgapC5_L1TE3_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapA5_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_hi_FgapC5_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportGroup),

        #----------- UPC HMT phase-1
        ChainProp(name='HLT_mb_sptrk_hi_FgapC5_L1VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sptrk_hi_FgapA5_L1VjTE200', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
   
        #----------- UPC diphotons/dielectrons
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC3_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC3_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk4_pt1_hi_FgapAC3_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sptrk_hi_FgapAC3_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),

        #----------- temporarily added to test the HLT despite TE simulation issue ATR-26798
        ChainProp(name='HLT_mb_sp_vpix15_hi_FgapAC3_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sp_vpix30_hi_FgapAC3_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_excl_1trk4_pt1_hi_FgapAC3_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryLegGroup),
        ChainProp(name='HLT_mb_sptrk_hi_FgapAC3_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),

        #----------- UPC diphotons/dielectrons - phase-1
        ChainProp(name='HLT_mb_excl_1trk4_pt1_hi_FgapAC5_L12eEM1_VjTE200', l1SeedThresholds=['FSNOSEED']*2,stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        

        #----------- UPC ditaus 
        ChainProp(name='HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VTE50', l1SeedThresholds=['FSNOSEED']*2, stream=[UPCStream], groups=MinBiasGroup+PrimaryLegGroup),

    ]


    chains['MinBias'] += [
        ChainProp(name='HLT_mb_sp_pix100_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp_pix200_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp_pix500_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp_pix1000_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp_vetospmbts2in_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_excl_1trk2_pt1_L1TRT_VTE20',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTReprocessing']),
        #----------- mbts
        ChainProp(name="HLT_mb_mbts_L1MBTS_2_2", l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:NoHLTReprocessing']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_3_3", l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:NoHLTReprocessing']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_4_4", l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:NoHLTReprocessing']),

        #----------- sptrk
        ChainProp(name='HLT_mb_sptrk_L1VTE50',          l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_1_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),

        #----------- UPC HMT
        ChainProp(name='HLT_mb_sp_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp_L1MBTS_1_VTE50',             l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_L1TE3_VTE50',                l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp15_trk15_hmt_L1MBTS_1_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp300_trk20_hmt_L1TE3_VTE50',   l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),

        ChainProp(name='HLT_mb_sp50_trk15_hmt_L1MBTS_1_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp50_trk15_hmt_L1MBTS_1_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_L1jTE3_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp400_trk25_hmt_L1jTE3_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_L1jTE5_VjTE200_GAP_A', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_sp700_trk35_hmt_L1jTE5_VjTE200_GAP_C', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),

        #----------- UPC exclusive dileptons
        ChainProp(name='HLT_mb_excl_1trk4_pt1_L12TAU1_VTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TAU1_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_L1TAU1_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TRT_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        
        #----------- UPC exclusive dileptons - phase-1
        ChainProp(name='HLT_mb_sp_vpix15_L12eEM1_VjTE200_GAP_AANDC', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sp_vpix30_L12eEM1_VjTE200_GAP_AANDC', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk4_pt1_L12eEM1_VjTE200_GAP_AANDC', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_mb_sptrk_L12eEM1_VjTE200_GAP_AANDC', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),

        ChainProp(name='HLT_mb_excl_1trk4_pt1_L12eEM1_VjTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk4_pt1_L12eEM2_VjTE200', l1SeedThresholds=['FSNOSEED'],stream=[UPCStream],groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1eEM1_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt2_L1eEM1_VjTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TRT_VjTE50', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt1_L1TRT_VjTE50_GAP_AANDC', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportPhIGroup),
        
        #----Physics streamer for 2022 Nov HI test run, ATR-26405
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_VTE5', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
    ]


    chains['HeavyIon'] += [
        #----------- UPC Fgap
        ChainProp(name='HLT_hi_FgapAC3_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_hi_FgapAC5_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_hi_FgapAC10_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_hi_FgapA10_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_hi_FgapC10_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),

    ]

    chains['Streaming'] += [
        ChainProp(name='HLT_noalg_L12TAU1_VTE50',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L12TAU1_VTE200',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU1_TE3_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU1_TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU1_VTE200',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU1_VTE200_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XOR_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1J15',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J30',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportLegGroup),
        
        ChainProp(name='HLT_noalg_L1jJ20',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ30',   l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportPhIGroup),
        
        ChainProp(name='HLT_noalg_L1eEM5',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM15',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1eEM18',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L12MU3V',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportGroup),

        #----TE MinBias----
        ChainProp(name='HLT_noalg_L1TE5',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE20',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE100', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE200', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        # really needed if we don't have UCC trigger in Run3?
        ChainProp(name='HLT_noalg_L1TE10000', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE12000', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        #----TE MinBias using restricted eta threshold----
        ChainProp(name='HLT_noalg_L1TE3p0ETA49',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE7p0ETA49',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE1500p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE3000p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE3500p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE6500p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE8000p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),

        #----should be pc/cc stream, to be fixed
        ChainProp(name='HLT_noalg_L1TE50_VTE600p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE600p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),

        #----TE MinBias OVERLAY----
        ChainProp(name='HLT_noalg_L1MBTS_1_VTE50_OVERLAY', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE50_OVERLAY',         l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE600p0ETA49_OVERLAY', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE1500p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE3000p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE3500p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE6500p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE8000p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasOverlayStream], groups=MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1MBTS_1_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE50_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TRT_VTE200',   l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TRT_VTE20',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1gTE200',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jTE200',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportPhIGroup),

        #----Physics streamer for 2022 Nov HI test run, ATR-26405
        ChainProp(name='HLT_noalg_L1VTE5',           l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1MBTS_1_VTE5',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_mb_L1MBTS_1_VTE5', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE5_VTE20',      l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TE20_VTE200',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportLegGroup),
    ]

    #---- beamspot trigger with VTE to avoid busy tracking in central events
    chains['Beamspot'] = [
        ChainProp(name='HLT_beamspot_trkFS_trkfast_BeamSpotPEB_L1J12_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['PS:Online', 'RATE:BeamSpot',  'BW:BeamSpot']+SupportLegGroup),
        ChainProp(name='HLT_beamspot_trkFS_trkfast_BeamSpotPEB_L1J12_VTE100',  l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['PS:Online', 'RATE:BeamSpot',  'BW:BeamSpot']+SupportLegGroup),
    ]


    #---- ID calib trigger with VTE to avoid busy tracking in central events
    chains['Calib'] = [
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1J100_VTE200', stream=['IDCalib'], groups=SupportLegGroup+['PS:Online','RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1XE50_VTE200', stream=['IDCalib'], groups=SupportLegGroup+['PS:Online','RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1J30_VTE200',  stream=['IDCalib'], groups=SupportLegGroup+['PS:Online','RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1XE35_VTE200', stream=['IDCalib'], groups=SupportLegGroup+['PS:Online','RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
    ]

    #---- heavy ion EB chains
    chains['EnhancedBias'] += [
        ChainProp(name='HLT_noalg_eb_L1MU3V',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TE50',         l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TE12000',      l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_eb_L1MBTS_1_1',     l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TE5_VTE200',   l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TE50_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L12TAU1_VTE50',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1MU3V_VTE50',   l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1RD1_FILLED',   l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_eb_L1TAU1_TE3_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["PS:Online", "RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        # need to add ZDC based eb chains for 2023 heavy ion runs
    ]

    return chains

def setupMenu(menu_name):

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )

    chains = HIp1_menu.getPhysicsHISignatures()

    log.info('[setupMenu] going to add the Dev menu chains now')

    for sig,chainsInSig in getDevHISignatures().items():
        chains[sig] += chainsInSig

    return chains
