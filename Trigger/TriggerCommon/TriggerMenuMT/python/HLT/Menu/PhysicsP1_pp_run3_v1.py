# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# PhysicsP1_pp_run3_v1.py menu
#
# Combines physics triggers with P1 monitoring chains
#------------------------------------------------------------------------#


from . import Physics_pp_run3_v1 as physics_menu 
from . import P1_run3_v1

from .SignatureDicts import ChainStore
from ..Config.Utility.ChainDefInMenu import ChainProp

from .Physics_pp_run3_v1 import (
    SingleMuonGroup,
    SingleJetGroup,
    MinBiasGroup,
    JetStreamersGroup,
    JetPhaseIStreamersGroup,
    TauStreamersGroup,
    TauPhaseIStreamersGroup,
    EgammaStreamersGroup,
    EgammaPhaseIStreamersGroup,
    METStreamersGroup,
    METPhaseIStreamersGroup,
    MuonXStreamersGroup,
    ZeroBiasGroup,
    Topo2Group,
    Topo3Group,
    SupportGroup,
    SupportLegGroup,
    SupportPhIGroup,
)


from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def addPhysicsP1Chains(chains):
    chainsP1 = ChainStore()

    # Add chains here that stream to Main but are only in data
    chainsP1['Muon'] = [
        # ATR-25219, 1mu, for alignment run
        ChainProp(name='HLT_mu5_mucombTag_L1MU20VFC',groups=['PS:Online']+SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu8_mucombTag_L1MU20VFC',groups=['PS:Online']+SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu10_mucombTag_L1MU20VFC',groups=['PS:Online']+SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu15_mucombTag_L1MU20VFC',groups=['PS:Online']+SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu20_mucombTag_L1MU20VFC',groups=['PS:Online']+SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu25_mucombTag_L1MU20VFC',groups=['PS:Online']+SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu30_mucombTag_L1MU20VFC',groups=['PS:Online']+SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
        ChainProp(name='HLT_mu50_mucombTag_L1MU20VFC',groups=['PS:Online']+SingleMuonGroup+SupportGroup, monGroups=['muonMon:online']),
    ]

    chainsP1['Jet'] = [
        # Support performance chains (for emulation+calibration studies) ATR-20624
        ChainProp(name='HLT_j0_perf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j0_perf_pf_ftf_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleJetGroup+SupportGroup+['RATE:CPS_RD0_FILLED']),
        ChainProp(name='HLT_j0_perf_L1J12_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleJetGroup+SupportLegGroup),
    ]
        
    # Streamers with L1 items removed from MC menu
    chainsP1['Streaming'] = [
        ChainProp(name='HLT_noalg_L1MU3VC',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU3EOF',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU4BO',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8FC',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8FH',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8EOF',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU9VF',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU9VFC',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU12FCH',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU14FCHR',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU14EOF',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU15VFCH',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU15VFCHR', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU20VFC',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),

        ChainProp(name='HLT_noalg_L1J25',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J85',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+JetStreamersGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1eTAU20L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU35',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU40HM',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1eEM7',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM10L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM15',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM18',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM22M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM24VM',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jEM20',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jEM20M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jJ30p0ETA25',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ40p0ETA25',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jJ55',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ55p0ETA23',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ70p0ETA23',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ80',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ80p0ETA25',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ85p0ETA21',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ140',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ180',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jLJ180',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),

        # Exotics support streamers
        ChainProp(name='HLT_noalg_L1MU14FCH_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportGroup+MuonXStreamersGroup),
        ChainProp(name='HLT_noalg_L1MU14FCH_UNPAIRED_ISO',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportGroup+MuonXStreamersGroup),

        # AFP calibration
        ChainProp(name='HLT_noalg_L1AFP_FSA_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),
        # all mu
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T2_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSA_TOF_T3_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T2_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1AFP_FSC_TOF_T3_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),
    ] + [
        ####### STREAMERS FROM Physics_pp #######
        # We likely want to put these back after validation of the restructured menus
        ChainProp(name='HLT_noalg_L1RD0_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportGroup),

        #zero bias
        ChainProp(name='HLT_noalg_L1RD1_FILLED',        l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'],groups=['PS:Online']+ZeroBiasGroup+SupportGroup),# ATR-25032

        # muon streamers
        ChainProp(name='HLT_noalg_L1MU3V',      l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU3VF',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU5VF',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8F',      l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8VF',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU8VFC',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU4BOM',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU10BO',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU10BOM',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU12BOM',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU14FCH',   l1SeedThresholds=['FSNOSEED'], stream=['Main','express'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),
        ChainProp(name='HLT_noalg_L1MU18VFCH',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SingleMuonGroup+SupportGroup),

        # L1 calo streamers
        ChainProp(name='HLT_noalg_L1EM3',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+EgammaStreamersGroup+SupportLegGroup),
        #ChainProp(name='HLT_noalg_L1EM7',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+EgammaStreamersGroup+SupportLegGroup),
        #ChainProp(name='HLT_noalg_L1EM12',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+EgammaStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM15',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+EgammaStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM10VH',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+EgammaStreamersGroup+SupportLegGroup),
        #ChainProp(name='HLT_noalg_L1EM15VH',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+EgammaStreamersGroup+SupportLegGroup),
        #ChainProp(name='HLT_noalg_L1EM20VH',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+EgammaStreamersGroup+SupportLegGroup),
        #ChainProp(name='HLT_noalg_L1EM22VHI', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+EgammaStreamersGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1TAU8',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+TauStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU40',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+TauStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU60',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+TauStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU12IM', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+TauStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1TAU20IM', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+TauStreamersGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1J15',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J20',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J30',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J40',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J50',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+JetStreamersGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J75',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+JetStreamersGroup+SupportLegGroup),

        ChainProp(name='HLT_noalg_L1XE60',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+METStreamersGroup+SupportLegGroup),

        #Phase-I
        ChainProp(name='HLT_noalg_L1eTAU12',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU20',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTAU20',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTAU30',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTAU30M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1cTAU20M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU20M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU30',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1cTAU30M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1cTAU35M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU60',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU80',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eTAU140',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+TauPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1eEM5',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM9',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM12L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM18L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM18M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM24L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM26',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM26L',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM26M',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1eEM26T',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+EgammaPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jJ30',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ40',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ40p31ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ50',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ50p31ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ60',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ60p31ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ90',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ90p31ETA49',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ125',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jJ125p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jLJ80',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jLJ120',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1gJ20p0ETA25',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ20p25ETA49',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ20p0ETA25_EMPTY',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ50p0ETA25',          l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gJ100p0ETA25',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1gLJ80p0ETA25',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gLJ100p0ETA25',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gLJ140p0ETA25',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+JetPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jXE70',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jXE80',         l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jXE110',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jXE500',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXEJWOJ70',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXEJWOJ80',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXERHO70',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gXENC70',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gMHT500',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),

        ChainProp(name='HLT_noalg_L1jXEC100',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1gTE200',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTE200',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTEC200',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTEFWD100',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTEFWDA100',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),
        ChainProp(name='HLT_noalg_L1jTEFWDC100',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportPhIGroup+METPhaseIStreamersGroup),


        # Exotics support streamers
        ChainProp(name='HLT_noalg_L110DR-MU14FCH-MU5VF_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportGroup+MuonXStreamersGroup+Topo2Group),
        ChainProp(name='HLT_noalg_L110DR-MU14FCH-MU5VF_UNPAIRED_ISO',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportGroup+MuonXStreamersGroup+Topo2Group),

        ChainProp(name='HLT_noalg_L1CEP-CjJ100', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportPhIGroup+Topo3Group),
        ChainProp(name='HLT_noalg_L1CEP-CjJ90', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportPhIGroup+Topo3Group),
        # TODO add once L1 items/thresholds are in place
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_J50', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_T0T1_J75', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_J50', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1AFP_A_AND_C_TOF_J75', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+MinBiasGroup+SupportLegGroup),

        #Muon streamers for L1Topo validation
        ChainProp(name='HLT_noalg_L1BPH-2M9-0DR15-2MU3VF',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportGroup+MuonXStreamersGroup+Topo2Group),
        ChainProp(name='HLT_noalg_L1BPH-8M15-0DR22-2MU5VF',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportGroup+MuonXStreamersGroup+Topo2Group),
        ChainProp(name='HLT_noalg_L12MU5VF',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=['PS:Online']+SupportGroup+MuonXStreamersGroup),

    ]

    for sig,chainsInSig in chainsP1.items():
        for chain in chainsInSig:
            for group in chain.groups:
                if 'Primary' in group:
                    log.error("chain %s in PhysicsP1 menu [%s] with Primary tag. Please move this to Physics menu file", chain.name, sig)
                    raise RuntimeError("Move %s chain to Physics menu file",chain.name)

    for sig,chainsInSig in chainsP1.items():
        chains[sig] += chainsInSig

def setupMenu(menu_name):
    log.info('setupMenu ...')

    # Add physics chains (data + MC)
    chains = physics_menu.setupMenu(menu_name)

    addPhysicsP1Chains(chains)

    # Add calibration and monitoring chains (not in Main)
    P1_run3_v1.addCommonP1Signatures(chains)
    P1_run3_v1.addHighMuP1Signatures(chains)

    return chains

