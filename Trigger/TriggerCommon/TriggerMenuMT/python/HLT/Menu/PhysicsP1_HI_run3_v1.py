# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# PhysicsP1_HI_run3_v1.py menu for the long shutdown development
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore
from .Physics_pp_run3_v1 import (
        SingleMuonGroup,
        SingleElectronGroup,
        #SinglePhotonGroup,
        SingleJetGroup,
        MultiJetGroup,
        MinBiasGroup,
        #PrimaryL1MuGroup,
        SupportGroup,
        SupportLegGroup,
        Topo3Group
)
from .PhysicsP1_pp_run3_v1 import addP1Signatures

PhysicsStream="Main"
HardProbesStream="HardProbes"
MinBiasStream="MinBias"
UPCStream="UPC"
ZDCPEBStream="ZDCCalib"
### following stream tags not used yet, need to be implemented in StreamInfo.py before use
#UCCStream="UCC"
#PCStream="PC"
#CCStream="CC"
#PCpebStream="PCPEB"
#CCpebStream="CCPEB"

LowMuGroup = ['LowMu']
LowMuGroupPhI = ['LowMuPhaseI']
LowMuGroupLeg = ['LowMuLegacy']

def setupMenu():

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('setupMenu ...')

    chains = ChainStore()

    chains['Muon'] = [
        #ChainProp(name='HLT_mu4_L1MU3V', stream=[HardProbesStream], groups=SingleMuonGroup),

#ART-26051, AFP+Muon triggers for LHCf runs
        ChainProp(name='HLT_mu20_L1MU5VF_AFP_A_OR_C',   l1SeedThresholds=['MU5VF'], stream=[PhysicsStream], groups=SingleMuonGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mu20_L1MU5VF_AFP_A_AND_C',  l1SeedThresholds=['MU5VF'], stream=[PhysicsStream], groups=SingleMuonGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mu20_L1MU5VF',              l1SeedThresholds=['MU5VF'], stream=[PhysicsStream], groups=SingleMuonGroup+LowMuGroup+['PS:Online']),

    ]

    chains['Egamma'] = [

#ART-26051, AFP+Electron triggers for LHCf runs
        ChainProp(name='HLT_e20_lhloose_L1EM7_AFP_A_OR_C',  l1SeedThresholds=['EM7'], stream=[PhysicsStream], groups=SingleElectronGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_e20_lhloose_L1EM7_AFP_A_AND_C', l1SeedThresholds=['EM7'], stream=[PhysicsStream], groups=SingleElectronGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_e20_lhloose_L1EM7',             l1SeedThresholds=['EM7'], stream=[PhysicsStream], groups=SingleElectronGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_e20_lhloose_L1eEM9_AFP_A_OR_C', l1SeedThresholds=['eEM9'],stream=[PhysicsStream], groups=SingleElectronGroup+LowMuGroupPhI+['PS:Online']),
        ChainProp(name='HLT_e20_lhloose_L1eEM9_AFP_A_AND_C',l1SeedThresholds=['eEM9'],stream=[PhysicsStream], groups=SingleElectronGroup+LowMuGroupPhI+['PS:Online']),
        ChainProp(name='HLT_e20_lhloose_L1eEM9',            l1SeedThresholds=['eEM9'],stream=[PhysicsStream], groups=SingleElectronGroup+LowMuGroupPhI+['PS:Online']),

    ]

    chains['Jet'] = [

#ART-26051, AFP+Jet triggers for LHCf runs
        ChainProp(name='HLT_j20_L1AFP_A_AND_C',     l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_j20_L1AFP_A_OR_C',      l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_j20_L1AFP_A_AND_C_J12', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_j20_L1AFP_A_OR_C_J12',  l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_j20_L1J12',             l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_j20_L1MBTS_2',          l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_j20_L1RD0_FILLED',      l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_j20f_L1AFP_A_OR_C',     l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_j20f_L1AFP_A_AND_C',    l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_j10f_L1MBTS_2',         l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_j10f_L1RD0_FILLED',     l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=SingleJetGroup+LowMuGroup+['PS:Online']),


        ChainProp(name='HLT_2j10_20detaAA_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MultiJetGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_2j10_20detaAA_L1AFP_A_OR_C',  l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MultiJetGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_2j20_20detaAA_L1RD0_FILLED',  l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MultiJetGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_2j20_20detaAA_L1MBTS_2',      l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MultiJetGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_2j20_L1RD0_FILLED',           l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream], groups=MultiJetGroup+LowMuGroup+['PS:Online']),

    ]


    chains['Combined'] = [
#ART-26051, AFP triggers for LHCf runs
        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J50',   l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J75',   l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ90',  l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupPhI+['PS:Online']),
        ChainProp(name='HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ125', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupPhI+['PS:Online']),
        ChainProp(name='HLT_2j135_mb_afprec_afpdijet_L1CEP-CjJ100',            l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupPhI+Topo3Group+['PS:Online']),
        ChainProp(name='HLT_2j120_mb_afprec_afpdijet_L1CEP-CjJ90',             l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroupPhI+Topo3Group+['PS:Online']),
        ChainProp(name='HLT_2j20_mb_afprec_afpdijet_L1RD0_FILLED',             l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream],groups=MinBiasGroup+LowMuGroup+['PS:Online']),
    ]


    chains['MinBias'] = [
#ART-26051, AFP MinBias triggers for LHCf runs
        ChainProp(name='HLT_mb_sp_L1RD0_FILLED',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt2_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt2_L1MBTS_2',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt4_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt4_L1MBTS_2',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt6_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt6_L1MBTS_2',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt8_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt8_L1MBTS_2',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt2_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt4_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt6_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt8_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt2_L1AFP_A_AND_C',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt4_L1AFP_A_AND_C',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt6_L1AFP_A_AND_C',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt8_L1AFP_A_AND_C',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt4_L1MBTS_1',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt6_L1MBTS_1',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_pt8_L1MBTS_1',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_alfaperf_L1RD0_FILLED',  l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'],     groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_alfaperf_L1RD0_EMPTY',   l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'],     groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_1",          l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_1_1",        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name="HLT_mb_mbts_L1MBTS_2",          l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name="HLT_mb_mbts_L1RD0_FILLED",      l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name="HLT_mb_mbts_L1RD0_EMPTY",       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name="HLT_mb_mbts_L1RD0_UNPAIRED_ISO",l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),

#ART-26051, AFP triggers for LHCf runs
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_J20',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_J20',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_J30',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_J30',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_J50',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_J50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_J75',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_J75',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupLeg+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ50',      l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupPhI+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ50', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupPhI+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ60',      l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupPhI+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ60', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupPhI+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ90',      l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupPhI+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ90', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupPhI+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ125',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupPhI+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1AFP_A_AND_C_TOF_T0T1_jJ125',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroupPhI+['PS:Online']),


#ART-26051, AFP triggers for LHCf runs
        ChainProp(name='HLT_mb_afprec_L1AFP_A_OR_C',            l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_afprec_L1RD0_FILLED',            l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),
        ChainProp(name='HLT_mb_sptrk_vetombts2in_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online']),


#ATR-26051 ZDC Minbias chains for LHCf runs
        ChainProp(name='HLT_mb_sptrk_L1ZDC_OR',            l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_XOR_E2',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_XOR_E1_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_E1_AND_E1',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_E1_AND_E2ORE3', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_E2_AND_E2',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_E2_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_E3_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sptrk_L1ZDC_A_AND_C',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_OR',            l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_XOR_E2',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_XOR_E1_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E1_AND_E1',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E1_AND_E2ORE3', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E2_AND_E2',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E2_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E3_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_A_AND_C',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),


#TRT seeded Minbias chains for testing in LHCf runs
        ChainProp(name='HLT_mb_excl_1trk2_pt1_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        ChainProp(name='HLT_mb_excl_1trk2_pt1_L1TRT_VTE50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+LowMuGroup+['PS:Online', 'PS:NoHLTReprocessing']),


    ]



    chains['Streaming'] = [

#ART-26051, AFP streamers for LHCf runs
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C',               l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_J12',           l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_J50',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_J75',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_J50',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_J75',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C',                l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_J12',            l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_EMPTY',          l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_FIRSTEMPTY',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_UNPAIRED_ISO',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_OR_C_UNPAIRED_NONISO',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_BGRP12',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T2_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T3_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_BGRP12',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T2_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T3_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_NSA_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_NSC_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1EM7_AFP_A_OR_C',    l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM7_AFP_A_AND_C',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1MU5VF_AFP_A_OR_C',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU5VF_AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1RD0_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']+SupportGroup), 

#Run2-style Heavy Ion ZDC streamers
        ChainProp(name='HLT_noalg_L1ZDC_A', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_L1ZDC_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_L1ZDC_AND', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']+SupportGroup),


        ChainProp(name='HLT_noalg_L1MBTS_1',      l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']), 
        ChainProp(name='HLT_noalg_L1MBTS_1_1',    l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']), 
        ChainProp(name='HLT_noalg_L1MBTS_2',      l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']), 
        ChainProp(name='HLT_noalg_mb_L1MBTS_1',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']), 
        ChainProp(name='HLT_noalg_mb_L1MBTS_1_1', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']), 
        ChainProp(name='HLT_noalg_mb_L1MBTS_2',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']), 
        ChainProp(name='HLT_noalg_L1MBTS_2_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']), 
        ChainProp(name='HLT_noalg_mb_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']), 

        ChainProp(name='HLT_noalg_L1MU3V',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name="HLT_noalg_L1MU5VF", l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1EM3',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM7',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J12',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online']+SupportLegGroup),


#ZDC streamer for LHCf+ZDC special run ATR-26051
        ChainProp(name='HLT_noalg_L1ZDC_OR',            l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XOR_E2',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_XOR_E1_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_E1_AND_E1',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_E1_AND_E2ORE3', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_E2_AND_E2',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_E2_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_E3_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_A_AND_C',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_OR_EMPTY',          l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_OR_UNPAIRED_ISO',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_OR_UNPAIRED_NONISO',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),


#ZDC bits and comb streamer 
        ChainProp(name='HLT_noalg_L1ZDC_BIT2',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_BIT1',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_BIT0',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_COMB0', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_COMB1', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_COMB2', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_COMB3', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_COMB4', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_COMB5', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_COMB6', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ZDC_COMB7', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),


#LHCf 
        ChainProp(name='HLT_noalg_L1LHCF', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online', 'PS:NoHLTReprocessing']+MinBiasGroup),


    ]


    chains['Calib'] += [
# 'ZDCCalib' stream
          ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_OR_LHCF', l1SeedThresholds=['FSNOSEED'], stream=[ZDCPEBStream], groups=['PS:Online','RATE:Calibration','BW:Detector']),
    ]


    chains['Monitor'] = [
          ChainProp(name='HLT_noalg_CostMonDS_L1All',        l1SeedThresholds=['FSNOSEED'], stream=['CostMonitoring'], groups=['RATE:Monitoring','BW:Other']),
    ]
    tempChains = ChainStore()
    addP1Signatures(tempChains)
    for sig, chainsInSig in tempChains.items():
        if sig == "Beamspot":  # HI needs special beam spot setup
                continue
        for c in chainsInSig:
                if "IDCalibPEB" in c.name: # heavy tracking
                        continue
                else:
                        chains[sig].append(c)
    return chains
