# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_HI_run3_v1.py menu for the long shutdown development
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],


from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp

from TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 import (
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
    SingleBjetGroup
)


import TriggerMenuMT.HLT.Menu.PhysicsP1_HI_run3_v1 as HIp1_menu

from TriggerMenuMT.HLT.Menu.PhysicsP1_HI_run3_v1 import HardProbesStream,MinBiasStream,UPCStream



def setupMenu():

    chains = HIp1_menu.setupMenu()

    from AthenaCommon.Logging                 import logging
    log = logging.getLogger( __name__ )
    log.info('setupMenu ...')

    chains['Muon'] += [
        #-- 1 mu
        ChainProp(name='HLT_mu8_L1MU5VF',  stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu10_L1MU5VF', stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu10_L1MU8F',  stream=[HardProbesStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
            
        #-- 2 mu
        ChainProp(name='HLT_2mu4_L12MU3V', stream=[HardProbesStream], groups=MultiMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V', stream=[HardProbesStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup),

        #-- UPC
        ChainProp(name='HLT_mu4_L1MU3V_VTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu6_L1MU3V_VTE50',        stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu8_L1MU5VF_VTE50',       stream=[UPCStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_2mu4_L12MU3V_VTE50',      stream=[UPCStream], groups=MultiMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V_VTE50',stream=[UPCStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup),
     ]

    chains['Egamma'] += [
        # ElectronChains----------
        #--------- legacy supporting electron chains
        ChainProp(name='HLT_e13_etcut_nogsf_ion_L1EM10', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e15_etcut_nogsf_ion_L1EM12', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e18_etcut_nogsf_ion_L1EM14', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e20_etcut_nogsf_ion_L1EM16', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e30_etcut_nogsf_ion_L1EM22', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e50_etcut_nogsf_ion_L1EM22', stream=[HardProbesStream] ,groups=SingleElectronGroup+SupportLegGroup),
 
        #--------- legacy physics electon chains
        ChainProp(name='HLT_e15_lhloose_nogsf_ion_L1EM12',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e15_lhmedium_nogsf_ion_L1EM12', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e15_loose_nogsf_ion_L1EM12',    stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e15_medium_nogsf_ion_L1EM12',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),

        ChainProp(name='HLT_e18_lhloose_nogsf_ion_L1EM14',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e18_lhmedium_nogsf_ion_L1EM14', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e18_loose_nogsf_ion_L1EM14',    stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e18_medium_nogsf_ion_L1EM14',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),

        ChainProp(name='HLT_e20_lhloose_nogsf_ion_L1EM16',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e20_lhmedium_nogsf_ion_L1EM16', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e20_loose_nogsf_ion_L1EM16',    stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e20_medium_nogsf_ion_L1EM16',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryLegGroup),

        ChainProp(name='HLT_2e20_loose_nogsf_ion_L12EM16',  stream=[HardProbesStream], groups=MultiElectronGroup+PrimaryLegGroup),
        
        #--------- phase-1 supproting electron chains
        ChainProp(name='HLT_e15_etcut_nogsf_ion_L1eEM9',    stream=[HardProbesStream], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e20_etcut_nogsf_ion_L1eEM15',   stream=[HardProbesStream], groups=SingleElectronGroup+SupportPhIGroup),

        #--------- phase-1 physics electron chains
        ChainProp(name='HLT_e15_lhloose_nogsf_ion_L1eEM9',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e15_lhmedium_nogsf_ion_L1eEM9', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e15_loose_nogsf_ion_L1eEM9',    stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e15_medium_nogsf_ion_L1eEM9',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_e20_lhloose_nogsf_ion_L1eEM15', stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_lhmedium_nogsf_ion_L1eEM15',stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_nogsf_ion_L1eEM15',   stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_nogsf_ion_L1eEM18L',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_medium_nogsf_ion_L1eEM15',  stream=[HardProbesStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_2e20_loose_nogsf_ion_L12eEM18L',stream=[HardProbesStream], groups=MultiElectronGroup+PrimaryPhIGroup),
        
        
        # PhotonChains----------
        #----------- legacy support photon chains
        ChainProp(name='HLT_g13_etcut_ion_L1EM10', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g18_etcut_ion_L1EM10', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g28_etcut_ion_L1EM10', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        #g15_etcut and g20_etcut have high thresholds, not sure they are needed, to be followed up
        ChainProp(name='HLT_g15_etcut_ion_L1EM12', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g20_etcut_ion_L1EM12', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g18_etcut_L1EM10',     stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g20_loose_L1EM12',     stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_2g15_loose_L12EM10',   stream=[HardProbesStream], groups=SinglePhotonGroup+SupportLegGroup),

        #----------- legacy primary photon chains
        ChainProp(name='HLT_g15_loose_ion_L1EM10',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryLegGroup),
        ChainProp(name='HLT_g15_loose_ion_L1EM12',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryLegGroup),
        ChainProp(name='HLT_g20_loose_ion_L1EM12',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryLegGroup),
        ChainProp(name='HLT_g30_loose_ion_L1EM16',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryLegGroup),
        ChainProp(name='HLT_g50_loose_ion_L1EM22',  stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryLegGroup),
        ChainProp(name='HLT_2g15_loose_ion_L12EM10',stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryLegGroup),


        #----------- phase-1 photon chains
        ChainProp(name='HLT_g15_etcut_ion_L1eEM9', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g20_etcut_ion_L1eEM9', stream=[HardProbesStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g15_loose_ion_L1eEM9', stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g20_loose_ion_L1eEM9', stream=[HardProbesStream], groups=SinglePhotonGroup+PrimaryPhIGroup),

        #---UPC legacy egamma chains
        ChainProp(name='HLT_e12_lhloose_nogsf_L1EM7_VTE200',     l1SeedThresholds=['EM7'],  stream=[UPCStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e12_loose_nogsf_L1EM7_VTE200',       l1SeedThresholds=['EM7'],  stream=[UPCStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e12_medium_nogsf_L1EM7_VTE200',      l1SeedThresholds=['EM7'],  stream=[UPCStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e12_loose_nogsf_ion_L1EM7_VTE200',   l1SeedThresholds=['EM7'],  stream=[UPCStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e12_medium_nogsf_ion_L1EM7_VTE200',  l1SeedThresholds=['EM7'],  stream=[UPCStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_g12_loose_L1EM7_VTE200',       l1SeedThresholds=['EM7'],  stream=[UPCStream], groups=SinglePhotonGroup+PrimaryLegGroup),
        ChainProp(name='HLT_g12_medium_L1EM7_VTE200',      l1SeedThresholds=['EM7'],  stream=[UPCStream], groups=SinglePhotonGroup+PrimaryLegGroup),
    ]

    chains['Jet'] += [
        ChainProp(name='HLT_j40_ion_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j50_ion_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j60_ion_L1J15', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j75_ion_L1J20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j75_ion_L1J30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j85_ion_L1J15', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j85_ion_L1J30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryLegGroup),

        #--- UPC jets
        ChainProp(name='HLT_j10_L1VTE200',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j15_L1TE5_VTE200',l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j20_L1TE5_VTE200',l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j40_L1TE5_VTE200',l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),

        ChainProp(name='HLT_j10_320eta490_L1VTE200',    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j15_320eta490_L1TE5_VTE200',l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j20_320eta490_L1TE5_VTE200',l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),
        ChainProp(name='HLT_j40_320eta490_L1TE5_VTE200',l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=SingleJetGroup+PrimaryLegGroup),

        ChainProp(name='HLT_j60_ion_L1jJ20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j75_ion_L1jJ20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j75_ion_L1jJ30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j85_ion_L1jJ20', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_j85_ion_L1jJ30', l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=SingleJetGroup+PrimaryPhIGroup),
    ]


    chains['Combined'] += [
        #----------- mu+j PrimaryLeg
        ChainProp(name='HLT_mu4_j20_L1MU3V_J12',            l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j20_dRAB04_L1MU3V_J12',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j20_ion_dRAB04_L1MU3V_J12', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j40_ion_dRAB04_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j50_ion_dRAB04_L1MU3V',     l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryLegGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j50_ion_dRAB04_L1MU3V_J12', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryLegGroup+SingleBjetGroup),

        #----------- mu+j with new calo
        ChainProp(name='HLT_mu4_j20_ion_dRAB04_L1MU3V_jJ30', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j35_ion_dRAB04_L1MU3V_jJ40', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=SupportPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j45_ion_dRAB04_L1MU3V_jJ40', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryPhIGroup+SingleBjetGroup),
        ChainProp(name='HLT_mu4_j60_ion_dRAB04_L1MU3V_jJ40', l1SeedThresholds=['MU3V','FSNOSEED'], stream=[HardProbesStream], groups=PrimaryPhIGroup+SingleBjetGroup),
    ]


    chains['MinBias'] += [
        ChainProp(name='HLT_mb_sp_pix100_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp_pix200_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp_pix500_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp_pix1000_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp_vetospmbts2in_L1TE5_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        #----------- mbts
        ChainProp(name="HLT_mb_mbts_L1MBTS_2_2", l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_3_3", l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_4_4", l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),

        #----------- sptrk
        ChainProp(name='HLT_mb_sptrk_L1VTE50',          l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_1_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),

        #----------- UPC HMT
        ChainProp(name='HLT_mb_sp_L1VTE50',                    l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp_L1MBTS_1_VTE50',             l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp_L1TE3_VTE50',                l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp15_trk15_hmt_L1MBTS_1_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_mb_sp300_trk20_hmt_L1TE3_VTE50',   l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),

    ]

    chains['Streaming'] += [
        ChainProp(name='HLT_noalg_L1TAU1_TE4_VTE200', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L12TAU1_VTE50',     l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XOR_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),

        ChainProp(name='HLT_noalg_L1J15',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1J30',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1EM12',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1EM16',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L12MU3V',  l1SeedThresholds=['FSNOSEED'], stream=[HardProbesStream], groups=MinBiasGroup+SupportGroup),

        #----TE MinBias----
        ChainProp(name='HLT_noalg_L1TE5',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE20',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE100', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE200', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        # really needed if we don't have UCC trigger in Run3?
        ChainProp(name='HLT_noalg_L1TE10000', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE12000', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        #----TE MinBias using restricted eta threshold----
        ChainProp(name='HLT_noalg_L1TE3p0ETA49',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE7p0ETA49',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE1500p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE3000p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE3500p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE6500p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE8000p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),

        #----should be pc/cc stream, to be fixed
        ChainProp(name='HLT_noalg_L1TE50_VTE600p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE600p0ETA49',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),

        #----TE MinBias OVERLAY----
        ChainProp(name='HLT_noalg_L1TE50_OVERLAY',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE600p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE1500p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE3000p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE3500p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE6500p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE8000p0ETA49_OVERLAY',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),

        ChainProp(name='HLT_noalg_L1MBTS_1_VTE50', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TE50_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1TRT_VTE200',   l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+SupportGroup),


    ]


    return chains
