# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# PhysicsP1_pp_lowMu_run3_v1.py menu
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],
from ..Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore
from .Physics_pp_run3_v1 import (
    PhysicsStream,
    SingleMuonGroup,
    MultiMuonGroup,
    SingleElectronGroup,
    MinBiasGroup,
    SupportGroup,
    SupportLegGroup,
    SupportPhIGroup,
    JetStreamersGroup,
    METStreamersGroup,
    TauStreamersGroup,
    EgammaStreamersGroup,
    PrimaryL1MuGroup,
    PrimaryLegGroup,
    PrimaryPhIGroup,
    SinglePhotonGroup,
)
from . import P1_run3_v1

LowMuGroup = ['LowMu']
LowMuGroupPhI = ['LowMuPhaseI']
LowMuGroupLeg = ['LowMuLegacy']


def getLowMuPhysicsSignatures():

    chains = ChainStore()

    chains['Muon'] = [

        #ART-23577
        ChainProp(name='HLT_mu20_L1MU5VF_AFP_A_OR_C',     l1SeedThresholds=['MU5VF'],   groups=SingleMuonGroup+LowMuGroup),
        ChainProp(name='HLT_mu20_L1MU5VF_AFP_A_AND_C',     l1SeedThresholds=['MU5VF'],   groups=SingleMuonGroup+LowMuGroup),
        ChainProp(name='HLT_mu20_L1MU5VF',     l1SeedThresholds=['MU5VF'],   groups=SingleMuonGroup+LowMuGroup),

        #ATR-27744
        #-- 1 mu
        ChainProp(name='HLT_mu4_L1MU3V',   stream=[PhysicsStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu6_L1MU3V',  stream=[PhysicsStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu8_L1MU5VF',  stream=[PhysicsStream, 'express'], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu10_L1MU8F',  stream=[PhysicsStream], groups=SingleMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu12_L1MU8F', stream=[PhysicsStream], groups=SingleMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu15_L1MU8F', stream=[PhysicsStream], groups=SingleMuonGroup+PrimaryL1MuGroup),

        #-- 2 mu
        ChainProp(name='HLT_2mu4_L12MU3V', stream=[PhysicsStream, 'express'], groups=MultiMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu4_mu6_L12MU3V', stream=[PhysicsStream], groups=MultiMuonGroup+PrimaryL1MuGroup),
        ChainProp(name='HLT_mu4_mu4noL1_L1MU3V', stream=[PhysicsStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup, monGroups=['muonMon:shifter','muonMon:online']),
        ChainProp(name='HLT_mu4_mu4noL1_L1MBTS_1', stream=[PhysicsStream], l1SeedThresholds=['MU3V','FSNOSEED'], groups=MultiMuonGroup+PrimaryL1MuGroup),



    ]

    chains['Egamma'] = [

        #ART-23577
        ChainProp(name='HLT_e20_lhloose_L1eEM9_AFP_A_OR_C', l1SeedThresholds=['eEM9'], groups=SingleElectronGroup+LowMuGroupPhI),
        ChainProp(name='HLT_e20_lhloose_L1eEM9_AFP_A_AND_C', l1SeedThresholds=['eEM9'], groups=SingleElectronGroup+LowMuGroupPhI),
        ChainProp(name='HLT_e20_lhloose_L1eEM9', l1SeedThresholds=['eEM9'], groups=LowMuGroupPhI),

        #ATR-27744
        # ElectronChains----------
        #--------- legacy supporting electron chains
        ChainProp(name='HLT_e15_etcut_L1EM12', stream=[PhysicsStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e18_etcut_L1EM15', stream=[PhysicsStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e20_etcut_L1EM15', stream=[PhysicsStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e30_etcut_L1EM15', stream=[PhysicsStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e50_etcut_L1EM15', stream=[PhysicsStream] ,groups=SingleElectronGroup+SupportLegGroup),
        ChainProp(name='HLT_e20_idperf_loose_nogsf_L1EM15', stream=[PhysicsStream, 'express'], groups=SingleElectronGroup+SupportLegGroup, monGroups=['idMon:t0']),


        #--------- legacy physics electron chains
        ChainProp(name='HLT_e15_lhloose_nogsf_L1EM12',  stream=[PhysicsStream, 'express'], groups=SingleElectronGroup+PrimaryLegGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:val']),
        ChainProp(name='HLT_e15_loose_nogsf_L1EM12',    stream=[PhysicsStream, 'express'], groups=SingleElectronGroup+PrimaryLegGroup, monGroups=['egammaMon:online','egammaMon:shifter_tag','egammaMon:val']),
        ChainProp(name='HLT_e15_lhmedium_nogsf_L1EM12', stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e15_medium_nogsf_L1EM12',   stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryLegGroup),

        ChainProp(name='HLT_e18_lhloose_nogsf_L1EM15',  stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryLegGroup, monGroups=['egammaMon:shifter_tp']),
        ChainProp(name='HLT_e18_loose_nogsf_L1EM15',    stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryLegGroup, monGroups=['egammaMon:shifter_tp']),
        ChainProp(name='HLT_e18_lhmedium_nogsf_L1EM15', stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e18_medium_nogsf_L1EM15',   stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryLegGroup),

        ChainProp(name='HLT_e20_lhloose_nogsf_L1EM15',  stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e20_lhmedium_nogsf_L1EM15', stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e20_loose_nogsf_L1EM15',    stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryLegGroup),
        ChainProp(name='HLT_e20_medium_nogsf_L1EM15',   stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryLegGroup),

        #--------- phase-1 supporting electron chains
        # replace L1eEM9 with L1eEM15 and L1eEM15 with eEM18, ATR-26366
        ChainProp(name='HLT_e15_etcut_L1eEM15',    stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e20_etcut_L1eEM18',   stream=[PhysicsStream], groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e30_etcut_L1eEM26', stream=[PhysicsStream] ,groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e50_etcut_L1eEM26', stream=[PhysicsStream] ,groups=SingleElectronGroup+SupportPhIGroup),
        ChainProp(name='HLT_e20_idperf_loose_nogsf_L1eEM18', stream=[PhysicsStream, 'express'], groups=SingleElectronGroup+SupportPhIGroup, monGroups=['idMon:t0']),

        #--------- phase-1 physics electron chains
        ChainProp(name='HLT_e15_lhloose_nogsf_L1eEM15',  stream=[PhysicsStream, 'express'], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e15_loose_nogsf_L1eEM15',    stream=[PhysicsStream, 'express'], groups=SingleElectronGroup+PrimaryPhIGroup, monGroups=['egammaMon:t0_tp']),
        ChainProp(name='HLT_e15_lhmedium_nogsf_L1eEM15', stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e15_medium_nogsf_L1eEM15',   stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        ChainProp(name='HLT_e20_lhloose_nogsf_L1eEM18', stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_lhmedium_nogsf_L1eEM18',stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_nogsf_L1eEM18',   stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_loose_nogsf_L1eEM18L',  stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_e20_medium_nogsf_L1eEM18',  stream=[PhysicsStream], groups=SingleElectronGroup+PrimaryPhIGroup),

        # PhotonChains----------
        #----------- legacy support photon chains
        ChainProp(name='HLT_g13_etcut_L1EM10', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g18_etcut_L1EM10', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g28_etcut_L1EM10', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g15_etcut_L1EM12', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g20_etcut_L1EM12', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportLegGroup),

        #----------- legacy primary photon chains
        ChainProp(name='HLT_g15_loose_L1EM10',  stream=[PhysicsStream, 'express'], groups=SinglePhotonGroup+PrimaryLegGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),
        ChainProp(name='HLT_g15_loose_L1EM12',  stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryLegGroup),
        ChainProp(name='HLT_g20_loose_L1EM12',  stream=[PhysicsStream, 'express'], groups=SinglePhotonGroup+PrimaryLegGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),
        ChainProp(name='HLT_g30_loose_L1EM15',  stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryLegGroup),
        ChainProp(name='HLT_g50_loose_L1EM15',  stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryLegGroup),


        #----------- phase-1 support photon chains
        ChainProp(name='HLT_g13_etcut_L1eEM12L', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g18_etcut_L1eEM12L', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g28_etcut_L1eEM18', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g15_etcut_L1eEM15', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g20_etcut_L1eEM15', stream=[PhysicsStream], groups=SinglePhotonGroup+SupportPhIGroup),

        #----------- phase-1 primary photon chains
        ChainProp(name='HLT_g15_loose_L1eEM12L',  stream=[PhysicsStream, 'express'], groups=SinglePhotonGroup+PrimaryPhIGroup, monGroups=['egammaMon:online','egammaMon:shifter','egammaMon:val']),
        ChainProp(name='HLT_g15_loose_L1eEM15', stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g20_loose_L1eEM15', stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g30_loose_L1eEM18',  stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),
        ChainProp(name='HLT_g50_loose_L1eEM26',  stream=[PhysicsStream], groups=SinglePhotonGroup+PrimaryPhIGroup),

    ]

    chains['Jet'] = [
        ChainProp(name='HLT_j20_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j20_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j20_L1AFP_A_OR_C_J12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_j20_L1AFP_A_AND_C_J12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_j20_L1AFP_A_OR_C_jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1AFP_A_AND_C_jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1AFP_A_OR_C_jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1AFP_A_AND_C_jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1J12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_j20_L1jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_j20_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j20f_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j20f_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j10f_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_j10f_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_2j20_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_2j20_20detaAA_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_2j20_20detaAA_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_2j10_20detaAA_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_2j10_20detaAA_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
    ]

    chains['Bjet'] = [

    ]

    chains['Combined'] = [
        # AFP + dijet
        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J50', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J75', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ90', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ125', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupPhI),
        # To follow up with forward physics
        # ChainProp(name='HLT_2j135_mb_afprec_afpdijet_L1CEP-CjJ100', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupPhI+Topo3Group),
        # ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1CEP-CjJ90', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupPhI+Topo3Group),

        ChainProp(name='HLT_2j20_mb_afprec_afpdijet_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroup),
    ]

    chains['MinBias'] = [
        ChainProp(name='HLT_mb_sptrk_L1RD0_FILLED',    l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=LowMuGroup+MinBiasGroup+['RATE:CPS_RD0_FILLED'], monGroups=['mbMon:online','mbMon:shifter']),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp_L1RD0_FILLED',       l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=LowMuGroup+MinBiasGroup+['RATE:CPS_RD0_FILLED']),

        ChainProp(name='HLT_mb_excl_2trk6_pt1_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_excl_1trk5_pt4_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        ChainProp(name='HLT_mb_excl_1trk2_pt1_vetombts2in_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_excl_1trk2_pt1_vetombts2in_L1TRT_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_excl_1trk2_pt1_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_excl_1trk2_pt1_L1TRT_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),



        ChainProp(name='HLT_mb_sptrk_pt2_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt2_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt4_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt4_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt6_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt6_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt8_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt8_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt2_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt4_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt6_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt8_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt2_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt4_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt6_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt8_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_alfaperf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'],     groups=LowMuGroup+MinBiasGroup),
        ChainProp(name='HLT_mb_alfaperf_L1RD0_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'],     groups=LowMuGroup+MinBiasGroup),
        ChainProp(name="HLT_mb_mbts_L1MBTS_1_EMPTY",               l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name="HLT_mb_mbts_L1MBTS_1_UNPAIRED_ISO",               l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name="HLT_mb_mbts_L1MBTS_1",                     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name="HLT_mb_mbts_L1MBTS_1_1",                   l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name="HLT_mb_mbts_L1MBTS_2",                     l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name="HLT_mb_mbts_L1RD0_FILLED",                 l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name="HLT_mb_mbts_L1RD0_EMPTY",                  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name="HLT_mb_mbts_L1RD0_UNPAIRED_ISO",                  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),


        ChainProp(name='HLT_mb_sptrk_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_2_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1MBTS_2_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp900_trk60_hmt_L1MBTS_1_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1400_trk90_hmt_L1TE5', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_sptrk_pt4_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt6_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_pt8_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        #ATR-27744
        ChainProp(name='HLT_mb_sptrk_L1ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_XOR_E2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_XOR_E1_E3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_E1_AND_E1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_E1_AND_E2ORE3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_E2_AND_E2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_E2_AND_E3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_E3_AND_E3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_XOR_E2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_XOR_E1_E3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E1_AND_E1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E1_AND_E2ORE3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E2_AND_E2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E2_AND_E3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E3_AND_E3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1TE3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1jTE3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroupPhI),


        # AFP
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_J20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_J20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_J30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_J30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_J50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_J50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_J75', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_J75', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupLeg),

        # Phase I jet inputs ATR-24411
        # AFP
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ60', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ125', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ125', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI),

        # HMT
        ChainProp(name='HLT_mb_sp500_trk40_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp500_trk40_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp500_trk40_hmt_L1MBTS_2_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp500_trk40_hmt_L1MBTS_3_3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_trk50_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_trk50_hmt_L1TE3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias','express'], groups=MinBiasGroup+LowMuGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_sp600_trk50_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_trk50_hmt_L1MBTS_2_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_trk50_hmt_L1MBTS_3_3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_trk60_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_trk60_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_trk60_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_trk60_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_trk60_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_trk60_hmt_L1TE3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_sp1000_trk80_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1000_trk80_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1000_trk80_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1000_trk80_hmt_L1TE5',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1500_trk100_hmt_L1TE5',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_sp2000_trk130_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp2000_trk130_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp2000_trk130_hmt_L1TE5',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_sp3000_trk200_hmt_L1TE20',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_sp4100_trk260_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_trk260_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_trk260_hmt_L1TE20',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupLeg),
        ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_trk290_hmt_L1TE50',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup+LowMuGroupLeg),

        # HMT with pileup suppression
        ChainProp(name='HLT_mb_sp15_pusup0_trk5_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp500_pusup7_trk40_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp500_pusup7_trk40_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp500_pusup7_trk40_hmt_L1MBTS_2_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp500_pusup7_trk40_hmt_L1MBTS_3_3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_pusup10_trk50_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_pusup10_trk50_hmt_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_pusup10_trk50_hmt_L1MBTS_2_2',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp600_pusup10_trk50_hmt_L1MBTS_3_3',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_pusup15_trk60_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp800_pusup15_trk60_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1000_pusup30_trk80_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1000_pusup30_trk80_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1500_pusup40_trk100_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp1500_pusup40_trk100_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp2000_pusup50_trk130_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp2000_pusup50_trk130_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp2000_pusup60_trk130_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp2000_pusup60_trk130_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp3000_pusup100_trk200_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp3000_pusup100_trk200_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp3000_pusup120_trk200_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp3000_pusup120_trk200_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup150_trk260_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup150_trk260_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup180_trk260_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup180_trk260_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup200_trk260_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp4100_pusup200_trk260_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup180_trk290_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup180_trk290_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup220_trk290_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup220_trk290_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup250_trk290_hmt_L1RD0_FILLED',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_sp5000_pusup250_trk290_hmt_L1MBTS_4_4',          l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

        # afprec chains
        ChainProp(name='HLT_mb_afprec_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_afprec_afptof_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroup,monGroups=['mbMon:online','mbMon:shifter']),
        # To follow up with forward physics
        # ChainProp(name='HLT_mb_afprec_L1CEP-CjJ100', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI+Topo3Group),
        # ChainProp(name='HLT_mb_afprec_L1CEP-CjJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+LowMuGroupPhI+Topo3Group),
        ChainProp(name='HLT_mb_sptrk_vetombts2in_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup),

    ]

    chains['Monitor'] = [
        ChainProp(name='HLT_noalg_CostMonDS_L1All',        l1SeedThresholds=['FSNOSEED'], stream=['CostMonitoring'], groups=['Primary:CostAndRate', 'RATE:Monitoring', 'BW:Other']), # HLT_costmonitor
    ]

    chains['Streaming'] = [

        ChainProp(name='HLT_noalg_L1TRT_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic','express'],groups=['RATE:SeededStreamers','BW:Other']),
        ChainProp(name='HLT_noalg_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic'],groups=['RATE:SeededStreamers','BW:Other']),

        ChainProp(name='HLT_noalg_L1RD0_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=MinBiasGroup),
        ChainProp(name='HLT_noalg_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=MinBiasGroup), 

        ChainProp(name='HLT_noalg_mb_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_mb_L1MBTS_1_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_mb_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_1_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_1_1_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_2_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup), 
        ChainProp(name='HLT_noalg_mb_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+LowMuGroup), 

        ChainProp(name='HLT_noalg_L1MU3V',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup),
        ChainProp(name='HLT_noalg_L1MU8VF',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup),
        ChainProp(name="HLT_noalg_L1MU5VF",      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup),
        #EM3 is removed from HI L1 menu
        ChainProp(name='HLT_noalg_L1EM15',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=EgammaStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM10VH',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=EgammaStreamersGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1TAU8',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=TauStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU60',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=TauStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU12IM',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=TauStreamersGroup+SupportLegGroup),        
        ChainProp(name='HLT_noalg_L1TAU20IM',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=TauStreamersGroup+SupportLegGroup),        

        ChainProp(name='HLT_noalg_L1J12',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J15',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J20',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J25',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J30',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J40',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J50',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J75',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J85',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J100',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1jJ20',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ30',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportPhIGroup),
        
        ChainProp(name='HLT_noalg_L1XE55',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=METStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1XE60',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=METStreamersGroup+SupportLegGroup),

        # Low mu AFP
        ChainProp(name='HLT_noalg_L1MU5VF_AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU5VF_AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),

        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_jJ20', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_jJ30', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_J12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_J12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MinBiasGroup+SupportGroup),

        # Calibration AFP
        # low mu
        ChainProp(name='HLT_noalg_L1AFP_NSA_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_NSC_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),

        # ChainProp(name='HLT_noalg_L1CEP-CjJ100', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportPhIGroup+Topo3Group),
        # ChainProp(name='HLT_noalg_L1CEP-CjJ90', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportPhIGroup+Topo3Group),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_J50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_J75', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_J50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_J75', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportLegGroup),

        # Calibration AFP
        # all mu
        ChainProp(name='HLT_noalg_L1AFP_FSA_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T2_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T3_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T2_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T3_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MinBiasGroup+SupportGroup),

        ChainProp(name='HLT_noalg_L1ZDC_A', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1ZDC_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1ZDC_A_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup+SupportGroup),

        
    ]
    return chains

def setupMenu(menu_name):
    # Add all standard monitoring chains from addP1Signatures function
    final_chains = ChainStore()

    chains = getLowMuPhysicsSignatures()
    P1_run3_v1.addCommonP1Signatures(chains)
    P1_run3_v1.addLowMuP1Signatures(chains)
    for sig, chainsInSig in chains.items():
        for c in chainsInSig:
                if "EM3" in c.name: # EM3 without VTE or AFP is removed from HI L1 menu to avoid L1Calo EM overflow
                    raise RuntimeError(f"EM3 not available in HI L1 menu, requested by chain {c.name}")
                if "EM7" in c.name: # EM7 without VTE or AFP is removed from HI L1 menu to avoid L1Calo EM overflow
                    raise RuntimeError(f"EM7 not available in HI L1 menu, requested by chain {c.name}")
                else:
                    final_chains[sig].append(c)

    return final_chains
