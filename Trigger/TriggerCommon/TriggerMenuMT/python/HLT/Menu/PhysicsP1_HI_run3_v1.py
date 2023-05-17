# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# PhysicsP1_HI_run3_v1.py menu
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],
from ..Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore
from .Physics_pp_run3_v1 import (
        SingleMuonGroup,
        #SingleElectronGroup,
        SingleJetGroup,
        #MultiJetGroup,
        MinBiasGroup,
        SupportGroup,
        #Topo3Group,
        SupportLegGroup
)
from . import P1_run3_v1

PhysicsStream="Main"
HardProbesStream="HardProbes"
MinBiasStream="MinBias"
UPCStream="UPC"
MinBiasOverlayStream="MinBiasOverlay"
### following stream tags not used yet, need to be implemented in StreamInfo.py before use
#UCCStream="UCC"
#PCStream="PC"
#CCStream="CC"
#PCpebStream="PCPEB"
#CCpebStream="CCPEB"

LowMuGroup = ['LowMu']
LowMuGroupPhI = ['LowMuPhaseI']
LowMuGroupLeg = ['LowMuLegacy']

def getPhysicsHISignatures():
    chains = ChainStore()

    chains['Muon'] = [
        ChainProp(name='HLT_mu4_L1MU3V', stream=[HardProbesStream], groups=SingleMuonGroup),
        # ALFA + dimuon triggers
        ChainProp(name='HLT_mu4_mu2noL1_L1MU3V_ALFA_ANY', l1SeedThresholds=['MU3V','FSNOSEED'], stream=['MinBias'], groups=['PS:Online','PS:NoHLTReprocessing']+SingleMuonGroup),
        ChainProp(name='HLT_mu4_mu2noL1_L1MU3V_ALFA_EINE', l1SeedThresholds=['MU3V','FSNOSEED'], stream=['MinBias'], groups=['PS:Online','PS:NoHLTReprocessing']+SingleMuonGroup),
    ]

    chains['Egamma'] = [

    ]

    chains['Jet'] = [

        # ALFA + jet triggers
        ChainProp(name='HLT_j15_L1ALFA_Jet_Phys', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online','PS:NoHLTReprocessing']+SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_2j10_L1ALFA_ELAS', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online','PS:NoHLTReprocessing']+SingleJetGroup),
        ChainProp(name='HLT_2j10_L1ALFA_SYS', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online','PS:NoHLTReprocessing']+SingleJetGroup),
    ]


    chains['Combined'] = [

    ]


    chains['MinBias'] = [
        #ATR-26051 ZDC Minbias chains for LHCf runs
        #Commented out for the 2022 Nov Pb+Pb test run as the corresponding L1 ZDC items were commented out in the L1 menu
        #These trigger will be needed for 2023 heavy ion runs
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_OR',            l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_XOR_E2',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_XOR_E1_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_E1_AND_E1',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_E1_AND_E2ORE3', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_E2_AND_E2',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_E2_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_E3_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sptrk_L1ZDC_A_AND_C',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_OR',            l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_XOR_E2',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_XOR_E1_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E1_AND_E1',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E1_AND_E2ORE3', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E2_AND_E2',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E2_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_E3_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),
        # ChainProp(name='HLT_mb_sp100_trk30_hmt_L1ZDC_A_AND_C',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online', 'PS:NoHLTReprocessing']),

    ]



    chains['Streaming'] = [

        ChainProp(name='HLT_noalg_L1RD0_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=[UPCStream], groups=MinBiasGroup+['PS:Online']+SupportGroup), 

        #Run2-style Heavy Ion ZDC streamers
        ChainProp(name='HLT_noalg_L1ZDC_A', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_L1ZDC_C', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_L1ZDC_AND', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']+SupportGroup),


        ChainProp(name='HLT_noalg_mb_L1MBTS_1',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']),
        ChainProp(name='HLT_noalg_mb_L1MBTS_1_1', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']),
        ChainProp(name='HLT_noalg_mb_L1MBTS_2',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']),
        ChainProp(name='HLT_noalg_L1MBTS_2_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']),
        ChainProp(name='HLT_noalg_mb_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+['PS:Online']),

        ChainProp(name='HLT_noalg_L1MU3V',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name="HLT_noalg_L1MU5VF", l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1J12',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:Online']+SupportLegGroup),


        #ZDC streamer for LHCf+ZDC special run ATR-26051
        #Commented out for the 2022 Nov Pb+Pb test run as the corresponding L1 ZDC items were commented out in the L1 menu
        #These trigger will be needed for 2023 heavy ion runs
        # ChainProp(name='HLT_noalg_L1ZDC_OR',            l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_XOR_E2',        l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_XOR_E1_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_E1_AND_E1',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_E1_AND_E2ORE3', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_E2_AND_E2',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_E2_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_E3_AND_E3',     l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_A_AND_C',       l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_OR_EMPTY',          l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_OR_UNPAIRED_ISO',   l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_OR_UNPAIRED_NONISO',l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),


        #ZDC bits and comb streamer
        #Commented out for the 2022 Nov Pb+Pb test run as the corresponding L1 ZDC items were commented out in the L1 menu
        #These trigger will be needed for 2023 heavy ion runs
        # ChainProp(name='HLT_noalg_L1ZDC_BIT2',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_BIT1',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_BIT0',  l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_COMB0', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_COMB1', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_COMB2', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_COMB3', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_COMB4', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_COMB5', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_COMB6', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        # ChainProp(name='HLT_noalg_L1ZDC_COMB7', l1SeedThresholds=['FSNOSEED'], stream=[MinBiasStream], groups=['PS:NoHLTReprocessing']+MinBiasGroup),


    ]

    chains['Monitor'] = [
          ChainProp(name='HLT_noalg_CostMonDS_L1All',        l1SeedThresholds=['FSNOSEED'], stream=['CostMonitoring'], groups=['RATE:Monitoring','BW:Other']),
    ]

    return chains


def setupMenu(menu_name):

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('setupMenu ...')

    chains = getPhysicsHISignatures()

    # We could use the menu name here now for other filtering
    P1_run3_v1.addCommonP1Signatures(chains)
    P1_run3_v1.addHeavyIonP1Signatures(chains)

    final_chains = ChainStore()
    for sig, chainsInSig in chains.items():
        if sig == "Beamspot":  # HI needs special beam spot setup
            log.warning('Default Beamspot signature removed in HI')
            continue
        for c in chainsInSig:
                if "IDCalibPEB" in c.name: # heavy tracking
                    raise RuntimeError(f"IDCalibPEB not safe in HI, requested by chain {c.name}")
                elif "EM3" in c.name: # EM3 without VTE and AFP is removed from HI L1 menu to avoid L1Calo EM overflow 
                    raise RuntimeError(f"EM3 not available in HI L1 menu, requested by chain {c.name}")
                elif "EM7" in c.name: # EM7 without VTE and AFP is removed from HI L1 menu to avoid L1Calo EM overflow 
                    raise RuntimeError(f"EM7 not available in HI L1 menu, requested by chain {c.name}")
                else:
                    final_chains[sig].append(c)
    return final_chains
